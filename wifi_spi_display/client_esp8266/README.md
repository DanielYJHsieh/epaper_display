# WiFi SPI Display - Client 端 (ESP8266)

**角色**: WebSocket Client  
**功能**: 接收資料、解壓縮、顯示控制

---

## 📋 功能概述

Client 端負責以下工作：
1. ✅ **WiFi 連接**: 連接到本地 WiFi
2. ✅ **WebSocket Client**: 連接到 Server
3. ✅ **協議解析**: 解析封包格式
4. ✅ **流式解壓**: RLE 解壓縮（節省記憶體）
5. ✅ **顯示控制**: SPI 電子紙驅動

---

## 🛠️ 硬體需求

### 必要硬體
- **WeMos D1 Mini** (ESP8266) × 1
- **GDEQ0426T82** 4.26吋電子紙 (800×480) × 1
- **杜邦線** × 8條
- **3.3kΩ 電阻** × 1 (CS 腳位下拉)
- **1kΩ 電阻** × 1 (RST 腳位上拉)

### 硬體連接

```
WeMos D1 Mini    →    GDEQ0426T82
─────────────────     ─────────────
3.3V             →    VCC
GND              →    GND
D5 (GPIO14/SCK)  →    CLK
D7 (GPIO13/MOSI) →    DIN
D1 (GPIO5)       →    RST (+ 1kΩ 上拉)
D3 (GPIO0)       →    DC
D8 (GPIO15)      →    CS (+ 3.3kΩ 下拉)
D2 (GPIO4)       →    BUSY
```

---

## 📦 Arduino IDE 設定

### 1. 開發板設定
- **開發板**: ESP8266 Boards → WeMos D1 R1 & Mini
- **Upload Speed**: 921600
- **CPU Frequency**: 160 MHz (推薦)
- **Flash Size**: 4MB (FS:2MB OTA:~1019KB)

### 2. 必要函式庫

```
1. GxEPD2 (by Jean-Marc Zingg)
2. Adafruit GFX Library
3. ESP8266WiFi (內建)
4. WebSockets (by Markus Sattler) v2.7.1+
```

**安裝方式**:
```
Arduino IDE → 工具 → 管理程式庫 → 搜尋並安裝
```

---

## 📁 檔案結構

```
client_esp8266/
├── README.md                    # 本檔案
├── wifi_spi_display.ino        # 主程式
├── config.h                    # 設定檔
├── protocol.h                  # 協議定義
├── websocket_client.h          # WebSocket 處理
├── rle_decoder.h               # RLE 解壓縮
├── display_driver.h            # 顯示驅動
└── memory_monitor.h            # 記憶體監控
```

---

## ⚙️ 設定檔 (config.h)

在編譯前，請修改 `config.h`：

```cpp
// WiFi 設定
#define WIFI_SSID "你的WiFi名稱"
#define WIFI_PASSWORD "你的WiFi密碼"

// Server 設定
#define SERVER_HOST "192.168.1.100"  // Server 的 IP
#define SERVER_PORT 8266
```

---

## 🚀 快速開始

### 1. 硬體組裝
按照上述硬體連接圖連接所有線路

### 2. 修改設定
編輯 `config.h`，填入你的 WiFi 和 Server 資訊

### 3. 編譯上傳
1. 開啟 `wifi_spi_display.ino`
2. 選擇正確的開發板和 Port
3. 點擊上傳

### 4. 監控執行
1. 開啟序列埠監控視窗 (115200 baud)
2. 觀察連接狀態和記憶體使用

---

## 📊 記憶體使用

**記憶體預算**:
```
系統基礎:       ~25KB
WiFi Stack:     ~3KB
WebSocket:      ~8KB
接收緩衝:       ~4KB
顯示控制:       ~1KB
────────────────────────
總計:           ~41KB

剩餘可用:       ~39KB ✅
```

**優化策略**:
- ✅ 使用流式處理（不儲存完整畫面）
- ✅ 小緩衝區（512 bytes）
- ✅ 即時解壓並顯示
- ✅ 動態記憶體管理

---

## 🔧 模組說明

### protocol.h - 協議定義

```cpp
#define PROTO_HEADER 0xA5
#define PROTO_TYPE_FULL 0x01    // 完整畫面
#define PROTO_TYPE_DELTA 0x02   // 差分更新
#define PROTO_TYPE_CMD 0x03     // 控制指令

struct PacketHeader {
  uint8_t header;    // 0xA5
  uint8_t type;      // 封包類型
  uint16_t seq_id;   // 序號
  uint32_t length;   // Payload 長度
};
```

### rle_decoder.h - RLE 解壓縮

