/*
  * 硬體連接 (備份文件中的原始配置)：
 * EPD_BUSY -> D2 (GPIO4)  - 忙碌偵測腳位
 * EPD_RST  -> D1 (GPIO5)  - 重置腳位 (建議加 1k 上拉電阻到 3.3V)
 * EPD_DC   -> D3 (GPIO0)  - 資料/命令選擇腳位
 * EPD_CS   -> D8 (GPIO15) - SPI 片選腳位 (必須加 3.3k 下拉電阻到 GND)
 * EPD_CLK  -> D5 (GPIO14) - SPI 時脈腳位 (SCK)
 * EPD_DIN  -> D7 (GPIO13) - SPI 資料輸入腳位 (MOSI)
 * VCC      -> 3V3
 * GND      -> GND6T82 強制 GxEPD2 測試程式
 * 專為 WeMos D1 Mini Pro 設計
 * 強制使用 GxEPD2 驅動，不依賴自動偵測
 * 
 * 硬體連接：
 * EPD_BUSY -> D6 (GPIO12) - MISO (重用為 BUSY)
 * EPD_RST  -> D4 (GPIO2)
 * EPD_DC   -> D3 (GPIO0)
 * EPD_CS   -> D8 (GPIO15) - SPI CS
 * EPD_CLK  -> D5 (GPIO14) - SPI SCK
 * EPD_DIN  -> D7 (GPIO13) - SPI MOSI
 * VCC      -> 3V3
 * GND      -> GND
 */

#include <SPI.h>
#include <GxEPD2_BW.h>
#include <gdeq/GxEPD2_426_GDEQ0426T82.h>  // 專用驅動
#include <Fonts/FreeMonoBold9pt7b.h>

// 硬體引腳定義 (備份文件中的原始配置)
#define EPD_CS    15  // D8 - CS (必須加 3.3k 下拉電阻到 GND)
#define EPD_DC     0  // D3 - DC
#define EPD_RST    5  // D1 - RST (建議加 1k 上拉電阻到 3.3V)
#define EPD_BUSY   4  // D2 - BUSY

// 強制使用 GxEPD2 驅動
GxEPD2_BW<GxEPD2_426_GDEQ0426T82, GxEPD2_426_GDEQ0426T82::HEIGHT> display(
  GxEPD2_426_GDEQ0426T82(EPD_CS, EPD_DC, EPD_RST, EPD_BUSY)
);

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("=== GDEQ0426T82 強制 GxEPD2 測試 ===");
  Serial.println("硬體: WeMos D1 Mini Pro");
  Serial.println("EPD: 4.26吋 黑白電子紙 (SSD1677)");
  Serial.println();
  
  // 顯示引腳配置 (備份文件中的原始配置)
  Serial.println("--- 引腳配置 ---");
  Serial.printf("BUSY: D2 (GPIO%d) - 忙碌偵測\n", EPD_BUSY);
  Serial.printf("RST:  D1 (GPIO%d) - 重置 (建議加 1k 上拉電阻)\n", EPD_RST);
  Serial.printf("DC:   D3 (GPIO%d) - 資料/命令選擇\n", EPD_DC);
  Serial.printf("CS:   D8 (GPIO%d) - SPI 片選 (必須加 3.3k 下拉電阻)\n", EPD_CS);
  Serial.printf("CLK:  D5 (GPIO%d) - SPI 時脈 (SCK)\n", 14);
  Serial.printf("DIN:  D7 (GPIO%d) - SPI 資料 (MOSI)\n", 13);
  Serial.println();
  
  // 檢查 BUSY 引腳初始狀態
  pinMode(EPD_BUSY, INPUT_PULLUP);
  Serial.printf("BUSY 引腳初始狀態: %s\n", digitalRead(EPD_BUSY) ? "HIGH" : "LOW");
  
  // 初始化 SPI
  Serial.println("--- 初始化 SPI ---");
  SPI.begin();  // ESP8266 使用預設引腳: SCK=D5, MISO=D6, MOSI=D7, SS=D8
  SPI.setFrequency(50000);     // 50kHz 低速，確保穩定
  SPI.setDataMode(SPI_MODE0);  // 模式 0
  Serial.println("✅ SPI 初始化完成 (50kHz, Mode 0)");
  Serial.println();
  
  // 初始化 GxEPD2 顯示器
  Serial.println("--- 初始化 GxEPD2 顯示器 ---");
  display.init(115200, true, 2, false);
  Serial.println("✅ GxEPD2 顯示器初始化成功！");
  Serial.printf("顯示器尺寸: %d x %d 像素\n", display.width(), display.height());
  Serial.println("驅動程式: GxEPD2_426_GDEQ0426T82 (專用)");
  Serial.println();
  
  Serial.println("=== 開始顯示測試 ===");
  runDisplayTests();
}

