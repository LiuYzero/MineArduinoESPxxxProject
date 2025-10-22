// EC11编码器引脚定义
#define EC11_CLK_PIN  5  // A相
#define EC11_DT_PIN   6  // B相  
#define EC11_SW_PIN   4  // 按键

// 编码器状态变量
volatile int encoderCount = 0;       // 编码器计数值
volatile bool encoderUpdate = false; // 编码器更新标志
int lastEncoderCount = 0;            // 用于主循环检查变化
int preEncoderCount = 0;

// 按键状态变量
volatile bool buttonPressed = false; // 按键按下标志
volatile unsigned long buttonPressTime = 0; // 用于记录按键按下时间

// 状态机相关变量
volatile int lastClkState = HIGH;
volatile int currentClkState = HIGH;
volatile int currentDtState = HIGH;

// CLK引脚中断服务函数
void IRAM_ATTR ec11ClkISR() {
  currentClkState = digitalRead(EC11_CLK_PIN);
  currentDtState = digitalRead(EC11_DT_PIN);
  
  // 检查CLK引脚状态是否发生变化
  if (currentClkState != lastClkState) {
    // 在CLK的变化沿（上升沿或下降沿）进行判断
    if (currentClkState == HIGH) {
      // CLK上升沿：根据DT状态判断方向
      if (currentDtState == LOW) {
        encoderCount++; // 正转
      } else {
        encoderCount--; // 反转
      }
    } else {
      // CLK下降沿：根据DT状态判断方向
      if (currentDtState == HIGH) {
        encoderCount++; // 正转
      } else {
        encoderCount--; // 反转
      }
    }
    encoderUpdate = true;
    lastClkState = currentClkState;
  }
}

// 按键引脚中断服务函数
void IRAM_ATTR ec11SwISR() {
  if (digitalRead(EC11_SW_PIN) == LOW) {
    buttonPressed = true;
    buttonPressTime = millis();
  } else {
    // 如果需要，可以在此处理按键释放逻辑
    // 例如，计算按下时长以区分单击和长按
    unsigned long pressDuration = millis() - buttonPressTime;
    if (pressDuration > 1000) {
      // 长按处理示例：复位计数器
      encoderCount = 0;
      encoderUpdate = true;
    }
  }
}

void setup() {
  Serial.begin(9600);
  
  // 初始化编码器引脚为上拉输入
  pinMode(EC11_CLK_PIN, INPUT_PULLUP);
  pinMode(EC11_DT_PIN, INPUT_PULLUP);
  pinMode(EC11_SW_PIN, INPUT_PULLUP);
  
  // 初始读取引脚状态
  lastClkState = digitalRead(EC11_CLK_PIN);
  
  // 附加中断服务函数，监视CLK引脚和按键的变化
  attachInterrupt(digitalPinToInterrupt(EC11_CLK_PIN), ec11ClkISR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(EC11_SW_PIN), ec11SwISR, CHANGE);
  
  Serial.println("EC11编码器初始化完成");
}

void loop() {
  // 检查编码器是否更新
  if (encoderUpdate) {
    encoderUpdate = false;
    
    Serial.print("编码器计数值: ");
    Serial.println(encoderCount);
    Serial.print("编码器变化值: ");
    Serial.println(encoderCount-preEncoderCount);
    preEncoderCount = encoderCount;
  }
  
  // 检查按键是否按下
  if (buttonPressed) {
    buttonPressed = false;
    Serial.println("按键被按下");
    
    // 示例：按键复位编码器计数值
    // encoderCount = 0;
    // encoderUpdate = true;
  }
  
  // 其他主循环任务
  delay(1);
}