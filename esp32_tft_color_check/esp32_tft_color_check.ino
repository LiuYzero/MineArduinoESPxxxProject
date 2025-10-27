#include <TFT_eSPI.h>
TFT_eSPI tft = TFT_eSPI();
TFT_eSprite sprite1 = TFT_eSprite(&tft);

void setup() {
  delay(1000);
  Serial.begin(9600);
  
  // 初始化TFT
  tft.init();
  
  // 尝试不同的设置组合，例如：
  // tft.setSwapBytes(true);   // 尝试交换字节
  // 或者在 User_Setup.h 中定义 TFT_RGB_ORDER
  
  // 创建精灵（确保尺寸合理，避免内存不足）
  if (!sprite1.createSprite(100, 100)) {
    Serial.println("精灵创建失败！");
    while (1) delay(100);
  }

  // 测试填充红色
  sprite1.fillSprite(TFT_RED);
  sprite1.pushSprite(0, 0);
  Serial.println("如果红色显示为黄色，请检查颜色顺序设置。");
}

void loop() {}