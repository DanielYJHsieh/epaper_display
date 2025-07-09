# GDEQ0426T82 + WeMos D1 Mini 電子紙顯示器專案

## 🎯 專案概述

這是一個使用 **WeMos D1 Mini (ESP8266)** 控制 **GDEQ0426T82 4.26吋電子紙顯示器** 的完整解決方案。

### ✨ 重要更新 (v2.0)
- ✅ **使用專用驅動程式**: 發現並採用 GxEPD2 函式庫中的專用 `GxEPD2_426_GDEQ0426T82` 驅動程式
- ✅ **正確顯示規格**: 800×480 像素 (之前錯誤為 480×800)
- ✅ **SSD1677 控制器**: 支援快速部分更新和全螢幕更新
- ✅ **最佳化效能**: 使用原廠專用驅動程式，效果更佳

## 📋 硬體規格

### GDEQ0426T82 電子紙顯示器
- **尺寸**: 4.26 吋
- **解析度**: 800×480 像素
- **顏色**: 黑白 (無彩色)
- **控制器**: SSD1677
- **介面**: SPI
- **電源**: 3.3V
- **功能**: 
  - ✅ 部分更新 (`hasPartialUpdate = true`)
  - ✅ 快速部分更新 (`hasFastPartialUpdate = true`)
  - ✅ 快速全螢幕更新 (`useFastFullUpdate = true`)

### WeMos D1 Mini (ESP8266)
- **處理器**: ESP8266 @ 80MHz
- **記憶體**: ~80KB 可用 RAM
- **Flash**: 4MB
- **電源**: 3.3V/5V (USB)
- **腳位**: 11個數位 I/O 腳位

### 必需硬體
- **WeMos D1 Mini** ESP8266 開發板 × 1
- **GDEQ0426T82** 4.26吋電子紙顯示器 × 1
- **3.3kΩ 電阻** × 1 (CS 腳位下拉電阻，必需)
- **1kΩ 電阻** × 1 (RST 腳位上拉電阻，建議)
- **杜邦線或連接線** × 8條

## 🔌 硬體連接

### 完整接線圖
```
WeMos D1 Mini    →    GDEQ0426T82         說明
─────────────────     ─────────────       ─────────────────
3.3V             →    VCC                 電源 (3.3V，不要用5V!)
GND              →    GND                 接地
D5 (GPIO14/SCK)  →    CLK                 SPI 時脈
D7 (GPIO13/MOSI) →    DIN                 SPI 資料輸入
D8 (GPIO15)      →    CS  + 3.3kΩ→GND     片選 + 下拉電阻 ⚠️
D3 (GPIO0)       →    DC                  資料/命令選擇
D1 (GPIO5)       →    RST + 1kΩ→3.3V      重置 + 上拉電阻
D2 (GPIO4)       →    BUSY                忙碌偵測
```

### ⚠️ 重要電阻配置
1. **CS 腳位 (GPIO15) 下拉電阻**: 
   - **必須** 連接 3.3kΩ 電阻從 GPIO15 到 GND
   - 沒有此電阻，ESP8266 將無法正常開機！

2. **RST 腳位 (GPIO5) 上拉電阻**: 
   - **建議** 連接 1kΩ 電阻從 GPIO5 到 3.3V
   - 提高重置信號穩定性

### 🔧 SPI 腳位說明
WeMos D1 Mini 硬體 SPI 腳位：
- **SCK** (Serial Clock): D5 (GPIO14)
- **MOSI** (Master Out Slave In): D7 (GPIO13)
- **MISO** (Master In Slave Out): D6 (GPIO12) - 不使用
- **CS** (Chip Select): D8 (GPIO15) - 需要下拉電阻

## 💻 軟體設定

### 1. Arduino IDE 環境設定

#### 安裝 ESP8266 開發板支援
1. 開啟 Arduino IDE
2. **檔案** → **偏好設定**
3. **額外的開發板管理員網址** 加入：
   ```
   http://arduino.esp8266.com/stable/package_esp8266com_index.json
   ```
4. **工具** → **開發板** → **開發板管理員**
5. 搜尋 "**ESP8266**" → 安裝 "**ESP8266 by ESP8266 Community**"

