void setup() {
  // 初始化LED引脚（ESP32 DevKit V1的板载LED通常接GPIO 2）
  pinMode(2, OUTPUT);
}

void loop() {
  digitalWrite(2, HIGH); // 点亮LED
  delay(1000);           // 延时1秒
  digitalWrite(2, LOW);  // 熄灭LED
  delay(1000);           // 延时1秒
}