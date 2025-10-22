#include <TFT_eSPI.h>
TFT_eSPI tft = TFT_eSPI();

void setup() {
  tft.init();
  tft.setRotation(3); // 屏幕方向（0-3）
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("Hello ESP32!", 20, 50, 4);
}

void loop() {
  // 绘制矩形
  tft.fillRect(50, 100, 80, 60, TFT_RED);
  delay(1000);
  tft.fillRect(50, 100, 80, 60, TFT_BLUE);
  delay(1000);
}