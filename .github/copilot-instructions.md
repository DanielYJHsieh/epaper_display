<!-- Use this file to provide workspace-specific custom instructions to Copilot. For more details, visit https://code.visualstudio.com/docs/copilot/copilot-customization#_use-a-githubcopilotinstructionsmd-file -->

# ESP8266 Arduino 專案開發指引

這是一個 ESP8266MOD Arduino 開發專案，專注於 LED 控制功能。

## 專案特色

- **基本 LED 控制**：簡單的開關控制
- **進階 LED 控制**：多種閃爍模式和速度調整
- **呼吸燈效果**：使用 PWM 控制的平滑亮度變化
- **多模式閃爍**：包含 SOS、心跳、隨機等多種模式

## 開發規範

### Arduino 程式設計
- 使用標準 Arduino IDE 或 PlatformIO 開發
- 遵循 Arduino 程式結構：setup() 和 loop()
- 使用非阻塞式程式設計模式
- 適當使用 millis() 而非 delay() 進行時間控制

### ESP8266 特定注意事項
- 板載 LED 為反向邏輯 (LOW = 開啟, HIGH = 關閉)
- PWM 範圍為 0-1023 (而非標準 Arduino 的 0-255)
- 序列埠鮑率建議使用 115200
- 使用 LED_BUILTIN 常數來代表板載 LED 腳位

### 程式碼風格
- 使用有意義的變數和函數名稱
- 適當添加註解說明功能
- 將複雜功能分解為獨立函數
- 提供清楚的序列埠輸出訊息

### 硬體相容性
- 支援 NodeMCU、Wemos D1 Mini 等常見開發板
- 考慮不同開發板的 LED 腳位差異
- 提供腳位對照表和連接說明

### 使用者體驗
- 提供直觀的序列埠指令界面
- 顯示清楚的狀態資訊和說明
- 實作錯誤處理和輸入驗證
- 提供豐富的除錯訊息
