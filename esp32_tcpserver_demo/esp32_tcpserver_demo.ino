#include <WiFi.h>
#include <Arduino.h>

#define MAX_RETRIES 20

const char* ssid = "FAST_20CC";
const char* pass = "409409409";
const int tcpPort=5678;

WiFiServer server(tcpPort);

bool connectWifi(){
  Serial.println("📶 开始连接WiFi...");
  WiFi.begin(ssid, pass);

  for(int i = 0; i < MAX_RETRIES; i++){
    delay(500);   
    if(WiFi.status() == WL_CONNECTED){
      Serial.println("✅ WiFi连接成功!");
      return true;
    }
  }
  
  if(WiFi.status() != WL_CONNECTED){
    Serial.println("❌ WiFi连接超时");
    return false;
  }
}

void setup() {
    delay(2000);
    Serial.begin(9600);
    Serial.println("\n\n🚀 系统启动中...");

    if(connectWifi()){
      Serial.println("✅ wifi连接成功");
      Serial.print("🛜");
      Serial.println(WiFi.localIP());
    }

    server.begin();
    delay(200);
    Serial.print("TCP Server started at ");
    Serial.print(tcpPort);
    Serial.println(" . Waiting for connections...");
}

void loop() {
  WiFiClient client = server.available();
  
  if (client) {
    while (client.connected()) {
      if (client.available()) {
        byte buffer[3];
        int bytesRead = client.readBytes(buffer, 3);

        int x = buffer[0];
        int y = buffer[1];
        uint16_t color = buffer[2];
        Serial.print(x);
        Serial.print(",");
        Serial.print(y);
        Serial.print(",");
        Serial.println(color);
      }
      delay(10);
    }
    
    Serial.println("Client disconnected");
    client.stop();

    delay(50);

  }
}
