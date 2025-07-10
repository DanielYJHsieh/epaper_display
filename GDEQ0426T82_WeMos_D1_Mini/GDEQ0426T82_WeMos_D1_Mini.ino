/*
 * GDEQ0426T82 + WeMos D1 Mini 主程式
 * 
 * 這是完整的電子紙顯示器測試程式，包含：
 * - 系統資訊顯示
 * - 白屏清除測試
 * - 文字顯示測試
 * - 圖形繪製測試
 * - 部分更新測試（修正壓縮問題）
 * - 中央區域部分更新測試
 * 
 * 使用 GxEPD2 函式庫驅動 GDEQ0426T82 4.26吋電子紙
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
  Serial.printf("顯示器型號: GDEQ0426T82 (4.26\" 黑白電子紙)\n");
  Serial.printf("解析度: 480 x 800 像素 (理論值)\n");
  Serial.printf("實際可用區域: %d x %d 像素\n", display.width(), display.height());
  Serial.println("驅動程式: GxEPD2_426_GDEQ0426T82 (專用)");
  Serial.println();
  
  Serial.println("=== 初始化完成 ===");
  Serial.println("開始顯示測試...");
  Serial.println();
}

void loop() {
  static int testStep = 0;
  static unsigned long lastTest = 0;
  
  if (millis() - lastTest > 8000) {  // 改為每 8 秒執行一次，增加間隔
    lastTest = millis();
    testStep++;
    
    // 清除序列埠緩衝區，確保輸出乾淨
    Serial.flush();
    delay(100);
    
    Serial.print("--- 測試步驟 ");
    Serial.print(testStep);
    Serial.println(" ---");
    
    Serial.print("BUSY pin 狀態: ");
    Serial.println(digitalRead(EPD_BUSY) ? "HIGH (空閒)" : "LOW (忙碌)");
    
    Serial.print("可用記憶體: ");
    Serial.print(ESP.getFreeHeap());
    Serial.println(" bytes");
    
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
      case 5:
        testPartialUpdateCenter();
        break;
      default:
        testStep = 0;  // 重置循環
        break;
    }
    
    // 確保序列埠輸出完成
    Serial.flush();
    Serial.println();
  }
}

void testClearScreen() {
  Serial.println("測試 1: 清除螢幕");
  // ESP8266 不支援 try-catch，直接執行
  display.setRotation(0);
  display.fillScreen(GxEPD_WHITE);
  display.display(false);  // 全螢幕更新
  waitForDisplayReady();   // 等待顯示完成
  Serial.println("✅ 白屏測試完成");
}

void testDrawText() {
  Serial.println("測試 2: 繪製文字");
  // ESP8266 不支援 try-catch，直接執行
  display.setRotation(0);
  display.fillScreen(GxEPD_WHITE);
  display.setTextColor(GxEPD_BLACK);
  display.setFont(&FreeMonoBold9pt7b);
  
  // 文字靠左對齊，使用較小的 x 座標
  display.setCursor(10, 100);  // 從 x=50 改為 x=10
  display.println("GDEQ0426T82 Test");
  display.setCursor(10, 150);  // 從 x=50 改為 x=10
  display.println("WeMos D1 Mini");
  display.setCursor(10, 200);  // 從 x=50 改為 x=10
  display.println("GxEPD2 Working!");
  
  display.display(false);
  waitForDisplayReady();   // 等待顯示完成
  Serial.println("✅ 文字測試完成");
}

void testDrawShapes() {
  Serial.println("測試 3: 繪製圖形");
  Serial.flush();
  
  // ESP8266 不支援 try-catch，直接執行
  display.setRotation(0);
  display.fillScreen(GxEPD_WHITE);
  
  // 取得實際螢幕尺寸
  int screenWidth = display.width();
  int screenHeight = display.height();
  
  Serial.print("實際螢幕尺寸: ");
  Serial.print(screenWidth);
  Serial.print(" x ");
  Serial.println(screenHeight);
  
  // 根據實際螢幕尺寸調整圖形座標
  // 繪製矩形 - 左上角區域
  display.drawRect(10, 50, screenWidth/3, screenHeight/6, GxEPD_BLACK);
  display.fillRect(screenWidth/2, 50, screenWidth/4, screenHeight/6, GxEPD_BLACK);
  
  // 繪製圓形 - 中央區域
  display.drawCircle(screenWidth/4, screenHeight/2, screenWidth/8, GxEPD_BLACK);
  display.fillCircle(3*screenWidth/4, screenHeight/2, screenWidth/12, GxEPD_BLACK);
  
  // 繪製線條 - 對角線
  display.drawLine(0, 0, screenWidth-1, screenHeight-1, GxEPD_BLACK);
  display.drawLine(0, screenHeight-1, screenWidth-1, 0, GxEPD_BLACK);
  
  // 繪製邊框
  display.drawRect(0, 0, screenWidth-1, screenHeight-1, GxEPD_BLACK);
  
  display.display(false);
  waitForDisplayReady();   // 等待顯示完成
  Serial.println("✅ 圖形測試完成");
  Serial.flush();
}

void testPartialUpdate() {
  Serial.println("測試 4: 部分更新");
  Serial.flush();
  
  // 先確保螢幕狀態正確
  display.setRotation(0);
  
  // 取得螢幕尺寸
  int screenWidth = display.width();
  int screenHeight = display.height();
  
  // 設定部分更新區域 - 下半部區域
  int updateX = 10;
  int updateY = screenHeight * 2 / 3;  // 下方 1/3 處開始
  int updateWidth = screenWidth - 20;  // 留邊界
  int updateHeight = screenHeight / 3 - 10;  // 高度為螢幕的 1/3
  
  Serial.print("部分更新區域: ");
  Serial.print(updateX);
  Serial.print(",");
  Serial.print(updateY);
  Serial.print(",");
  Serial.print(updateWidth);
  Serial.print("x");
  Serial.println(updateHeight);
  
  display.setPartialWindow(updateX, updateY, updateWidth, updateHeight);
  display.fillScreen(GxEPD_WHITE);
  display.setTextColor(GxEPD_BLACK);
  display.setFont(&FreeMonoBold9pt7b);
  
  // 繪製邊框以確認更新區域
  display.drawRect(updateX, updateY, updateWidth, updateHeight, GxEPD_BLACK);
  
  // 顯示時間戳記
  display.setCursor(updateX + 10, updateY + 30);
  display.println("Partial Update Test");
  display.setCursor(updateX + 10, updateY + 60);
  display.print("Time: ");
  display.print(millis());
  display.println(" ms");
  display.setCursor(updateX + 10, updateY + 90);
  display.print("RAM: ");
  display.print(ESP.getFreeHeap());
  display.println(" bytes");
  
  display.display(true);  // 部分更新
  waitForDisplayReady();   // 等待顯示完成
  
  Serial.print("部分更新完成 (總螢幕: ");
  Serial.print(screenWidth);
  Serial.print(" x ");
  Serial.print(screenHeight);
  Serial.println(")");
  Serial.println("✅ 部分更新測試完成");
  Serial.flush();
}

void testPartialUpdateCenter() {
  Serial.println("測試 5: 中央部分更新");
  Serial.flush();
  
  // 先清除整個螢幕，確保座標系統正確
  display.setRotation(0);
  display.fillScreen(GxEPD_WHITE);
  display.display(false);  // 先全螢幕更新清除
  waitForDisplayReady();   // 等待清除完成
  
  // 計算螢幕中央區域
  int centerX = display.width() / 4;
  int centerY = display.height() / 4;
  int updateWidth = display.width() / 2;
  int updateHeight = display.height() / 2;
  
  Serial.print("螢幕中央更新區域: ");
  Serial.print(centerX);
  Serial.print(",");
  Serial.print(centerY);
  Serial.print(",");
  Serial.print(updateWidth);
  Serial.print("x");
  Serial.println(updateHeight);
  
  // 設定中央部分更新區域
  display.setPartialWindow(centerX, centerY, updateWidth, updateHeight);
  display.fillScreen(GxEPD_WHITE);
  display.setTextColor(GxEPD_BLACK);
  display.setFont(&FreeMonoBold9pt7b);
  
  // 繪製邊框確認更新區域
  display.drawRect(centerX, centerY, updateWidth, updateHeight, GxEPD_BLACK);
  
  // 在中央顯示資訊
  display.setCursor(centerX + 20, centerY + 40);
  display.println("CENTER UPDATE");
  display.setCursor(centerX + 20, centerY + 70);
  display.print("Region: ");
  display.print(updateWidth);
  display.print("x");
  display.println(updateHeight);
  display.setCursor(centerX + 20, centerY + 100);
  display.print("Time: ");
  display.println(millis() / 1000);
  
  display.display(true);  // 部分更新
  waitForDisplayReady();   // 等待顯示完成
  Serial.println("✅ 中央部分更新測試完成");
  Serial.flush();
}

// 等待顯示器完成更新的輔助函數
void waitForDisplayReady() {
  Serial.print("等待顯示器完成...");
  Serial.flush();  // 確保訊息先輸出
  
  unsigned long timeout = millis() + 10000;  // 10秒超時
  while (digitalRead(EPD_BUSY) == LOW && millis() < timeout) {
    Serial.print(".");
    delay(500);
    yield();  // 讓 ESP8266 處理其他任務
  }
  
  Serial.println(" 完成");
  Serial.flush();  // 確保輸出完成
  delay(200);  // 額外等待確保穩定
}
