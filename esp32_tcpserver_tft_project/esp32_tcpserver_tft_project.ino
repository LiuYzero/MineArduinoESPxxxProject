#include <WiFi.h>
#include <Arduino.h>
#include <TFT_eSPI.h> 
#include <SPI.h>

#define TFT_GREY 0x5AEB
#define MAX_RETRIES 20

const char* ssid = "FAST_20CC";
const char* pass = "409409409";
const int tcpPort=5678;

WiFiServer server(tcpPort);
TFT_eSPI tft = TFT_eSPI();

bool connectWifi(){
  Serial.println("📶 开始连接WiFi...");
  WiFi.begin(ssid, pass);

  for(int i = 0; i < MAX_RETRIES; i++){
    delay(500);
    
    if (i % 4 == 0) { 
      Serial.println("🔄 WiFi连接中... 喂狗");
    }
    
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

    Serial.print("tft init...");
    tft.init();
    tft.setRotation(0);
    tft.invertDisplay(0);
    tft.fillScreen(TFT_WHITE);
    Serial.print("tft inited");

    server.begin();
    delay(200);
    Serial.print("TCP Server started at ");
    Serial.print(tcpPort);
    Serial.println(" . Waiting for connections...");
}

void loop(){
  int x = random(0, 240);
  int y = random(0, 240);
  
  // 随机选择颜色
  uint16_t colors[] = {
    TFT_RED, TFT_GREEN, TFT_BLUE, TFT_YELLOW, 
    TFT_CYAN, TFT_MAGENTA, TFT_WHITE, TFT_ORANGE,
    TFT_PINK, TFT_BROWN, TFT_PURPLE, TFT_DARKGREEN,
    TFT_NAVY, TFT_MAROON, TFT_SILVER, TFT_GOLD
  };
  uint16_t color = colors[random(0, 16)];
  
  // 绘制单个像素
  Serial.print(x);
  Serial.print(",");
  Serial.print(y);
  Serial.print(",");
  Serial.println(color);
  tft.drawPixel(x, y, color);  
  delay(50);
}

// void loop() {
//   WiFiClient client = server.available();
  
//   if (client) {
//     tft.fillScreen(TFT_BLACK);
//     tft.setTextColor(TFT_WHITE, TFT_BLACK);
//     tft.setTextSize(2);
//     tft.setCursor(0, 0);
//     Serial.println();
//     Serial.println("🆕New client connected!");
    
//     while (client.connected()) {
//       if (client.available()) {
//         String data = client.readString();
//         tft.println(data);
//         Serial.print("Received: ");
//         Serial.println(data);

//       }
//       delay(10);
//     }
    
//     Serial.println("Client disconnected");
//     client.stop();

//     delay(50);

//   }
// }
