#include <WiFi.h>
#include <PubSubClient.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include "DHT.h"

#define DHTPIN 14
#define DHTTYPE DHT11

#define MAX_RETRIES 20
#define MQTT_CALLBACK 1

// 看门狗配置
#define WDT_TIMEOUT 60  // 看门狗超时时间（秒）
unsigned long lastFeedTime = 0;
const unsigned long FEED_INTERVAL = 20000;  // 每20秒喂狗一次
bool wdtEnabled = false;
unsigned long wdtStartTime = 0;

const char* ssid = "FAST_20CC";
const char* pass = "409409409";
const char *device_id = "esp32_001";
const char *mqtt_broker = "192.168.1.111";  
const char *topic = "espiot";
const char *mqtt_username = "";
const char *mqtt_password = "";
const int mqtt_port = 1883;
const char *client_id = "mqtt-client-esp32-01";

String heartBeatMsg;
DHT dht(DHTPIN, DHTTYPE);
WiFiClient espClient;
PubSubClient client(espClient);

// 看门狗初始化函数
void initWatchdog() {
  wdtEnabled = true;
  wdtStartTime = millis();
  lastFeedTime = millis();
  Serial.println("==================================");
  Serial.println("🐕 看门狗初始化完成");
  Serial.println("⏰ 超时时间: " + String(WDT_TIMEOUT) + "秒");
  Serial.println("🔄 喂狗间隔: " + String(FEED_INTERVAL/1000) + "秒");
  Serial.println("==================================");
}

// 喂狗函数
void feedWatchdog() {
  if (wdtEnabled) {
    unsigned long currentTime = millis();
    lastFeedTime = currentTime;
    
    // 计算系统运行时间
    unsigned long uptime = (currentTime - wdtStartTime) / 1000;
    
    Serial.println("✅ 看门狗已喂食 | 运行时间: " + String(uptime) + "秒 | 下次喂食: " + String(FEED_INTERVAL/1000) + "秒后");
  }
}

// 检查看门狗超时
void checkWatchdog() {
  if (wdtEnabled) {
    unsigned long currentTime = millis();
    unsigned long timeSinceLastFeed = currentTime - lastFeedTime;
    
    if (timeSinceLastFeed > WDT_TIMEOUT * 1000) {
      Serial.println("❌❌❌ 看门狗超时警报 ❌❌❌");
      Serial.println("⏰ 距离上次喂狗: " + String(timeSinceLastFeed/1000) + "秒");
      Serial.println("🚨 系统可能卡死，即将重启...");
      delay(100);
      ESP.restart();
    } else if (timeSinceLastFeed > (WDT_TIMEOUT * 1000 * 0.8)) {
      // 接近超时时警告
      Serial.println("⚠️ 警告: 接近看门狗超时! 剩余时间: " + String((WDT_TIMEOUT * 1000 - timeSinceLastFeed)/1000) + "秒");
    }
  }
}

// MQTT监听函数
void mqttCallback(char *topic, byte *payload, unsigned int length) {
  Serial.println("📡 MQTT消息到达，喂狗...");
  feedWatchdog();
  
  Serial.print("📨 主题: ");
  Serial.println(topic);
  Serial.print("💬 内容: ");
  String charPayload;
  for (int i = 0; i < length; i++) {
    charPayload += (char) payload[i];
  }
  Serial.println(charPayload);
  Serial.println("-----------------------");
}

// 网络连接函数（添加看门狗支持）
bool connectWifi(){
  Serial.println("📶 开始连接WiFi...");
  WiFi.begin(ssid, pass);

  for(int i = 0; i < MAX_RETRIES; i++){
    delay(500);
    
    // 在等待连接时定期喂狗
    if (i % 4 == 0) { // 每2秒喂一次狗
      Serial.println("🔄 WiFi连接中... 喂狗");
      feedWatchdog();
    }
    
    if(WiFi.status() == WL_CONNECTED){
      Serial.println("✅ WiFi连接成功!");
      feedWatchdog();
      return true;
    }
  }
  
  if(WiFi.status() != WL_CONNECTED){
    Serial.println("❌ WiFi连接超时");
    return false;
  }
}

// MQTT服务器连接函数（添加看门狗支持）
bool connectMqtt(){
  Serial.println("🔗 开始连接MQTT服务器...");
  client.setServer(mqtt_broker, mqtt_port);

  #if MQTT_CALLBACK
  Serial.println("🔄 设置MQTT回调函数");
  client.setCallback(mqttCallback);
  #endif

  if(WiFi.status() == WL_CONNECTED){
    for(int i = 0; i < MAX_RETRIES; i++){
      delay(1000);
      
      // 在等待连接时定期喂狗
      Serial.println("🔄 MQTT连接尝试 " + String(i+1) + "/" + String(MAX_RETRIES) + "... 喂狗");
      feedWatchdog();
      
      if(client.connect(client_id, mqtt_username, mqtt_password)){
        Serial.println("✅ MQTT服务器连接成功!");
        feedWatchdog();
        return true;
      }
    }
    
    if(!client.connected()){
      Serial.println("❌ MQTT服务器连接超时");
      return false;
    } 
  } else {
    Serial.println("❌ 网络未连接，无法连接MQTT");
    return false;
  }
}

// 创建dht11的json数据
String createDhtMsg(float temperature, float humidity) {
  StaticJsonDocument<400> doc;
  doc["source"] = device_id;
  doc["target"] = "server";
  doc["msgType"] = "temp-humi";
  doc["temperature"] = temperature;
  doc["humidity"] = humidity;

  String jsonString;
  serializeJson(doc, jsonString);
  doc.clear();
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
  doc.clear();
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
  Serial.println("\n\n🚀 系统启动中...");
  
  // 初始化看门狗
  initWatchdog();
  
  dht.begin();
  Serial.println("✅ DHT11传感器初始化完成");
  feedWatchdog();

  // 连接网络
  if(connectWifi()){
    feedWatchdog();
    // 连接MQTT
    connectMqtt();
  }

  heartBeatMsg = createHeartBeatMsg(ipToString(WiFi.localIP()));
  Serial.println("✅ 心跳消息准备完成");
  
  Serial.println("🎉 系统初始化全部完成!");
  feedWatchdog();
}

void loop() {
  // 检查看门狗状态
  checkWatchdog();
  
  // 处理MQTT消息
  client.loop();
  
  // 定期喂狗检查
  unsigned long currentTime = millis();
  if (currentTime - lastFeedTime >= FEED_INTERVAL) {
    Serial.println("⏰ 定期喂狗时间到");
    feedWatchdog();
  }

  // 发送心跳
  Serial.println("💓 发送心跳消息...");
  if (client.publish(topic, heartBeatMsg.c_str())) {
    Serial.println("✅ 心跳消息发送成功");
    feedWatchdog();
  } else {
    Serial.println("❌ 心跳消息发送失败");
  }
  
  delay(2000);

  // 读取传感器数据
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println("❌ 读取DHT11失败！");
    
  } else {
    Serial.printf("📊 传感器读数 - 湿度: %.1f%% 温度: %.1f°C\n", h, t);

    String dhtMsg = createDhtMsg(t, h);
    Serial.println("📨 准备发送传感器数据...");
    if (client.publish(topic, dhtMsg.c_str())) {
      Serial.println("✅ 传感器数据发送成功");
      feedWatchdog();
    } else {
      Serial.println("❌ 传感器数据发送失败");
    }
  }

  delay(3000);
  
  Serial.println("🔄 主循环完成，准备下一轮...");
}