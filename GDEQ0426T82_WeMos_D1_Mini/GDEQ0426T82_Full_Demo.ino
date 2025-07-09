/*
 * GDEQ0426T82 完整 GxEPD2 範例
 * WeMos D1 Mini + GDEQ0426T82 4.26" E-Paper Display
 * 
 * 前提條件:
 * 1. 已安裝 GxEPD2 函式庫
 * 2. 已安裝 Adafruit GFX 函式庫
 * 3. 硬體連接正確
 * 4. 電阻已正確安裝
 * 
 * 這個範例假設 GxEPD2 庫已正確安裝並且可以使用 GxEPD2_420 驅動程式
 * 如果 GDEQ0426T82 有專用驅動程式，請修改相關的 #include 和 display 定義
 */

#include <GxEPD2_BW.h>
#include <GxEPD2_420.h>  // 使用 4.2" 驅動程式作為 4.26" 的相容選項
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeMonoBold18pt7b.h>

// ==================== 腳位定義 ====================
#define EPD_CS    15  // D8 - CS (必須加 3.3k 下拉電阻到 GND)
#define EPD_DC     0  // D3 - DC
#define EPD_RST    5  // D1 - RST (建議加 1k 上拉電阻到 3.3V)  
#define EPD_BUSY   4  // D2 - BUSY
// SPI 腳位自動配置: SCK=D5(GPIO14), MOSI=D7(GPIO13)

// ==================== 顯示器物件 ====================
// 使用 GxEPD2_420 作為 GDEQ0426T82 的相容驅動程式
GxEPD2_BW<GxEPD2_420, GxEPD2_420::HEIGHT> display(GxEPD2_420(EPD_CS, EPD_DC, EPD_RST, EPD_BUSY));

// ==================== 全域變數 ====================
int demoStep = 0;
unsigned long lastDemoUpdate = 0;
const unsigned long DEMO_INTERVAL = 5000; // 5秒切換一次演示

// ==================== 初始化 ====================
void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("=== GDEQ0426T82 + WeMos D1 Mini 完整範例 ===");
  
  // 顯示硬體資訊
  printHardwareInfo();
  
  // 初始化顯示器
  Serial.println("初始化顯示器...");
  display.init(115200, true, 2, false); // 使用 2ms 重置脈衝
  
  Serial.printf("顯示器尺寸: %d x %d 像素\n", display.width(), display.height());
  Serial.printf("顏色深度: %s\n", "黑白 (1-bit)");
  Serial.printf("部分更新支援: %s\n", display.epd2.hasPartialUpdate ? "是" : "否");
  
  // 設定基本參數
  display.setRotation(0);
  display.setTextColor(GxEPD_BLACK);
  
  Serial.println("顯示器初始化完成!");
  
  // 執行啟動序列
  showStartupSequence();
  
  Serial.println("進入演示模式...");
}

// ==================== 主循環 ====================
void loop() {
  unsigned long currentTime = millis();
  
  // 自動演示模式
  if (currentTime - lastDemoUpdate > DEMO_INTERVAL) {
    runDemoStep(demoStep);
    demoStep = (demoStep + 1) % 6; // 6個演示步驟
    lastDemoUpdate = currentTime;
  }
  
  // 監控系統狀態
  static unsigned long lastStatusUpdate = 0;
  if (currentTime - lastStatusUpdate > 30000) { // 每30秒
    printSystemStatus();
    lastStatusUpdate = currentTime;
  }
  
  delay(100);
}

// ==================== 硬體資訊 ====================
void printHardwareInfo() {
  Serial.println("\n--- 硬體配置 ---");
  Serial.printf("ESP8266 晶片 ID: 0x%08X\n", ESP.getChipId());
  Serial.printf("Flash 大小: %u MB\n", ESP.getFlashChipSize() / (1024*1024));
  Serial.printf("可用記憶體: %u bytes\n", ESP.getFreeHeap());
  Serial.printf("CPU 頻率: %u MHz\n", ESP.getCpuFreqMHz());
  
  Serial.println("\n腳位配置:");
  Serial.printf("  BUSY: D2 (GPIO%d)\n", EPD_BUSY);
  Serial.printf("  RST:  D1 (GPIO%d) - 需要 1k 上拉電阻\n", EPD_RST);
  Serial.printf("  DC:   D3 (GPIO%d)\n", EPD_DC);
  Serial.printf("  CS:   D8 (GPIO%d) - 需要 3.3k 下拉電阻\n", EPD_CS);
  Serial.printf("  SCK:  D5 (GPIO%d)\n", 14);
  Serial.printf("  MOSI: D7 (GPIO%d)\n", 13);
}

