# ESP32 藍芽 LED 控制程式

## ✅ ESP32 的優勢

ESP32 比 ESP8266 更強大：
- ✅ **內建藍芽 + WiFi**
- ✅ **更快的處理器**
- ✅ **更多 GPIO 腳位**
- ✅ **低功耗藍芽 (BLE)**
- ✅ **經典藍芽 (Classic Bluetooth)**

## 📱 ESP32 藍芽版本程式碼

```cpp
#include "BluetoothSerial.h"

BluetoothSerial SerialBT;
bool deviceConnected = false;

void setup() {
  Serial.begin(115200);
  SerialBT.begin("DYJ_BT"); // 藍芽設備名稱
  pinMode(LED_BUILTIN, OUTPUT);
  
  Serial.println("ESP32 藍芽已啟動，設備名稱: DYJ_BT");
}

void loop() {
  // 檢查藍芽連接狀態
  if (SerialBT.hasClient()) {
    if (!deviceConnected) {
      deviceConnected = true;
      digitalWrite(LED_BUILTIN, HIGH); // ESP32 LED 正向邏輯
      Serial.println("藍芽已連接 - LED 恆亮");
    }
  } else {
    if (deviceConnected) {
      deviceConnected = false;
      Serial.println("藍芽已斷線 - LED 閃爍");
    }
    // LED 閃爍邏輯
    static unsigned long lastBlink = 0;
    static bool ledState = false;
    
    if (millis() - lastBlink > (ledState ? 200 : 800)) {
      ledState = !ledState;
      digitalWrite(LED_BUILTIN, ledState);
      lastBlink = millis();
    }
  }
  
  delay(100);
}
```

## 🛒 購買建議

**ESP32 開發板推薦**：
- ESP32 DevKit V1 (約 150-250 台幣)
- ESP32-WROOM-32 (約 200-300 台幣)
- NodeMCU-32S (約 200-350 台幣)

## 📋 對比總結

| 功能 | ESP8266 + HC-05 | ESP8266 WiFi | ESP32 |
|------|----------------|--------------|-------|
| 藍芽連接 | ✅ 需外接模組 | ❌ | ✅ 內建 |
| WiFi 連接 | ✅ | ✅ | ✅ |
| 成本 | 中等 | 最低 | 中等 |
| 複雜度 | 高 | 低 | 低 |
| 推薦度 | ⭐⭐⭐ | ⭐⭐⭐⭐ | ⭐⭐⭐⭐⭐ |
