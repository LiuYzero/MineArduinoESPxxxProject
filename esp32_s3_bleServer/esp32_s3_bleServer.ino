#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

// 定义BLE服务和特征UUID
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define SERVICE_NAME_UUID   "2A00"  // 标准设备名称特征UUID

// 创建BLE特征回调类
class MyCharacteristicCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    String value = pCharacteristic->getValue();
    
    if (value.length() > 0) {
      Serial.print("通过BLE接收到: ");
      Serial.println(value);
      
      // 显示HEX格式
      Serial.print("HEX: ");
      for (int i = 0; i < value.length(); i++) {
        if (value[i] < 16) Serial.print("0");
        Serial.print(value[i], HEX);
        Serial.print(" ");
      }
      Serial.println();
    }
  }
};

void setup() {
  Serial.begin(9600);
  Serial.println("ESP32-S3 BLE服务启动...");
  
  // 初始化BLE设备并设置设备名称
  BLEDevice::init("ESP32-S3-Controller");  // 这是广播的设备名称
  
  // 创建BLE服务器
  BLEServer *pServer = BLEDevice::createServer();
  
  // 创建自定义服务
  BLEService *pService = pServer->createService(SERVICE_UUID);
  
  // 1. 创建服务名称特征（使用标准设备名称UUID）
  BLECharacteristic *pServiceNameCharacteristic = pService->createCharacteristic(
    SERVICE_NAME_UUID,
    BLECharacteristic::PROPERTY_READ
  );
  
  // 设置服务名称
  String serviceName = "ESP32-S3控制服务";
  pServiceNameCharacteristic->setValue(serviceName.c_str());
  
  // 2. 创建数据通信特征
  BLECharacteristic *pDataCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_READ |
    BLECharacteristic::PROPERTY_WRITE
  );
  
  // 设置数据特征的初始值和回调
  String customServiceName = "Change BLE Name";
  pDataCharacteristic->setValue(customServiceName.c_str());
  pDataCharacteristic->setCallbacks(new MyCharacteristicCallbacks());
  
  // 启动服务
  pService->start();
  
  // 开始广播
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  
  // 设置广播参数
  pAdvertising->setMinPreferred(0x06);
  pAdvertising->setMinPreferred(0x12);
  
  BLEDevice::startAdvertising();
  
  Serial.println("BLE服务已启动！");
  Serial.println("设备名称: ESP32-S3-Controller");
  Serial.println("服务名称: ESP32-S3控制服务");
  Serial.println("等待手机连接...");
}

void loop() {
  delay(1000);
}