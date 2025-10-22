#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>
// 定义红外接收引脚
const uint16_t kRecvPin = 14; // GPIO14 (D5)
// 初始化红外接收对象
IRrecv irrecv(kRecvPin);
decode_results results;
void setup() {
 delay(2000);
 Serial.begin(9600);
 Serial.println("program in");
 irrecv.enableIRIn(); // 启动红外接收
 Serial.println("ir recv on");
}
void loop() {
 // 检查是否接收到红外信号
 if (irrecv.decode(&results)) {
   // 打印解码结果
   Serial.println(resultToHumanReadableBasic(&results));
   Serial.println(resultToSourceCode(&results)); // 打印原始数据
   irrecv.resume(); // 准备接收下一个信号
 }
}