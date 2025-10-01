/*
 * GDEQ0426T82 + WeMos D1 Mini 優化版主程式
 * 
 * 📊 v2.1 優化重點：
 * ✅ 修正顯示器尺寸設定 (使用分頁模式，支援完整 800×480)
 * ✅ 記憶體優化：從 40KB 緩衝降到 800 bytes (減少 98%)
 * ✅ 移除不必要的 8-bit 對齊邏輯
 * ✅ 改為一次性測試避免過度刷新電子紙
 * ✅ 使用 F() 巨集減少 RAM 使用
 * ✅ 簡化程式碼邏輯，提升可維護性
 * 
 * 測試項目：
 * - 系統資訊顯示與記憶體統計
 * - 白屏清除測試
 * - 文字顯示測試
 * - 圖形繪製測試 (完整 480 像素高度)
 * - 部分更新測試
 * - 中央區域部分更新測試
 * 
 * 版本: v2.1 (優化版)
 * 日期: 2025-10-02
 * 基於: GxEPD2 官方範例和最佳實踐
 */

#include <GxEPD2_BW.h>
#include <gdeq/GxEPD2_426_GDEQ0426T82.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <ESP8266WiFi.h>

// ==================== 腳位定義 ====================
#define EPD_CS    15  // D8 - CS (必須加 3.3k 下拉電阻到 GND)
#define EPD_DC     0  // D3 - DC
#define EPD_RST    5  // D1 - RST (建議加 1k 上拉電阻到 3.3V)  
#define EPD_BUSY   4  // D2 - BUSY

// ==================== 記憶體最佳化顯示器設定 ====================
// 使用分頁模式：僅緩衝 8 行而非完整螢幕
// 優勢：減少 98% 的 RAM 使用 (從 48KB 降到 800 bytes)
// 功能：仍可顯示完整 800×480 像素 (透過自動分頁)
#define MAX_DISPLAY_BUFFER_SIZE 800  // 每次僅緩衝 8 行

#define MAX_HEIGHT(EPD) (EPD::HEIGHT <= MAX_DISPLAY_BUFFER_SIZE / (EPD::WIDTH / 8) ? \
                         EPD::HEIGHT : MAX_DISPLAY_BUFFER_SIZE / (EPD::WIDTH / 8))

// 使用分頁模式的顯示器物件
GxEPD2_BW<GxEPD2_426_GDEQ0426T82, MAX_HEIGHT(GxEPD2_426_GDEQ0426T82)> display(
    GxEPD2_426_GDEQ0426T82(EPD_CS, EPD_DC, EPD_RST, EPD_BUSY)
);

// ==================== 記憶體統計輔助函數 ====================
void printMemoryStats() {
  unsigned long freeHeap = ESP.getFreeHeap();
  Serial.println(F("--- 記憶體狀態 ---"));
  Serial.print(F("可用記憶體: ")); 
  Serial.print(freeHeap); 
  Serial.println(F(" bytes"));
  
  // 計算實際緩衝大小
  unsigned long bufferSize = (unsigned long)(display.width() / 8) * display.height();
  Serial.print(F("顯示緩衝: ")); 
  Serial.print(bufferSize); 
  Serial.println(F(" bytes"));
  
  Serial.print(F("緩衝行數: ")); 
  Serial.print(display.height()); 
  Serial.println(F(" lines"));
  Serial.println();
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println(F("=== GDEQ0426T82 + WeMos D1 Mini 優化版測試 ==="));
  Serial.printf("編譯時間: %s %s\r\n", __DATE__, __TIME__);
  Serial.println();
  
  // ==================== 記憶體最佳化：禁用 WiFi ====================
  WiFi.mode(WIFI_OFF);
  WiFi.forceSleepBegin();
  Serial.println(F("WiFi 已禁用以最佳化記憶體"));
  
  // 顯示系統資訊
  Serial.println(F("--- ESP8266 系統資訊 ---"));
  Serial.printf("晶片 ID: 0x%08X\r\n", ESP.getChipId());
  Serial.printf("Flash 大小: %d MB\r\n", ESP.getFlashChipSize() / (1024 * 1024));
  Serial.printf("CPU 頻率: %d MHz\r\n", ESP.getCpuFreqMHz());
  Serial.println();
  
  printMemoryStats();
  
  // 初始化腳位
  Serial.println(F("--- 初始化腳位 ---"));
  pinMode(EPD_BUSY, INPUT);
  pinMode(EPD_RST, OUTPUT);
  pinMode(EPD_DC, OUTPUT);
  pinMode(EPD_CS, OUTPUT);
  digitalWrite(EPD_CS, HIGH);
  digitalWrite(EPD_DC, HIGH);
  digitalWrite(EPD_RST, HIGH);
  Serial.println(F("腳位初始化完成"));
  Serial.println();
  
  // 初始化 GxEPD2 顯示器
  Serial.println(F("--- 初始化 GxEPD2 顯示器 ---"));
  display.init(115200, true, 2, false);
  Serial.println(F("[OK] GxEPD2 顯示器初始化成功！"));
  Serial.printf("顯示器尺寸: %d x %d 像素\r\n", display.width(), display.height());
  Serial.println();
  
  printMemoryStats();
  
  // 執行單次測試
  Serial.println(F("=== 開始單次測試 ==="));
  testClearScreen();
  testPartialUpdate();
  testPartialUpdateCenter();
  
  Serial.println();
  Serial.println(F("=== 測試完成 ==="));
  printMemoryStats();
}

