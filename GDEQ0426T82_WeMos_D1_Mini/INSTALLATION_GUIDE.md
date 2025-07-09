# GDEQ0426T82 安裝和配置指南

## 第一步：硬體準備

### 1.1 所需零件
- **WeMos D1 Mini** ESP8266 開發板
- **GDEQ0426T82** 4.26吋電子紙顯示器 (480x800)
- **3.3k 電阻** 1個 (下拉電阻)
- **1k 電阻** 1個 (上拉電阻，建議)
- **杜邦線** 或面包板連接線
- **麵包板** (可選，方便測試)

### 1.2 硬體連接

```
WeMos D1 Mini    電阻    GDEQ0426T82    說明
─────────────────────────────────────────────────
3.3V             ────    VCC             電源正極
GND              ────    GND             電源負極
D5 (GPIO14)      ────    CLK             SPI 時脈
D7 (GPIO13)      ────    DIN             SPI 資料
D8 (GPIO15)      3.3k    CS              片選 + 下拉電阻到 GND
                 ↓GND
D3 (GPIO0)       ────    DC              資料/命令選擇
D1 (GPIO5)       1k      RST             重置 + 上拉電阻到 3.3V
                 ↑3.3V
D2 (GPIO4)       ────    BUSY            忙碌偵測
```

### 1.3 重要注意事項

⚠️ **電阻必須正確安裝，否則可能導致：**
- CS 沒有下拉電阻：ESP8266 無法正常開機
- RST 沒有上拉電阻：顯示器復位不穩定

⚠️ **電壓警告：**
- 只能使用 3.3V 電源，不可接 5V
- 所有信號線都是 3.3V 邏輯準位

## 第二步：軟體環境設定

### 2.1 Arduino IDE 設定

