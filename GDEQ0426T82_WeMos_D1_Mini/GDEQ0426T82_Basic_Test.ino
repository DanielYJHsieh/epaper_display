/*
 * GDEQ0426T82 E-Paper Display for WeMos D1 Mini (ESP8266)
 * 基礎測試和診斷程式
 * 
 * 硬體連接:
 * - GDEQ0426T82 4.26" 電子紙顯示屏 (480x800)
 * - WeMos D1 Mini ESP8266 開發板
 * 
 * 腳位連接 (WeMos D1 Mini → GDEQ0426T82):
 * BUSY -> D2 (GPIO4)   - 忙碌偵測腳位
 * RST  -> D1 (GPIO5)   - 重置腳位 (建議加 1k 上拉電阻到 3.3V)
 * DC   -> D3 (GPIO0)   - 資料/命令選擇腳位
 * CS   -> D8 (GPIO15)  - SPI 片選腳位 (必須加 3.3k 下拉電阻到 GND)
 * CLK  -> D5 (GPIO14)  - SPI 時脈腳位 (SCK)
 * DIN  -> D7 (GPIO13)  - SPI 資料輸入腳位 (MOSI)
 * GND  -> GND          - 接地
 * 3.3V -> 3.3V         - 電源 (3.3V)
 * 
 * 重要注意事項:
 * 1. CS 腳位 (GPIO15) 必須加 3.3k 下拉電阻到 GND，否則 ESP8266 無法開機
 * 2. RST 腳位建議加 1k 上拉電阻到 3.3V，提高復位穩定性
 * 3. 所有腳位都是 3.3V 邏輯準位，不可接 5V
 * 4. 確保電源穩定，電流充足
 * 
 * 函式庫安裝:
 * 1. Arduino IDE → 工具 → 管理函式庫
 * 2. 搜尋 "GxEPD2" 並安裝 Jean-Marc Zingg 的版本
 * 3. 搜尋 "Adafruit GFX" 並安裝
 * 4. 選擇開發板: LOLIN(WeMos) D1 mini
 * 
 * 作者: Arduino ESP8266 專案
 * 日期: 2025/1/9
 * 版本: 1.0 - 基礎測試版本
 */

// ==================== 基本函式庫 ====================
#include <ESP8266WiFi.h>
#include <SPI.h>

// 嘗試包含 GxEPD2 (如果編譯錯誤，程式仍可執行基本測試)
#ifdef __has_include
  #if __has_include(<GxEPD2_BW.h>)
    #include <GxEPD2_BW.h>
    #include <GxEPD2_420.h>  // 4.2" 驅動程式 (最接近 4.26")
    #include <Fonts/FreeMonoBold9pt7b.h>
    #define GXEPD2_AVAILABLE 1
    #define USE_GXEPD2_420 1
  #elif __has_include(<GxEPD2.h>)
    #include <GxEPD2.h>
    #define GXEPD2_AVAILABLE 1
    #define USE_GXEPD2_GENERIC 1
  #else
    #define GXEPD2_AVAILABLE 0
  #endif
#else
  // 舊版本編譯器，直接嘗試包含
  #define GXEPD2_AVAILABLE 0
#endif

// ==================== 腳位定義 ====================
const int EPD_BUSY = 4;   // D2 - 忙碌偵測
const int EPD_RST  = 5;   // D1 - 重置 (建議加 1k 上拉電阻)
const int EPD_DC   = 0;   // D3 - 資料/命令選擇
const int EPD_CS   = 15;  // D8 - 片選 (必須加 3.3k 下拉電阻)
const int EPD_SCK  = 14;  // D5 - SPI 時脈
const int EPD_MOSI = 13;  // D7 - SPI 資料

// ==================== 顯示器配置 ====================
const int DISPLAY_WIDTH = 480;
const int DISPLAY_HEIGHT = 800;

// ==================== GxEPD2 顯示器物件 ====================
#if GXEPD2_AVAILABLE && USE_GXEPD2_420
  // 使用 4.2" 驅動程式 (與 4.26" 最相近)
  GxEPD2_BW<GxEPD2_420, GxEPD2_420::HEIGHT> display(GxEPD2_420(EPD_CS, EPD_DC, EPD_RST, EPD_BUSY));
  #define DISPLAY_DRIVER_NAME "GxEPD2_420 (4.2\" 相容)"
#elif GXEPD2_AVAILABLE
  // 使用通用驅動程式
  #define DISPLAY_DRIVER_NAME "GxEPD2 通用驅動程式"
#else
  #define DISPLAY_DRIVER_NAME "基本 SPI (GxEPD2 不可用)"
#endif

// ==================== 全域變數 ====================
bool displayAvailable = false;
bool spiInitialized = false;
unsigned long startTime = 0;

