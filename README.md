# ESP8266MOD Arduino 開發 - LED 控制專案

## 專案簡介

這個專案示範如何使用 ESP8266MOD 模組進行 Arduino 程式開發，主要功能是控制板載 LED 進行閃爍。專案包含基本和進階的 LED 控制範例。

## 硬體需求

- ESP8266MOD 開發板 (如 NodeMCU、Wemos D1 Mini 等)
- USB 傳輸線 (Micro USB 或 USB-C，依開發板而定)
- 電腦 (Windows/Mac/Linux)

## 軟體需求

### 1. 安裝 Arduino IDE

1. 下載並安裝 [Arduino IDE](https://www.arduino.cc/en/software) (版本 2.0 或以上)
2. 啟動 Arduino IDE

### 2. 安裝 ESP8266 開發板支援

1. 開啟 Arduino IDE
2. 前往 **檔案** → **偏好設定**
3. 在 **額外的開發板管理員網址** 中加入：
   ```
   http://arduino.esp8266.com/stable/package_esp8266com_index.json
   ```
4. 前往 **工具** → **開發板** → **開發板管理員**
5. 搜尋 "ESP8266" 並安裝 **ESP8266 by ESP8266 Community**

### 3. 選擇開發板

1. 前往 **工具** → **開發板**
2. 選擇對應的 ESP8266 開發板：
   - **NodeMCU 1.0 (ESP-12E Module)**
   - **LOLIN(WEMOS) D1 R2 & mini**
   - **Generic ESP8266 Module**

## LED 腳位說明

| 開發板類型 | 板載 LED 腳位 | GPIO 編號 | 備註 |
|-----------|--------------|----------|------|
| NodeMCU   | D0           | GPIO16   | 板載藍色 LED |
| Wemos D1 Mini | D4       | GPIO2    | 板載藍色 LED |
| 通用      | LED_BUILTIN  | 自動偵測  | 由 Arduino 核心定義 |

**重要**：ESP8266 的板載 LED 通常是低電位觸發 (LOW = 開啟, HIGH = 關閉)

## 專案結構

```
esp8266-led-control/
├── README.md                    # 專案說明文件
├── basic_led_blink/
│   └── basic_led_blink.ino     # 基本 LED 閃爍程式
├── advanced_led_control/
│   └── advanced_led_control.ino # 進階 LED 控制程式
├── bluetooth_led_control/
│   └── bluetooth_led_control.ino # 藍芽 LED 控制程式 (需外接模組)
├── wifi_led_control/
│   └── wifi_led_control.ino    # WiFi LED 控制程式 (網頁控制)
├── epaper_display/
│   └── epaper_display.ino      # E-Paper 顯示器控制程式 ⭐ 新增
└── examples/
    ├── breathing_led.ino       # 呼吸燈效果
    ├── multi_pattern.ino       # 多種閃爍模式
    └── bluetooth_test.ino      # 藍芽模組測試
```

## 快速開始

### 1. 基本 LED 閃爍

開啟 `basic_led_blink/basic_led_blink.ino` 檔案，這是最簡單的 LED 控制程式：

- LED 每秒閃爍一次
- 透過序列埠輸出狀態訊息
- 適合初學者學習基本概念

### 2. 進階 LED 控制

開啟 `advanced_led_control/advanced_led_control.ino` 檔案，提供更多功能：

- 可調整閃爍頻率 (1-9 速度等級)
- 序列埠指令控制
- 非阻塞式程式設計
- 即時狀態回饋

### 3. 藍芽 LED 控制

開啟 `bluetooth_led_control/bluetooth_led_control.ino` 檔案，實現藍芽無線控制：

**⚠️ 重要說明**：ESP8266 **沒有內建藍芽模組**，需要外接 HC-05 或 HC-06 藍芽模組。

- **藍芽設備名稱**：DYJ_BT
- **已連接狀態**：LED 恆亮
- **未連接狀態**：LED 閃爍 (亮200ms，暗800ms)
- **序列埠監控**：顯示連接狀態和統計資訊
- **藍芽指令**：支援透過藍芽發送指令控制

#### 藍芽模組連接 (HC-05/HC-06)

| 藍芽模組 | ESP8266 | 說明 |
|----------|---------|------|
| VCC      | 3.3V    | 電源正極 |
| GND      | GND     | 電源負極 |
| RX       | D1 (GPIO5) | 資料接收 |
| TX       | D2 (GPIO4) | 資料傳送 |

### 4. WiFi LED 控制 ⭐ 推薦

開啟 `wifi_led_control/wifi_led_control.ino` 檔案，使用 WiFi 控制（無需額外硬體）：

- **WiFi 熱點名稱**：DYJ_LED_Control
- **連接密碼**：12345678
- **網頁控制**：http://192.168.4.1
- **手機連接時**：LED 恆亮
- **手機斷線時**：LED 閃爍 (亮200ms，暗800ms)
- **即時網頁控制**：開啟/關閉/切換 LED

#### WiFi 控制使用步驟

1. 上傳程式到 ESP8266
2. 手機連接 WiFi 熱點 "DYJ_LED_Control" (密碼: 12345678)
3. 瀏覽器開啟 http://192.168.4.1
4. 透過網頁控制 LED
5. 斷線後 LED 自動開始閃爍

### 5. E-Paper 顯示器控制 ⭐ 新功能

開啟 `epaper_display/epaper_display.ino` 檔案，控制 E-Paper 顯示器：

- **顯示器型號**：GDEQ0426T82 (4.2 寸)
- **驅動IC**：SSD1677
- **解析度**：800x480 像素
- **介面**：SPI
- **功能**：文字、圖形、圖片顯示

#### E-Paper 硬體連接

| E-Paper | ESP8266 | 功能 |
|---------|---------|------|
| VCC     | 3.3V    | 電源 |
| GND     | GND     | 接地 |
| DIN     | D7 (GPIO13) | SPI 資料 |
| CLK     | D5 (GPIO14) | SPI 時脈 |
| CS      | D8 (GPIO15) | 晶片選擇 |
| DC      | D3 (GPIO0)  | 資料/命令 |
| RST     | D4 (GPIO2)  | 重置 |
| BUSY    | D2 (GPIO4)  | 忙碌狀態 |

#### E-Paper 控制指令

| 指令 | 功能 |
|------|------|
| `help` | 顯示說明 |
| `clear` | 清空顯示器 |
| `test` | 顯示測試圖案 |
| `welcome` | 顯示歡迎訊息 |
| `info` | 顯示系統資訊 |
| `text <內容>` | 顯示自訂文字 |
| `sleep` | 進入深度睡眠 |

#### 藍芽配對步驟 (需外接模組)

1. 上傳程式到 ESP8266
2. 開啟手機藍芽功能
3. 搜尋藍芽設備 "DYJ_BT"
4. 配對時如需PIN碼，請輸入 "1234"
5. 連接成功後 LED 會恆亮
6. 斷線後 LED 會開始閃爍

## 上傳程式步驟

1. 使用 USB 線連接 ESP8266 開發板到電腦
2. 在 Arduino IDE 中選擇正確的 **連接埠** (工具 → 連接埠)
3. 選擇正確的 **開發板** (工具 → 開發板)
4. 設定開發板參數：
   - **CPU Frequency**: 80 MHz
   - **Flash Size**: 4MB (根據實際開發板調整)
   - **Upload Speed**: 115200
5. 開啟 .ino 檔案
6. 點擊 **上傳** 按鈕 (→)
7. 等待編譯和上傳完成

## 序列埠監控

1. 上傳完成後，開啟 **工具** → **序列埠監控視窗**
2. 設定鮑率為 **115200**
3. 觀察程式運行狀態和除錯訊息

### 進階控制指令

#### 基本 LED 控制指令

在序列埠監控視窗中輸入以下指令：

| 指令 | 功能 |
|------|------|
| 1-9  | 設定閃爍速度 (1=最快, 9=最慢) |
| s    | 停止閃爍 |
| r    | 恢復閃爍 |
| i    | 顯示系統資訊 |

#### 藍芽 LED 控制指令

| 指令 | 功能 |
|------|------|
| status (s) | 顯示目前連接狀態和 LED 狀態 |
| info (i)   | 顯示系統資訊和藍芽設定 |
| help (h)   | 顯示指令說明 |
| test       | 測試藍芽模組連接 |
| clear      | 清除統計資料 |
| reset (r)  | 重新啟動系統 |

## 故障排除

### 常見問題

1. **無法連接到開發板**
   - 檢查 USB 線連接是否穩固
   - 確認驅動程式已安裝 (CH340/CP2102/FTDI)
   - 嘗試按住 FLASH 按鈕再上傳
   - 檢查是否選擇了正確的連接埠

2. **編譯錯誤**
   - 確認已安裝 ESP8266 開發板套件
   - 檢查選擇的開發板型號是否正確
   - 確認 Arduino IDE 版本相容性

3. **LED 不閃爍**
   - 確認腳位定義正確
   - 檢查電源供應是否穩定
   - 確認程式已成功上傳

4. **序列埠無法開啟**
   - 檢查其他程式是否佔用序列埠
   - 確認鮑率設定為 115200
   - 重新插拔 USB 線

### 除錯技巧

- 使用 `Serial.println()` 輸出除錯訊息
- 檢查 LED_BUILTIN 常數值：`Serial.println(LED_BUILTIN);`
- 測試 GPIO 腳位：使用三用電表量測電壓變化
- 確認開發板規格：不同廠牌的 LED 腳位可能不同

## 進階開發

### 1. 藍芽整合

將 LED 控制與藍芽功能結合，實現無線控制：

```cpp
#include <SoftwareSerial.h>

// 藍芽模組連接
SoftwareSerial bluetooth(D1, D2); // RX, TX

void setup() {
  bluetooth.begin(9600);
  bluetooth.print("AT+NAME=DYJ_BT"); // 設定藍芽名稱
}

void loop() {
  if (bluetooth.available()) {
    String command = bluetooth.readString();
    // 處理藍芽指令
  }
}
```

### 2. WiFi 整合

將 LED 控制與 WiFi 功能結合：

```cpp
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

// WiFi 設定
const char* ssid = "你的WiFi名稱";
const char* password = "你的WiFi密碼";

// 網頁伺服器
ESP8266WebServer server(80);
```

### 3. 呼吸燈效果

使用 PWM 控制實現呼吸燈：

```cpp
// 使用 analogWrite() 控制 LED 亮度
for (int brightness = 0; brightness <= 255; brightness++) {
  analogWrite(LED_BUILTIN, 255 - brightness); // ESP8266 反向邏輯
  delay(10);
}
```

### 4. 多重模式

實作多種閃爍模式：
- 單次閃爍
- 雙重閃爍
- 漸變效果
- 隨機模式

## 電路擴展

### 外接 LED

如果要控制外接 LED，建議使用以下腳位：

| 腳位 | GPIO | 建議用途 |
|------|------|----------|
| D1   | GPIO5 | 數位輸出 |
| D2   | GPIO4 | 數位輸出 |
| D5   | GPIO14 | PWM 輸出 |
| D6   | GPIO12 | PWM 輸出 |

### 電路連接

```
ESP8266  →  LED
GPIO5    →  電阻 220Ω → LED 正極
GND      →  LED 負極
```

## 相關資源

### 官方文件
- [ESP8266 Arduino Core](https://arduino-esp8266.readthedocs.io/)
- [ESP8266 腳位對照表](https://randomnerdtutorials.com/esp8266-pinout-reference-gpios/)
- [Arduino 官方教學](https://www.arduino.cc/en/Tutorial/HomePage)

### 社群資源
- [ESP8266 Community Forum](https://www.esp8266.com/)
- [Arduino Forum ESP8266 Section](https://forum.arduino.cc/c/hardware/esp8266/25)
- [Random Nerd Tutorials](https://randomnerdtutorials.com/projects-esp8266/)

### 開發工具
- [VS Code + PlatformIO](https://platformio.org/) - 替代 Arduino IDE
- [ESP8266 Flash Download Tool](https://www.espressif.com/en/support/download/other-tools) - 韌體燒錄工具

## 授權聲明

此專案採用 MIT 授權條款，您可以自由使用、修改和分發。

## 貢獻指南

歡迎提交 Issue 和 Pull Request 來改善這個專案。

---

**作者**：DYJ Hsieh  
**更新日期**：2025年7月  
**版本**：v1.0

如有任何問題，請透過 GitHub Issues 與我們聯繫。
