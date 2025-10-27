#include <WiFi.h>
#include <Arduino.h>
#include <TFT_eSPI.h> 
#include <SPI.h>

// 常量定义
#define MAX_RETRIES 20
#define TCP_PORT 5678
#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 240
#define SPRITE_HEIGHT 120
#define UPDATE_INTERVAL 100
#define PIXEL_BATCH_SIZE 240
#define BAUD_RATE 9600

const char* ssid = "FAST_20CC";
const char* pass = "409409409";

WiFiServer server(TCP_PORT);
TFT_eSPI tft = TFT_eSPI();
TFT_eSprite sprite1 = TFT_eSprite(&tft);
TFT_eSprite sprite2 = TFT_eSprite(&tft);

// 颜色修正函数
uint16_t fixColor(uint16_t color) {
    // 如果颜色显示不正确，尝试以下方案之一：
    
    // 方案1: 交换红蓝通道（如果红色显示为蓝色）
    // uint8_t r = (color >> 11) & 0x1F;
    // uint8_t g = (color >> 5) & 0x3F;
    // uint8_t b = color & 0x1F;
    // return (b << 11) | (g << 5) | r;
    
    // 方案2: 字节交换
    // return (color << 8) | (color >> 8);
    
    // 方案3: 直接返回原色
    return color;
}

bool connectWifi() {
    Serial.println("📶 开始连接WiFi...");
    WiFi.begin(ssid, pass);

    for(int i = 0; i < MAX_RETRIES; i++) {
        delay(500);
        
        if (i % 4 == 0) { 
            Serial.println("🔄 WiFi连接中...");
        }
        
        if(WiFi.status() == WL_CONNECTED) {
            Serial.println("✅ WiFi连接成功!");
            return true;
        }
    }
    
    Serial.println("❌ WiFi连接超时");
    return false;
}

bool initializeSprites() {
    bool success = true;
    
    if (!sprite1.createSprite(SCREEN_WIDTH, SPRITE_HEIGHT)) {
        Serial.println("❌ 精灵1创建失败");
        success = false;
    }
    
    if (!sprite2.createSprite(SCREEN_WIDTH, SPRITE_HEIGHT)) {
        Serial.println("❌ 精灵2创建失败");
        success = false;
    }
    
    if (success) {
        Serial.println("✅ 所有小精灵创建成功");
        
        // 初始化精灵
        sprite1.fillSprite(TFT_BLACK);
        sprite2.fillSprite(TFT_BLACK);
        
        // 测试绘制
        sprite1.fillSprite(TFT_RED);
        sprite1.setTextColor(TFT_WHITE);
        sprite1.drawString("Sprite1", 10, 10);
        
        sprite2.fillSprite(TFT_GREEN);
        sprite2.setTextColor(TFT_BLACK);
        sprite2.drawString("Sprite2", 10, 10);
        
        // 推送到屏幕
        sprite1.pushSprite(0, 0);
        sprite2.pushSprite(0, SPRITE_HEIGHT);
    }
    
    return success;
}

void setup() {
    delay(2000);
    Serial.begin(BAUD_RATE);

    Serial.println("\n\n🚀 系统启动中...");

    // TFT初始化
    Serial.print("🖥️ TFT初始化...");
    tft.init();
    tft.setRotation(0); // 根据屏幕方向调整
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK); // 前景色，背景色
    tft.setTextSize(2);



    // WiFi连接
    if(connectWifi()) {
        Serial.print("✅ WiFi连接成功 - IP: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("❌ WiFi连接失败，系统无法继续");
        tft.drawString("WiFi连接失败，系统无法继续",30,10);
        return;
    }


    // 启动TCP服务器
    server.begin();
    Serial.print("🔌 TCP服务器已启动，端口: ");
    Serial.print(TCP_PORT);
    Serial.println("，等待连接...");

    // 展示信息
    String ipString = WiFi.localIP().toString();
    tft.drawString("socket:", 10, 10);
    tft.drawString(ipString, 10, 30);
    tft.drawString("5678", 10, 50);
    delay(10000);
    Serial.println("完成");

    // 精灵初始化
    if (!initializeSprites()) {
        Serial.println("❌ 精灵初始化失败，系统无法继续");
        return;
    }
}

void handleClient(WiFiClient &client) {
    Serial.println("🆕 新客户端连接!");
    
    bool needsUpdate = false;
    unsigned long lastUpdate = millis();
    byte pixelBuffer[4 * PIXEL_BATCH_SIZE];
    int pixelsInBuffer = 0;
    
    while (client.connected()) {
        // 批量读取像素数据
        while (client.available() >= 4 && pixelsInBuffer < PIXEL_BATCH_SIZE) {
            int bytesRead = client.readBytes(&pixelBuffer[pixelsInBuffer * 4], 4);
            if (bytesRead == 4) {
                pixelsInBuffer++;
            }
        }
        
        // 处理批量像素
        if (pixelsInBuffer > 0) {
            for (int i = 0; i < pixelsInBuffer; i++) {
                int x = (int)pixelBuffer[i * 4];
                int y = (int)pixelBuffer[i * 4 + 1];
                uint16_t color = fixColor((pixelBuffer[i * 4 + 2] << 8) | pixelBuffer[i * 4 + 3]);
                
                // 验证坐标并绘制像素
                if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT) {
                    if (y < SPRITE_HEIGHT) {
                        sprite1.drawPixel(x, y, color);
                    } else {
                        sprite2.drawPixel(x, y - SPRITE_HEIGHT, color);
                    }
                }
                
                // 调试输出（可注释掉以提高性能）
                #ifdef DEBUG
                Serial.printf("像素: %d,%d, 颜色: 0x%04X\n", x, y, color);
                #endif
            }
            
            pixelsInBuffer = 0;
            needsUpdate = true;
        }
        
        // 定期更新屏幕
        if (needsUpdate && (millis() - lastUpdate >= UPDATE_INTERVAL)) {
            sprite1.pushSprite(0, 0);
            sprite2.pushSprite(0, SPRITE_HEIGHT);
            needsUpdate = false;
            lastUpdate = millis();
            
            #ifdef DEBUG
            Serial.println("🔄 精灵已推送到屏幕");
            #endif
        }
        
        // 检查客户端是否断开
        if (!client.connected()) {
            break;
        }
        
        delay(1);
    }
    
    // 确保所有更改都已推送到屏幕
    if (needsUpdate) {
        sprite1.pushSprite(0, 0);
        sprite2.pushSprite(0, SPRITE_HEIGHT);
        Serial.println("🔄 最终更新屏幕");
    }
    
    Serial.println("🚫 客户端断开连接");
    client.stop();
}

void loop() {
    WiFiClient client = server.available();
    
    if (client) {
        handleClient(client);
    }
    
    delay(10);
}