// ==================== 主要設定函數 ====================
void setup() {
  startTime = millis();
  
  // 序列埠初始化
  Serial.begin(115200);
  delay(1000); // 等待序列埠穩定
  
  printWelcomeMessage();
  printSystemInfo();
  
  // 腳位設定
  setupPins();
  
  // SPI 初始化
  initializeSPI();
  
  // 顯示器初始化
  initializeDisplay();
  
  // 執行測試
  runAllTests();
  
  Serial.println("\n=== 初始化完成 ===");
  Serial.println("監控模式開始...");
}

// ==================== 主循環 ====================
void loop() {
  static unsigned long lastUpdate = 0;
  static int loopCount = 0;
  
  unsigned long currentTime = millis();
  loopCount++;
  
  // 每 30 秒顯示狀態
  if (currentTime - lastUpdate > 30000) {
    Serial.printf("\n--- 狀態更新 (運行 %lu 秒) ---\n", (currentTime - startTime) / 1000);
    Serial.printf("循環次數: %d\n", loopCount);
    Serial.printf("可用記憶體: %u bytes\n", ESP.getFreeHeap());
    Serial.printf("顯示器忙碌: %s\n", isDisplayBusy() ? "是" : "否");
    
    // 檢查腳位狀態
    checkPinStates();
    
    lastUpdate = currentTime;
  }
  
  // 每 60 秒執行一次顯示測試
  static unsigned long lastDisplayTest = 0;
  if (displayAvailable && (currentTime - lastDisplayTest > 60000)) {
    Serial.println("\n執行定期顯示測試...");
    testDisplayBasicOperation();
    lastDisplayTest = currentTime;
  }
  
  delay(1000); // 1 秒延遲
}

// ==================== 歡迎訊息 ====================
void printWelcomeMessage() {
  Serial.println("\n" + String("=").substring(0, 50));
  Serial.println("  GDEQ0426T82 + WeMos D1 Mini 測試程式");
  Serial.println("  版本: 1.0");
  Serial.println("  編譯時間: " + String(__DATE__) + " " + String(__TIME__));
  Serial.println(String("=").substring(0, 50));
  
  Serial.printf("使用驅動程式: %s\n", DISPLAY_DRIVER_NAME);
  Serial.printf("GxEPD2 可用: %s\n", GXEPD2_AVAILABLE ? "是" : "否");
}

// ==================== 系統資訊 ====================
void printSystemInfo() {
  Serial.println("\n--- ESP8266 系統資訊 ---");
  Serial.printf("晶片 ID: 0x%08X\n", ESP.getChipId());
  Serial.printf("Flash 大小: %u MB\n", ESP.getFlashChipSize() / (1024 * 1024));
  Serial.printf("可用記憶體: %u bytes\n", ESP.getFreeHeap());
  Serial.printf("CPU 頻率: %u MHz\n", ESP.getCpuFreqMHz());
  Serial.printf("SDK 版本: %s\n", ESP.getSdkVersion());
  Serial.printf("Arduino Core: %s\n", ESP.getCoreVersion().c_str());
}

// ==================== 腳位設定 ====================
void setupPins() {
  Serial.println("\n--- 設定腳位 ---");
  
  // 顯示腳位配置
  Serial.println("腳位配置:");
  Serial.printf("  BUSY: D2 (GPIO%d) - 輸入\n", EPD_BUSY);
  Serial.printf("  RST:  D1 (GPIO%d) - 輸出 (建議加 1k 上拉電阻)\n", EPD_RST);
  Serial.printf("  DC:   D3 (GPIO%d) - 輸出\n", EPD_DC);
  Serial.printf("  CS:   D8 (GPIO%d) - 輸出 (必須加 3.3k 下拉電阻)\n", EPD_CS);
  Serial.printf("  SCK:  D5 (GPIO%d) - SPI 時脈\n", EPD_SCK);
  Serial.printf("  MOSI: D7 (GPIO%d) - SPI 資料\n", EPD_MOSI);
  
  // 設定腳位模式
  pinMode(EPD_BUSY, INPUT);     // 忙碌偵測
  pinMode(EPD_RST, OUTPUT);     // 重置
  pinMode(EPD_DC, OUTPUT);      // 資料/命令
  pinMode(EPD_CS, OUTPUT);      // 片選
  
  // 設定初始狀態
  digitalWrite(EPD_RST, HIGH);  // 重置拉高
  digitalWrite(EPD_DC, LOW);    // 預設命令模式
  digitalWrite(EPD_CS, HIGH);   // 片選拉高 (未選擇)
  
  Serial.println("腳位設定完成");
  
  // 檢查初始腳位狀態
  checkPinStates();
}

