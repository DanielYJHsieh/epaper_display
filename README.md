# ESP8266 電子紙顯示與WiFi控制專案

## 🚀 專案簡介

這是一個基於 **WeMos D1 Mini (ESP8266)** 的雙功能專案，包含：

1. **GDEQ0426T82 電子紙顯示器** - 4.26吋 480×800 黑白電子紙顯示控制
2. **WiFi LED 控制系統** - 透過網頁介面遠端控制 LED

專案已完全精簡，僅保留核心功能，適合直接使用和進一步開發。

## 📦 專案結構

```
epaper_display/
├── GDEQ0426T82_WeMos_D1_Mini/          # 🎯 電子紙顯示器專案
│   ├── GDEQ0426T82_WeMos_D1_Mini.ino   # 主程式 (完整功能)
│   ├── GDEQ0426T82_Basic_Test.ino      # 基本測試程式
│   ├── GDEQ0426T82_Full_Demo.ino       # 完整示範程式
│   ├── GxEPD2_display_selection_GDEQ0426T82.h  # 顯示器配置
│   ├── README.md                       # 詳細使用說明
│   ├── INSTALLATION_GUIDE.md           # 安裝指南
│   ├── WIRING_DIAGRAM.md               # 接線圖說明
│   └── PROJECT_SUMMARY.md              # 專案摘要
├── wifi_led_control/                   # 🎯 WiFi LED 控制專案
│   └── wifi_led_control.ino            # WiFi LED 控制程式
└── README.md                           # 專案總說明
```

## 🛠️ 硬體需求

### 共同硬體
- **WeMos D1 Mini** ESP8266 開發板
- USB 傳輸線 (Micro USB)
- 電腦 (Windows/Mac/Linux)

### 電子紙顯示器專案額外需求
- **GDEQ0426T82** 4.26吋 480×800 電子紙顯示器
- 杜邦線或連接線
- **3.3kΩ 電阻** (CS 腳位下拉電阻，必需)
- **1kΩ 電阻** (RST 腳位上拉電阻，建議)

## ⚡ 快速開始

### 1. 環境設定

