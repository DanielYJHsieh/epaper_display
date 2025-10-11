# 電池供電與充放電管理 - 軟體架構設計

本文件說明 ESP8266 電子紙顯示系統在加入電池供電後的軟體架構、低功耗策略、電源管理邏輯與韌體設計。

---

## 🎯 設計目標

- **最大化續航**: 透過 Deep Sleep 降低平均功耗至 < 1mA
- **智能喚醒**: 支援定時喚醒、外部中斷喚醒
- **電量監控**: 即時偵測電池電壓與充電狀態
- **自動保護**: 低電量自動進入休眠，防止過放
- **無縫整合**: 與現有 `client_esp8266.ino` 架構相容

---

## 📊 軟體架構圖

```
┌─────────────────────────────────────────────────────────────┐
│                    主程式流程 (loop)                          │
│                                                               │
│  ┌──────────────┐   ┌──────────────┐   ┌──────────────┐   │
│  │ 電源管理器   │   │ 電池監控器   │   │ 顯示控制器   │   │
│  │PowerManager  │   │BatteryMonitor│   │DisplayCtrl   │   │
│  └──────┬───────┘   └──────┬───────┘   └──────┬───────┘   │
│         │                   │                   │            │
│         ├─> checkBattery() ─┤                   │            │
│         │   (讀 ADC)        │                   │            │
│         │                   │                   │            │
│         ├─> decideSleep() ──┤                   │            │
│         │   (判斷休眠)      │                   │            │
│         │                   │                   │            │
│         ├─> enterDeepSleep()│                   │            │
│         │   (GPIO16->RST)   │                   │            │
│         │                   │                   │            │
│         └─> checkCharging() ┘                   │            │
│             (GPIO14/12)                          │            │
│                                                  │            │
└──────────────────────────────────────────────────┼───────────┘
                                                   │
                              ┌────────────────────┘
                              │
                       ┌──────▼──────────┐
                       │  WiFi + WebSocket│
                       │  (原有通訊架構)  │
                       └─────────────────┘
```

---

## 🔋 電池監控模組（BatteryMonitor）

### 功能列表

1. **電壓讀取**: 透過 ADC (A0) 讀取電池電壓
2. **電量估算**: 根據電壓推算剩餘百分比
3. **狀態分類**: 正常 / 低電量 / 極低電量 / 充電中
4. **資料上報**: 透過 WebSocket 回傳電池資訊給 Server

### 電壓與電量對應表

| 電壓 (V) | 電量 (%) | 狀態 | 動作 |
|----------|---------|------|------|
| 4.2 | 100% | 充滿 | 正常運行 |
| 4.0 | ~90% | 良好 | 正常運行 |
| 3.7 | ~50% | 正常 | 正常運行 |
| 3.5 | ~30% | 正常 | 正常運行 |
| 3.3 | ~15% | 低電量 | 發送警告 + 降低更新頻率 |
| 3.0 | ~5% | 極低 | 強制 Deep Sleep（不喚醒）|
| < 2.75 | 0% | 保護 | 硬體保護板切斷 |

### 核心函式設計

```cpp
// battery_monitor.h

#ifndef BATTERY_MONITOR_H
#define BATTERY_MONITOR_H

#include <Arduino.h>

class BatteryMonitor {
public:
    // 初始化
    void begin();
    
    // 讀取電池電壓（校正後）
    float readVoltage();
    
    // 電量百分比（0-100）
    uint8_t getPercentage();
    
    // 電池狀態
    enum BatteryState {
        BATTERY_FULL,        // > 4.0V
        BATTERY_GOOD,        // 3.5-4.0V
        BATTERY_NORMAL,      // 3.3-3.5V
        BATTERY_LOW,         // 3.0-3.3V
        BATTERY_CRITICAL,    // < 3.0V
        BATTERY_CHARGING     // 正在充電
    };
    
    BatteryState getState();
    
    // 充電檢測
    bool isCharging();
    
    // JSON 格式輸出（用於 WebSocket 回傳）
    String toJSON();
    
private:
    const int ADC_PIN = A0;
    const int PIN_CHRG = 14;  // D5
    const int PIN_STDBY = 12; // D6
    
    // 電壓校正係數（根據實測調整）
    const float VOLTAGE_CALIBRATION = 4.2 / 1023.0;
    
    // 低通濾波（平滑 ADC 讀值）
    float filteredVoltage = 0.0;
    const float FILTER_ALPHA = 0.2; // 0-1，越小越平滑
};

#endif
```

