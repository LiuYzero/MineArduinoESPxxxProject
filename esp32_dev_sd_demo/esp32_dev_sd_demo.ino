#include "FS.h"
#include "SD.h"
#include "SPI.h"
// not usefull
// 引脚定义
#define CS_PIN 5
#define SCK_PIN 18
#define MISO_PIN 19
#define MOSI_PIN 23

void listDir(fs::FS &fs, const char * dirname, uint8_t levels) {
  Serial.printf("列出目录: %s\n", dirname);
  
  File root = fs.open(dirname);
  if (!root) {
    Serial.println("打开目录失败");
    return;
  }
  if (!root.isDirectory()) {
    Serial.println("不是目录");
    return;
  }
  
  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      Serial.print("  目录: ");
      Serial.println(file.name());
      if (levels) {
        listDir(fs, file.name(), levels - 1);
      }
    } else {
      Serial.print("  文件: ");
      Serial.print(file.name());
      Serial.print("  大小: ");
      Serial.println(file.size());
    }
    file = root.openNextFile();
  }
}

void setup() {
  Serial.begin(9600);
  delay(3000);  // 等待电源稳定
  
  Serial.println("=== ESP32 SD卡全面测试 ===");
  
  // 初始化SPI
  SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, CS_PIN);
  
  // 尝试初始化SD卡
  Serial.println("初始化SD卡...");
  if (!SD.begin(CS_PIN, SPI, 4000000)) {
    Serial.println("SD卡挂载失败");
    Serial.println("可能的原因:");
    Serial.println("1. SD卡未正确插入");
    Serial.println("2. 引脚连接错误");
    Serial.println("3. SD卡需要格式化");
    Serial.println("4. 电源不稳定");
    return;
  }
  
  Serial.println("✅ SD卡挂载成功");
  
  // 获取SD卡信息
  uint8_t cardType = SD.cardType();
  Serial.print("SD卡类型: ");
  switch (cardType) {
    case CARD_MMC: Serial.println("MMC"); break;
    case CARD_SD: Serial.println("SDSC"); break;
    case CARD_SDHC: Serial.println("SDHC"); break;
    default: Serial.println("未知"); break;
  }
  
  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  uint64_t cardUsed = SD.usedBytes() / (1024 * 1024);
  Serial.printf("SD卡总大小: %lluMB\n", cardSize);
  Serial.printf("已使用空间: %lluMB\n", cardUsed);
  
  // 列出根目录
  listDir(SD, "/", 0);
  
  Serial.println("\n🎉 SD卡测试完成！");
}

void loop() {}