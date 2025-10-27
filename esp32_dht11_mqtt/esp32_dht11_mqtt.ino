#include <WiFi.h>
#include <PubSubClient.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include "DHT.h"
#include "esp_task_wdt.h"

#define DHTPIN 14
#define DHTTYPE DHT11

#define MAX_RETRIES 20
#define MQTT_CALLBACK 1

// 看门狗配置
#define WDT_TIMEOUT 60  // 看门狗超时时间（秒）

const char* ssid = "FAST_20CC";
const char* pass = "409409409";
const char *device_id = "esp32_001";
const char *mqtt_broker = "192.168.1.111";  
const char *topic = "espiot";
const char *mqtt_username = "";
const char *mqtt_password = "";
const int mqtt_port = 1883;
const char *client_id = "mqtt-client-esp32-01";

// 时间间隔配置（毫秒）
const unsigned long HEARTBEAT_INTERVAL = 5000;    // 5秒发送一次心跳
const unsigned long SENSOR_INTERVAL = 10000;     // 10秒读取一次传感器
const unsigned long RECONNECT_INTERVAL = 15000;  // 15秒尝试重连MQTT

// 全局变量
String heartBeatMsg;
DHT dht(DHTPIN, DHTTYPE);
WiFiClient espClient;
PubSubClient client(espClient);

// 定时器变量
unsigned long lastHeartbeat = 0;
unsigned long lastSensorRead = 0;
unsigned long lastReconnectAttempt = 0;

// MQTT监听函数
void mqttCallback(char *topic, byte *payload, unsigned int length) {
  Serial.print("📨 MQTT消息: ");
  Serial.print(topic);
  Serial.print(" = ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

// 网络连接函数
bool connectWifi(){
  Serial.println("📶 开始连接WiFi...");
  WiFi.begin(ssid, pass);

  for(int i = 0; i < MAX_RETRIES; i++){
    delay(500);
    esp_task_wdt_reset(); // 喂狗
    
    if(WiFi.status() == WL_CONNECTED){
      Serial.println("✅ WiFi连接成功!");
      Serial.print("🌐 IP地址: ");
      Serial.println(WiFi.localIP());
      return true;
    }
    Serial.print(".");
  }
  
  Serial.println("\n❌ WiFi连接失败");
  return false;
}

// MQTT服务器连接函数
bool connectMqtt(){
  Serial.println("🔗 开始连接MQTT服务器...");
  client.setServer(mqtt_broker, mqtt_port);
  client.setCallback(mqttCallback);

  if(WiFi.status() == WL_CONNECTED){
    for(int i = 0; i < MAX_RETRIES; i++){
      esp_task_wdt_reset(); // 喂狗
      
      if(client.connect(client_id, mqtt_username, mqtt_password)){
        Serial.println("✅ MQTT服务器连接成功!");
        return true;
      }
      Serial.print("⏳ MQTT连接尝试 ");
      Serial.print(i+1);
      Serial.println(" 失败，1秒后重试...");
      delay(1000);
    }
    Serial.println("❌ MQTT服务器连接失败");
    return false;
  } else {
    Serial.println("❌ 网络未连接，无法连接MQTT");
    return false;
  }
}

// 创建DHT11的JSON数据
String createDhtMsg(float temperature, float humidity) {
  StaticJsonDocument<400> doc;
  doc["source"] = device_id;
  doc["target"] = "server";
  doc["msgType"] = "temp-humi";
  doc["temperature"] = temperature;
  doc["humidity"] = humidity;

  String jsonString;
  serializeJson(doc, jsonString);
  return jsonString;
}

// 创建心跳数据
String createHeartBeatMsg(String ip) {
  StaticJsonDocument<400> doc;
  doc["source"] = device_id;
  doc["target"] = "server";
  doc["msgType"] = "heartbeat";
  doc["ssid"] = ssid;
  doc["ip"] = ip;

  String jsonString;
  serializeJson(doc, jsonString);
  return jsonString;
}

String ipToString(IPAddress ip) {
  String result = "";
  for (int i = 0; i < 4; i++) {
    result += String(ip[i]);
    if (i < 3) {
      result += ".";
    }
  }
  return result;
}

void setup() {
  Serial.begin(9600);
  Serial.println("\n\n🚀 ESP32-S3 IoT设备启动中...");

  // 初始化硬件看门狗 - 新版本API
  esp_task_wdt_config_t wdt_config = {
    .timeout_ms = WDT_TIMEOUT * 1000,
    .idle_core_mask = 0,
    .trigger_panic = true
  };
  esp_task_wdt_init(&wdt_config);
  esp_task_wdt_add(NULL);
  Serial.println("🐕 硬件看门狗已启用 (" + String(WDT_TIMEOUT) + "秒)");

  // 初始化传感器
  dht.begin();
  Serial.println("✅ DHT11传感器初始化完成");
  esp_task_wdt_reset(); // 喂狗

  // 连接WiFi
  if(connectWifi()){
    // 连接MQTT
    if(connectMqtt()){
      heartBeatMsg = createHeartBeatMsg(ipToString(WiFi.localIP()));
      Serial.println("✅ 心跳消息准备完成");
    }
  }

  Serial.println("🎉 系统初始化完成!");
  esp_task_wdt_reset(); // 喂狗
}

void loop() {
  // 喂硬件看门狗
  esp_task_wdt_reset();

  // 处理MQTT消息
  client.loop();

  unsigned long currentTime = millis();

  // 自动重连MQTT
  if (!client.connected() && (currentTime - lastReconnectAttempt > RECONNECT_INTERVAL)) {
    Serial.println("🔄 尝试重新连接MQTT...");
    connectMqtt();
    lastReconnectAttempt = currentTime;
  }

  // 发送心跳消息
  if (client.connected() && (currentTime - lastHeartbeat > HEARTBEAT_INTERVAL)) {
    if (client.publish(topic, heartBeatMsg.c_str())) {
      Serial.println("💓 心跳消息发送成功");
    } else {
      Serial.println("❌ 心跳消息发送失败");
    }
    lastHeartbeat = currentTime;
  }

  // 读取传感器数据
  if (client.connected() && (currentTime - lastSensorRead > SENSOR_INTERVAL)) {
    float h = dht.readHumidity();
    float t = dht.readTemperature();

    if (isnan(h) || isnan(t)) {
      Serial.println("❌ 读取DHT11失败！");
    } else {
      Serial.printf("📊 传感器读数 - 湿度: %.1f%% 温度: %.1f°C\n", h, t);

      String dhtMsg = createDhtMsg(t, h);
      if (client.publish(topic, dhtMsg.c_str())) {
        Serial.println("✅ 传感器数据发送成功");
      } else {
        Serial.println("❌ 传感器数据发送失败");
      }
    }
    lastSensorRead = currentTime;
  }

  // 短暂延时，防止过于频繁的循环
  delay(10);
}