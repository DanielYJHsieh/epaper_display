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
└── examples/
    ├── breathing_led.ino       # 呼吸燈效果
    └── multi_pattern.ino       # 多種閃爍模式
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

在序列埠監控視窗中輸入以下指令：

| 指令 | 功能 |
|------|------|
| 1-9  | 設定閃爍速度 (1=最快, 9=最慢) |
| s    | 停止閃爍 |
| r    | 恢復閃爍 |
| i    | 顯示系統資訊 |

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

### 1. WiFi 整合

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

### 2. 呼吸燈效果

使用 PWM 控制實現呼吸燈：

```cpp
// 使用 analogWrite() 控制 LED 亮度
for (int brightness = 0; brightness <= 255; brightness++) {
  analogWrite(LED_BUILTIN, 255 - brightness); // ESP8266 反向邏輯
  delay(10);
}
```

### 3. 多重模式

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

**作者**：Arduino 愛好者  
**更新日期**：2025年7月  
**版本**：v1.0

如有任何問題，請透過 GitHub Issues 與我們聯繫。
