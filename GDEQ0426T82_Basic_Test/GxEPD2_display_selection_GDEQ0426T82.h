// GxEPD2_display_selection_GDEQ0426T82.h
// WeMos D1 Mini + GDEQ0426T82 4.26" E-Paper 顯示器配置

#ifndef _GxEPD2_DISPLAY_SELECTION_GDEQ0426T82_H_
#define _GxEPD2_DISPLAY_SELECTION_GDEQ0426T82_H_

// ✅ GDEQ0426T82 有專用的 GxEPD2 驅動程式！
// 位於: GxEPD2/src/gdeq/GxEPD2_426_GDEQ0426T82.h
// GitHub: https://github.com/ZinggJM/GxEPD2/tree/master/src/gdeq

// GDEQ0426T82 實際規格 (根據 GxEPD2_426_GDEQ0426T82.h):
// - 尺寸: 4.26" 
// - 解析度: 800x480 像素 (寬 x 高)
// - 控制器: SSD1677d:\1_myproject\epaper_display\epaper_display\GDEQ0426T82_WeMos_D1_Mini\GDEQ0426T82_WeMos_D1_Mini.ino
// - 支援: 快速部分更新、全螢幕更新d:\1_myproject\epaper_display\epaper_display\GDEQ0426T82_WeMos_D1_Mini\GDEQ0426T82_WeMos_D1_Mini.ino
// - 電源開啟時間: 100ms
// - 電源關閉時間: 200ms

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

// 使用專用的 GDEQ0426T82 驅動程式
#include <gdeq/GxEPD2_426_GDEQ0426T82.h>

// 顯示器類別配置
#define GxEPD2_DISPLAY_CLASS GxEPD2_BW
#define GxEPD2_DRIVER_CLASS GxEPD2_426_GDEQ0426T82
#define MAX_DISPLAY_BUFFER_SIZE 65536ul

// 顯示器實際尺寸
#define DISPLAY_WIDTH  800
#define DISPLAY_HEIGHT 480

// 驅動程式資訊
#define DRIVER_NAME "GxEPD2_426_GDEQ0426T82"
#define CONTROLLER_TYPE "SSD1677"
#define DISPLAY_TYPE "GDEQ0426T82"

// 功能支援 (根據 GxEPD2_426_GDEQ0426T82.h)
#define HAS_COLOR false
#define HAS_PARTIAL_UPDATE true
#define HAS_FAST_PARTIAL_UPDATE true
#define USE_FAST_FULL_UPDATE true

// 電源時序
#define POWER_ON_TIME_MS 100
#define POWER_OFF_TIME_MS 200

// 顯示器初始化巨集
#define GDEQ0426T82_DISPLAY_INIT(cs, dc, rst, busy) \
  GxEPD2_BW<GxEPD2_426_GDEQ0426T82, GxEPD2_426_GDEQ0426T82::HEIGHT> display(GxEPD2_426_GDEQ0426T82(cs, dc, rst, busy))

// 使用說明:
// 1. 確保安裝了 GxEPD2 函式庫 (by Jean-Marc Zingg)
// 2. 在主程式中 #include 此檔案
// 3. 使用 GDEQ0426T82_DISPLAY_INIT 巨集初始化顯示器
// 4. 或直接使用: 
//    GxEPD2_BW<GxEPD2_426_GDEQ0426T82, GxEPD2_426_GDEQ0426T82::HEIGHT> display(GxEPD2_426_GDEQ0426T82(EPD_CS, EPD_DC, EPD_RST, EPD_BUSY));

#endif // _GxEPD2_DISPLAY_SELECTION_GDEQ0426T82_H_
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
