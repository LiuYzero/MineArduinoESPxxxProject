#include "DHT.h"   // 包含DHT库
#include <WiFi.h>
#include <PubSubClient.h>
#include <Arduino.h>
#include <ArduinoJson.h>

 
#define DHTPIN 14  // 定义DHT11数据引脚连接到ESP32的GPIO14
#define DHTTYPE DHT11   // 定义传感器类型为DHT11


#define MAX_RETRIES 20          // 网络最大连接次数
#define MQTT_CALLBACK 1         // 是否开启MQTT回调函数

const char* ssid="FAST_20CC";         // 网络信息
const char* pass="409409409";

// 设备标识
const char *device_id = "esp32_001";

// 设置MQTT borker信息
const char *mqtt_broker = "192.168.1.111";  
const char *topic = "espiot";
const char *mqtt_username = "";
const char *mqtt_password = "";
const int mqtt_port = 1883;
const char *client_id = "mqtt-client-esp32-01";

String heartBeatMsg;

DHT dht(DHTPIN, DHTTYPE);  // 创建DHT传感器对象
WiFiClient espClient;
PubSubClient client(espClient);

// MQTT监听函数
void mqttCallback(char *topic, byte *payload, unsigned int length) {
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
  Serial.print("Message:");
  String charPayload;
  for (int i = 0; i < length; i++) {
    charPayload += (char) payload[i];
  }
  Serial.println(charPayload);
  Serial.println("-----------------------");
}

// 网络连接函数
bool connectWifi(){
  Serial.println("调用WiFi连接函数");
  WiFi.begin(ssid, pass);

  for(int i=0; i<MAX_RETRIES; i++){
    delay(500);
    if(WiFi.status()==WL_CONNECTED){
      Serial.println("网络连接成功");
      return true;
    }
  }
  if(WiFi.status()!=WL_CONNECTED){
    Serial.println("网络连接超时");
    return false;
  }
}

// MQTT服务器连接函数
bool connectMqtt(){
  Serial.println("调用MQTT连接函数");
  client.setServer(mqtt_broker, mqtt_port);

  #if MQTT_CALLBACK
  Serial.println("调用MQTT回调函数");
  client.setCallback(mqttCallback);
  #endif

  if(WiFi.status()==WL_CONNECTED){
    for(int i=0; i<MAX_RETRIES; i++){
      delay(1000);
      if(client.connect(client_id, mqtt_username, mqtt_password)){
        Serial.println("MQTT服务器连接成功");
        return true;
      }
    }
    if(!client.connected()){
      Serial.println("MQTT服务器连接超时");
      return false;
    } 
  }else{
    Serial.println("网络连接失败");
    return false;
  }
}

//创建dht11的json数据
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


//创建心跳数据
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
  Serial.begin(9600);   // 初始化串口通信，波特率设置为9600
  dht.begin();          // 初始化DHT11传感器
    if(connectWifi()){
    connectMqtt();
  };

  heartBeatMsg = createHeartBeatMsg(ipToString(WiFi.localIP()));
  
  
}
 
void loop() {
  Serial.println(heartBeatMsg);
  client.publish(topic, heartBeatMsg.c_str());
  delay(1000); 


  // 读取湿度和温度值
  float h = dht.readHumidity();          // 读取湿度
  float t = dht.readTemperature();       // 读取温度
 
  // 检查读取是否成功
  if (isnan(h) || isnan(t)) {
    Serial.println("读取DHT11失败！");  // 如果读取失败，在串口监视器打印失败信息
    return;
  } 
  // 串口打印温湿度信息
  Serial.printf("湿度: %.1f%% 温度: %.1f°C\n", h, t);  // 格式化输出湿度和温度



  // 打印dht数据
  String dhtMsg = createDhtMsg(t,h);
  Serial.println(dhtMsg);
  client.publish(topic, dhtMsg.c_str());


  delay(2000);
}