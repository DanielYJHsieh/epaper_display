/*
 * GDEQ0426T82 E-Paper Display for WeMos D1 Mini (ESP8266)
 * 
 * 硬體連接:
 * - GDEQ0426T82 4.26" 電子紙顯示屏
 * - WeMos D1 Mini ESP8266 開發板
 * 
 * 腳位連接 (WeMos D1 Mini → GDEQ0426T82):
 * BUSY -> D2 (GPIO4)   - 忙碌偵測腳位
 * RST  -> D1 (GPIO5)   - 重置腳位
 * DC   -> D3 (GPIO0)   - 資料/命令選擇腳位
 * CS   -> D8 (GPIO15)  - SPI 片選腳位 (需要 3.3k 下拉電阻到 GND)
 * CLK  -> D5 (GPIO14)  - SPI 時脈腳位 (SCK)
 * DIN  -> D7 (GPIO13)  - SPI 資料輸入腳位 (MOSI)
 * GND  -> GND          - 接地
 * 3.3V -> 3.3V         - 電源 (3.3V)
 * 
 * 注意事項:
 * 1. CS 腳位 (GPIO15) 需要 3.3k 下拉電阻連接到 GND，避免開機問題
 * 2. RST 腳位可能需要 1k 上拉電阻連接到 3.3V
 * 3. 所有腳位都是 3.3V 邏輯準位
 * 
 * 所需函式庫:
 * - GxEPD2 by Jean-Marc Zingg
 * - Adafruit GFX Library
 * 
 * 作者: Arduino ESP8266 專案
 * 日期: 2025/1/9
 */

// 包含必要的 GxEPD2 庫
#include <GxEPD2_BW.h>
#include <Fonts/FreeMonoBold9pt7b.h>

// WeMos D1 Mini 腳位定義
#define EPD_CS    15  // D8 - CS (需要 3.3k 下拉電阻)
#define EPD_DC     0  // D3 - DC
#define EPD_RST    5  // D1 - RST (可能需要 1k 上拉電阻)
#define EPD_BUSY   4  // D2 - BUSY

// SPI 腳位 (WeMos D1 Mini 硬體 SPI)
// SCK  -> D5 (GPIO14) - 自動設定
// MOSI -> D7 (GPIO13) - 自動設定

// GDEQ0426T82 顯示器配置 (4.26" 480x800 黑白)
// 由於 GDEQ0426T82 可能沒有專用驅動程式，我們使用相容的驅動程式
// 根據 GxEPD2 庫文檔，對於 4.26" 顯示器可以嘗試以下驅動程式:

// 選項 1: 嘗試 GxEPD2_420 (4.20" 相容)
#ifdef GxEPD2_420
  GxEPD2_BW<GxEPD2_420, GxEPD2_420::HEIGHT> display(GxEPD2_420(EPD_CS, EPD_DC, EPD_RST, EPD_BUSY));
  #define DISPLAY_DRIVER "GxEPD2_420"
// 選項 2: 嘗試 GxEPD2_420_M01 
#elif defined(GxEPD2_420_M01)
  GxEPD2_BW<GxEPD2_420_M01, GxEPD2_420_M01::HEIGHT> display(GxEPD2_420_M01(EPD_CS, EPD_DC, EPD_RST, EPD_BUSY));
  #define DISPLAY_DRIVER "GxEPD2_420_M01"
// 選項 3: 使用基本的 2.9" 驅動程式作為測試
#else
  // 如果沒有找到合適的驅動程式，使用較小的顯示器進行測試
  // 注意：這只是為了驗證連線，實際顯示效果可能不正確
  #include <GxEPD2_290.h>
  GxEPD2_BW<GxEPD2_290, GxEPD2_290::HEIGHT> display(GxEPD2_290(EPD_CS, EPD_DC, EPD_RST, EPD_BUSY));
  #define DISPLAY_DRIVER "GxEPD2_290 (測試用)"
#endif