### 實作範例

```cpp
// battery_monitor.cpp 關鍵部分

void BatteryMonitor::begin() {
    pinMode(PIN_CHRG, INPUT_PULLUP);
    pinMode(PIN_STDBY, INPUT_PULLUP);
    
    // 初始化濾波器
    filteredVoltage = readVoltage();
}

float BatteryMonitor::readVoltage() {
    int adcValue = analogRead(ADC_PIN);
    float rawVoltage = adcValue * VOLTAGE_CALIBRATION;
    
    // 低通濾波
    filteredVoltage = (FILTER_ALPHA * rawVoltage) + 
                      ((1.0 - FILTER_ALPHA) * filteredVoltage);
    
    return filteredVoltage;
}

uint8_t BatteryMonitor::getPercentage() {
    float voltage = readVoltage();
    
    // 線性映射（簡化版）
    if (voltage >= 4.2) return 100;
    if (voltage <= 3.0) return 0;
    
    // 3.0V (0%) ~ 4.2V (100%)
    return (uint8_t)((voltage - 3.0) / 1.2 * 100);
}

BatteryMonitor::BatteryState BatteryMonitor::getState() {
    if (isCharging()) {
        return BATTERY_CHARGING;
    }
    
    float voltage = readVoltage();
    
    if (voltage >= 4.0) return BATTERY_FULL;
    if (voltage >= 3.5) return BATTERY_GOOD;
    if (voltage >= 3.3) return BATTERY_NORMAL;
    if (voltage >= 3.0) return BATTERY_LOW;
    return BATTERY_CRITICAL;
}

bool BatteryMonitor::isCharging() {
    bool chrg = (digitalRead(PIN_CHRG) == LOW);
    bool stdby = (digitalRead(PIN_STDBY) == LOW);
    return chrg || stdby;
}

String BatteryMonitor::toJSON() {
    float voltage = readVoltage();
    uint8_t percent = getPercentage();
    const char* state = getStateString(getState());
    
    return String("{\"voltage\":") + voltage + 
           ",\"percentage\":" + percent + 
           ",\"state\":\"" + state + 
           "\",\"charging\":" + (isCharging() ? "true" : "false") + "}";
}
```

---

## ⚡ 電源管理模組（PowerManager）

### 功能列表

1. **休眠決策**: 根據電池狀態與運行時間決定是否進入 Deep Sleep
2. **定時喚醒**: 設定 RTC 定時器喚醒（如每 1 小時更新一次）
3. **緊急保護**: 極低電量強制休眠，防止過放
4. **充電優化**: 充電時降低喚醒頻率，減少熱量

### 休眠策略表

| 電池狀態 | 喚醒間隔 | 備註 |
|---------|---------|------|
| 充電中 | 30 分鐘 | 避免頻繁充放 |
| 正常 (>3.5V) | 1 小時 | 標準模式 |
| 低電量 (3.0-3.5V) | 3 小時 | 省電模式 |
| 極低 (<3.0V) | 不喚醒 | 直到充電或手動 RST |

### 核心函式設計