#### 開發板設定
- **開發板**: LOLIN(WeMos) D1 mini
- **CPU Frequency**: 80 MHz
- **Flash Size**: 4MB (FS:2MB OTA:~1019KB)
- **Upload Speed**: 921600 (或 115200 if 921600 fails)

### 2. 必要函式庫安裝

#### GxEPD2 函式庫 (主要驅動程式)
```
Arduino IDE → 工具 → 管理函式庫 → 搜尋 "GxEPD2" 
→ 安裝 "GxEPD2 by Jean-Marc Zingg"
```

#### Adafruit GFX 函式庫 (圖形支援)
```
Arduino IDE → 工具 → 管理函式庫 → 搜尋 "Adafruit GFX" 
→ 安裝 "Adafruit GFX Library"
```

## 🚀 快速開始

### 1. 專用驅動程式確認
GDEQ0426T82 在 GxEPD2 函式庫中有專用驅動程式：
- **檔案位置**: `GxEPD2/src/gdeq/GxEPD2_426_GDEQ0426T82.h`
- **GitHub**: https://github.com/ZinggJM/GxEPD2/tree/master/src/gdeq
- **類別名稱**: `GxEPD2_426_GDEQ0426T82`

### 2. 程式碼結構
```cpp
// 包含專用驅動程式
#include <GxEPD2_BW.h>
#include <gdeq/GxEPD2_426_GDEQ0426T82.h>

// 腳位定義
#define EPD_CS    15  // D8 - 需要 3.3k 下拉電阻
#define EPD_DC     0  // D3
#define EPD_RST    5  // D1 - 建議 1k 上拉電阻  
#define EPD_BUSY   4  // D2

// 使用專用驅動程式初始化
GxEPD2_BW<GxEPD2_426_GDEQ0426T82, GxEPD2_426_GDEQ0426T82::HEIGHT> 
  display(GxEPD2_426_GDEQ0426T82(EPD_CS, EPD_DC, EPD_RST, EPD_BUSY));
```

### 3. 基本顯示流程
```cpp
void setup() {
  // 初始化顯示器
  display.init(115200);
  
  // 開始顯示
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
    display.setCursor(10, 30);
    display.print("Hello GDEQ0426T82!");
  } while (display.nextPage());
  
  // 省電模式
  display.powerOff();
}
```

## 📂 專案檔案說明

### 📋 程式檔案詳細功能

#### 1. **`GDEQ0426T82_WeMos_D1_Mini.ino`** - 主要範例程式 ⭐
> **適用對象**: 已安裝 GxEPD2 函式庫且硬體連接正確的使用者

**特色**:
- ✅ 使用專用 `GxEPD2_426_GDEQ0426T82` 驅動程式
- ✅ 正確的 800×480 解析度設定
- ✅ 完整的顯示功能示範
- ✅ 一次性執行模式 (適合測試)

**包含功能**:
- 🏠 **歡迎畫面**: 顯示硬體資訊和規格
- ℹ️ **系統資訊**: ESP8266 詳細資訊和腳位配置
- 🎨 **圖案測試**: 矩形、圓形、線條、文字、網格、像素圖案
- ⚡ **部分更新測試**: 快速部分更新功能測試
- 🛠️ **錯誤處理**: 完整的錯誤顯示機制

**執行流程**: setup() → 歡迎畫面 → 系統資訊 → 圖案測試 → 省電模式

---

#### 2. **`GDEQ0426T82_Basic_Test.ino`** - 診斷測試程式 🔧
> **適用對象**: 新手或遇到問題需要診斷的使用者

**特色**:
- ✅ 不依賴 GxEPD2 函式庫也能執行基本診斷
- ✅ 詳細的硬體檢測和腳位狀態監控
- ✅ 持續監控模式 (適合長期診斷)
- ✅ 智慧函式庫偵測 (有則使用，無則基本測試)

**包含功能**:
- 🔍 **硬體診斷**: ESP8266 系統資訊檢測
- 📌 **腳位檢查**: 所有連接腳位狀態監控
- 🌐 **SPI 測試**: SPI 通訊基本功能驗證
- 📡 **忙碌偵測**: BUSY 腳位即時狀態監控
- ⏰ **定期檢測**: 每 30 秒狀態報告，每 60 秒顯示測試