void runDisplayTests() {
  Serial.println("測試 1: 清除螢幕 (白色)");
  display.setRotation(0);
  display.fillScreen(GxEPD_WHITE);
  display.display(false);  // 全螢幕更新
  Serial.println("✅ 白屏測試完成");
  delay(3000);
  
  Serial.println("測試 2: 顯示基本文字");
  display.fillScreen(GxEPD_WHITE);
  display.setTextColor(GxEPD_BLACK);
  display.setFont(&FreeMonoBold9pt7b);
  display.setCursor(50, 100);
  display.println("GDEQ0426T82 Test");
  display.setCursor(50, 150);
  display.println("WeMos D1 Mini Pro");
  display.setCursor(50, 200);
  display.println("GxEPD2 Working!");
  display.display(false);
  Serial.println("✅ 文字顯示完成");
  delay(3000);
  
  Serial.println("測試 3: 繪製簡單圖形");
  display.fillScreen(GxEPD_WHITE);
  
  // 繪製邊框
  display.drawRect(10, 10, display.width()-20, display.height()-20, GxEPD_BLACK);
  
  // 繪製矩形
  display.drawRect(50, 50, 200, 100, GxEPD_BLACK);
  display.fillRect(300, 50, 100, 100, GxEPD_BLACK);
  
  // 繪製圓形
  display.drawCircle(150, 300, 50, GxEPD_BLACK);
  display.fillCircle(350, 300, 30, GxEPD_BLACK);
  
  display.display(false);
  Serial.println("✅ 圖形繪製完成");
  delay(3000);
  
  Serial.println("測試 4: 部分更新");
  display.setPartialWindow(50, 50, 300, 100);
  display.fillScreen(GxEPD_WHITE);
  display.setTextColor(GxEPD_BLACK);
  display.setFont(&FreeMonoBold9pt7b);
  display.setCursor(60, 90);
  display.println("Partial Update");
  display.setCursor(60, 120);
  display.printf("Time: %lu ms", millis());
  display.display(true);  // 部分更新
  Serial.println("✅ 部分更新完成");
  
  Serial.println();
  Serial.println("=== 所有測試完成 ===");
  Serial.println("如果 EPD 有顯示變化，代表驅動正常工作！");
  Serial.println("如果沒有顯示，請檢查：");
  Serial.println("1. 硬體連接是否正確");
  Serial.println("2. 電源是否穩定 (3.3V)");
  Serial.println("3. SPI 線路是否接好");
  Serial.println("4. BUSY 引腳是否正常");
}

void loop() {
  // 每 30 秒更新一次時間戳
  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate > 30000) {
    lastUpdate = millis();
    
    Serial.println("--- 時間戳更新 ---");
    display.setPartialWindow(50, 250, 300, 50);
    display.fillScreen(GxEPD_WHITE);
    display.setTextColor(GxEPD_BLACK);
    display.setFont(&FreeMonoBold9pt7b);
    display.setCursor(60, 280);
    display.printf("Alive: %lu s", millis() / 1000);
    display.display(true);
    Serial.printf("時間戳更新: %lu 秒\n", millis() / 1000);
  }
  
  delay(100);
}
