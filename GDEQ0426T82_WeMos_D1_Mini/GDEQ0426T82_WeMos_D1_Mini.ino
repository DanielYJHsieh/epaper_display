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
  Serial.printf("編譯時間: %s %s\r\n", __DATE__, __TIME__);
  Serial.println();
  
  // 顯示系統資訊
  Serial.println("--- ESP8266 系統資訊 ---");
  Serial.printf("晶片 ID: 0x%08X\r\n", ESP.getChipId());
  Serial.printf("Flash 大小: %d MB\r\n", ESP.getFlashChipSize() / (1024 * 1024));
  Serial.printf("可用記憶體: %d bytes\r\n", ESP.getFreeHeap());
  Serial.printf("CPU 頻率: %d MHz\r\n", ESP.getCpuFreqMHz());
  
  // 腳位設定與硬體檢查
  Serial.println("--- 設定腳位與硬體檢查 ---");
  pinMode(EPD_BUSY, INPUT);
  pinMode(EPD_RST, OUTPUT);
  pinMode(EPD_DC, OUTPUT);
  pinMode(EPD_CS, OUTPUT);
  
  // 初始狀態設定
  digitalWrite(EPD_CS, HIGH);    // CS 預設為 HIGH
  digitalWrite(EPD_DC, HIGH);    // DC 預設為 HIGH
  digitalWrite(EPD_RST, HIGH);   // RST 預設為 HIGH
  
  delay(100);  // 等待腳位穩定
  
  Serial.println("腳位配置:");
  Serial.printf("  BUSY: D2 (GPIO%d) - 輸入\r\n", EPD_BUSY);
  Serial.printf("  RST:  D1 (GPIO%d) - 輸出\r\n", EPD_RST);
  Serial.printf("  DC:   D3 (GPIO%d) - 輸出\r\n", EPD_DC);
  Serial.printf("  CS:   D8 (GPIO%d) - 輸出\r\n", EPD_CS);
  
  // 硬體重置序列
  Serial.println("執行硬體重置序列...");
  digitalWrite(EPD_RST, LOW);   // 拉低 RST
  delay(200);                   // 保持 200ms
  digitalWrite(EPD_RST, HIGH);  // 拉高 RST
  delay(200);                   // 等待 200ms
  
  // 檢查 BUSY 腳位狀態
  Serial.println("硬體狀態檢查:");
  for (int i = 0; i < 5; i++) {
  Serial.printf("  檢查 %d: BUSY=%s, RST=%s, DC=%s, CS=%s\r\n", 
                  i+1,
                  digitalRead(EPD_BUSY) ? "HIGH" : "LOW",
                  digitalRead(EPD_RST) ? "HIGH" : "LOW", 
                  digitalRead(EPD_DC) ? "HIGH" : "LOW",
                  digitalRead(EPD_CS) ? "HIGH" : "LOW");
    delay(500);
  }
  
  // 如果 BUSY 一直是 LOW，可能是硬體問題
  if (digitalRead(EPD_BUSY) == LOW) {
    Serial.println("[警告] BUSY pin 持續為 LOW");
    Serial.println("可能原因:");
    Serial.println("1. BUSY 腳位連接錯誤");
    Serial.println("2. EPD 模組未正確供電");
    Serial.println("3. EPD 模組故障");
    Serial.println("4. 線路短路或接觸不良");
    Serial.println("建議檢查硬體連接！");
  }
  
  Serial.println("腳位設定完成");
  Serial.println();
  
  // 初始化 GxEPD2 顯示器
  Serial.println("--- 初始化 GxEPD2 顯示器 ---");
  // ESP8266 不支援 try-catch，直接執行初始化
  display.init(115200, true, 2, false);
  Serial.println("[OK] GxEPD2 顯示器初始化成功！");
  Serial.printf("顯示器尺寸: %d x %d 像素\r\n", display.width(), display.height());
  Serial.printf("顯示器型號: GDEQ0426T82 (4.26\" 黑白電子紙)\r\n");
  Serial.printf("解析度: 480 x 800 像素 (理論值)\r\n");
  Serial.printf("實際可用區域: %d x %d 像素\r\n", display.width(), display.height());
  Serial.println("驅動程式: GxEPD2_426_GDEQ0426T82 (專用)");
  Serial.println();
  
  Serial.println("=== 初始化完成 ===");
  Serial.println("開始顯示測試...");
  Serial.println();
}