// 顯示器尺寸 (實際 GDEQ0426T82 規格)
#define ACTUAL_WIDTH  480
#define ACTUAL_HEIGHT 800

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("=== GDEQ0426T82 WeMos D1 Mini 範例 ===");
  Serial.println("初始化電子紙顯示器...");
  
  // 顯示腳位配置資訊
  Serial.println("\n腳位配置:");
  Serial.printf("BUSY: D2 (GPIO%d)\n", EPD_BUSY);
  Serial.printf("RST:  D1 (GPIO%d)\n", EPD_RST);
  Serial.printf("DC:   D3 (GPIO%d)\n", EPD_DC);
  Serial.printf("CS:   D8 (GPIO%d) - 需要 3.3k 下拉電阻\n", EPD_CS);
  Serial.printf("CLK:  D5 (GPIO%d) - SPI SCK\n", 14);
  Serial.printf("DIN:  D7 (GPIO%d) - SPI MOSI\n", 13);
  
  // 初始化顯示器
  display.init(115200); // 使用 2ms 重置脈衝
  
  Serial.println("顯示器初始化完成!");
  Serial.printf("顯示器尺寸: %d x %d\n", display.width(), display.height());
  
  // 執行顯示測試
  showWelcomeScreen();
  delay(3000);
  
  showSystemInfo();
  delay(3000);
  
  showPatternTest();
  delay(3000);
  
  Serial.println("所有測試完成!");
  
  // 進入深度睡眠模式以節省電力
  display.powerOff();
  Serial.println("顯示器已進入睡眠模式");
}

void loop() {
  // 主循環保持空白，所有顯示在 setup() 中完成
  // 可以在這裡添加週期性更新的程式碼
  delay(10000);
}

/**
 * 顯示歡迎畫面
 */
void showWelcomeScreen() {
  Serial.println("顯示歡迎畫面...");
  
  display.setRotation(0); // 設定旋轉角度
  display.setFont(&FreeMonoBold9pt7b);
  display.setTextColor(GxEPD_BLACK);
  
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
    
    // 標題
    display.setCursor(50, 100);
    display.print("GDEQ0426T82");
    
    display.setCursor(80, 130);
    display.print("E-Paper");
    
    display.setCursor(80, 160);
    display.print("Display");
    
    // 硬體資訊
    display.setCursor(20, 220);
    display.print("WeMos D1 Mini");
    
    display.setCursor(20, 250);
    display.print("ESP8266");
    
    // 尺寸資訊
    display.setCursor(20, 320);
    display.print("480 x 800 pixels");
    
    display.setCursor(20, 350);
    display.print("4.26 inch");
    
    // 狀態
    display.setCursor(20, 420);
    display.print("Status: Ready");
    
    // 繪製邊框
    display.drawRect(10, 10, display.width()-20, display.height()-20, GxEPD_BLACK);
    
  } while (display.nextPage());
  
  Serial.println("歡迎畫面顯示完成");
}

/**
 * 顯示系統資訊
 */
void showSystemInfo() {
  Serial.println("顯示系統資訊...");
  
  display.setRotation(0);
  display.setFont(&FreeMonoBold9pt7b);
  display.setTextColor(GxEPD_BLACK);
  
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
    
    // 標題
    display.setCursor(20, 40);
    display.print("System Information");
    
    // ESP8266 資訊
    display.setCursor(20, 100);
    display.print("Chip ID: ");
    display.print(ESP.getChipId(), HEX);
    
    display.setCursor(20, 130);
    display.print("Flash Size: ");
    display.print(ESP.getFlashChipSize() / 1024 / 1024);
    display.print("MB");
    
    display.setCursor(20, 160);
    display.print("Free Heap: ");
    display.print(ESP.getFreeHeap());
    display.print("B");
    
    display.setCursor(20, 190);
    display.print("CPU Freq: ");
    display.print(ESP.getCpuFreqMHz());
    display.print("MHz");
    
    // 腳位配置
    display.setCursor(20, 250);
    display.print("Pin Configuration:");
    
    display.setCursor(20, 280);
    display.print("BUSY: D2 (GPIO4)");
    
    display.setCursor(20, 310);
    display.print("RST:  D1 (GPIO5)");
    
    display.setCursor(20, 340);
    display.print("DC:   D3 (GPIO0)");
    
    display.setCursor(20, 370);
    display.print("CS:   D8 (GPIO15)");
    
    display.setCursor(20, 400);
    display.print("CLK:  D5 (GPIO14)");
    
    display.setCursor(20, 430);
    display.print("DIN:  D7 (GPIO13)");
    
    // 版本資訊
    display.setCursor(20, 490);
    display.print("GxEPD2 Library");
    
    display.setCursor(20, 520);
    display.print("Arduino ESP8266");
    
    // 繪製分隔線
    display.drawLine(20, 220, display.width()-20, 220, GxEPD_BLACK);
    
  } while (display.nextPage());
  
  Serial.println("系統資訊顯示完成");
}

