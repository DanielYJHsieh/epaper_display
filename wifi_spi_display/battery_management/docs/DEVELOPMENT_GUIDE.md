# 電池供電與充放電管理 - 開發流程與測試計畫

本文件提供完整的開發流程、階段性驗證步驟、測試方法與故障排除指南，幫助你順利完成電池供電系統整合。

---

## 🎯 開發總覽

整個開發流程分為 **4 個階段**，每階段有明確的交付成果與驗收標準：

| 階段 | 目標 | 預估時間 | 驗收標準 |
|------|------|---------|---------|
| Phase 1 | 硬體組裝與驗證 | 1-2 天 | 電池正常充放電，電壓正確 |
| Phase 2 | 基礎韌體開發 | 2-3 天 | ADC 讀取準確，Deep Sleep 可喚醒 |
| Phase 3 | 功能整合測試 | 2-3 天 | 與現有系統無縫整合，正常顯示 |
| Phase 4 | 長期測試與優化 | 5-7 天 | 續航達標，穩定運行 |

**總時程**: 約 10-15 天（依經驗與硬體到貨速度調整）

---

## 📦 Phase 1: 硬體組裝與驗證

### 1.1 材料準備

按照 `HARDWARE_ARCHITECTURE.md` 的 BOM 表採購：

- [ ] 18650 電池 × 1（推薦 2600mAh+）
- [ ] TP4056 充電模組（含保護板）× 1
- [ ] MT3608 升壓模組 × 1
- [ ] 電阻 100kΩ × 1
- [ ] 電阻 33kΩ × 1
- [ ] 18650 電池座 × 1
- [ ] 杜邦線若干
- [ ] 麵包板或洞洞板 × 1
- [ ] 萬用表（必備）
- [ ] USB 線與 5V 充電器

### 1.2 組裝步驟

#### 步驟 1: 電池連接 TP4056

```
⚠️ 注意極性！反接會損壞模組！

18650 正極（凸起端）→ TP4056 B+
18650 負極（平端）  → TP4056 B-
```

**驗證**:
- 插入 USB 充電器，TP4056 紅燈應亮起（充電中）
- 用萬用表測 B+ 與 B- 間電壓，應為 3.7V 左右

#### 步驟 2: 充電測試

```
插入 USB → 等待 2-4 小時 → 藍/綠燈亮起（充滿）
```

**驗證**:
- 充滿後電池電壓應為 **4.15-4.20V**
- 拔掉 USB，OUT+ 與 OUT- 間電壓 = 電池電壓（保護板正常）

#### 步驟 3: 升壓模組接線

```
TP4056 OUT+ → MT3608 IN+
TP4056 OUT- → MT3608 IN- (共地)
```

**調整輸出電壓**:
1. 用萬用表測量 MT3608 的 OUT+ 與 OUT-
2. 用小螺絲刀**順時針**旋轉電位器升壓，**逆時針**降壓
3. 調整至 **5.0V ±0.1V**

#### 步驟 4: ESP8266 供電測試

```
MT3608 OUT+ (5V) → WeMos D1 Mini 的 5V 腳位
MT3608 OUT- (GND) → WeMos D1 Mini 的 GND
```

**驗證**:
- ESP8266 應正常開機（LED 閃爍）
- 用萬用表測 ESP8266 的 3.3V 腳位，應為 3.3V（板載 LDO 正常）

#### 步驟 5: 分壓電路組裝

```
TP4056 OUT+ ─┬─ R1(100kΩ) ─┬─ R2(33kΩ) ─┬─ GND
             │               │             │
             │               └─> A0        │
             │                             │
             └─────────────────────────────┘
```

**驗證**（用萬用表測 A0 節點電壓）:
- 電池 4.2V → A0 應為 **~1.04V**
- 電池 3.7V → A0 應為 **~0.92V**
- 電池 3.0V → A0 應為 **~0.74V**

#### 步驟 6: 充電狀態檢測（可選）

```
TP4056 CHRG  → ESP8266 GPIO14 (D5)
TP4056 STDBY → ESP8266 GPIO12 (D6)
```

**驗證**:
- 寫簡單程式讀取 GPIO14/12 狀態
- 插入 USB 充電，應能正確識別「充電中」與「充滿」

### 1.3 階段驗收

