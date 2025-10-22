#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>

// not work
// 红外接收引脚
#define kRecvPin  10

// 创建红外接收对象
IRrecv irrecv(kRecvPin);
decode_results results;

void setup() {
  Serial.begin(115200);   // 初始化串口
  irrecv.enableIRIn();    // 启动红外接收
}

void loop() {
  if (irrecv.decode(&results)) {
    // 打印红外码（十六进制）
    serialPrintUint64(results.value, HEX);
    Serial.println("");
    irrecv.resume();  // 接收下一个值
  }
  delay(100);
}