#### 安裝 Arduino IDE
1. 下載並安裝 [Arduino IDE](https://www.arduino.cc/en/software) (版本 2.0+)

#### 安裝 ESP8266 開發板支援
1. 開啟 Arduino IDE
2. **檔案** → **偏好設定**
3. **額外的開發板管理員網址** 加入：
   ```
   http://arduino.esp8266.com/stable/package_esp8266com_index.json
   ```
4. **工具** → **開發板** → **開發板管理員**
5. 搜尋 "ESP8266" → 安裝 **ESP8266 by ESP8266 Community**

#### 選擇開發板
**工具** → **開發板** → **LOLIN(WEMOS) D1 R2 & mini**

### 2. 必要函式庫安裝

#### 電子紙顯示器專案需要的函式庫
**工具** → **管理程式庫** → 搜尋並安裝：
- **GxEPD2** by Jean-Marc Zingg
- **Adafruit GFX Library**

#### WiFi LED 控制專案函式庫
ESP8266 核心已包含所需函式庫：
- `ESP8266WiFi.h`
- `ESP8266WebServer.h`

## 🖥️ 專案使用說明

### 📺 GDEQ0426T82 電子紙顯示器

#### 硬體連接 (WeMos D1 Mini → GDEQ0426T82)
```
BUSY → D2 (GPIO4)    - 忙碌偵測腳位
RST  → D1 (GPIO5)    - 重置腳位 + 1kΩ電阻到3.3V
DC   → D3 (GPIO0)    - 資料/命令選擇腳位
CS   → D8 (GPIO15)   - SPI片選腳位 + 3.3kΩ電阻到GND ⚠️
CLK  → D5 (GPIO14)   - SPI時脈腳位 (SCK)
DIN  → D7 (GPIO13)   - SPI資料腳位 (MOSI)
GND  → GND           - 接地
3.3V → 3.3V          - 電源 (3.3V)
```

> ⚠️ **重要**：CS 腳位 (GPIO15) 必須加 3.3kΩ 下拉電阻到 GND，否則 ESP8266 無法正常開機！

#### 程式功能
- **歡迎畫面**：顯示基本硬體資訊
- **系統資訊**：ESP8266 晶片資訊和腳位配置
- **圖案測試**：矩形、圓形、線條、三角形、文字、網格、像素等
- **錯誤處理**：顯示錯誤訊息

#### 快速測試
1. 按照接線圖連接硬體
2. 開啟 `GDEQ0426T82_WeMos_D1_Mini/GDEQ0426T82_WeMos_D1_Mini.ino`
3. 上傳程式到 WeMos D1 Mini
4. 打開序列埠監控視窗 (115200 baud)
5. 觀察電子紙顯示器顯示測試內容

### 📶 WiFi LED 控制系統

#### 功能特色
- **WiFi 熱點模式**：ESP8266 建立 WiFi 熱點
- **網頁控制介面**：透過瀏覽器控制 LED
- **即時狀態回饋**：LED 狀態即時更新
- **連線狀態指示**：連線時恆亮，斷線時閃爍

#### 快速使用
1. 開啟 `wifi_led_control/wifi_led_control.ino`
2. 上傳程式到 WeMos D1 Mini
3. 手機或電腦連接 WiFi 熱點：`DYJ_LED_Control`
4. 密碼：`12345678`
5. 瀏覽器開啟：`http://192.168.4.1`
6. 透過網頁介面控制 LED 開關

#### WiFi 設定
預設為熱點模式，如需連接現有 WiFi，請編輯程式：
```cpp
// 取消註解並填入您的 WiFi 資訊
const char* wifi_ssid = "您的WiFi名稱";
const char* wifi_password = "您的WiFi密碼";
```

## 🔧 上傳程式步驟

1. **連接硬體**：USB 線連接 WeMos D1 Mini 到電腦
2. **選擇連接埠**：**工具** → **連接埠** → 選擇對應 COM 埠
3. **設定參數**：
   - **CPU Frequency**: 80 MHz
   - **Flash Size**: 4MB
   - **Upload Speed**: 115200
4. **編譯上傳**：點擊上傳按鈕 (→)
5. **監控輸出**：**工具** → **序列埠監控視窗** (115200 baud)

## 🐛 故障排除

### 常見問題

#### 1. 無法連接到開發板
- ✅ 檢查 USB 線連接
- ✅ 確認 CP2102/CH340 驅動程式已安裝
- ✅ 選擇正確的連接埠
- ✅ 按住 FLASH 按鈕重新上傳

#### 2. 電子紙顯示器無反應
- ✅ 確認所有接線正確
- ✅ 檢查 CS 腳位是否有 3.3kΩ 下拉電阻
- ✅ 確認電源為 3.3V (不是 5V)
- ✅ 檢查 GxEPD2 函式庫是否已安裝

#### 3. WiFi 連線問題
- ✅ 確認 WiFi 名稱和密碼正確
- ✅ 檢查手機是否連接到正確的熱點
- ✅ 確認 IP 位址：`192.168.4.1`
- ✅ 重啟 ESP8266 重新建立熱點

#### 4. 編譯錯誤
- ✅ 確認 ESP8266 開發板套件已安裝
- ✅ 檢查必要函式庫是否已安裝
- ✅ 確認選擇了正確的開發板型號

## 📚 進階開發

### 電子紙顯示器擴展
- 自訂字型和圖片顯示
- 部分更新功能 (如果支援)
- 整合感測器資料顯示
- 低功耗模式實作

### WiFi 控制系統擴展
- MQTT 協議整合
- 多 LED 或 RGB LED 控制
- 感測器資料即時顯示
- 移動 App 開發

### 專案整合
將兩個專案整合成一個完整系統：
- 電子紙顯示 WiFi 狀態和控制介面
- 透過網頁控制電子紙顯示內容
- 感測器資料收集與顯示

## 📖 相關資源

### 官方文件
- [ESP8266 Arduino Core](https://arduino-esp8266.readthedocs.io/)
- [GxEPD2 函式庫文件](https://github.com/ZinggJM/GxEPD2)
- [WeMos D1 Mini 腳位圖](https://randomnerdtutorials.com/esp8266-pinout-reference-gpios/)

### 技術支援
- [ESP8266 Community Forum](https://www.esp8266.com/)
- [Arduino Forum ESP8266](https://forum.arduino.cc/c/hardware/esp8266/25)
- [GxEPD2 GitHub Issues](https://github.com/ZinggJM/GxEPD2/issues)

## 📄 版本資訊

- **版本**: v2.0
- **更新日期**: 2025年7月
- **作者**: Arduino ESP8266 專案團隊
- **授權**: MIT License

## 🤝 貢獻

歡迎提交 Issue 和 Pull Request 來改善專案！

---

> 💡 **提示**：建議先從 WiFi LED 控制開始測試，確認基本功能正常後再進行電子紙顯示器的硬體連接。

如有任何問題，請查看各專案資料夾中的詳細說明文件，或透過 GitHub Issues 聯繫我們。
