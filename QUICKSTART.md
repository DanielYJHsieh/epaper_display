# 快速開始指南

## 🚀 如何運行專案

### 方法 1: 使用 VS Code 任務
1. 在 VS Code 中按 `Ctrl+Shift+P`
2. 輸入 "Tasks: Run Task"
3. 選擇 "Open Arduino IDE"

### 方法 2: 手動啟動
1. 雙擊 `start_arduino.bat` 檔案
2. 或在命令提示字元中運行：`start_arduino.bat`

### 方法 3: 直接開啟 Arduino IDE
1. 啟動 Arduino IDE
2. 開啟專案中的 `.ino` 檔案

## 📁 專案檔案說明

### 基本程式
- `basic_led_blink/basic_led_blink.ino` - 最簡單的 LED 閃爍
- 適合初學者，每秒閃爍一次

### 進階程式  
- `advanced_led_control/advanced_led_control.ino` - 功能豐富的控制程式
- 支援 9 種速度、5 種模式、序列埠指令控制

### 藍芽控制程式
- `bluetooth_led_control/bluetooth_led_control.ino` - 藍芽無線控制
- 藍芽名稱：DYJ_BT，連線時 LED 恆亮，未連線時閃爍

### 範例程式
- `examples/breathing_led.ino` - 呼吸燈效果 (PWM 控制)
- `examples/multi_pattern.ino` - 8 種閃爍模式 (SOS、心跳等)

## ⚙️ Arduino IDE 設定

### 1. 安裝 ESP8266 支援
在 Arduino IDE 中：
1. 檔案 → 偏好設定
2. 額外的開發板管理員網址加入：
   ```
   http://arduino.esp8266.com/stable/package_esp8266com_index.json
   ```
3. 工具 → 開發板 → 開發板管理員
4. 搜尋並安裝 "ESP8266 by ESP8266 Community"

### 2. 選擇開發板
工具 → 開發板 → ESP8266 → 選擇：
- **NodeMCU 1.0 (ESP-12E Module)** (推薦)
- **LOLIN(WEMOS) D1 R2 & mini**
- **Generic ESP8266 Module**

### 3. 設定參數
- **CPU Frequency**: 80 MHz
- **Flash Size**: 4MB (根據開發板調整)
- **Upload Speed**: 115200

## 🔌 硬體連接

| 開發板 | 板載 LED 腳位 | GPIO |
|--------|--------------|------|
| NodeMCU | D0 | GPIO16 |
| Wemos D1 Mini | D4 | GPIO2 |

**注意**: ESP8266 板載 LED 為反向邏輯 (LOW=開啟, HIGH=關閉)

## 🖥️ 序列埠監控

1. 上傳程式後，開啟 **工具 → 序列埠監控視窗**
2. 設定鮑率為 **115200**
3. 可以看到程式執行狀態和除錯訊息

### 進階控制指令 (advanced_led_control.ino)
- `1-9`: 設定閃爍速度
- `s`: 停止閃爍
- `r`: 恢復閃爍  
- `p`: 切換模式
- `h`: 顯示說明

### 藍芽控制指令 (bluetooth_led_control.ino)
- `status` 或 `s`: 顯示連接狀態
- `info` 或 `i`: 顯示系統資訊
- `help` 或 `h`: 顯示說明
- `test`: 測試藍芽模組
- `clear`: 清除統計資料
- `reset` 或 `r`: 重新啟動

#### 📱 藍芽配對步驟
1. 上傳 `bluetooth_led_control.ino` 到 ESP8266
2. 手機開啟藍芽，搜尋 "DYJ_BT"
3. 配對 (PIN碼: 1234，如果需要)
4. 連接成功→ LED 恆亮，斷線→ LED 閃爍

## ❓ 常見問題

### 無法上傳程式
1. 檢查 USB 線連接
2. 確認選擇正確的 COM 埠
3. 嘗試按住 FLASH 按鈕再上傳

### LED 不閃爍
1. 確認程式已成功上傳
2. 檢查開發板型號選擇是否正確
3. 確認 LED_BUILTIN 腳位定義

### 序列埠無法開啟
1. 關閉其他佔用序列埠的程式
2. 重新插拔 USB 線
3. 確認鮑率設定為 115200

## 📞 需要幫助？

查看 `README.md` 獲得完整的說明文件，包含：
- 詳細的安裝指南
- 故障排除方案
- 進階開發技巧
- 電路擴展建議

---
**快樂編程！** 🎯
