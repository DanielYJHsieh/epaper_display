# 電池供電與充放電管理系統

本專案為 ESP8266 電子紙顯示系統提供完整的電池供電解決方案，包含硬體設計、韌體開發、測試流程與整合文件。

---

## 📋 專案目錄

```
battery_management/
├── README.md                    # 本檔案（專案總覽）
├── docs/                        # 📄 詳細文件
│   ├── HARDWARE_ARCHITECTURE.md # 硬體架構設計與 BOM（方案 A: 完整模組）
│   ├── SIMPLIFIED_USB_CHARGING.md # 簡化方案（方案 B: 利用板載 USB）⭐ 新增
│   ├── SOFTWARE_ARCHITECTURE.md # 軟體架構與低功耗策略
│   └── DEVELOPMENT_GUIDE.md     # 開發流程與測試計畫
├── hardware/                    # 🔧 硬體資源
│   └── WIRING_DIAGRAM.md        # 接線圖與檢查清單
└── firmware/                    # 💻 韌體程式碼
    ├── battery_monitor.h        # 電池監控模組（標頭檔）
    ├── battery_monitor.cpp      # 電池監控模組（實作）
    ├── power_manager.h          # 電源管理模組（標頭檔）
    ├── power_manager.cpp        # 電源管理模組（實作）
    ├── test_battery_monitor/    # 電池監控測試程式
    │   └── test_battery_monitor.ino
    └── test_power_manager/      # 電源管理測試程式
        └── test_power_manager.ino
```

---

## 🎯 專案目標

為 ESP8266 電子紙顯示系統加入：
- ✅ **電池供電**: 使用 18650 鋰電池（2600mAh），擺脫 USB 線束縛
- ✅ **充放電管理**: TP4056 充電模組，支援邊充邊用（Pass-through）
- ✅ **長續航**: Deep Sleep 模式，目標續航 **7+ 天**（每日更新 1-2 次）
- ✅ **智能管理**: 電量監控、充電檢測、低電量保護
- ✅ **無縫整合**: 與現有 `client_esp8266` 架構完美結合

---

## 🔀 硬體方案選擇

本專案提供 **兩種硬體架構**，根據你的開發板與需求選擇：

### 方案 A: 完整模組方案（標準）
- **適用**: 所有 ESP8266 開發板
- **組成**: TP4056 充電 + MT3608 升壓 + 分壓電路
- **優點**: 供電穩定（5V），電壓範圍廣，安全可靠
- **成本**: NT$130
- **文件**: [HARDWARE_ARCHITECTURE.md](docs/HARDWARE_ARCHITECTURE.md)

### 方案 B: 簡化 USB 充電方案（推薦新手）⭐
- **適用**: WeMos D1 Mini、NodeMCU 等標準開發板
- **組成**: TP4056 充電 + LDO 穩壓 + 分壓電路
- **優點**: 
  - 利用 ESP8266 板載 USB，無需升壓模組
  - 硬體更簡單，成本更低（NT$115）
  - 體積更小，接線更少
- **文件**: [SIMPLIFIED_USB_CHARGING.md](docs/SIMPLIFIED_USB_CHARGING.md) ⭐ **推薦先看**

**選擇建議**:
- 🆕 **新手/原型開發** → 方案 B（更簡單）
- 🔧 **進階應用/特殊板子** → 方案 A（更穩定）

---

## ⚡ 快速開始

### 階段 1: 閱讀文件（10 分鐘）

**選擇你的方案**:

**方案 B - 簡化 USB 充電**（推薦新手）:
1. [SIMPLIFIED_USB_CHARGING.md](docs/SIMPLIFIED_USB_CHARGING.md) - 利用板載 USB，更簡單
2. [SOFTWARE_ARCHITECTURE.md](docs/SOFTWARE_ARCHITECTURE.md) - 軟體架構（兩方案通用）
3. [DEVELOPMENT_GUIDE.md](docs/DEVELOPMENT_GUIDE.md) - 測試流程（兩方案通用）

**方案 A - 完整模組方案**（進階）:
1. [HARDWARE_ARCHITECTURE.md](docs/HARDWARE_ARCHITECTURE.md) - 完整硬體架構
2. [SOFTWARE_ARCHITECTURE.md](docs/SOFTWARE_ARCHITECTURE.md) - 軟體架構
3. [DEVELOPMENT_GUIDE.md](docs/DEVELOPMENT_GUIDE.md) - 開發流程

### 階段 2: 材料採購（1-2 天）

**方案 B（推薦）**:
- 18650 電池（帶保護板）
- TP4056 充電模組（微型版）
- MCP1700-3.3 LDO（穩壓用）
- 電阻（100kΩ + 33kΩ）
- **預算**: 約 NT$115