| 檢查項目 | 期望結果 | 實測結果 |
|---------|---------|---------|
| 電池充電至 4.2V | ✅ 通過 / ❌ 失敗 | |
| 升壓輸出穩定 5V | ✅ 通過 / ❌ 失敗 | |
| ESP8266 正常開機 | ✅ 通過 / ❌ 失敗 | |
| ADC 分壓電壓正確 | ✅ 通過 / ❌ 失敗 | |
| 充電狀態檢測正常 | ✅ 通過 / ❌ 失敗 | |

---

## 💻 Phase 2: 基礎韌體開發

### 2.1 建立測試專案

在 `battery_management/firmware/` 下建立測試程式：

```
firmware/
├── test_battery_monitor/
│   ├── test_battery_monitor.ino
│   ├── battery_monitor.h
│   └── battery_monitor.cpp
└── test_power_manager/
    ├── test_power_manager.ino
    ├── power_manager.h
    └── power_manager.cpp
```

### 2.2 測試 1: ADC 電壓讀取

**目標**: 驗證 ADC 讀取精度與濾波效果

**測試程式** (`test_battery_monitor.ino`):

```cpp
#include "battery_monitor.h"

BatteryMonitor battery;

void setup() {
    Serial.begin(115200);
    battery.begin();
    
    Serial.println("=== 電池監控測試 ===");
}

void loop() {
    float voltage = battery.readVoltage();
    uint8_t percent = battery.getPercentage();
    BatteryMonitor::BatteryState state = battery.getState();
    bool charging = battery.isCharging();
    
    Serial.printf("電壓: %.3fV | 電量: %d%% | 狀態: %s | 充電: %s\n",
                  voltage, percent, 
                  battery.getStateString(state),
                  charging ? "是" : "否");
    
    delay(1000);
}
```

**驗收標準**:
- [ ] 電壓讀值與萬用表誤差 < 0.1V
- [ ] 濾波後讀值穩定（波動 < 0.05V）
- [ ] 充電狀態正確識別
- [ ] 百分比對應合理（4.2V=100%, 3.0V=0%）

### 2.3 測試 2: Deep Sleep 喚醒

**目標**: 驗證 Deep Sleep 進入與定時喚醒

**重要**: 必須將 **GPIO16 (D0)** 連接到 **RST**！

**測試程式** (`test_power_manager.ino`):

```cpp
void setup() {
    Serial.begin(115200);
    delay(1000); // 等待序列埠穩定
    
    Serial.println("=== Deep Sleep 測試 ===");
    Serial.println("喚醒原因: " + ESP.getResetReason());
    
    // 讀取喚醒次數（存在 RTC 記憶體）
    uint32_t count = 0;
    ESP.rtcUserMemoryRead(0, &count, sizeof(count));
    count++;
    ESP.rtcUserMemoryWrite(0, &count, sizeof(count));
    
    Serial.printf("喚醒次數: %d\n", count);
    Serial.println("10 秒後進入 Deep Sleep (30 秒)...");
    
    delay(10000);
    
    Serial.println("進入 Deep Sleep");
    Serial.flush();
    
    // Deep Sleep 30 秒
    ESP.deepSleep(30 * 1000000ULL); // 微秒
}

void loop() {
    // 不會執行到這裡
}
```

**驗收標準**:
- [ ] 每 30 秒自動喚醒一次
- [ ] 喚醒次數正確累加
- [ ] Deep Sleep 期間電流 < 0.1mA（需電流表）
- [ ] 序列埠輸出正常

### 2.4 測試 3: 低電量保護

**模擬方法**: 修改 `battery_monitor.cpp` 的電壓校正係數，讓 ADC 讀值低於 3.0V

```cpp
// 臨時修改（測試用）
float BatteryMonitor::readVoltage() {
    return 2.9; // 模擬低電量
}
```

**驗收標準**:
- [ ] 檢測到低電量時，程式立即進入 Deep Sleep
- [ ] 設定為「不喚醒」（sleepTime = 0 或超長時間）
- [ ] 序列埠輸出警告訊息

### 2.5 階段驗收

| 功能模組 | 測試結果 | 備註 |
|---------|---------|------|
| ADC 電壓讀取 | ✅ / ❌ | 誤差: ____V |
| 電量百分比計算 | ✅ / ❌ | |
| 充電狀態檢測 | ✅ / ❌ | |
| Deep Sleep 進入 | ✅ / ❌ | |
| 定時喚醒 | ✅ / ❌ | 誤差: ____秒 |
| 低電量保護 | ✅ / ❌ | |

