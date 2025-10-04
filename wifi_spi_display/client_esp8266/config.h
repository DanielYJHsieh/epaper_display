/*
 * WiFi SPI Display - 設定檔
 * 
 * 設定 WiFi 連線和 Server 資訊
 */

#ifndef CONFIG_H
#define CONFIG_H

// ============================================
// WiFi 設定
// ============================================
#define WIFI_SSID "lulumi_ap"        // 修改為你的 WiFi 名稱
#define WIFI_PASSWORD "1978120505" // 修改為你的 WiFi 密碼

// ============================================
// Server 設定
// ============================================
#define SERVER_HOST "192.168.0.43"  // 修改為你的 PC IP 位址
#define SERVER_PORT 8266             // Server 監聽埠號

// ============================================
// 顯示器設定
// ============================================
#define DISPLAY_WIDTH 800
#define DISPLAY_HEIGHT 480
#define DISPLAY_BUFFER_SIZE (DISPLAY_WIDTH * DISPLAY_HEIGHT / 8)  // 48000 bytes

// ============================================
// 記憶體設定
// ============================================
#define RX_BUFFER_SIZE 512           // WebSocket 接收緩衝（bytes）
#define STREAM_BUFFER_SIZE 256       // 流式解壓緩衝（bytes）

// ============================================
// 連接設定
// ============================================
#define WIFI_CONNECT_TIMEOUT 30000   // WiFi 連接逾時（毫秒）
#define WS_RECONNECT_INTERVAL 3000   // WebSocket 重連間隔（毫秒）
#define MAX_RETRY_COUNT 3            // 最大重試次數

// ============================================
// 除錯設定
// ============================================
#define DEBUG_SERIAL 1               // 啟用序列埠除錯（0=關閉, 1=開啟）
#define SERIAL_BAUD 115200          // 序列埠速率

// ============================================
// WiFi 優化設定
// ============================================
#define USE_802_11N 1                // 啟用 802.11n 模式
#define WIFI_TX_POWER 20.5          // WiFi 發射功率（dBm）

// ============================================
// 記憶體監控
// ============================================
#define MEMORY_CHECK_INTERVAL 10000  // 記憶體檢查間隔（毫秒）
#define MEMORY_WARNING_THRESHOLD 10000  // 最小可用記憶體警告（bytes）

#endif // CONFIG_H