**執行流程**: 初始化 → 硬體檢測 → 持續監控循環

---

#### 3. **`GDEQ0426T82_Full_Demo.ino`** - 完整展示程式 🎪
> **適用對象**: 需要循環展示或動態內容的使用者

**特色**:
- ✅ 使用通用 `GxEPD2_420` 相容驅動程式
- ✅ 自動循環展示模式
- ✅ 多種字型和版面設計
- ✅ 動態內容更新

**包含功能**:
- 🎬 **啟動序列**: 漸進式歡迎畫面
- 🔄 **循環展示**: 自動切換不同演示內容
- 🎨 **多樣圖形**: 進階圖形繪製範例
- 📊 **即時狀態**: 動態系統資訊顯示
- ⚙️ **多字型**: 9pt、12pt、18pt 字型展示

**執行流程**: 啟動序列 → 循環演示 (每 5 秒切換) → 狀態監控

---

### 📁 配置檔案
- **`GxEPD2_display_selection_GDEQ0426T82.h`**: 專用驅動程式配置檔案

---

### 🚀 選擇建議

| 情況 | 推薦檔案 | 原因 |
|------|----------|------|
| **初次使用** | `GDEQ0426T82_Basic_Test.ino` | 診斷硬體連接和函式庫安裝 |
| **正常使用** | `GDEQ0426T82_WeMos_D1_Mini.ino` | 最佳效能，專用驅動程式 |
| **展示用途** | `GDEQ0426T82_Full_Demo.ino` | 持續循環展示，動態內容 |
| **故障排除** | `GDEQ0426T82_Basic_Test.ino` | 詳細診斷資訊 |

### 📝 使用順序建議
1. **第一步**: 執行 `GDEQ0426T82_Basic_Test.ino` 確認硬體正常
2. **第二步**: 執行 `GDEQ0426T82_WeMos_D1_Mini.ino` 測試完整功能
3. **第三步**: 根據需求選擇適合的程式進行開發

## 🔧 測試步驟

### 第一步：硬體檢查
1. ✅ 確認所有連線正確
2. ✅ 檢查 3.3kΩ 下拉電阻已連接 (CS → GND)
3. ✅ 檢查 1kΩ 上拉電阻已連接 (RST → 3.3V，可選)
4. ✅ 確認電源為 3.3V (不是 5V)

### 第二步：程式上傳
1. 選擇正確的連接埠 (COM)
2. 選擇開發板：LOLIN(WeMos) D1 mini
3. 編譯並上傳程式
4. 開啟序列埠監控視窗 (115200 baud)

### 第三步：功能驗證
1. 觀察序列埠輸出是否正常
2. 檢查電子紙是否顯示歡迎畫面
3. 等待 3 秒後切換到系統資訊畫面
4. 再等 3 秒後顯示圖案測試
5. 程式執行完畢後顯示器進入睡眠模式

## 🐛 常見問題與解決方案

### ❌ 問題 1: ESP8266 無法開機/卡在開機畫面
**症狀**: 上電後沒有反應或序列埠沒有輸出
**原因**: CS 腳位 (GPIO15) 沒有下拉電阻
**解決**: 
```
必須添加 3.3kΩ 電阻從 GPIO15 到 GND
這是 ESP8266 開機的必要條件！
```

### ❌ 問題 2: 顯示器沒有任何反應
**檢查清單**:
- ✅ 電源連接：3.3V 和 GND 是否正確
- ✅ BUSY 腳位：D2 (GPIO4) 連接是否正確
- ✅ SPI 連線：SCK (D5), MOSI (D7), CS (D8) 是否正確
- ✅ 控制腳位：DC (D3), RST (D1) 是否正確