// ==================== 啟動序列 ====================
void showStartupSequence() {
  Serial.println("顯示啟動畫面...");
  
  // 清除螢幕
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
  } while (display.nextPage());
  
  delay(1000);
  
  // 顯示歡迎訊息
  display.setFont(&FreeMonoBold18pt7b);
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
    
    // 標題
    display.setCursor(50, 100);
    display.print("GDEQ0426T82");
    
    display.setCursor(120, 140);
    display.print("E-Paper");
    
    // 副標題
    display.setFont(&FreeMonoBold12pt7b);
    display.setCursor(80, 200);
    display.print("WeMos D1 Mini");
    
    display.setCursor(100, 230);
    display.print("ESP8266");
    
    // 狀態
    display.setFont(&FreeMonoBold9pt7b);
    display.setCursor(50, 300);
    display.print("狀態: 初始化完成");
    
    display.setCursor(50, 330);
    display.printf("尺寸: %dx%d 像素", display.width(), display.height());
    
    display.setCursor(50, 360);
    display.printf("記憶體: %u bytes", ESP.getFreeHeap());
    
    // 繪製邊框
    display.drawRect(20, 20, display.width()-40, display.height()-40, GxEPD_BLACK);
    
  } while (display.nextPage());
  
  Serial.println("啟動畫面顯示完成");
  delay(3000);
}

// ==================== 演示步驟 ====================
void runDemoStep(int step) {
  Serial.printf("執行演示步驟 %d...\n", step + 1);
  
  switch (step) {
    case 0:
      showSystemInfo();
      break;
    case 1:
      showGraphicsDemo();
      break;
    case 2:
      showTextDemo();
      break;
    case 3:
      showPatternDemo();
      break;
    case 4:
      showAnimationDemo();
      break;
    case 5:
      showStatusDisplay();
      break;
  }
  
  Serial.printf("演示步驟 %d 完成\n", step + 1);
}

// ==================== 系統資訊顯示 ====================
void showSystemInfo() {
  display.setFont(&FreeMonoBold9pt7b);
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
    
    // 標題
    display.setFont(&FreeMonoBold12pt7b);
    display.setCursor(150, 40);
    display.print("系統資訊");
    
    // 系統資訊
    display.setFont(&FreeMonoBold9pt7b);
    int y = 80;
    
    display.setCursor(20, y); y += 25;
    display.printf("晶片 ID: 0x%08X", ESP.getChipId());
    
    display.setCursor(20, y); y += 25;
    display.printf("Flash: %u MB", ESP.getFlashChipSize() / (1024*1024));
    
    display.setCursor(20, y); y += 25;
    display.printf("可用記憶體: %u bytes", ESP.getFreeHeap());
    
    display.setCursor(20, y); y += 25;
    display.printf("CPU: %u MHz", ESP.getCpuFreqMHz());
    
    display.setCursor(20, y); y += 25;
    display.printf("運行時間: %lu 秒", millis() / 1000);
    
    display.setCursor(20, y); y += 25;
    display.printf("SDK: %s", ESP.getSdkVersion());
    
    // 顯示器資訊
    y += 20;
    display.setCursor(20, y); y += 25;
    display.print("=== 顯示器資訊 ===");
    
    display.setCursor(20, y); y += 25;
    display.printf("尺寸: %dx%d", display.width(), display.height());
    
    display.setCursor(20, y); y += 25;
    display.printf("旋轉: %d 度", display.getRotation() * 90);
    
    display.setCursor(20, y); y += 25;
    display.printf("部分更新: %s", display.epd2.hasPartialUpdate ? "支援" : "不支援");
    
    // 邊框
    display.drawRect(10, 10, display.width()-20, display.height()-20, GxEPD_BLACK);
    
  } while (display.nextPage());
}

