/*
 * GDEQ0426T82 + WeMos D1 Mini 強制 GxEPD2 測試程式
 * 
 * 這個版本會強制使用 GxEPD2 驅動程式，不進行可用性檢測
 * 如果編譯成功，代表 GxEPD2 庫已正確安裝
 */

#include <GxEPD2_BW.h>
#include <gdeq/GxEPD2_426_GDEQ0426T82.h>
#include <Fonts/FreeMonoBold9pt7b.h>

// ==================== 腳位定義 ====================
#define EPD_CS    15  // D8 - CS (必須加 3.3k 下拉電阻到 GND)
#define EPD_DC     0  // D3 - DC
#define EPD_RST    5  // D1 - RST (建議加 1k 上拉電阻到 3.3V)  
#define EPD_BUSY   4  // D2 - BUSY

// ==================== 顯示器物件 ====================
// 強制使用 GxEPD2_426_GDEQ0426T82 驅動程式
GxEPD2_BW<GxEPD2_426_GDEQ0426T82, GxEPD2_426_GDEQ0426T82::HEIGHT> display(GxEPD2_426_GDEQ0426T82(EPD_CS, EPD_DC, EPD_RST, EPD_BUSY));

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("=== GDEQ0426T82 + WeMos D1 Mini 強制 GxEPD2 測試 ===");
  Serial.printf("編譯時間: %s %s\n", __DATE__, __TIME__);
  Serial.println();
  
  // 顯示系統資訊
  Serial.println("--- ESP8266 系統資訊 ---");
  Serial.printf("晶片 ID: 0x%08X\n", ESP.getChipId());
  Serial.printf("Flash 大小: %d MB\n", ESP.getFlashChipSize() / (1024 * 1024));
  Serial.printf("可用記憶體: %d bytes\n", ESP.getFreeHeap());
  Serial.printf("CPU 頻率: %d MHz\n", ESP.getCpuFreqMHz());
  Serial.println();
  
  // 腳位設定
  Serial.println("--- 設定腳位 ---");
  pinMode(EPD_BUSY, INPUT);
  pinMode(EPD_RST, OUTPUT);
  pinMode(EPD_DC, OUTPUT);
  pinMode(EPD_CS, OUTPUT);
  
  digitalWrite(EPD_RST, HIGH);
  digitalWrite(EPD_DC, HIGH);
  digitalWrite(EPD_CS, HIGH);
  
  Serial.println("腳位配置:");
  Serial.printf("  BUSY: D2 (GPIO%d) - 輸入\n", EPD_BUSY);
  Serial.printf("  RST:  D1 (GPIO%d) - 輸出\n", EPD_RST);
  Serial.printf("  DC:   D3 (GPIO%d) - 輸出\n", EPD_DC);
  Serial.printf("  CS:   D8 (GPIO%d) - 輸出\n", EPD_CS);
  Serial.println("腳位設定完成");
  
  // 顯示當前腳位狀態
  Serial.println("當前腳位狀態:");
  Serial.printf("  BUSY: %s\n", digitalRead(EPD_BUSY) ? "HIGH" : "LOW");
  Serial.printf("  RST:  %s\n", digitalRead(EPD_RST) ? "HIGH" : "LOW");
  Serial.printf("  DC:   %s\n", digitalRead(EPD_DC) ? "HIGH" : "LOW");
  Serial.printf("  CS:   %s\n", digitalRead(EPD_CS) ? "HIGH" : "LOW");
  Serial.println();
  
  // 初始化 GxEPD2 顯示器
  Serial.println("--- 初始化 GxEPD2 顯示器 ---");
  // ESP8266 不支援 try-catch，直接執行初始化
  display.init(115200, true, 2, false);
  Serial.println("✅ GxEPD2 顯示器初始化成功！");
  Serial.printf("顯示器尺寸: %d x %d 像素\n", display.width(), display.height());
  Serial.println("驅動程式: GxEPD2_426_GDEQ0426T82 (專用)");
  Serial.println();
  
  Serial.println("=== 初始化完成 ===");
  Serial.println("開始顯示測試...");
  Serial.println();
}

void loop() {
  static int testStep = 0;
  static unsigned long lastTest = 0;
  
  if (millis() - lastTest > 5000) {  // 每 5 秒執行一次測試
    lastTest = millis();
    testStep++;
    
    Serial.printf("--- 測試步驟 %d ---\n", testStep);
    Serial.printf("BUSY pin 狀態: %s\n", digitalRead(EPD_BUSY) ? "HIGH (空閒)" : "LOW (忙碌)");
    Serial.printf("可用記憶體: %d bytes\n", ESP.getFreeHeap());
    
    switch (testStep) {
      case 1:
        testClearScreen();
        break;
      case 2:
        testDrawText();
        break;
      case 3:
        testDrawShapes();
        break;
      case 4:
        testPartialUpdate();
        break;
      default:
        testStep = 0;  // 重置循環
        break;
    }
    Serial.println();
  }
}

void testClearScreen() {
  Serial.println("測試 1: 清除螢幕");
  // ESP8266 不支援 try-catch，直接執行
  display.setRotation(0);
  display.fillScreen(GxEPD_WHITE);
  display.display(false);  // 全螢幕更新
  Serial.println("✅ 白屏測試完成");
}

void testDrawText() {
  Serial.println("測試 2: 繪製文字");
  // ESP8266 不支援 try-catch，直接執行
  display.setRotation(0);
  display.fillScreen(GxEPD_WHITE);
  display.setTextColor(GxEPD_BLACK);
  display.setFont(&FreeMonoBold9pt7b);
  display.setCursor(50, 100);
  display.println("GDEQ0426T82 Test");
  display.setCursor(50, 150);
  display.println("WeMos D1 Mini");
  display.setCursor(50, 200);
  display.println("GxEPD2 Working!");
  display.display(false);
  Serial.println("✅ 文字測試完成");
}

void testDrawShapes() {
  Serial.println("測試 3: 繪製圖形");
  // ESP8266 不支援 try-catch，直接執行
  display.setRotation(0);
  display.fillScreen(GxEPD_WHITE);
  
  // 繪製矩形
  display.drawRect(50, 50, 200, 100, GxEPD_BLACK);
  display.fillRect(300, 50, 100, 100, GxEPD_BLACK);
  
  // 繪製圓形
  display.drawCircle(150, 300, 50, GxEPD_BLACK);
  display.fillCircle(350, 300, 30, GxEPD_BLACK);
  
  // 繪製線條
  display.drawLine(0, 0, display.width()-1, display.height()-1, GxEPD_BLACK);
  display.drawLine(0, display.height()-1, display.width()-1, 0, GxEPD_BLACK);
  
  display.display(false);
  Serial.println("✅ 圖形測試完成");
}

void testPartialUpdate() {
  Serial.println("測試 4: 部分更新");
  // ESP8266 不支援 try-catch，直接執行
  // 設定部分更新區域
  display.setPartialWindow(50, 50, 300, 100);
  display.fillScreen(GxEPD_WHITE);
  display.setTextColor(GxEPD_BLACK);
  display.setFont(&FreeMonoBold9pt7b);
  display.setCursor(60, 90);
  display.printf("Time: %lu ms", millis());
  display.display(true);  // 部分更新，使用 display(true) 代替 displayWindow
  Serial.println("✅ 部分更新測試完成");
}