/**
 * 顯示圖案測試
 */
void showPatternTest() {
  Serial.println("顯示圖案測試...");
  
  display.setRotation(0);
  display.setFont(&FreeMonoBold9pt7b);
  display.setTextColor(GxEPD_BLACK);
  
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
    
    // 標題
    display.setCursor(150, 40);
    display.print("Pattern Test");
    
    // 繪製各種圖案
    
    // 矩形
    display.drawRect(50, 80, 100, 60, GxEPD_BLACK);
    display.fillRect(55, 85, 90, 50, GxEPD_BLACK);
    display.setCursor(170, 120);
    display.print("Rectangle");
    
    // 圓形
    display.drawCircle(100, 200, 30, GxEPD_BLACK);
    display.fillCircle(100, 200, 20, GxEPD_BLACK);
    display.setCursor(170, 210);
    display.print("Circle");
    
    // 線條
    for (int i = 0; i < 5; i++) {
      display.drawLine(50 + i*20, 280, 150 - i*20, 320, GxEPD_BLACK);
    }
    display.setCursor(170, 310);
    display.print("Lines");
    
    // 三角形
    display.drawTriangle(75, 380, 50, 420, 100, 420, GxEPD_BLACK);
    display.fillTriangle(80, 385, 60, 415, 95, 415, GxEPD_BLACK);
    display.setCursor(170, 410);
    display.print("Triangle");
    
    // 文字測試
    display.setCursor(50, 500);
    display.print("Text rendering test:");
    display.setCursor(50, 530);
    display.print("ABCDEFGHIJKLMNOP");
    display.setCursor(50, 560);
    display.print("0123456789");
    display.setCursor(50, 590);
    display.print("Special: !@#$%^&*()");
    
    // 網格
    for (int x = 300; x < 450; x += 20) {
      display.drawLine(x, 80, x, 200, GxEPD_BLACK);
    }
    for (int y = 80; y < 200; y += 20) {
      display.drawLine(300, y, 450, y, GxEPD_BLACK);
    }
    display.setCursor(320, 230);
    display.print("Grid");
    
    // 像素圖案
    for (int x = 0; x < 50; x++) {
      for (int y = 0; y < 50; y++) {
        if ((x + y) % 4 == 0) {
          display.drawPixel(320 + x, 280 + y, GxEPD_BLACK);
        }
      }
    }
    display.setCursor(320, 350);
    display.print("Pixels");
    
  } while (display.nextPage());
  
  Serial.println("圖案測試顯示完成");
}

/**
 * 顯示部分更新測試 (如果支援)
 */
void showPartialUpdateTest() {
  if (!display.epd2.hasPartialUpdate) {
    Serial.println("此顯示器不支援部分更新");
    return;
  }
  
  Serial.println("執行部分更新測試...");
  
  // 部分更新測試程式碼
  display.setPartialWindow(100, 100, 200, 100);
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
    display.setCursor(110, 150);
    display.print("Partial Update");
  } while (display.nextPage());
  
  Serial.println("部分更新測試完成");
}

/**
 * 清除螢幕
 */
void clearScreen() {
  Serial.println("清除螢幕...");
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
  } while (display.nextPage());
  Serial.println("螢幕已清除");
}

/**
 * 顯示錯誤訊息
 */
void showError(const char* errorMsg) {
  Serial.printf("錯誤: %s\n", errorMsg);
  
  display.setRotation(0);
  display.setFont(&FreeMonoBold9pt7b);
  display.setTextColor(GxEPD_BLACK);
  
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
    
    display.setCursor(50, 100);
    display.print("ERROR:");
    
    display.setCursor(50, 150);
    display.print(errorMsg);
    
    // 錯誤邊框
    display.drawRect(30, 50, display.width()-60, 150, GxEPD_BLACK);
    display.drawRect(32, 52, display.width()-64, 146, GxEPD_BLACK);
    
  } while (display.nextPage());
}