```cpp
// power_manager.h

#ifndef POWER_MANAGER_H
#define POWER_MANAGER_H

#include <Arduino.h>
#include "battery_monitor.h"

class PowerManager {
public:
    void begin(BatteryMonitor* batteryMonitor);
    
    // 決定是否需要休眠（true=需要）
    bool shouldSleep();
    
    // 進入 Deep Sleep（微秒）
    void enterDeepSleep(uint64_t sleepTimeUs);
    
    // 預設休眠時間（根據電池狀態自動調整）
    uint64_t getDefaultSleepTime();
    
    // 緊急低電量處理
    void handleLowBattery();
    
    // 更新休眠計時器
    void updateTimer();
    
private:
    BatteryMonitor* battery;
    
    // 上次活動時間
    unsigned long lastActiveTime = 0;
    
    // 活動超時閾值（秒）
    const uint32_t ACTIVE_TIMEOUT = 30; // 30 秒無活動即休眠
    
    // Deep Sleep GPIO 配置
    const int WAKE_PIN = 16; // D0 (GPIO16 連接 RST)
};

#endif
```

### 實作範例

```cpp
// power_manager.cpp 關鍵部分

void PowerManager::begin(BatteryMonitor* batteryMonitor) {
    battery = batteryMonitor;
    lastActiveTime = millis();
    
    // 檢查是否為喚醒
    if (ESP.getResetReason() == "Deep-Sleep Wake") {
        Serial.println("從 Deep Sleep 喚醒");
    }
}

bool PowerManager::shouldSleep() {
    // 1. 檢查電池狀態
    BatteryMonitor::BatteryState state = battery->getState();
    
    // 極低電量立即休眠
    if (state == BatteryMonitor::BATTERY_CRITICAL) {
        Serial.println("⚠️ 電量極低，強制休眠");
        return true;
    }
    
    // 2. 檢查活動超時
    unsigned long elapsed = (millis() - lastActiveTime) / 1000;
    if (elapsed > ACTIVE_TIMEOUT) {
        Serial.println("活動超時，準備休眠");
        return true;
    }
    
    return false;
}

uint64_t PowerManager::getDefaultSleepTime() {
    BatteryMonitor::BatteryState state = battery->getState();
    
    switch (state) {
        case BatteryMonitor::BATTERY_CHARGING:
            return 30 * 60 * 1000000ULL; // 30 分鐘（微秒）
            
        case BatteryMonitor::BATTERY_FULL:
        case BatteryMonitor::BATTERY_GOOD:
            return 60 * 60 * 1000000ULL; // 1 小時
            
        case BatteryMonitor::BATTERY_NORMAL:
            return 2 * 60 * 60 * 1000000ULL; // 2 小時
            
        case BatteryMonitor::BATTERY_LOW:
            return 3 * 60 * 60 * 1000000ULL; // 3 小時
            
        case BatteryMonitor::BATTERY_CRITICAL:
            return 0; // 不喚醒，需充電或手動重啟
            
        default:
            return 60 * 60 * 1000000ULL; // 預設 1 小時
    }
}

void PowerManager::enterDeepSleep(uint64_t sleepTimeUs) {
    Serial.println("進入 Deep Sleep...");
    Serial.print("喚醒時間: ");
    Serial.print(sleepTimeUs / 1000000);
    Serial.println(" 秒後");
    
    // 確保序列埠輸出完成
    Serial.flush();
    delay(100);
    
    // 進入 Deep Sleep（GPIO16 自動觸發 RST 喚醒）
    ESP.deepSleep(sleepTimeUs);
}

void PowerManager::handleLowBattery() {
    Serial.println("🔋 電池電量不足，進入節能模式");
    
    // 發送警告給 Server（如果連線中）
    // webSocket.sendTXT("{\"type\":\"warning\",\"msg\":\"Low battery\"}");
    
    // 進入長時間休眠
    enterDeepSleep(3 * 60 * 60 * 1000000ULL); // 3 小時
}

void PowerManager::updateTimer() {
    lastActiveTime = millis();
}
```

---

## 🔄 主程式整合流程

### setup() 初始化

