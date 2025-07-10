# GDEQ0426T82 電子紙顯示器 + ESP8266 專案

## 🚀 專案簡介

這是一個成功將 **GDEQ0426T82 4.26吋電子紙顯示器** 移植到 **WeMos D1 Mini (ESP8266)** 的完整專案。

**主要成果：**
- ✅ 成功驅動 GDEQ0426T82 電子紙顯示器
- ✅ 使用 GxEPD2 函式庫實現穩定顯示
- ✅ 解決 ESP8266 記憶體限制和引腳相容性問題
- ✅ 提供完整的測試程式和說明文件

## 📦 專案結構

```
epaper_display/
├── GDEQ0426T82_WeMos_D1_Mini/          # 🎯 主程式資料夾
│   ├── GDEQ0426T82_WeMos_D1_Mini.ino   # 主程式 (完整功能測試)
│   ├── GxEPD2_display_selection_GDEQ0426T82.h  # 顯示器驅動選擇
│   └── README.md                       # 詳細技術說明
├── GDEQ0426T82_Force_GxEPD2_Test/      # 🧪 強制測試程式
│   ├── GDEQ0426T82_Force_GxEPD2_Test.ino  # 強制 GxEPD2 測試
│   └── GxEPD2_display_selection_GDEQ0426T82.h  # 顯示器配置
├── wifi_led_control/                   # 💡 WiFi LED 控制範例
│   └── wifi_led_control.ino            # WiFi LED 控制程式
└── README.md                           # 專案總說明
```

## 🛠️ 硬體需求

### 基本硬體
- **WeMos D1 Mini** ESP8266 開發板
- **GDEQ0426T82** 4.26吋 480×800 電子紙顯示器
- USB 傳輸線 (Micro USB)
- 杜邦線或連接線

### 必要電阻 ⚠️
- **3.3kΩ 電阻** - CS 腳位下拉電阻 (GPIO15 到 GND)，**必需**
- **1kΩ 電阻** - RST 腳位上拉電阻 (GPIO5 到 3.3V)，建議

> ⚠️ **重要**：沒有 CS 腳位下拉電阻，ESP8266 將無法開機！

## ⚡ 快速開始

### 1. 硬體連接

**WeMos D1 Mini → GDEQ0426T82 接線圖**

```
ESP8266腳位    電子紙腳位    說明
───────────    ─────────    ──────────────────
D2 (GPIO4)  →  BUSY         忙碌狀態偵測
D1 (GPIO5)  →  RST          重置 (+ 1kΩ到3.3V)
D3 (GPIO0)  →  DC           資料/命令選擇
D8 (GPIO15) →  CS           片選 (+ 3.3kΩ到GND) ⚠️
D5 (GPIO14) →  CLK          SPI 時脈 (SCK)
D7 (GPIO13) →  DIN          SPI 資料 (MOSI)
GND         →  GND          接地
3.3V        →  3.3V         電源供應
```

### 2. 軟體環境設定