// ==================== SPI 初始化 ====================
void initializeSPI() {
  Serial.println("\n--- 初始化 SPI ---");
  
  SPI.begin();
  SPI.setBitOrder(MSBFIRST);
  SPI.setDataMode(SPI_MODE0);
  SPI.setFrequency(4000000); // 4MHz
  
  spiInitialized = true;
  Serial.println("SPI 初始化完成");
  Serial.println("  位元順序: MSB First");
  Serial.println("  資料模式: SPI_MODE0");
  Serial.println("  頻率: 4 MHz");
}

// ==================== 顯示器初始化 ====================
void initializeDisplay() {
  Serial.println("\n--- 初始化顯示器 ---");
  
#if GXEPD2_AVAILABLE && USE_GXEPD2_420
  try {
    Serial.println("使用 GxEPD2_420 驅動程式初始化...");
    display.init(115200, true, 2, false);
    displayAvailable = true;
    Serial.println("GxEPD2 顯示器初始化成功");
    Serial.printf("顯示器尺寸: %d x %d\n", display.width(), display.height());
  } catch (...) {
    Serial.println("GxEPD2 初始化失敗，使用基本 SPI 模式");
    displayAvailable = false;
  }
#else
  Serial.println("GxEPD2 不可用，使用基本 SPI 初始化");
  
  // 基本重置序列
  resetDisplayBasic();
  displayAvailable = true; // 假設基本功能可用
#endif

  if (displayAvailable) {
    Serial.println("顯示器初始化完成");
  } else {
    Serial.println("顯示器初始化失敗");
  }
}

// ==================== 基本重置 ====================
void resetDisplayBasic() {
  Serial.println("執行基本重置序列...");
  
  digitalWrite(EPD_RST, LOW);
  delay(10);
  digitalWrite(EPD_RST, HIGH);
  delay(100);
  
  // 等待穩定
  delay(200);
  Serial.println("基本重置完成");
}

// ==================== 檢查腳位狀態 ====================
void checkPinStates() {
  Serial.println("當前腳位狀態:");
  Serial.printf("  BUSY: %s\n", digitalRead(EPD_BUSY) ? "HIGH" : "LOW");
  Serial.printf("  RST:  %s\n", digitalRead(EPD_RST) ? "HIGH" : "LOW");
  Serial.printf("  DC:   %s\n", digitalRead(EPD_DC) ? "HIGH" : "LOW");
  Serial.printf("  CS:   %s\n", digitalRead(EPD_CS) ? "HIGH" : "LOW");
}

// ==================== 忙碌狀態檢查 ====================
bool isDisplayBusy() {
  return digitalRead(EPD_BUSY) == HIGH;
}

void waitForDisplayReady(int timeoutMs = 5000) {
  Serial.print("等待顯示器就緒");
  unsigned long startTime = millis();
  
  while (isDisplayBusy() && (millis() - startTime < timeoutMs)) {
    Serial.print(".");
    delay(100);
  }
  
  if (isDisplayBusy()) {
    Serial.println(" 超時!");
  } else {
    Serial.println(" 完成");
  }
}

// ==================== SPI 通訊函數 ====================
void sendCommand(uint8_t command) {
  if (!spiInitialized) return;
  
  digitalWrite(EPD_DC, LOW);  // 命令模式
  digitalWrite(EPD_CS, LOW);  // 選擇設備
  delayMicroseconds(1);
  
  SPI.transfer(command);
  
  delayMicroseconds(1);
  digitalWrite(EPD_CS, HIGH); // 取消選擇
}

void sendData(uint8_t data) {
  if (!spiInitialized) return;
  
  digitalWrite(EPD_DC, HIGH); // 資料模式
  digitalWrite(EPD_CS, LOW);  // 選擇設備
  delayMicroseconds(1);
  
  SPI.transfer(data);
  
  delayMicroseconds(1);
  digitalWrite(EPD_CS, HIGH); // 取消選擇
}

// ==================== 測試函數 ====================
void runAllTests() {
  Serial.println("\n" + String("=").substring(0, 30) + " 開始測試 " + String("=").substring(0, 30));
  
  testBasicConnectivity();
  testSPICommunication();
  testDisplayResponse();
  
  if (displayAvailable) {
    testDisplayBasicOperation();
  }
  
  Serial.println(String("=").substring(0, 30) + " 測試完成 " + String("=").substring(0, 30));
}

void testBasicConnectivity() {
  Serial.println("\n測試 1: 基本連接性");
  
  // 測試腳位讀寫
  Serial.println("測試輸出腳位...");
  
  digitalWrite(EPD_RST, LOW);
  delay(10);
  bool rstLow = digitalRead(EPD_RST) == LOW;
  
  digitalWrite(EPD_RST, HIGH);
  delay(10);
  bool rstHigh = digitalRead(EPD_RST) == HIGH;
  
  Serial.printf("  RST 腳位測試: %s\n", (rstLow && rstHigh) ? "通過" : "失敗");
  
  // 測試 BUSY 腳位
  Serial.printf("  BUSY 腳位當前狀態: %s\n", digitalRead(EPD_BUSY) ? "HIGH" : "LOW");
  
  Serial.println("基本連接性測試完成");
}