---

## 🔗 Phase 3: 功能整合測試

### 3.1 整合到現有專案

複製測試通過的 `battery_monitor.h/cpp` 與 `power_manager.h/cpp` 到：

```
client_esp8266/
├── client_esp8266.ino
├── config.h
├── protocol.h
├── battery_monitor.h    ← 新增
├── battery_monitor.cpp  ← 新增
├── power_manager.h      ← 新增
└── power_manager.cpp    ← 新增
```

### 3.2 修改 config.h

按照 `SOFTWARE_ARCHITECTURE.md` 的範例新增配置區塊。

### 3.3 修改主程式

**setup() 加入電池初始化**:

```cpp
void setup() {
    Serial.begin(115200);
    
    // === 電池管理初始化 ===
    batteryMonitor.begin();
    powerManager.begin(&batteryMonitor);
    
    // 檢查電池狀態
    float voltage = batteryMonitor.readVoltage();
    Serial.printf("電池: %.2fV\n", voltage);
    
    if (batteryMonitor.getState() == BatteryMonitor::BATTERY_CRITICAL) {
        Serial.println("⚠️ 電量過低，無法啟動");
        powerManager.enterDeepSleep(0);
        return;
    }
    
    // === 原有初始化 ===
    connectWiFi();
    initWebSocket();
    initDisplay();
}
```

**loop() 加入電池檢查**:

```cpp
void loop() {
    webSocket.loop();
    
    // 定期檢查電池（每 30 秒）
    static unsigned long lastCheck = 0;
    if (millis() - lastCheck > 30000) {
        float voltage = batteryMonitor.readVoltage();
        Serial.printf("電池: %.2fV\n", voltage);
        
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

### 3.4 整合測試案例

#### 測試案例 1: 完整更新流程

1. ESP8266 從 Deep Sleep 喚醒
2. 連接 WiFi
3. 連接 WebSocket Server
4. 接收並顯示圖像
5. 檢查電池狀態
6. 進入 Deep Sleep（1 小時）

**驗收標準**:
- [ ] 整個流程順暢，無卡頓或重啟
- [ ] 電池電壓與狀態正確回傳給 Server
- [ ] Deep Sleep 喚醒週期準確

#### 測試案例 2: 充電時行為

1. 連接 USB 充電
2. ESP8266 偵測到充電狀態
3. 調整喚醒間隔為 30 分鐘（降低頻率）

**驗收標準**:
- [ ] 充電狀態正確識別
- [ ] 喚醒間隔自動調整
- [ ] 充電指示燈正常（紅→藍/綠）

#### 測試案例 3: 低電量場景

1. 模擬電池電壓降至 3.2V
2. 系統發出低電量警告
3. 自動進入省電模式（3 小時喚醒）

**驗收標準**:
- [ ] 警告訊息透過 WebSocket 傳送
- [ ] 自動延長休眠時間
- [ ] 極低電量（<3.0V）時不再喚醒

### 3.5 階段驗收

| 整合功能 | 測試結果 | 備註 |
|---------|---------|------|
| 電池監控與顯示系統無衝突 | ✅ / ❌ | |
| WiFi 連接正常 | ✅ / ❌ | |
| 圖像更新正常 | ✅ / ❌ | |
| Deep Sleep 不影響下次喚醒 | ✅ / ❌ | |
| Server 收到電池狀態 | ✅ / ❌ | |
| 低電量自動保護 | ✅ / ❌ | |

---

## ⏱️ Phase 4: 長期測試與優化

### 4.1 續航測試

**測試方法**:

1. 電池充滿（4.2V）
2. 啟動裝置，設定每小時更新一次
3. 記錄電池電壓變化
4. 直到電池降至 3.0V 或自動關機

**記錄表格**:

| 時間 (小時) | 電壓 (V) | 百分比 (%) | 喚醒次數 | 備註 |
|------------|---------|-----------|---------|------|
| 0 | 4.20 | 100% | 0 | 初始充滿 |
| 24 | 4.15 | ~95% | 24 | |
| 48 | 4.08 | ~85% | 48 | |
| 72 | 4.00 | ~75% | 72 | |
| ... | ... | ... | ... | |
| X | 3.00 | ~5% | Y | 自動關機 |

**目標續航**:
- **低頻更新** (每天 1 次): > 60 天
- **中頻更新** (每小時 1 次): > 5 天
- **高頻更新** (每 15 分鐘): > 2 天

### 4.2 功耗優化

若續航不達標，依序檢查：

#### 優化 1: WiFi 省電模式

```cpp
WiFi.setSleepMode(WIFI_LIGHT_SLEEP);
WiFi.setOutputPower(15); // 降低 TX Power
```

#### 優化 2: 降低 CPU 頻率

```cpp
system_update_cpu_freq(80); // 非關鍵時段 80MHz
```

#### 優化 3: 關閉不必要的 GPIO

```cpp
// 關閉板載 LED
pinMode(LED_BUILTIN, OUTPUT);
digitalWrite(LED_BUILTIN, HIGH);
```

#### 優化 4: 縮短活動時間

```cpp
// 原本 30 秒超時，改為 15 秒
#define ACTIVE_TIMEOUT 15
```

### 4.3 穩定性測試

**測試項目**:

1. **重複喚醒測試**: 連續喚醒 100 次，記錄失敗次數
2. **WiFi 連接測試**: 遠距離或弱訊號環境，測試重連機制
3. **溫度測試**: 充電時測量 TP4056 與 ESP8266 溫度（< 60°C）
4. **跌落測試**: 模擬意外掉落，檢查電池與接線

### 4.4 階段驗收

| 測試項目 | 目標值 | 實測值 | 結果 |
|---------|-------|-------|------|
| 續航時間（每小時更新）| > 5 天 | ____ 天 | ✅ / ❌ |
| Deep Sleep 電流 | < 0.1mA | ____ mA | ✅ / ❌ |
| 喚醒成功率 | > 99% | ____ % | ✅ / ❌ |
| 充電溫度 | < 60°C | ____ °C | ✅ / ❌ |
| 長期穩定性 | > 7 天無故障 | ____ 天 | ✅ / ❌ |

---

## 🐛 故障排除指南

### 問題 1: 電池充電緩慢或不充電

**可能原因**:
- USB 供電不足（< 500mA）
- TP4056 的 Rprog 電阻過大

**解決方法**:
1. 更換 5V/2A 充電器
2. 檢查 USB 線品質（數據線 vs 充電線）
3. 測量 TP4056 的 IN+ 與 IN- 電壓（應為 5V）
4. 若需加速充電，更換 Rprog 電阻為 1kΩ（1A）或 0.5kΩ（2A）

### 問題 2: ESP8266 重啟或不穩定

**可能原因**:
- 升壓模組輸出電壓不穩定
- 電池電量不足（< 3.5V）
- WiFi 瞬間電流過大

**解決方法**:
1. 檢查 MT3608 輸出電壓是否穩定 5V
2. 在 MT3608 輸出端加 100µF 電解電容（緩衝）
3. 充滿電池後再測試
4. 降低 WiFi TX Power: `WiFi.setOutputPower(15)`

### 問題 3: ADC 讀值飄移或不準

**可能原因**:
- 分壓電阻值誤差大
- 電源噪聲干擾
- ADC 未做多次採樣平均

**解決方法**:
1. 更換 1% 精度電阻（金屬膜電阻）
2. 在 ADC 輸入端加 0.1µF 陶瓷電容到 GND
3. 程式碼加入多次採樣平均：

```cpp
float readVoltageAverage() {
    float sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += analogRead(A0);
        delay(10);
    }
    return (sum / 10.0) * VOLTAGE_CALIBRATION;
}
```

### 問題 4: Deep Sleep 後無法喚醒

**可能原因**:
- GPIO16 (D0) 未連接 RST
- 焊接不良或接觸不良

**解決方法**:
1. 用萬用表測量 D0 與 RST 之間是否導通
2. 重新焊接或更換杜邦線
3. 測試程式：

```cpp
void setup() {
    Serial.begin(115200);
    Serial.println("測試 Deep Sleep 喚醒");
    delay(5000);
    ESP.deepSleep(10e6); // 10 秒
}
```

### 問題 5: 續航時間遠低於預期

**檢查清單**:
- [ ] Deep Sleep 電流是否 < 0.1mA（用電流表量測）
- [ ] WiFi 是否正確關閉（檢查 `WiFi.disconnect()` 與 `WiFi.mode(WIFI_OFF)`）
- [ ] E-Paper 是否進入休眠（`display.hibernate()`）
- [ ] 是否有 `delay()` 或 `while()` 導致 CPU 空轉
- [ ] 板載 LED 是否關閉

---

## 📊 量測工具與方法

### 電流量測

**工具**: 數位萬用表（支援 µA 檔位）或 USB 電流表

**方法 1: 直接串聯**（最準確）

```
電池 → [萬用表 µA 檔] → ESP8266
```

**方法 2: USB 電流表**（簡便）

```
充電器 → [USB 電流表] → TP4056
```

**典型讀值**:
- Active (WiFi): 100-170 mA
- Deep Sleep: 0.02-0.1 mA（理想值 < 0.02 mA）

### 電壓量測

**工具**: 數位萬用表

**測量點**:
- 電池電壓: B+ 與 B- 之間
- 升壓輸出: OUT+ 與 OUT- 之間
- ADC 輸入: A0 與 GND 之間

### 溫度量測

**工具**: 紅外線測溫槍或熱電偶

**測量點**:
- TP4056 IC 表面（充電時）
- MT3608 IC 表面
- ESP8266 模組（WiFi 傳輸時）

**安全範圍**: < 70°C

---

## 📝 測試報告範本

```markdown
## 電池供電系統測試報告