```cpp
#include "battery_monitor.h"
#include "power_manager.h"

BatteryMonitor batteryMonitor;
PowerManager powerManager;

void setup() {
    Serial.begin(115200);
    
    // 初始化電池監控
    batteryMonitor.begin();
    
    // 初始化電源管理
    powerManager.begin(&batteryMonitor);
    
    // 檢查電池狀態
    float voltage = batteryMonitor.readVoltage();
    uint8_t percent = batteryMonitor.getPercentage();
    
    Serial.printf("電池電壓: %.2fV (%d%%)\n", voltage, percent);
    
    // 極低電量直接休眠
    if (batteryMonitor.getState() == BatteryMonitor::BATTERY_CRITICAL) {
        Serial.println("⚠️ 電量過低，無法啟動");
        powerManager.enterDeepSleep(0); // 不喚醒，等待充電
        return;
    }
    
    // 正常初始化（WiFi、WebSocket、Display 等）
    connectWiFi();
    initWebSocket();
    initDisplay();
}
```

### loop() 主迴圈

```cpp
void loop() {
    // 處理 WebSocket 事件
    webSocket.loop();
    
    // 定期檢查電池狀態（每 30 秒）
    static unsigned long lastCheck = 0;
    if (millis() - lastCheck > 30000) {
        float voltage = batteryMonitor.readVoltage();
        Serial.printf("電池: %.2fV\n", voltage);
        
        // 低電量處理
        if (batteryMonitor.getState() == BatteryMonitor::BATTERY_LOW) {
            powerManager.handleLowBattery();
        }
        
        lastCheck = millis();
    }
    
    // 檢查是否需要休眠
    if (powerManager.shouldSleep()) {
        uint64_t sleepTime = powerManager.getDefaultSleepTime();
        powerManager.enterDeepSleep(sleepTime);
    }
    
    delay(100);
}
```

### WebSocket 事件處理（加入電池資訊回傳）

```cpp
void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
    switch(type) {
        case WStype_CONNECTED:
            Serial.println("WebSocket 已連接");
            
            // 連接成功後回傳電池狀態
            String batteryInfo = batteryMonitor.toJSON();
            webSocket.sendTXT(batteryInfo);
            
            powerManager.updateTimer(); // 重置活動計時器
            break;
            
        case WStype_BIN:
            // 處理圖像資料（原有邏輯）
            handleImageData(payload, length);
            
            powerManager.updateTimer(); // 重置活動計時器
            break;
    }
}
```

---

## 🛠️ 配置檔設計（config.h 擴充）

```cpp
// config.h 新增電源管理配置

// ==================== 電源管理設定 ====================

// 啟用電池供電模式
#define BATTERY_POWERED true

// 電池電壓校正係數（根據實測調整）
#define VOLTAGE_CALIBRATION 4.2 / 1023.0

// 低電量警告閾值 (V)
#define BATTERY_LOW_VOLTAGE 3.3

// 極低電量關機閾值 (V)
#define BATTERY_CRITICAL_VOLTAGE 3.0

// Deep Sleep 預設喚醒時間（秒）
#define DEFAULT_SLEEP_TIME 3600  // 1 小時

// 活動超時時間（秒）- 超時自動休眠
#define ACTIVE_TIMEOUT 30

// 充電時喚醒間隔（秒）
#define CHARGING_SLEEP_TIME 1800  // 30 分鐘

// 低電量模式喚醒間隔（秒）
#define LOW_BATTERY_SLEEP_TIME 10800  // 3 小時

// GPIO 定義
#define PIN_ADC_BATTERY A0
#define PIN_CHARGE_STATUS 14  // D5 (TP4056 CHRG)
#define PIN_STANDBY_STATUS 12 // D6 (TP4056 STDBY)
#define PIN_DEEP_SLEEP_WAKE 16 // D0 (連接 RST)

// ==================== 原有設定 ====================
// WiFi、Server、Display 等配置...
```

---

## 📈 功耗優化技巧