1. **下載並安裝 Arduino IDE**
   - 從 [arduino.cc](https://www.arduino.cc/en/software) 下載最新版本
   - 建議使用 Arduino IDE 2.0 或更新版本

2. **安裝 ESP8266 開發板支援**
   ```
   檔案 → 偏好設定 → 額外的開發板管理員網址
   
   添加：http://arduino.esp8266.com/stable/package_esp8266com_index.json
   
   工具 → 開發板 → 開發板管理員
   搜尋 "ESP8266" → 安裝 "ESP8266 by ESP8266 Community"
   ```

3. **選擇正確的開發板**
   ```
   工具 → 開發板 → ESP8266 → LOLIN(WeMos) D1 mini
   ```

4. **開發板參數設定**
   ```
   CPU Frequency: 80 MHz
   Flash Size: 4MB (FS:2MB OTA:~1019KB)  
   Upload Speed: 921600
   Port: 選擇對應的 COM 埠
   ```

### 2.2 必要函式庫安裝

#### 方法 1：使用 Arduino IDE 函式庫管理員

1. **安裝 GxEPD2**
   ```
   工具 → 管理函式庫 → 搜尋 "GxEPD2"
   安裝 "GxEPD2 by Jean-Marc Zingg"
   ```

2. **安裝 Adafruit GFX**
   ```
   工具 → 管理函式庫 → 搜尋 "Adafruit GFX"
   安裝 "Adafruit GFX Library"
   ```

#### 方法 2：手動安裝

1. **下載 GxEPD2**
   - 從 [GitHub](https://github.com/ZinggJM/GxEPD2) 下載
   - 解壓到 Arduino/libraries/ 目錄

2. **下載 Adafruit GFX**
   - 從 [GitHub](https://github.com/adafruit/Adafruit-GFX-Library) 下載
   - 解壓到 Arduino/libraries/ 目錄

### 2.3 驅動程式相容性

GDEQ0426T82 可能沒有專用的 GxEPD2 驅動程式。按優先順序嘗試：

1. **GxEPD2_420** - 4.2吋驅動程式 (推薦)
2. **GxEPD2_420_M01** - 4.2吋變體
3. **GxEPD2_426** - 如果有 4.26吋專用驅動

## 第三步：測試和驗證

### 3.1 基本連接測試

1. **上傳基本測試程式**
   - 使用 `GDEQ0426T82_Basic_Test.ino`
   - 這個程式不依賴特定的 GxEPD2 驅動程式

2. **檢查序列埠輸出**
   ```
   工具 → 序列埠監控視窗
   鮑率設定為 115200
   ```

3. **預期輸出**
   ```
   === GDEQ0426T82 + WeMos D1 Mini 測試程式 ===
   ESP8266 晶片 ID: 0x12345678
   腳位設定完成
   SPI 初始化完成
   顯示器初始化完成
   ```

### 3.2 驅動程式測試

1. **上傳完整演示程式**
   - 使用 `GDEQ0426T82_Full_Demo.ino`
   - 需要正確安裝 GxEPD2 函式庫

2. **檢查顯示效果**
   - 顯示器應該顯示歡迎畫面
   - 然後循環顯示不同的演示內容

### 3.3 常見問題診斷

#### 問題 1：編譯錯誤
```
錯誤：找不到 GxEPD2_BW.h
解決：確認 GxEPD2 函式庫已正確安裝
```

#### 問題 2：上傳失敗
```
錯誤：無法連接到開發板
解決：
1. 檢查 USB 連接
2. 確認 COM 埠選擇正確
3. 按住 FLASH 按鈕再上傳
```

#### 問題 3：ESP8266 無法開機
```
現象：序列埠沒有輸出或重複重啟
原因：CS 腳位沒有下拉電阻
解決：添加 3.3k 電阻從 GPIO15 到 GND
```

#### 問題 4：顯示器沒有反應
```
檢查清單：
☐ 電源連接 (3.3V 和 GND)
☐ SPI 連接 (SCK, MOSI, CS)
☐ 控制腳位 (DC, RST, BUSY)
☐ 電阻安裝 (CS 下拉, RST 上拉)
```

#### 問題 5：顯示內容錯亂
```
可能原因：
1. 驅動程式不相容
2. 顯示器尺寸設定錯誤
3. 顯示器型號識別錯誤

解決方法：
1. 嘗試不同的 GxEPD2 驅動程式
2. 調整顯示器參數設定
3. 查看 GxEPD2 文檔尋找相容驅動程式
```

## 第四步：進階設定

### 4.1 效能最佳化

1. **記憶體管理**
   ```cpp
   // ESP8266 記憶體有限，注意使用量
   Serial.printf("可用記憶體: %u bytes\n", ESP.getFreeHeap());
   
   // 大型圖像使用分頁顯示
   if (display.pages() > 1) {
     // 使用分頁模式
   }
   ```

2. **更新策略**
   ```cpp
   // 全螢幕更新（慢但完整）
   display.display(false);
   
   // 部分更新（快但可能有殘影）
   if (display.epd2.hasPartialUpdate) {
     display.displayWindow(x, y, w, h);
   }
   ```

### 4.2 電源管理

1. **顯示器睡眠**
   ```cpp
   // 顯示完成後進入睡眠模式
   display.powerOff();
   ```

2. **ESP8266 深度睡眠**
   ```cpp
   // 進入深度睡眠 (需要連接 D0 到 RST)
   ESP.deepSleep(30e6); // 30 秒後喚醒
   ```

### 4.3 WiFi 整合

1. **基本 WiFi 連接**
   ```cpp
   #include <ESP8266WiFi.h>
   
   WiFi.begin("SSID", "PASSWORD");
   while (WiFi.status() != WL_CONNECTED) {
     delay(500);
   }
   ```

2. **網路資料顯示**
   - 天氣資料
   - 新聞摘要
   - IoT 感測器資料

## 第五步：故障排除工具

### 5.1 硬體測試

1. **萬用表檢查**
   - 電源電壓：3.3V ± 0.1V
   - 連線連續性
   - 電阻值確認

2. **示波器檢查**（進階）
   - SPI 信號波形
   - 時脈頻率
   - 信號準位

### 5.2 軟體除錯

1. **序列埠監控**
   ```cpp
   Serial.begin(115200);
   Serial.println("除錯訊息");
   ```

2. **腳位狀態檢查**
   ```cpp
   Serial.printf("BUSY: %s\n", digitalRead(EPD_BUSY) ? "HIGH" : "LOW");
   ```

3. **記憶體監控**
   ```cpp
   Serial.printf("可用記憶體: %u\n", ESP.getFreeHeap());
   ```

## 參考資源

- [GxEPD2 GitHub](https://github.com/ZinggJM/GxEPD2)
- [WeMos D1 Mini 文檔](https://lastminuteengineers.com/wemos-d1-mini-pinout-reference/)
- [ESP8266 Arduino Core](https://github.com/esp8266/Arduino)
- [Adafruit GFX 教學](https://learn.adafruit.com/adafruit-gfx-graphics-library)

## 技術支援

如果遇到問題，請檢查：
1. 硬體連接是否正確
2. 函式庫版本是否相容
3. 開發板設定是否正確
4. 序列埠輸出的錯誤訊息

建議按照順序逐步測試，先確保基本連接正常，再測試進階功能。
