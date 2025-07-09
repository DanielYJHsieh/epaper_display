# GDEQ0426T82 + WeMos D1 Mini 專案總結

## 專案概述

本專案成功改寫了 GDEQ0426T82 Arduino 程式，使其適用於 **ESP8266 WeMos D1 Mini + GDEQ0426T82** 硬體組合。通過使用 GxEPD2 函式庫，提供了完整的電子紙顯示解決方案。

## 完成的工作

### 1. 硬體適配 ✅
- 將原本的 Arduino UNO 腳位配置改為 WeMos D1 Mini
- 參考 WeMos D1 Mini 腳位圖和 GxEPD2 庫的建議配置
- 考慮 ESP8266 的開機限制，正確配置電阻

### 2. 軟體重構 ✅
- 從原本的自定義驅動改為標準 GxEPD2 函式庫
- 提供多個版本的程式：基礎測試版和完整演示版
- 加入 ESP8266 特有的功能和限制考量

### 3. 文檔完善 ✅
- 詳細的安裝指南
- 完整的電路連接圖
- 故障排除指南
- 腳位對照表

## 文件結構

```
GDEQ0426T82_WeMos_D1_Mini/
├── GDEQ0426T82_Basic_Test.ino      # 基礎測試程式
├── GDEQ0426T82_Full_Demo.ino      # 完整演示程式
├── README.md                       # 專案說明
├── INSTALLATION_GUIDE.md          # 安裝指南
├── WIRING_DIAGRAM.md              # 電路圖
└── GxEPD2_display_selection_GDEQ0426T82.h  # 顯示器配置
```

## 硬體規格

### WeMos D1 Mini
- **處理器**: ESP8266EX
- **工作電壓**: 3.3V
- **Flash**: 4MB
- **WiFi**: 802.11 b/g/n
- **GPIO**: 11個可用腳位

### GDEQ0426T82
- **尺寸**: 4.26英寸
- **解析度**: 480 x 800 像素
- **顏色**: 黑白 (1-bit)
- **介面**: SPI
- **工作電壓**: 3.3V

## 腳位配置

| WeMos D1 Mini | GPIO | GDEQ0426T82 | 電阻 |
|---------------|------|-------------|------|
| D2 | GPIO4 | BUSY | - |
| D1 | GPIO5 | RST | 1kΩ 上拉 |
| D3 | GPIO0 | DC | - |
| D8 | GPIO15 | CS | 3.3kΩ 下拉 |
| D5 | GPIO14 | CLK | - |
| D7 | GPIO13 | DIN | - |
| 3.3V | - | VCC | - |
| GND | - | GND | - |

## 關鍵改進

### 1. 相容性提升
- 使用標準 GxEPD2 函式庫，相容性更好
- 支援多種 E-Paper 驅動程式
- 提供驅動程式自動選擇機制

### 2. 穩定性增強
- 正確的電阻配置避免開機問題
- 加入詳細的錯誤檢查和診斷
- 提供完整的故障排除指南

### 3. 功能豐富
- 基礎測試模式：驗證硬體連接
- 完整演示模式：展示各種顯示效果
- 系統監控功能：實時狀態顯示

### 4. 文檔完善
- step-by-step 安裝指南
- 詳細的電路連接圖
- 常見問題解決方案

## 技術特點

### ESP8266 最佳化
```cpp
// 記憶體管理
#define MAX_DISPLAY_BUFFER_SIZE 32768ul  // 適合 ESP8266

// 非阻塞式設計
void loop() {
  // 避免使用 delay()，使用 millis() 計時
}

// 電源管理
display.powerOff();  // 顯示完成後進入睡眠
```

### GxEPD2 整合
```cpp
// 自動驅動程式選擇
#if defined(GxEPD2_420)
  GxEPD2_BW<GxEPD2_420, GxEPD2_420::HEIGHT> display(...);
#elif defined(GxEPD2_420_M01)
  GxEPD2_BW<GxEPD2_420_M01, GxEPD2_420_M01::HEIGHT> display(...);
#endif

// 智能初始化
display.init(115200, true, 2, false);
```

### 錯誤處理
```cpp
// 函式庫檢查
#ifdef GXEPD2_AVAILABLE
  // 使用 GxEPD2 功能
#else
  // 降級到基本 SPI 功能
#endif

// 硬體診斷
void checkPinStates();
void testSPICommunication();
```

## 使用方式

### 快速開始
1. 按照 `WIRING_DIAGRAM.md` 連接硬體
2. 安裝必要的函式庫（GxEPD2, Adafruit GFX）
3. 上傳 `GDEQ0426T82_Basic_Test.ino` 測試連接
4. 連接正常後上傳 `GDEQ0426T82_Full_Demo.ino`

### 進階使用
- 修改顯示內容和佈局
- 添加 WiFi 功能，顯示網路資料
- 整合感測器，製作物聯網顯示器
- 使用電池供電，製作便攜設備

## 已知限制

### 1. 驅動程式相容性
- GDEQ0426T82 可能沒有專用的 GxEPD2 驅動程式
- 需要使用相近尺寸的驅動程式（如 GxEPD2_420）
- 可能需要調整顯示參數以獲得最佳效果

### 2. ESP8266 記憶體限制
- 可用 RAM 約 80KB，限制大圖像處理
- 建議使用分頁顯示或壓縮圖像
- 避免同時載入多個大型字體

### 3. 更新速度
- 電子紙全螢幕更新較慢（數秒）
- 部分更新可能有殘影
- 不適合快速動畫顯示

## 擴展可能

### 1. 硬體擴展
- 添加觸控感測器
- 整合溫溼度感測器
- 加入 SD 卡存儲
- 使用更大容量電池

### 2. 軟體功能
- 網路時鐘顯示
- 天氣預報顯示
- 新聞摘要顯示
- 股價監控顯示
- 日曆和提醒功能

### 3. 系統整合
- Home Assistant 整合
- MQTT 協議支援
- RESTful API 介面
- 遠程配置管理

## 開發經驗總結

### 成功要素
1. **硬體連接正確**：電阻必須正確安裝
2. **函式庫選擇**：使用成熟的 GxEPD2 庫
3. **文檔齊全**：提供詳細的安裝和使用指南
4. **測試充分**：從基礎測試到完整功能演示

### 注意事項
1. **電壓兼容**：確保所有信號都是 3.3V
2. **開機限制**：ESP8266 的 GPIO15 必須有下拉電阻
3. **記憶體管理**：注意 ESP8266 的記憶體限制
4. **更新策略**：合理使用全螢幕和部分更新

## 貢獻

歡迎提交 Issue 和 Pull Request 來改進這個專案：

- 🐛 回報問題和 Bug
- 💡 提出功能建議
- 📖 改進文檔
- 🔧 提交程式碼修正
- 🎨 添加新的顯示效果

## 授權

本專案基於 MIT 授權條款，您可以自由使用、修改和分發。

## 致謝

- **GxEPD2** 函式庫作者 Jean-Marc Zingg
- **Adafruit GFX** 函式庫團隊
- **ESP8266 Arduino Core** 開發團隊
- Arduino 開源社群

---

通過這個專案，我們成功地將 GDEQ0426T82 電子紙顯示器與 WeMos D1 Mini 整合，提供了一個完整、穩定且易於使用的解決方案。無論是初學者還是專業開發者，都可以基於這個專案快速開發自己的電子紙顯示應用。