void loop() {
  static int testStep = 0;
  static unsigned long lastTest = 0;
  static bool hasCheckedPartialUpdate = false;
  
  if (millis() - lastTest > 8000) {  // 改為每 8 秒執行一次，增加間隔
    lastTest = millis();
    testStep++;
    
    // 檢查部分更新能力（只檢查一次）
    if (!hasCheckedPartialUpdate && testStep == 1) {
      Serial.println("=== 檢查 EPD 部分更新能力 ===");
      Serial.println("EPD 面板類型: GDEQ0426T82 (SSD1677)");
      Serial.println("驅動程式: GxEPD2_426_GDEQ0426T82");
      Serial.println("理論上支援部分更新功能");
      Serial.println("注意: 部分更新需要先有完整背景圖像");
      Serial.println("=====================================");
      hasCheckedPartialUpdate = true;
    }
    
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
  
  // 檢查是否需要等待 BUSY
  if (digitalRead(EPD_BUSY) == HIGH) {
    waitForDisplayReady();   // 只有在 BUSY 為 HIGH時才等待
  } else {
    Serial.println("BUSY pin 為 LOW，跳過等待，使用固定延遲...");
    delay(5000);  // 使用固定的 5 秒延遲
    Serial.println("固定延遲完成");
  }
  
  Serial.println("[OK] 白屏測試完成");
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
  Serial.println("[OK] 文字測試完成");
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
  Serial.println("[OK] 圖形測試完成");
  Serial.flush();
}

void testPartialUpdate() {
  Serial.println("測試 4: 部分更新");
  Serial.flush();
  
  // 步驟 1: 先建立完整背景圖像
  Serial.println("步驟 1: 建立背景圖像");
  display.setRotation(0);
  display.fillScreen(GxEPD_WHITE);
  display.setTextColor(GxEPD_BLACK);
  display.setFont(&FreeMonoBold9pt7b);
  
  // 在螢幕上畫一些背景內容
  display.setCursor(10, 50);
  display.println("Background Content");
  display.setCursor(10, 100);
  display.println("Ready for partial update");
  display.drawRect(5, 5, display.width()-10, display.height()-10, GxEPD_BLACK);
  
  display.display(false);  // 先全螢幕顯示背景
  waitForDisplayReady();
  Serial.println("背景圖像建立完成");
  
  // 步驟 2: 等待一下讓使用者看到背景
  delay(2000);
  
  // 步驟 3: 執行部分更新
  Serial.println("步驟 2: 執行部分更新");
  
  // 取得螢幕尺寸
  int screenWidth = display.width();
  int screenHeight = display.height();
  
  // 設定部分更新區域 - 下半部區域（較大的區域更容易看到效果）
  int updateX = 20;
  int updateY = screenHeight / 2;  // 從中間開始
  int updateWidth = screenWidth - 40;  // 留更多邊界
  int updateHeight = screenHeight / 2 - 20;  // 下半部
  
  Serial.print("部分更新區域: ");
  Serial.print(updateX);
  Serial.print(",");
  Serial.print(updateY);
  Serial.print(",");
  Serial.print(updateWidth);
  Serial.print("x");
  Serial.println(updateHeight);
  
  // 設定部分更新視窗
  display.setPartialWindow(updateX, updateY, updateWidth, updateHeight);
  
  // 在部分更新區域繪製新內容（相對座標）
  display.fillScreen(GxEPD_BLACK);  // 用黑色背景讓效果更明顯
  display.setTextColor(GxEPD_WHITE);
  display.setFont(&FreeMonoBold9pt7b);
  
  // 繪製白色邊框
  display.drawRect(2, 2, updateWidth-4, updateHeight-4, GxEPD_WHITE);
  
  // 顯示時間戳記 (使用相對座標)
  display.setCursor(10, 30);
  display.println("PARTIAL UPDATE");
  display.setCursor(10, 60);
  display.print("Time: ");
  display.print(millis() / 1000);
  display.println("s");
  display.setCursor(10, 90);
  display.print("RAM: ");
  display.print(ESP.getFreeHeap());
  display.println("B");
  display.setCursor(10, 120);
  display.print("Size: ");
  display.print(updateWidth);
  display.print("x");
  display.println(updateHeight);
  
  // 執行部分更新
  display.display(true);  // 部分更新
  waitForDisplayReady();   // 等待顯示完成
  
  Serial.print("部分更新完成 (總螢幕: ");
  Serial.print(screenWidth);
  Serial.print(" x ");
  Serial.print(screenHeight);
  Serial.println(")");
  Serial.println("[OK] 部分更新測試完成");
  Serial.flush();
}

void testPartialUpdateCenter() {
  Serial.println("測試 5: 中央部分更新");
  Serial.flush();
  
  // 步驟 1: 建立背景圖像
  Serial.println("步驟 1: 建立中央更新背景");
  display.setRotation(0);
  display.fillScreen(GxEPD_WHITE);
  display.setTextColor(GxEPD_BLACK);
  display.setFont(&FreeMonoBold9pt7b);
  
  // 繪製網格背景以便觀察部分更新效果
  for (int i = 50; i < display.width(); i += 50) {
    display.drawLine(i, 0, i, display.height(), GxEPD_BLACK);
  }
  for (int i = 50; i < display.height(); i += 50) {
    display.drawLine(0, i, display.width(), i, GxEPD_BLACK);
  }
  
  display.setCursor(10, 30);
  display.println("Grid Background");
  display.setCursor(10, display.height() - 20);
  display.println("Center update coming...");
  
  display.display(false);  // 先全螢幕顯示背景
  waitForDisplayReady();
  Serial.println("網格背景建立完成");
  
  // 步驟 2: 等待一下
  delay(3000);
  
  // 步驟 3: 執行中央部分更新
  Serial.println("步驟 2: 執行中央部分更新");
  
  // 計算螢幕中央區域（更大的區域）
  int centerX = display.width() / 6;   // 更靠左一點
  int centerY = display.height() / 6;  // 更靠上一點
  int updateWidth = display.width() * 2 / 3;   // 2/3 寬度
  int updateHeight = display.height() * 2 / 3; // 2/3 高度
  
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
  
  // 用對比色填充，讓效果更明顯
  display.fillScreen(GxEPD_BLACK);  // 黑色背景
  display.setTextColor(GxEPD_WHITE);
  display.setFont(&FreeMonoBold9pt7b);
  
  // 繪製白色邊框 (使用相對座標)
  display.drawRect(5, 5, updateWidth-10, updateHeight-10, GxEPD_WHITE);
  display.drawRect(10, 10, updateWidth-20, updateHeight-20, GxEPD_WHITE);
  
  // 在中央顯示資訊 (使用相對座標)
  display.setCursor(20, 40);
  display.println("CENTER UPDATE");
  display.setCursor(20, 70);
  display.println("SUCCESS!");
  display.setCursor(20, 100);
  display.print("Region: ");
  display.print(updateWidth);
  display.print("x");
  display.println(updateHeight);
  display.setCursor(20, 130);
  display.print("Time: ");
  display.print(millis() / 1000);
  display.println("s");
  display.setCursor(20, 160);
  display.print("Free RAM: ");
  display.println(ESP.getFreeHeap());
  
  // 在角落加上小方塊確認位置
  display.fillRect(0, 0, 15, 15, GxEPD_WHITE);
  display.fillRect(updateWidth-15, 0, 15, 15, GxEPD_WHITE);
  display.fillRect(0, updateHeight-15, 15, 15, GxEPD_WHITE);
  display.fillRect(updateWidth-15, updateHeight-15, 15, 15, GxEPD_WHITE);
  
  display.display(true);  // 部分更新
  waitForDisplayReady();   // 等待顯示完成
  Serial.println("[OK] 中央部分更新測試完成");
  Serial.flush();
}

// 等待顯示器完成更新的輔助函數
void waitForDisplayReady() {
  Serial.print("等待顯示器完成...");
  Serial.flush();  // 確保訊息先輸出
  
  // 記錄開始時間和 BUSY 狀態
  unsigned long startTime = millis();
  bool initialBusyState = digitalRead(EPD_BUSY);
  Serial.print(" (初始BUSY: ");
  Serial.print(initialBusyState ? "HIGH" : "LOW");
  Serial.print(") ");
  
  // 如果初始狀態就是 LOW，可能是硬體問題
  if (!initialBusyState) {
    Serial.println();
    Serial.println("[注意] BUSY pin 初始狀態為 LOW");
    Serial.println("這可能表示:");
    Serial.println("1. EPD 正在執行操作");
    Serial.println("2. BUSY 腳位連接問題");
    Serial.println("3. EPD 模組故障");
    Serial.println("將嘗試等待或跳過...");
  }
  
  unsigned long timeout = millis() + 10000;  // 10秒超時（縮短超時時間）
  int dotCount = 0;
  bool busyChanged = false;
  
  while (digitalRead(EPD_BUSY) == LOW && millis() < timeout) {
    Serial.print(".");
    dotCount++;
    if (dotCount % 10 == 0) {
      Serial.print(" ");
      Serial.print((millis() - startTime) / 1000);
      Serial.print("s ");
    }
    delay(200);  // 縮短延遲
    yield();  // 讓 ESP8266 處理其他任務
    
    // 檢查 BUSY 狀態是否有變化
    if (digitalRead(EPD_BUSY) != initialBusyState) {
      busyChanged = true;
    }
  }
  
  unsigned long elapsedTime = millis() - startTime;
  bool finalBusyState = digitalRead(EPD_BUSY);
  
  Serial.print(" 完成 (耗時: ");
  Serial.print(elapsedTime);
  Serial.print("ms, 最終BUSY: ");
  Serial.print(finalBusyState ? "HIGH" : "LOW");
  Serial.println(")");
  
  if (millis() >= timeout) {
    Serial.println("[警告] 等待顯示器完成時發生超時");
    if (!busyChanged) {
      Serial.println("[建議] BUSY pin 狀態未變化，可能需要檢查硬體連接");
      Serial.println("   或者該 EPD 模組不使用 BUSY 信號");
    }
  } else if (finalBusyState) {
    Serial.println("[OK] BUSY pin 已變為 HIGH，顯示器應已完成操作");
  }
  
  Serial.flush();  // 確保輸出完成
  delay(200);  // 額外等待確保穩定
}