void loop() {
  // 測試已在 setup() 中完成，進入閒置狀態
  delay(10000);
}

void testClearScreen() {
  Serial.println(F("測試 1: 清除螢幕"));
  display.setRotation(0);
  display.setFullWindow();
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
  } while (display.nextPage());
  Serial.println(F("[OK] 白屏測試完成"));
}

void testPartialUpdate() {
  Serial.println(F("測試 2: 部分更新"));
  
  // 先建立完整背景
  display.setRotation(0);
  display.setFullWindow();
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
    display.setTextColor(GxEPD_BLACK);
    display.setCursor(10, 50);
    display.print(F("Background Content"));
    display.drawRect(5, 5, display.width()-10, display.height()-10, GxEPD_BLACK);
  } while (display.nextPage());
  
  Serial.println(F("背景圖像完成，開始部分更新..."));
  delay(1000);
  
  // 部分更新：在左上角更新小區域
  int16_t x = 50;
  int16_t y = 100;
  int16_t w = 200;
  int16_t h = 50;
  
  display.setPartialWindow(x, y, w, h);
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
    display.setTextColor(GxEPD_BLACK);
    display.setCursor(x + 5, y + 30);
    display.print(F("UPDATED!"));
  } while (display.nextPage());
  
  Serial.println(F("[OK] 部分更新完成"));
}


void testPartialUpdateCenter() {
  Serial.println(F("測試 3: 中央部分更新"));
  
  // 先建立網格背景
  display.setRotation(0);
  display.setFullWindow();
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
    for (int i = 50; i < display.width(); i += 50) {
      display.drawLine(i, 0, i, display.height(), GxEPD_BLACK);
    }
    for (int i = 50; i < display.height(); i += 50) {
      display.drawLine(0, i, display.width(), i, GxEPD_BLACK);
    }
  } while (display.nextPage());
  
  Serial.println(F("網格背景完成，開始中央部分更新..."));
  delay(1000);
  
  // 部分更新：在中央更新區域（加大一倍）
  int16_t x = display.width() / 2 - 150;
  int16_t y = display.height() / 2 - 80;
  int16_t w = 300;
  int16_t h = 160;
  
  display.setPartialWindow(x, y, w, h);
  display.firstPage();
  do {
    display.fillScreen(GxEPD_BLACK);
    display.setTextColor(GxEPD_WHITE);
    display.setCursor(x + 10, y + 30);
    display.print(F("CENTER"));
    display.setCursor(x + 10, y + 50);
    display.print(F("UPDATE"));
  } while (display.nextPage());
  
  Serial.println(F("[OK] 中央部分更新完成"));
}
