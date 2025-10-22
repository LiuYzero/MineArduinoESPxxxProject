#include <U8g2lib.h>
#include <Wire.h>

// 初始化 U8g2，明确指定 SDA/SCL 引脚（ESP32 的 Wire 对象需先配置）
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(
  U8G2_R0,                    // 屏幕旋转方向
  /* reset=*/ U8X8_PIN_NONE,             // 复位引脚（若未使用，填 U8X8_PIN_NONE）
  /* clock=*/ 18,             // SCL 引脚
  /* data=*/ 17               // SDA 引脚
);

void setup(void) {
  u8g2.begin();
}

void loop(void) {
  u8g2.clearBuffer();                   // clear the internal memory
  u8g2.setFont(u8g2_font_open_iconic_weather_6x_t);
  u8g2.drawGlyph(0,48,0x0045);
  u8g2.sendBuffer();                    // transfer internal memory to the display
  delay(1000);  
}