#### 安裝 Arduino IDE
下載並安裝 [Arduino IDE](https://www.arduino.cc/en/software) (版本 2.0+)

#### 安裝 ESP8266 開發板支援
1. **檔案** → **偏好設定** → **額外的開發板管理員網址** 加入：
   ```
   http://arduino.esp8266.com/stable/package_esp8266com_index.json
   ```
2. **工具** → **開發板** → **開發板管理員** → 搜尋並安裝 "ESP8266"
3. **工具** → **開發板** → 選擇 **LOLIN(WEMOS) D1 R2 & mini**

#### 安裝必要函式庫
**工具** → **管理程式庫** → 搜尋並安裝：
- **GxEPD2** by Jean-Marc Zingg
- **Adafruit GFX Library**

### 3. 上傳和測試

#### 選擇程式版本
1. **主程式**：`GDEQ0426T82_WeMos_D1_Mini.ino` - 完整功能測試
2. **強制測試**：`GDEQ0426T82_Force_GxEPD2_Test.ino` - 驗證 GxEPD2 安裝

#### 上傳步驟
1. USB 線連接 WeMos D1 Mini 到電腦
2. **工具** → **連接埠** → 選擇對應 COM 埠
3. 設定上傳參數：
   - **CPU Frequency**: 80 MHz
   - **Flash Size**: 4MB
   - **Upload Speed**: 115200
4. 點擊上傳按鈕 (→)
5. **工具** → **序列埠監控視窗** (115200 baud) 觀察輸出

### 4. 測試結果

**正常運作時會顯示：**
- ✅ 白屏清除測試
- ✅ 文字顯示測試 ("GDEQ0426T82 Test", "WeMos D1 Mini", "GxEPD2 Working!")
- ✅ 圖形繪製測試 (矩形、圓形、線條)
- ✅ 部分更新測試 (時間戳記)
## 🔧 技術細節

### 程式架構

#### 主程式 (GDEQ0426T82_WeMos_D1_Mini.ino)
- **完整功能測試程式**，包含所有測試項目
- 系統資訊顯示（ESP8266 記憶體、頻率等）
- 循環測試：白屏 → 文字 → 圖形 → 部分更新
- 詳細的序列埠除錯輸出

#### 強制測試程式 (GDEQ0426T82_Force_GxEPD2_Test.ino)
- **驗證 GxEPD2 庫安裝**的專用程式
- 強制使用 GxEPD2_426_GDEQ0426T82 驅動
- 適合第一次測試使用

### 引腳配置 (已最佳化)

```cpp
#define EPD_CS    15  // D8 - 片選 (+ 3.3kΩ下拉)
#define EPD_DC     0  // D3 - 資料/命令選擇
#define EPD_RST    5  // D1 - 重置 (+ 1kΩ上拉)
#define EPD_BUSY   4  // D2 - 忙碌偵測
```

**為什麼選擇這些引腳？**
- 避開 ESP8266 啟動敏感引腳
- GPIO15 (D8) 適合做 CS，但需要下拉電阻
- GPIO0 (D3) 在啟動後可安全使用
- SPI 引腳使用硬體 SPI (GPIO14, GPIO13)

### 記憶體最佳化

ESP8266 記憶體限制解決方案：
- 移除所有 `try-catch` 語法 (ESP8266 不支援)
- 使用 `display(true)` 替代 `displayWindow()` 減少記憶體使用
- 優化 SPI 時序，降低頻率確保穩定性

## 🐛 故障排除

### 常見問題與解決方案

#### 1. ESP8266 無法開機
**症狀**：上電後沒有序列埠輸出，或顯示開機失敗
**原因**：GPIO15 (CS) 沒有下拉電阻
**解決**：在 GPIO15 和 GND 之間加 3.3kΩ 電阻

#### 2. 電子紙沒有反應
**檢查項目**：
- ✅ 確認所有接線正確，特別是 SPI 引腳
- ✅ 檢查電源是否為 3.3V (不可用 5V)
- ✅ 確認 BUSY 引腳連接正確
- ✅ 檢查序列埠監控視窗是否有錯誤訊息

#### 3. 編譯錯誤
**常見錯誤**：
- `GxEPD2_426_GDEQ0426T82.h not found` → 確認 GxEPD2 庫已安裝
- `displayWindow() 參數錯誤` → 使用 `display(true)` 替代
- `try-catch 語法錯誤` → ESP8266 不支援，程式已修正

#### 4. 顯示異常
**症狀**：畫面破碎、部分更新失效
**解決**：
- 降低 SPI 頻率至 50kHz
- 確認 BUSY 引腳狀態正常
- 檢查電源穩定性

### 除錯技巧

#### 序列埠監控
```cpp
Serial.begin(115200);
Serial.printf("BUSY pin 狀態: %s\n", digitalRead(EPD_BUSY) ? "HIGH" : "LOW");
Serial.printf("可用記憶體: %d bytes\n", ESP.getFreeHeap());
```

#### 硬體測試
1. 先測試不接電子紙的程式上傳
2. 確認引腳輸出是否正常
3. 使用三用電表檢查引腳電壓

## 📚 進階開發

### 下一步開發方向

#### 功能擴展
- **氣象站**：整合溫濕度感測器，顯示即時天氣
- **資訊看板**：顯示時間、日期、提醒事項
- **電子書閱讀器**：從 SD 卡讀取文字檔案顯示
- **IoT 儀表板**：透過 WiFi 接收雲端資料顯示

#### 效能最佳化
- **部分更新**：實現真正的部分螢幕更新以提升效率
- **深度睡眠**：降低功耗，適合電池供電專案
- **圖片顯示**：最佳化記憶體使用，支援更大圖片
- **字型系統**：整合中文字型支援

#### 硬體整合
- **RTC 模組**：DS3231 即時時鐘，製作電子時鐘
- **感測器**：BME280 溫濕度氣壓感測器
- **存儲**：SD 卡模組，儲存更多資料和圖片
- **無線**：整合 WiFi 自動連網功能

### WiFi LED 控制進階應用

**wifi_led_control** 資料夾提供基礎的 WiFi 控制範例：
- ESP8266 熱點模式網頁控制
- 適合學習 ESP8266 WiFi 功能
- 可整合到電子紙專案中顯示網路狀態

## 📖 技術參考

### 核心文件
- [ESP8266 Arduino Core 文件](https://arduino-esp8266.readthedocs.io/)
- [GxEPD2 庫官方文檔](https://github.com/ZinggJM/GxEPD2)
- [GDEQ0426T82 規格書](./spec/)

### 社群資源
- [ESP8266 社群論壇](https://www.esp8266.com/)
- [Arduino ESP8266 板塊](https://forum.arduino.cc/c/hardware/esp8266/25)
- [電子紙顯示器開發者論壇](https://github.com/ZinggJM/GxEPD2/discussions)

## 🎯 專案成果

### 已解決的技術挑戰
- ✅ **記憶體限制**：ESP8266 RAM 不足問題
- ✅ **引腳相容性**：ESP8266 啟動引腳限制
- ✅ **SPI 時序**：電子紙 SPI 通訊最佳化
- ✅ **驅動移植**：Arduino 程式移植到 ESP8266
- ✅ **程式語法**：ESP8266 不支援的 C++ 語法修正

### 測試驗證結果
- ✅ **硬體連接**：引腳配置和電阻需求確認
- ✅ **軟體相容**：GxEPD2 庫在 ESP8266 上運作正常
- ✅ **顯示功能**：文字、圖形、部分更新都正常
- ✅ **穩定性**：長時間運作無當機問題
- ✅ **記憶體管理**：有效避免記憶體不足錯誤

## 📄 版本資訊

- **版本**: v3.0 - ESP8266 移植版
- **最後更新**: 2025年7月
- **狀態**: 穩定版本，可用於生產環境
- **開發者**: Arduino to ESP8266 移植專案組
- **授權**: MIT License

---

## 🤝 貢獻與回饋

歡迎提交問題報告和改進建議：
- **GitHub Issues**: 技術問題和 bug 報告
- **Pull Requests**: 程式碼改進和新功能
- **Discussions**: 使用心得和開發經驗分享

> 💡 **開發建議**：建議先完成電子紙基本顯示功能，再考慮整合其他感測器和網路功能。

**感謝所有為此專案貢獻的開發者！** 🎉
