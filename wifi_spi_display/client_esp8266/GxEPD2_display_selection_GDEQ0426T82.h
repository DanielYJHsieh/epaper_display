// GxEPD2_display_selection_GDEQ0426T82.h
// WeMos D1 Mini + GDEQ0426T82 4.26" E-Paper 顯示器配置

#ifndef _GxEPD2_DISPLAY_SELECTION_GDEQ0426T82_H_
#define _GxEPD2_DISPLAY_SELECTION_GDEQ0426T82_H_

// ✅ GDEQ0426T82 有專用的 GxEPD2 驅動程式！
// 位於: GxEPD2/src/gdeq/GxEPD2_426_GDEQ0426T82.h
// GitHub: https://github.com/ZinggJM/GxEPD2/tree/master/src/gdeq

// GDEQ0426T82 實際規格:
// - 尺寸: 4.26" 
// - 解析度: 800x480 像素 (寬 x 高)
// - 控制器: SSD1677
// - 支援: 快速部分更新、全螢幕更新
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
#include <gdeq/GxEPD2_426_GDEQ0426T82.h>

// 顯示器實例化 - 使用分頁模式以節省記憶體
// 第二個參數設為較小值（如 1）表示不使用內部緩衝
// 我們將使用自己的 currentFrame 緩衝
GxEPD2_BW<GxEPD2_426_GDEQ0426T82, 1> display(
  GxEPD2_426_GDEQ0426T82(EPD_CS, EPD_DC, EPD_RST, EPD_BUSY)
);

#endif // _GxEPD2_DISPLAY_SELECTION_GDEQ0426T82_H_