// ==================== 圖形演示 ====================
void showGraphicsDemo() {
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
    
    // 標題
    display.setFont(&FreeMonoBold12pt7b);
    display.setCursor(150, 40);
    display.print("圖形演示");
    
    // 矩形
    display.drawRect(50, 70, 100, 60, GxEPD_BLACK);
    display.fillRect(55, 75, 90, 50, GxEPD_BLACK);
    
    // 圓形
    display.drawCircle(100, 200, 40, GxEPD_BLACK);
    display.fillCircle(100, 200, 30, GxEPD_BLACK);
    
    // 三角形
    display.drawTriangle(200, 70, 170, 130, 230, 130, GxEPD_BLACK);
    display.fillTriangle(205, 80, 180, 120, 220, 120, GxEPD_BLACK);
    
    // 線條
    for (int i = 0; i < 8; i++) {
      display.drawLine(250 + i*15, 70, 320 - i*8, 130, GxEPD_BLACK);
    }
    
    // 像素圖案
    for (int x = 0; x < 60; x++) {
      for (int y = 0; y < 60; y++) {
        if ((x + y) % 4 == 0) {
          display.drawPixel(250 + x, 180 + y, GxEPD_BLACK);
        }
      }
    }
    
    // 弧線
    for (int i = 0; i < 180; i += 20) {
      int x = 100 + 80 * cos(i * PI / 180);
      int y = 350 + 80 * sin(i * PI / 180);
      display.drawPixel(x, y, GxEPD_BLACK);
    }
    
    // 標籤
    display.setFont(&FreeMonoBold9pt7b);
    display.setCursor(60, 150);
    display.print("矩形");
    
    display.setCursor(60, 250);
    display.print("圓形");
    
    display.setCursor(180, 150);
    display.print("三角形");
    
    display.setCursor(280, 150);
    display.print("線條");
    
    display.setCursor(260, 260);
    display.print("像素");
    
    display.setCursor(60, 380);
    display.print("弧線");
    
  } while (display.nextPage());
}

// ==================== 文字演示 ====================
void showTextDemo() {
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
    
    // 標題
    display.setFont(&FreeMonoBold18pt7b);
    display.setCursor(120, 50);
    display.print("文字演示");
    
    // 不同大小的字體
    display.setFont(&FreeMonoBold18pt7b);
    display.setCursor(50, 100);
    display.print("大字體 18pt");
    
    display.setFont(&FreeMonoBold12pt7b);
    display.setCursor(50, 140);
    display.print("中字體 12pt");
    
    display.setFont(&FreeMonoBold9pt7b);
    display.setCursor(50, 170);
    display.print("小字體 9pt");
    
    // 數字測試
    display.setCursor(50, 220);
    display.print("數字: 0123456789");
    
    // 特殊字符
    display.setCursor(50, 250);
    display.print("符號: !@#$%^&*()");
    
    // 英文字母
    display.setCursor(50, 280);
    display.print("英文: ABCDEFGHIJKLM");
    display.setCursor(50, 310);
    display.print("     abcdefghijklm");
    
    // 動態內容
    display.setCursor(50, 360);
    display.printf("時間: %lu 秒", millis() / 1000);
    
    display.setCursor(50, 390);
    display.printf("記憶體: %u bytes", ESP.getFreeHeap());
    
    // 文字方框
    display.drawRect(40, 80, display.width()-80, 340, GxEPD_BLACK);
    
  } while (display.nextPage());
}

// ==================== 圖案演示 ====================
void showPatternDemo() {
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
    
    // 標題
    display.setFont(&FreeMonoBold12pt7b);
    display.setCursor(150, 40);
    display.print("圖案演示");
    
    // 棋盤圖案
    int squareSize = 20;
    for (int x = 0; x < 8; x++) {
      for (int y = 0; y < 8; y++) {
        if ((x + y) % 2 == 0) {
          display.fillRect(50 + x*squareSize, 70 + y*squareSize, 
                          squareSize, squareSize, GxEPD_BLACK);
        }
      }
    }
    
    display.setFont(&FreeMonoBold9pt7b);
    display.setCursor(70, 260);
    display.print("棋盤");
    
    // 同心圓
    for (int r = 10; r <= 50; r += 10) {
      display.drawCircle(350, 150, r, GxEPD_BLACK);
    }
    
    display.setCursor(320, 220);
    display.print("同心圓");
    
    // 格子圖案
    for (int x = 50; x <= 200; x += 15) {
      display.drawLine(x, 300, x, 400, GxEPD_BLACK);
    }
    for (int y = 300; y <= 400; y += 15) {
      display.drawLine(50, y, 200, y, GxEPD_BLACK);
    }
    
    display.setCursor(80, 430);
    display.print("格子");
    
    // 放射線
    int centerX = 350, centerY = 350;
    for (int angle = 0; angle < 360; angle += 30) {
      int x = centerX + 50 * cos(angle * PI / 180);
      int y = centerY + 50 * sin(angle * PI / 180);
      display.drawLine(centerX, centerY, x, y, GxEPD_BLACK);
    }
    
    display.setCursor(320, 430);
    display.print("放射線");
    
  } while (display.nextPage());
}

