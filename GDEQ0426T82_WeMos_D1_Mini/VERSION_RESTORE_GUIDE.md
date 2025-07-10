# 📂 版本恢復指南

## 恢復完成 ✅

我已經將程式恢復到備份文件中的原始配置。

### 🔄 恢復的引腳配置

從備份文件 `GDEQ0426T82_Basic_Test.ino.backup` 中恢復的引腳配置：

```cpp
#define EPD_CS    15  // D8 - CS (必須加 3.3k 下拉電阻到 GND)
#define EPD_DC     0  // D3 - DC
#define EPD_RST    5  // D1 - RST (建議加 1k 上拉電阻到 3.3V)
#define EPD_BUSY   4  // D2 - BUSY
```

### 🔌 硬體連接

```
EPD_BUSY -> D2 (GPIO4)  - 忙碌偵測腳位
EPD_RST  -> D1 (GPIO5)  - 重置腳位 (建議加 1k 上拉電阻到 3.3V)
EPD_DC   -> D3 (GPIO0)  - 資料/命令選擇腳位
EPD_CS   -> D8 (GPIO15) - SPI 片選腳位 (必須加 3.3k 下拉電阻到 GND)
EPD_CLK  -> D5 (GPIO14) - SPI 時脈腳位 (SCK)
EPD_DIN  -> D7 (GPIO13) - SPI 資料輸入腳位 (MOSI)
VCC      -> 3V3
GND      -> GND
```

### 📁 已恢復的檔案

| 檔案 | 狀態 | 說明 |
|------|------|------|
| `GDEQ0426T82_WeMos_D1_Mini.ino` | ✅ 恢復完成 | 主程式，使用備份引腳配置 |
| `GDEQ0426T82_Force_GxEPD2_Test.ino` | ✅ 恢復完成 | 強制測試程式，使用相同配置 |

### 🔧 編譯狀態

兩個程式都編譯成功：
- **主程式**: 97% RAM, 24% Flash ✅
- **Force 測試**: 97% RAM, 24% Flash ✅

### 🎯 重要保留的修正

雖然恢復了引腳配置，但保留了重要的 ESP8266 相容性修正：
- ✅ 移除了所有 `try-catch` 異常處理
- ✅ 修正了 `displayWindow()` 函數調用
- ✅ 保持 GxEPD2 強制啟用

### 🚀 下一步

現在您可以：
1. 上傳程式到 WeMos D1 Mini Pro
2. 使用備份文件中記錄的硬體連接
3. 確保添加必要的電阻（CS: 3.3k下拉, RST: 1k上拉）
4. 觀察 Serial Monitor 輸出 (115200 baud)

### 📋 版本歷史

- **當前版本**: 恢復到備份文件的引腳配置
- **上一版本**: 使用 D6/D4/D3/D8 配置
- **修正內容**: ESP8266 相容性 + 備份引腳配置

如果這個版本不工作，我們還可以：
1. 嘗試其他引腳組合
2. 檢查硬體連接
3. 調整 SPI 參數
