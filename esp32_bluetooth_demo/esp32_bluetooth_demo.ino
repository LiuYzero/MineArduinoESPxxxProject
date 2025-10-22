#include "BluetoothSerial.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

BluetoothSerial SerialBT; // 创建蓝牙串口对象

void setup() {
  Serial.begin(115200); // 初始化硬件串口，用于调试输出
  SerialBT.begin("ESP32_Bluetooth"); // 初始化蓝牙，设备名称可自定义
  Serial.println("ESP32蓝牙已启动，等待连接...");
  Serial.println("设备名称: ESP32_Bluetooth");
}

void loop() {
  // 检查蓝牙串口是否有数据传入
  if (SerialBT.available()) {
    String incomingData = SerialBT.readString(); // 读取手机发送的ASCII指令
    Serial.print("通过蓝牙接收到: "); 
    Serial.println(incomingData); // 将指令通过硬件串口打印出来
    
    // 可选：通过蓝牙回传给手机，确认收到
    // SerialBT.print("Echo: ");
    // SerialBT.println(incomingData);
  }

  // 以下部分实现了双向透传，如果需要ESP32也能接收串口指令控制手机，可以保留
  // 如果只需要手机->ESP32->串口单向通信，可注释掉这段
  if (Serial.available()) {
    SerialBT.write(Serial.read());
  }
  
  delay(20); // 短暂的延迟
}