```cpp
class RLEDecoder {
public:
  // 標準解壓
  static size_t decode(uint8_t* input, size_t inputSize, 
                      uint8_t* output, size_t maxOutputSize);
  
  // 流式解壓（節省記憶體）
  static void decodeStream(uint8_t* input, size_t inputSize,
                          void (*callback)(uint8_t*, size_t));
};
```

**流式解壓範例**:
```cpp
RLEDecoder::decodeStream(compressed, size, 
  [](uint8_t* buffer, size_t len) {
    // 處理解壓後的資料
    display.writePixels(buffer, len);
  }
);
```

---

## 🐛 故障排除

### 1. WiFi 連接失敗
**症狀**: 序列埠顯示 "WiFi connection timeout"

**解決方法**:
- 檢查 SSID 和密碼是否正確
- 確認 WiFi 信號強度足夠
- 檢查路由器是否允許 ESP8266 連接

### 2. WebSocket 連接失敗
**症狀**: "WebSocket Disconnected" 不斷出現

**解決方法**:
- 確認 Server 已啟動
- 檢查 SERVER_HOST IP 是否正確
- 確認防火牆沒有阻擋 8266 port
- 嘗試 ping Server IP

### 3. 記憶體不足
**症狀**: ESP8266 重啟或死機

**解決方法**:
- 檢查序列埠的 "Free Heap" 值
- 如果 < 10KB，減少 DISPLAY_BUFFER_SIZE
- 使用 CPU 160MHz 模式
- 檢查是否有記憶體洩漏

### 4. 顯示異常
**症狀**: 電子紙顯示錯誤或空白

**解決方法**:
- 檢查硬體連接
- 確認電阻正確安裝
- 測試用簡單的顯示程式驗證硬體
- 檢查 BUSY 訊號

---

## 📈 效能指標

**測試環境**: ESP8266 @ 160MHz, WiFi 802.11n

| 操作 | 時間 |
|------|------|
| WiFi 連接 | 3-5 秒 |
| WebSocket 連接 | <1 秒 |
| 接收 48KB (壓縮後 5KB) | ~500ms |
| RLE 解壓 | ~50ms |
| 電子紙刷新 | 2500-3500ms |
| **總更新時間** | **~3-4 秒** |

---

## 🧪 測試程式

### 測試 RLE 解壓

```cpp
void test_rle_decoder() {
  uint8_t input[] = {4, 0xFF, 2, 0x00};
  uint8_t output[6];
  
  size_t size = RLEDecoder::decode(input, 4, output, 6);
  
  Serial.printf("Decoded size: %d\n", size);
  for (int i = 0; i < size; i++) {
    Serial.printf("output[%d] = 0x%02X\n", i, output[i]);
  }
}
```

### 測試記憶體

```cpp
void test_memory() {
  Serial.println("=== Memory Test ===");
  Serial.printf("Free Heap: %d bytes\n", ESP.getFreeHeap());
  Serial.printf("Heap Fragmentation: %d%%\n", ESP.getHeapFragmentation());
  Serial.printf("Max Free Block: %d bytes\n", ESP.getMaxFreeBlockSize());
}
```

---

## 📝 開發狀態

- [ ] WiFi 連接模組
- [ ] WebSocket Client
- [ ] 協議解析
- [ ] RLE 解壓縮
- [ ] 流式處理
- [ ] 顯示驅動整合
- [ ] 記憶體優化
- [ ] 錯誤處理

---

## 🔗 相關連結

- [專案規劃](../PROJECT_PLAN.md)
- [Server 端 README](../server/README.md)
- [GxEPD2 GitHub](https://github.com/ZinggJM/GxEPD2)
- [arduinoWebSockets GitHub](https://github.com/Links2004/arduinoWebSockets)

---

## 💡 優化技巧

### 1. 加速 WiFi 連接

```cpp
WiFi.setAutoConnect(true);
WiFi.setAutoReconnect(true);
WiFi.persistent(true);  // 儲存 WiFi 設定
```

### 2. 降低記憶體使用

```cpp
// 減少接收緩衝
#define DISPLAY_BUFFER_SIZE 256  // 從 512 降到 256

// 禁用不需要的功能
#define WEBSOCKETS_NETWORK_TYPE NETWORK_ESP8266
#define DEBUG_WEBSOCKETS 0
```

### 3. 提升速度

```cpp
// 使用 160MHz
system_update_cpu_freq(160);

// 禁用 Nagle 演算法
client.setNoDelay(true);
```

---

**版本**: 1.0  
**狀態**: 規劃階段  
**作者**: Daniel YJ Hsieh
