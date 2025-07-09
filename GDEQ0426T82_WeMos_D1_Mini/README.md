# GDEQ0426T82 + WeMos D1 Mini 設定指南

## 硬體需求

- **WeMos D1 Mini** (ESP8266 開發板)
- **GDEQ0426T82** (4.26吋 480x800 電子紙顯示器)
- 杜邦線或麵包板連接線
- 3.3k 電阻 (下拉電阻用)
- 1k 電阻 (上拉電阻用，可選)

## 連線圖

```
WeMos D1 Mini    →    GDEQ0426T82
─────────────────     ─────────────
3.3V             →    VCC (3.3V)
GND              →    GND
D5 (GPIO14/SCK)  →    CLK (SPI時脈)
D7 (GPIO13/MOSI) →    DIN (SPI資料)
D8 (GPIO15)      →    CS  (片選) + 3.3k電阻到GND
D3 (GPIO0)       →    DC  (資料/命令)
D1 (GPIO5)       →    RST (重置) + 可選1k電阻到3.3V
D2 (GPIO4)       →    BUSY (忙碌偵測)
```

## 重要注意事項

### 1. 電阻需求
- **CS (GPIO15)**: 必須加 3.3k 下拉電阻到 GND，否則 ESP8266 可能無法正常開機
- **RST (GPIO5)**: 建議加 1k 上拉電阻到 3.3V，提高復位穩定性

### 2. 電壓準位
- 所有信號線都是 3.3V 邏輯準位
- 不要連接 5V 電源，會損壞顯示器

### 3. SPI 腳位
- WeMos D1 Mini 的硬體 SPI 腳位：
  - SCK: D5 (GPIO14)
  - MOSI: D7 (GPIO13)
  - MISO: D6 (GPIO12) - 不使用

## 軟體需求

### Arduino IDE 設定

1. **安裝 ESP8266 開發板支援**
   - 檔案 → 偏好設定 → 額外的開發板管理員網址
   - 添加：`http://arduino.esp8266.com/stable/package_esp8266com_index.json`
   - 工具 → 開發板 → 開發板管理員 → 搜尋 "ESP8266" → 安裝

2. **選擇開發板**
   - 工具 → 開發板 → ESP8266 → LOLIN(WeMos) D1 mini

3. **開發板設定**
   - CPU Frequency: 80 MHz
   - Flash Size: 4MB (FS:2MB OTA:~1019KB)
   - Upload Speed: 921600

### 必要函式庫

#### GxEPD2 函式庫
```bash
# 在 Arduino IDE 中：
# 工具 → 管理函式庫 → 搜尋 "GxEPD2" → 安裝 Jean-Marc Zingg 的 GxEPD2
```

#### Adafruit GFX 函式庫
```bash
# 在 Arduino IDE 中：
# 工具 → 管理函式庫 → 搜尋 "Adafruit GFX" → 安裝
```

## 驅動程式相容性

GDEQ0426T82 可能沒有專用的 GxEPD2 驅動程式。可以嘗試以下相容驅動程式：

1. **GxEPD2_420** - 4.2吋驅動程式 (最相近)
2. **GxEPD2_420_M01** - 4.2吋變體
3. **GxEPD2_426** - 如果有 4.26吋專用驅動

## 測試步驟

1. **檢查連線**
   - 確認所有連線正確
   - 特別檢查電阻是否正確連接

2. **上傳測試程式**
   - 使用提供的範例程式
   - 監控序列埠輸出

3. **觀察顯示效果**
   - 顯示器應該顯示測試圖案
   - 如果顯示不正確，嘗試不同的驅動程式

## 疑難排解

### 問題 1: ESP8266 無法開機
**原因**: CS 腳位 (GPIO15) 沒有下拉電阻
**解決**: 添加 3.3k 電阻從 GPIO15 到 GND

### 問題 2: 顯示器沒有反應
**檢查項目**:
- 電源連接 (3.3V 和 GND)
- BUSY 腳位連接
- SPI 連線 (SCK, MOSI, CS)

### 問題 3: 顯示內容錯亂
**可能原因**:
- 驅動程式不相容
- 顯示器尺寸設定錯誤
**解決**: 嘗試不同的驅動程式

### 問題 4: 編譯錯誤
**檢查項目**:
- 函式庫是否正確安裝
- 開發板選擇是否正確
- 程式碼語法是否有誤

## 效能最佳化

### 記憶體使用
- ESP8266 記憶體有限 (~80KB 可用)
- 對於大型顯示器，考慮使用分頁顯示

### 電力管理
- 顯示完成後調用 `display.powerOff()`
- 使用 ESP8266 的深度睡眠模式

### 更新速度
- 全螢幕更新較慢 (數秒)
- 如果支援，使用部分更新

## 範例程式解說

範例程式包含以下功能：
1. **系統初始化** - 設定腳位和顯示器
2. **歡迎畫面** - 顯示基本資訊
3. **系統資訊** - ESP8266 和腳位配置
4. **圖案測試** - 各種圖形和文字
5. **錯誤處理** - 顯示錯誤訊息

## 進階應用

### WiFi 連接
可以結合 WiFi 功能，從網路獲取資料並顯示：
- 天氣資訊
- 新聞摘要
- IoT 感測器資料

### 定時更新
使用 ESP8266 的定時器功能，週期性更新顯示內容

### 電池供電
使用鋰電池和電源管理模組，實現便攜式顯示器

## 參考資料

- [GxEPD2 GitHub](https://github.com/ZinggJM/GxEPD2)
- [WeMos D1 Mini 腳位圖](https://lastminuteengineers.com/wemos-d1-mini-pinout-reference/)
- [ESP8266 Arduino Core](https://github.com/esp8266/Arduino)
- [Adafruit GFX 文檔](https://learn.adafruit.com/adafruit-gfx-graphics-library)