### ❌ 問題 3: 顯示內容錯亂或尺寸不對
**原因**: 使用了錯誤的驅動程式或設定
**解決**: 
```cpp
// 確保使用專用驅動程式
#include <gdeq/GxEPD2_426_GDEQ0426T82.h>
GxEPD2_BW<GxEPD2_426_GDEQ0426T82, GxEPD2_426_GDEQ0426T82::HEIGHT> 
  display(GxEPD2_426_GDEQ0426T82(EPD_CS, EPD_DC, EPD_RST, EPD_BUSY));
```

### ❌ 問題 4: 編譯錯誤
**常見錯誤**:
- `#include errors detected`: GxEPD2 函式庫未安裝
- `class not found`: 選擇錯誤的開發板
- `undefined reference`: Adafruit GFX 函式庫未安裝

**解決步驟**:
1. 重新安裝 GxEPD2 和 Adafruit GFX 函式庫
2. 確認開發板選擇：LOLIN(WeMos) D1 mini
3. 清理並重新編譯專案

### ❌ 問題 5: 上傳失敗
**解決方案**:
1. 檢查 USB 線連接
2. 確認驅動程式已安裝 (CP2102/CH340)
3. 降低上傳速度：115200
4. 按住 FLASH 按鈕重新上傳

## ⚡ 效能最佳化

### 記憶體管理
```cpp
// ESP8266 記憶體有限，建議：
- 避免使用大型陣列
- 使用 F() 巨集存放字串到 Flash
- 分批顯示大量內容
```

### 電力管理
```cpp
// 省電技巧
display.powerOff();          // 顯示完畢後關閉顯示器
ESP.deepSleep(60e6);         // 深度睡眠 60 秒 (微秒)
```

### 顯示速度
```cpp
// 更新速度比較
全螢幕更新: 約 2-4 秒
部分更新:   約 0.3-1 秒 (如果支援)
快速更新:   約 0.1-0.5 秒 (如果支援)
```

## 🔧 進階應用範例

### WiFi 整合
```cpp
#include <ESP8266WiFi.h>

// 連接 WiFi 後可以：
- 從網路 API 獲取天氣資料
- 顯示即時新聞或股價
- IoT 感測器資料可視化
- 遠端控制顯示內容
```

### 定時更新
```cpp
#include <Ticker.h>

Ticker displayTimer;
displayTimer.attach(300, updateDisplay);  // 每 5 分鐘更新
```

### 電池供電
```cpp
// 建議配置：
- 18650 鋰電池 + 充電模組
- 電壓監控 (ESP8266 ADC)
- 低電量警告顯示
- 深度睡眠省電模式
```

## 📚 技術參考

### 官方文檔
- **GxEPD2 GitHub**: https://github.com/ZinggJM/GxEPD2
- **GDEQ0426T82 驅動程式**: https://github.com/ZinggJM/GxEPD2/tree/master/src/gdeq
- **ESP8266 Arduino**: https://arduino-esp8266.readthedocs.io/
- **WeMos D1 Mini**: https://www.wemos.cc/en/latest/d1/d1_mini.html

### 社群資源
- **ESP8266 社群**: https://www.esp8266.com/
- **Arduino 論壇**: https://forum.arduino.cc/c/hardware/esp8266/25
- **GxEPD2 Issues**: https://github.com/ZinggJM/GxEPD2/issues

## 📄 版本歷史

### v2.0 (2025/7/9) - 專用驅動程式版本
- ✅ 發現並使用 GxEPD2_426_GDEQ0426T82 專用驅動程式
- ✅ 修正顯示器尺寸為正確的 800×480 像素
- ✅ 新增 SSD1677 控制器詳細資訊
- ✅ 優化程式碼結構和效能
- ✅ 更新完整的技術文檔

### v1.0 (2025/1/9) - 初始版本
- 基本的相容性驅動程式實作
- 基礎顯示功能

---

## 💡 開發建議

1. **先測試基本連線**: 使用 `GDEQ0426T82_Basic_Test.ino` 驗證硬體
2. **注意電阻配置**: CS 下拉電阻是成功的關鍵
3. **使用專用驅動程式**: 效果比相容性驅動程式好很多
4. **善用部分更新**: 可大幅提升使用者體驗
5. **考慮電力管理**: 電池供電專案很重要

如有任何問題，請參考故障排除章節或查看 GitHub Issues。

**Happy Coding! 🚀**