**方案 A**:
- 按照 [HARDWARE_ARCHITECTURE.md - BOM 表](docs/HARDWARE_ARCHITECTURE.md#完整硬體-bom-表)
- **預算**: 約 NT$130

### 階段 3: 硬體組裝（1-2 天）

依照 [DEVELOPMENT_GUIDE.md - Phase 1](docs/DEVELOPMENT_GUIDE.md#phase-1-硬體組裝與驗證) 步驟：
1. 電池連接 TP4056
2. USB 充電測試
3. 升壓模組調整至 5V
4. ESP8266 供電驗證
5. 分壓電路組裝
6. 充電狀態檢測（可選）

### 階段 4: 韌體測試（2-3 天）

1. **測試電池監控**:
   ```cpp
   // 上傳 firmware/test_battery_monitor/test_battery_monitor.ino
   // 驗證 ADC 讀取精度與充電檢測
   ```

2. **測試電源管理**:
   ```cpp
   // ⚠️ 記得連接 GPIO16 (D0) 到 RST！
   // 上傳 firmware/test_power_manager/test_power_manager.ino
   // 驗證 Deep Sleep 進入與喚醒
   ```

### 階段 5: 整合到現有專案（1 天）

參考 [SOFTWARE_ARCHITECTURE.md - 主程式整合流程](docs/SOFTWARE_ARCHITECTURE.md#主程式整合流程)：
1. 複製 `battery_monitor.h/cpp` 與 `power_manager.h/cpp` 到 `client_esp8266/`
2. 修改 `config.h` 加入電源管理配置
3. 在 `setup()` 初始化電池監控與電源管理
4. 在 `loop()` 加入電池檢查與休眠判斷

---

## 📊 系統架構總覽

### 硬體架構

```
USB 5V ──> [TP4056 充電] ──> [保護板] ──> [升壓 5V] ──> ESP8266
                │                          ↑
                └──> [18650 電池 3.7V] ────┘
                           │
                           └──> [分壓電路] ──> A0 (電量偵測)
```

### 軟體架構

```
主程式 (loop)
    │
    ├─> BatteryMonitor (電池監控)
    │   ├─> readVoltage()      // ADC 讀取
    │   ├─> getPercentage()    // 電量計算
    │   ├─> getState()         // 狀態判斷
    │   └─> isCharging()       // 充電檢測
    │
    └─> PowerManager (電源管理)
        ├─> shouldSleep()      // 休眠決策
        ├─> getDefaultSleepTime() // 時間調整
        └─> enterDeepSleep()   // 進入休眠
```

---

## 🔋 功耗與續航

### 各模式功耗

| 模式 | 功耗 | 說明 |
|------|------|------|
| WiFi 傳輸 | ~170mA | 接收圖像資料 |
| 活動模式 | ~60mA | CPU 運作，WiFi 待機 |
| Deep Sleep | ~0.02mA | 最低功耗 |

### 續航估算

**場景 1**: 每天更新 1 次（低頻）
```
每日耗電: ~0.66mAh
續航時間: 2600mAh ÷ 0.66mAh ≈ 60-90 天
```

**場景 2**: 每小時更新 1 次（中頻）
```
每日耗電: ~15.84mAh
續航時間: 2600mAh ÷ 15.84mAh ≈ 131 天
```

實際續航受電池容量、溫度、WiFi 訊號等因素影響。

---

## 🛠️ 核心功能

### 1. 電池監控（BatteryMonitor）

```cpp
BatteryMonitor battery;
battery.begin();

float voltage = battery.readVoltage();    // 讀取電壓（3.0-4.2V）
uint8_t percent = battery.getPercentage(); // 電量百分比 (0-100%)
bool charging = battery.isCharging();      // 充電狀態
String json = battery.toJSON();            // JSON 格式輸出
```

**功能特色**:
- ADC 低通濾波（減少噪聲）
- 充電狀態自動檢測（TP4056 CHRG/STDBY）
- JSON 格式輸出（可透過 WebSocket 回傳）

### 2. 電源管理（PowerManager）

```cpp
PowerManager power;
power.begin(&battery);

if (power.shouldSleep()) {
    uint64_t sleepTime = power.getDefaultSleepTime();
    power.enterDeepSleep(sleepTime);
}
```

**智能休眠策略**:
- **充電中**: 30 分鐘喚醒（降低頻率）
- **正常/良好**: 1 小時喚醒
- **低電量**: 3 小時喚醒（省電模式）
- **極低電量**: 不喚醒（防過放）

---

## 📈 開發時程

| 階段 | 目標 | 預估時間 |
|------|------|---------|
| Phase 1 | 硬體組裝與驗證 | 1-2 天 |
| Phase 2 | 基礎韌體開發 | 2-3 天 |
| Phase 3 | 功能整合測試 | 2-3 天 |
| Phase 4 | 長期測試與優化 | 5-7 天 |

**總時程**: 約 10-15 天

---

## ✅ 驗收標準

### 硬體驗收
- [ ] 電池充電至 4.2V
- [ ] 升壓輸出穩定 5V（誤差 < 0.1V）
- [ ] ESP8266 正常開機
- [ ] ADC 電壓讀取準確（誤差 < 0.1V）
- [ ] 充電狀態正確檢測

### 韌體驗收
- [ ] ADC 讀值穩定（波動 < 0.05V）
- [ ] Deep Sleep 定時喚醒準確（誤差 < 1 分鐘）
- [ ] 低電量自動保護（< 3.0V 停止喚醒）
- [ ] Deep Sleep 電流 < 0.1mA

### 整合驗收
- [ ] 與現有系統無衝突
- [ ] 圖像更新正常
- [ ] 電池狀態正確回傳 Server
- [ ] 續航達到設計目標（> 7 天，每日更新 1 次）

---

## 🐛 常見問題

### Q1: 電池充電緩慢？
**A**: 檢查 USB 充電器是否為 5V/2A，或更換 TP4056 的 Rprog 電阻（預設 1.2kΩ → 1kΩ 可提升至 1A）。

### Q2: Deep Sleep 後無法喚醒？
**A**: 確認 **GPIO16 (D0)** 已用杜邦線連接到 **RST** 腳位。

### Q3: ADC 讀值不穩定？
**A**: 
1. 加入低通濾波（程式碼已內建）
2. 在 A0 與 GND 間加 0.1µF 陶瓷電容
3. 多次採樣取平均

### Q4: 續航時間不如預期？
**A**: 依序檢查：
- Deep Sleep 電流是否 < 0.1mA（用萬用表量測）
- WiFi 是否正確關閉
- E-Paper 是否進入休眠（`display.hibernate()`）
- 是否有不必要的 `delay()` 或迴圈

更多問題請參考 [DEVELOPMENT_GUIDE.md - 故障排除](docs/DEVELOPMENT_GUIDE.md#故障排除指南)。

---

## 📚 延伸閱讀

### 官方文件
- [ESP8266 Low Power Solutions (PDF)](https://www.espressif.com/sites/default/files/9b-esp8266-low_power_solutions_en_0.pdf)
- [TP4056 Datasheet](https://www.electrodragon.com/w/images/6/6a/TP4056.pdf)
- [MT3608 Datasheet](https://www.olimex.com/Products/Breadboarding/BB-PWR-3608/resources/MT3608.pdf)

### 相關資源
- [Battery University - 鋰電池充放電曲線](https://batteryuniversity.com/article/bu-501-basics-about-discharging)
- [Arduino ESP8266 Core 文件](https://arduino-esp8266.readthedocs.io/)

---

## 🔄 升級路徑

### 短期（原型驗證）
- [x] 使用模組化方案（TP4056 + MT3608）
- [x] 麵包板或洞洞板接線
- [ ] 測試功耗與續航

### 中期（小批量）
- [ ] 整合 PCB 設計，減少體積
- [ ] 加入電量顯示 LED（3 段式）
- [ ] 太陽能充電擴充接口

### 長期（量產）
- [ ] 改用 MCP73831 + TPS61070（更小體積）
- [ ] 升級至 ESP32-C3（更低功耗 + BLE）
- [ ] 加入無線充電（Qi）

---

## 🤝 貢獻與反饋

歡迎透過以下方式參與：
- 提交 Issue 回報問題或建議
- 發送 Pull Request 改進程式碼或文件
- 分享你的測試結果與使用心得

---

## 📄 授權

本專案採用 MIT License，詳見 [LICENSE](../../../LICENSE) 檔案。

---

## 👤 作者

**battery_management 專案團隊**  
- 相關專案: [wifi_spi_display](../)

---

**版本**: 1.0  
**撰寫日期**: 2025-10-12  
**最後更新**: 2025-10-12

---

## 🚀 開始你的電池供電之旅！

1. 📖 閱讀 `docs/` 下的詳細文件
2. 🛒 按照 BOM 表採購材料
3. 🔧 按步驟組裝硬體
4. 💻 上傳測試程式驗證
5. 🔗 整合到現有專案
6. 📊 長期測試與優化

**有任何問題？** 請查看 [DEVELOPMENT_GUIDE.md](docs/DEVELOPMENT_GUIDE.md) 的故障排除章節！