### 1. WiFi 優化

```cpp
// 連接後立即設定省電模式
WiFi.setSleepMode(WIFI_LIGHT_SLEEP);

// 降低 TX Power（犧牲訊號強度換省電）
WiFi.setOutputPower(15); // 預設 20.5dBm，降至 15dBm 可省 ~30mA
```

### 2. CPU 頻率調整

```cpp
// 非關鍵任務時降頻
system_update_cpu_freq(80); // 80MHz（預設 160MHz）

// 恢復高頻（需要 WiFi 傳輸時）
system_update_cpu_freq(160);
```

### 3. 關閉不必要功能

```cpp
// 關閉 WiFi 的自動重連（改為手動）
WiFi.setAutoReconnect(false);

// 關閉 LED（如 WeMos D1 Mini 的板載 LED）
pinMode(LED_BUILTIN, OUTPUT);
digitalWrite(LED_BUILTIN, HIGH); // 反相邏輯，HIGH=關閉
```

### 4. E-Paper 顯示優化

```cpp
// 更新完畢後讓 E-Paper 進入休眠
display.hibernate(); // GxEPD2 函式
```

---

## 🧪 測試與驗證

### 測試項目

| 項目 | 測試方法 | 驗收標準 |
|------|---------|---------|
| 電壓讀取精度 | 萬用表對比 ADC 讀值 | 誤差 < ±0.1V |
| 充電檢測 | 插拔 USB，觀察 GPIO 狀態 | 正確識別充電/放電 |
| Deep Sleep 功耗 | 電流表串聯電池 | < 0.1mA |
| 定時喚醒準確度 | 設 1 小時，實測時間 | 誤差 < 1 分鐘 |
| 低電量保護 | 模擬 3.0V 電壓 | 自動進入休眠 |
| 續航測試 | 充滿後自然耗電 | > 7 天（每日更新 1 次）|

### 功耗量測方法

```cpp
// 序列埠輸出即時功耗估算
void printPowerStats() {
    float voltage = batteryMonitor.readVoltage();
    float current = 0.0; // 需外接電流表或 INA219 模組
    
    Serial.printf("電壓: %.2fV, 電流: %.2fmA, 功率: %.2fmW\n",
                  voltage, current, voltage * current);
}
```

---

## 🔧 故障排除

### 問題 1: Deep Sleep 後無法喚醒

**原因**: GPIO16 (D0) 未連接 RST

**解決**:
1. 用杜邦線連接 D0 與 RST
2. 檢查焊接品質（若為 PCB）

### 問題 2: ADC 讀值不穩定

**原因**: 電源噪聲、接地問題

**解決**:
1. 加入低通濾波（軟體）
2. 分壓電阻加 0.1µF 陶瓷電容到地
3. 多次採樣取平均

### 問題 3: 充電中無法正常工作

**原因**: TP4056 輸出電流不足

**解決**:
1. 確認 TP4056 的 Rprog 電阻（預設 1.2kΩ = 1A 輸出）
2. 若電流不足，更換為更小電阻（如 1kΩ）

### 問題 4: 續航時間短於預期

**檢查清單**:
- [ ] 確認 Deep Sleep 電流 < 0.1mA
- [ ] 檢查 WiFi 是否正確關閉
- [ ] 確認 E-Paper 已進入休眠
- [ ] 檢查程式是否有無限迴圈或延遲

---

## 📚 延伸閱讀

- [ESP8266 Deep Sleep 官方指南](https://www.espressif.com/sites/default/files/9b-esp8266-low_power_solutions_en_0.pdf)
- [鋰電池充放電曲線與電量估算](https://batteryuniversity.com/article/bu-501-basics-about-discharging)
- [ADC 噪聲濾波技術](https://www.analog.com/media/en/training-seminars/tutorials/MT-021.pdf)

---

**版本**: 1.0  
**撰寫日期**: 2025-10-12  
**維護**: battery_management 專案團隊
