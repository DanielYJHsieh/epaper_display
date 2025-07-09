// GxEPD2_display_selection_GDEQ0426T82.h
// WeMos D1 Mini + GDEQ0426T82 4.26" E-Paper 顯示器配置

#ifndef _GxEPD2_DISPLAY_SELECTION_GDEQ0426T82_H_
#define _GxEPD2_DISPLAY_SELECTION_GDEQ0426T82_H_

// 根據 GxEPD2 庫的命名慣例，GDEQ0426T82 應該歸類在 4.26" 類別

// 確認是否有支援 GDEQ0426T82 的驅動程式
// 如果沒有，我們需要使用相容的驅動程式

// 對於 4.26" 480x800 黑白顯示器，可能的選項：
// - GxEPD2_426 (4.26" generic)
// - GxEPD2_420 (4.20" 相容)

// WeMos D1 Mini 腳位配置
#define EPD_CS    15  // D8 - CS (需要 3.3k 下拉電阻到 GND)
#define EPD_DC     0  // D3 - DC  
#define EPD_RST    5  // D1 - RST (可能需要 1k 上拉電阻到 3.3V)
#define EPD_BUSY   4  // D2 - BUSY

// SPI 腳位自動配置：
// SCK  -> D5 (GPIO14)
// MOSI -> D7 (GPIO13)
// MISO -> D6 (GPIO12) - 不使用但為完整性列出

// 包含必要的 GxEPD2 標頭檔
#include <GxEPD2_BW.h>

// 檢查是否有 GDEQ0426T82 的專用驅動程式
#if defined(GxEPD2_DRIVER_CLASS) && defined(GxEPD2_426_GDEQ0426T82)
  // 使用專用的 GDEQ0426T82 驅動程式
  #define GxEPD2_DISPLAY_CLASS GxEPD2_BW
  #define GxEPD2_DRIVER_CLASS GxEPD2_426_GDEQ0426T82
  #define MAX_DISPLAY_BUFFER_SIZE 65536ul
#elif defined(GxEPD2_426)
  // 使用 4.26" 通用驅動程式
  #define GxEPD2_DISPLAY_CLASS GxEPD2_BW
  #define GxEPD2_DRIVER_CLASS GxEPD2_426
  #define MAX_DISPLAY_BUFFER_SIZE 65536ul
#elif defined(GxEPD2_420)
  // 使用 4.20" 相容驅動程式作為備選
  #define GxEPD2_DISPLAY_CLASS GxEPD2_BW
  #define GxEPD2_DRIVER_CLASS GxEPD2_420
  #define MAX_DISPLAY_BUFFER_SIZE 65536ul
#else
  // 使用基本的 GxEPD2_BW 驅動程式
  #define GxEPD2_DISPLAY_CLASS GxEPD2_BW
  #define GxEPD2_DRIVER_CLASS GxEPD2_BW
  #define MAX_DISPLAY_BUFFER_SIZE 65536ul
#endif

// ESP8266 記憶體限制考量
#if defined(ESP8266)
  // ESP8266 記憶體有限，使用較小的緩衝區
  #define MAX_HEIGHT(EPD) (EPD::HEIGHT <= 400 ? EPD::HEIGHT : 400)
  #undef MAX_DISPLAY_BUFFER_SIZE
  #define MAX_DISPLAY_BUFFER_SIZE 32768ul // 32KB 對 ESP8266 較安全
#endif

// 顯示器實例化巨集
#define DISPLAY_INSTANCE(DRIVER_CLASS) \
  GxEPD2_DISPLAY_CLASS<DRIVER_CLASS, MAX_HEIGHT(DRIVER_CLASS)> \
  display(DRIVER_CLASS(EPD_CS, EPD_DC, EPD_RST, EPD_BUSY))

// 安全的顯示器初始化巨集
#define SAFE_DISPLAY_INIT() \
  do { \
    display.init(115200, true, 2, false); \
    display.setRotation(0); \
  } while(0)

// 顯示器資訊
#define DISPLAY_WIDTH 480
#define DISPLAY_HEIGHT 800
#define DISPLAY_COLOR_DEPTH 1  // 黑白顯示器

// 調試資訊
#ifdef DEBUG_GDEQ0426T82
  #define DEBUG_PRINT(x) Serial.print(x)
  #define DEBUG_PRINTLN(x) Serial.println(x)
  #define DEBUG_PRINTF(fmt, ...) Serial.printf(fmt, ##__VA_ARGS__)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
  #define DEBUG_PRINTF(fmt, ...)
#endif

// 連線檢查函數聲明
bool checkDisplayConnection();
void printPinConfiguration();
void displayDiagnostics();

#endif // _GxEPD2_DISPLAY_SELECTION_GDEQ0426T82_H_