// ==================== 動畫演示 ====================
void showAnimationDemo() {
  display.setFont(&FreeMonoBold9pt7b);
  
  // 簡單的動畫效果 - 移動的點
  for (int frame = 0; frame < 5; frame++) {
    display.firstPage();
    do {
      display.fillScreen(GxEPD_WHITE);
      
      // 標題
      display.setFont(&FreeMonoBold12pt7b);
      display.setCursor(120, 40);
      display.print("動畫演示");
      
      // 移動的圓點
      int x = 50 + frame * 80;
      display.fillCircle(x, 150, 20, GxEPD_BLACK);
      
      display.setFont(&FreeMonoBold9pt7b);
      display.setCursor(150, 200);
      display.printf("幀 %d/5", frame + 1);
      
      // 進度條
      display.drawRect(50, 250, 300, 20, GxEPD_BLACK);
      display.fillRect(52, 252, (frame + 1) * 60 - 4, 16, GxEPD_BLACK);
      
    } while (display.nextPage());
    
    delay(1000); // 每幀延遲 1 秒
  }
}

// ==================== 狀態顯示 ====================
void showStatusDisplay() {
  display.setFont(&FreeMonoBold9pt7b);
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
    
    // 標題
    display.setFont(&FreeMonoBold12pt7b);
    display.setCursor(150, 40);
    display.print("即時狀態");
    
    // 現在時間
    display.setFont(&FreeMonoBold9pt7b);
    unsigned long currentTime = millis();
    int hours = (currentTime / 3600000) % 24;
    int minutes = (currentTime / 60000) % 60;
    int seconds = (currentTime / 1000) % 60;
    
    display.setCursor(50, 80);
    display.printf("運行時間: %02d:%02d:%02d", hours, minutes, seconds);
    
    // 記憶體狀態
    display.setCursor(50, 110);
    display.printf("可用記憶體: %u bytes", ESP.getFreeHeap());
    
    display.setCursor(50, 140);
    display.printf("記憶體碎片: %u%%", ESP.getHeapFragmentation());
    
    // 系統狀態
    display.setCursor(50, 170);
    display.printf("CPU 頻率: %u MHz", ESP.getCpuFreqMHz());
    
    display.setCursor(50, 200);
    display.printf("顯示器忙碌: %s", digitalRead(EPD_BUSY) ? "是" : "否");
    
    // WiFi 狀態 (如果啟用)
    display.setCursor(50, 230);
    display.printf("WiFi 狀態: 未啟用");
    
    // 視覺化記憶體使用
    int memUsagePercent = 100 - (ESP.getFreeHeap() * 100 / 81920); // 假設總記憶體 80KB
    display.setCursor(50, 280);
    display.print("記憶體使用率:");
    
    display.drawRect(50, 290, 200, 20, GxEPD_BLACK);
    display.fillRect(52, 292, memUsagePercent * 2 - 4, 16, GxEPD_BLACK);
    
    display.setCursor(260, 305);
    display.printf("%d%%", memUsagePercent);
    
    // 更新時間戳
    display.setCursor(50, 350);
    display.printf("最後更新: %lu", millis() / 1000);
    
  } while (display.nextPage());
}

// ==================== 系統狀態監控 ====================
void printSystemStatus() {
  Serial.println("\n--- 系統狀態 ---");
  Serial.printf("運行時間: %lu 秒\n", millis() / 1000);
  Serial.printf("可用記憶體: %u bytes\n", ESP.getFreeHeap());
  Serial.printf("記憶體碎片: %u%%\n", ESP.getHeapFragmentation());
  Serial.printf("最大可用塊: %u bytes\n", ESP.getMaxFreeBlockSize());
  Serial.printf("顯示器忙碌: %s\n", digitalRead(EPD_BUSY) ? "是" : "否");
  Serial.printf("當前演示步驟: %d\n", demoStep + 1);
  Serial.println("---------------");
}