**測試日期**: 2025-XX-XX  
**測試人員**: ______  
**硬體版本**: v1.0  
**韌體版本**: v1.0  

### Phase 1: 硬體驗證
- [x] 電池充電至 4.2V
- [x] 升壓輸出穩定 5V
- [x] ESP8266 正常開機
- [x] ADC 分壓電壓正確（誤差 0.05V）
- [x] 充電狀態檢測正常

### Phase 2: 韌體測試
- [x] ADC 讀取精度: 誤差 0.08V
- [x] Deep Sleep 喚醒: 30 秒週期，誤差 < 1 秒
- [x] 低電量保護: 3.0V 自動關機

### Phase 3: 整合測試
- [x] 完整更新流程順暢
- [x] 充電時自動調整喚醒間隔
- [x] 低電量警告正常傳送

### Phase 4: 長期測試
- 續航時間: **7.2 天**（每小時更新一次）
- Deep Sleep 電流: **0.03 mA**
- 喚醒成功率: **100%** (100/100)
- 充電溫度: **45°C**

### 結論
✅ 全部測試通過，系統達到預期目標。

### 改進建議
1. 考慮加入電量顯示 LED（3 段式）
2. 增加太陽能充電接口
```

---

## 🎓 開發技巧與建議

### 建議 1: 分階段提交 Git

```bash
# Phase 1 完成後
git add battery_management/
git commit -m "feat(battery): Add hardware design and BOM"

# Phase 2 完成後
git commit -m "feat(battery): Implement battery monitor and power manager"

# Phase 3 完成後
git commit -m "feat(battery): Integrate battery management with main system"
```

### 建議 2: 使用序列埠日誌監控

```cpp
// 加入時間戳記
Serial.printf("[%lu] 電池: %.2fV\n", millis(), voltage);
```

### 建議 3: RTC 記憶體儲存狀態

```cpp
// 儲存喚醒次數、累計電量等
struct RTCData {
    uint32_t wakeCount;
    float totalEnergy;
};

RTCData rtcData;
ESP.rtcUserMemoryRead(0, (uint32_t*)&rtcData, sizeof(rtcData));
```

### 建議 4: 使用看門狗定時器

```cpp
ESP.wdtEnable(5000); // 5 秒看門狗
// 定期餵狗
ESP.wdtFeed();
```

---

## 📚 參考資源

- [Arduino ESP8266 Core 官方文件](https://arduino-esp8266.readthedocs.io/)
- [GxEPD2 顯示庫 Deep Sleep 範例](https://github.com/ZinggJM/GxEPD2/tree/master/examples)
- [ESP8266 低功耗應用筆記 (PDF)](https://www.espressif.com/sites/default/files/9b-esp8266-low_power_solutions_en_0.pdf)

---

**版本**: 1.0  
**撰寫日期**: 2025-10-12  
**維護**: battery_management 專案團隊
