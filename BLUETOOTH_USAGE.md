# 藍芽LED控制功能使用說明

## 🎉 功能已完成！

您要求的藍芽LED控制功能已經完全實現並加入專案中：

### ✅ 實現的功能

1. **藍芽設備名稱**：DYJ_BT
2. **連線狀態控制**：
   - 🔵 **有手機連接**：LED 恆亮
   - 🔴 **無手機連接**：LED 每秒閃爍（亮 200ms，暗 800ms）
3. **完整的狀態監控和統計功能**
4. **豐富的序列埠指令控制**

### 📁 新增的檔案

- `bluetooth_led_control/bluetooth_led_control.ino` - 主要藍芽控制程式
- `examples/bluetooth_test.ino` - 藍芽模組測試程式
- 更新了 `README.md` 和 `QUICKSTART.md` 文件
- 新增了 VS Code 編譯和上傳任務

### 🚀 如何使用

#### 方法 1：使用 Arduino IDE
1. 雙擊 `start_arduino.bat` 啟動 Arduino IDE
2. 開啟 `bluetooth_led_control/bluetooth_led_control.ino`
3. 選擇正確的開發板（NodeMCU 1.0 或 Wemos D1 Mini）
4. 選擇正確的 COM 連接埠
5. 點擊上傳按鈕

#### 方法 2：使用 VS Code 任務
1. 在 VS Code 中按 `Ctrl+Shift+P`
2. 輸入 "Tasks: Run Task"
3. 選擇 "Compile Bluetooth LED" 編譯程式
4. 選擇 "Upload Bluetooth LED" 上傳程式

### 📱 藍芽配對步驟

1. **上傳程式**到 ESP8266
2. **開啟手機藍芽**功能
3. **搜尋藍芽設備** "DYJ_BT"
4. **配對連接**（PIN碼：1234，如果需要）
5. **觀察 LED 狀態**：
   - 連接成功 → LED 恆亮 ✅
   - 斷線 → LED 開始閃爍 🔄

### 🖥️ 序列埠監控

開啟序列埠監控視窗（鮑率 115200）可以看到：
- 連接狀態變化
- 統計資訊（閃爍次數、連接次數等）
- 系統運行狀態

**可用指令**：
- `status` 或 `s` - 顯示目前狀態
- `info` 或 `i` - 顯示系統資訊
- `help` 或 `h` - 顯示說明
- `test` - 測試藍芽模組
- `clear` - 清除統計資料
- `reset` 或 `r` - 重新啟動

### 🔧 硬體連接（如使用外接藍芽模組）

| 藍芽模組 | ESP8266 |
|----------|---------|
| VCC      | 3.3V    |
| GND      | GND     |
| RX       | D1 (GPIO5) |
| TX       | D2 (GPIO4) |

### 🎯 程式特色

- **非阻塞式設計**：使用 millis() 而非 delay()
- **智能連接檢測**：自動檢測藍芽連接狀態
- **完整的 AT 指令支援**：自動設定藍芽模組
- **詳細的狀態回饋**：透過序列埠顯示所有資訊
- **統計功能**：記錄閃爍次數、連接次數和運行時間
- **錯誤處理**：包含完整的錯誤檢測和處理機制

## 🔥 立即開始使用

1. 雙擊 `start_arduino.bat` 開啟 Arduino IDE
2. 載入 `bluetooth_led_control/bluetooth_led_control.ino`
3. 上傳到您的 ESP8266 開發板
4. 用手機搜尋並連接 "DYJ_BT"
5. 享受無線 LED 控制的樂趣！

---
**專案版本**：v1.0  
**更新日期**：2025年7月  
**功能狀態**：✅ 完全實現