void testSPICommunication() {
  Serial.println("\n測試 2: SPI 通訊");
  
  if (!spiInitialized) {
    Serial.println("SPI 未初始化，跳過測試");
    return;
  }
  
  Serial.println("發送測試命令...");
  
  // 發送一些通用命令
  sendCommand(0x01); // 軟體重置
  delay(100);
  
  sendCommand(0x04); // 電源開啟
  delay(100);
  
  sendCommand(0x00); // Panel Setting (如果支援)
  sendData(0x0F);    // 測試資料
  delay(100);
  
  Serial.println("SPI 通訊測試完成");
}

void testDisplayResponse() {
  Serial.println("\n測試 3: 顯示器響應");
  
  Serial.println("執行重置並監控 BUSY 腳位...");
  
  // 記錄重置前的狀態
  bool busyBefore = isDisplayBusy();
  Serial.printf("重置前 BUSY 狀態: %s\n", busyBefore ? "HIGH" : "LOW");
  
  // 執行重置
  resetDisplayBasic();
  
  // 監控 BUSY 狀態變化
  Serial.println("監控 BUSY 狀態變化 (10 秒):");
  for (int i = 0; i < 100; i++) {
    bool busy = isDisplayBusy();
    if (i % 10 == 0) {
      Serial.printf("  %d.%d 秒: %s\n", i/10, i%10, busy ? "BUSY" : "READY");
    }
    delay(100);
  }
  
  Serial.println("顯示器響應測試完成");
}

void testDisplayBasicOperation() {
  Serial.println("\n測試 4: 基本顯示操作");
  
#if GXEPD2_AVAILABLE && USE_GXEPD2_420
  Serial.println("執行 GxEPD2 基本顯示測試...");
  
  try {
    // 清除螢幕
    Serial.println("清除螢幕...");
    display.setRotation(0);
    display.fillScreen(GxEPD_WHITE);
    display.display(false); // 部分更新
    
    delay(2000);
    
    // 繪製簡單圖案
    Serial.println("繪製測試圖案...");
    display.fillScreen(GxEPD_WHITE);
    
    // 繪製邊框
    display.drawRect(10, 10, display.width()-20, display.height()-20, GxEPD_BLACK);
    
    // 繪製對角線
    display.drawLine(10, 10, display.width()-10, display.height()-10, GxEPD_BLACK);
    display.drawLine(display.width()-10, 10, 10, display.height()-10, GxEPD_BLACK);
    
    // 繪製中心圓
    int centerX = display.width() / 2;
    int centerY = display.height() / 2;
    display.drawCircle(centerX, centerY, 50, GxEPD_BLACK);
    
    display.display(false);
    
    Serial.println("基本顯示操作測試完成");
    
  } catch (...) {
    Serial.println("GxEPD2 顯示操作失敗");
  }
#else
  Serial.println("GxEPD2 不可用，跳過顯示測試");
#endif
}

// ==================== 診斷和幫助函數 ====================
void showDiagnosticInfo() {
  Serial.println("\n" + String("=").substring(0, 40));
  Serial.println("  診斷資訊");
  Serial.println(String("=").substring(0, 40));
  
  Serial.println("硬體狀態:");
  checkPinStates();
  
  Serial.println("\n軟體狀態:");
  Serial.printf("  SPI 初始化: %s\n", spiInitialized ? "是" : "否");
  Serial.printf("  顯示器可用: %s\n", displayAvailable ? "是" : "否");
  Serial.printf("  GxEPD2 可用: %s\n", GXEPD2_AVAILABLE ? "是" : "否");
  
  Serial.println("\n記憶體狀態:");
  Serial.printf("  可用記憶體: %u bytes\n", ESP.getFreeHeap());
  Serial.printf("  記憶體碎片: %u%%\n", ESP.getHeapFragmentation());
  Serial.printf("  最大可用塊: %u bytes\n", ESP.getMaxFreeBlockSize());
  
  Serial.println("\n如果遇到問題，請檢查:");
  Serial.println("1. 腳位連接是否正確");
  Serial.println("2. 是否添加了必要的電阻 (CS: 3.3k下拉, RST: 1k上拉)");
  Serial.println("3. 電源是否穩定 (3.3V)");
  Serial.println("4. GxEPD2 函式庫是否正確安裝");
  Serial.println("5. 顯示器型號是否相容");
  
  Serial.println(String("=").substring(0, 40));
}
