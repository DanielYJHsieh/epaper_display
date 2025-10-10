# WebSocket 封包大小測試報告

## 📊 測試目的

找出 ESP8266 在 WebSocket 傳輸中能穩定接收的最大封包大小，以優化顯示更新時間。

---

## 🧪 測試方法

### 測試工具
- **Server**: `packet_size_test.py` - 自動化測試程式
- **Client**: ESP8266 (測試模式，接收但不顯示)
- **協議**: WebSocket binary frames

### 測試配置
- **WiFi**: 802.11n, 2.4GHz
- **ESP8266**: WeMos D1 Mini (80MHz, 4MB Flash)
- **可用記憶體**: ~23KB (運行時)
- **測試間隔**: 3 秒

---

## 📈 測試結果

### 第一輪測試 (預設配置)

| 尺寸 | 資料大小 | 結果 | 說明 |
|------|---------|------|------|
| 800×120 | 12,000 bytes (12KB) | ✅ 成功 | 原始穩定配置 |
| 800×130 | 13,000 bytes (13KB) | ✅ 成功 | |
| 800×140 | 14,000 bytes (14KB) | ✅ 成功 | |
| 800×150 | 15,000 bytes (15KB) | ✅ 成功 | **預設上限** |
| 800×160 | 16,000 bytes (16KB) | ❌ 失敗 | WebSocket 斷線 |

**結論**: 預設 `WEBSOCKETS_MAX_DATA_SIZE = 15KB`

---

### 第二輪測試 (提升到 18KB)

修改 arduinoWebSockets 函式庫：
```cpp
#define WEBSOCKETS_MAX_DATA_SIZE (18 * 1024)  // 18KB
```

| 尺寸 | 資料大小 | 結果 |
|------|---------|------|
| 800×160 | 16,000 bytes (16KB) | ✅ 成功 |
| 800×170 | 17,000 bytes (17KB) | ✅ 成功 |
| 800×180 | 18,000 bytes (18KB) | ✅ 成功 |

**結論**: 提升上限成功，繼續測試。

---

### 第三輪測試 (提升到 24KB)

修改函式庫：
```cpp
#define WEBSOCKETS_MAX_DATA_SIZE (24 * 1024)  // 24KB
```

| 尺寸 | 資料大小 | 結果 | 說明 |
|------|---------|------|------|
| 800×190 | 19,000 bytes (19KB) | ✅ 成功 | |
| 800×200 | 20,000 bytes (20KB) | ✅ 成功 | |
| 800×210 | 21,000 bytes (21KB) | ✅ 成功 | **最大穩定值** |
| 800×220 | 22,000 bytes (22KB) | ❌ 失敗 | WebSocket 斷線 |
| 800×230 | 23,000 bytes (23KB) | ⏭️ 未測試 | 已找到上限 |
| 800×240 | 24,000 bytes (24KB) | ⏭️ 未測試 | 已找到上限 |

**最終結論**: **穩定上限 = 21KB**

---

## 🔍 技術分析

### 為何剛好是 21KB？

#### 1. TCP/IP 協議棧限制
```
21,000 bytes ÷ 1460 bytes/packet ≈ 14.4 packets
22,000 bytes ÷ 1460 bytes/packet ≈ 15.1 packets
```
- ESP8266 lwIP 能緩衝約 **14-15 個 TCP 封包**
- 超過此數量可能導致緩衝區溢出

#### 2. 記憶體碎片化
```
運行時可用記憶體: ~23KB
WebSocket 接收緩衝: 需要連續記憶體空間
21KB: 可以找到連續空間 ✅
22KB: 記憶體碎片化，無法分配 ❌
```

#### 3. WebSocket 函式庫限制
```cpp
// arduinoWebSockets/src/WebSocketsClient.h
#ifndef WEBSOCKETS_MAX_DATA_SIZE
#define WEBSOCKETS_MAX_DATA_SIZE (15 * 1024)  // 預設 15KB
#endif
```
- 修改此值可提升上限
- 但仍受 ESP8266 記憶體和 TCP 協議棧限制

---

## 🎯 最終配置

### 選擇 16KB 的原因

雖然測試顯示 21KB 可用，但選擇 **16KB** 作為生產配置：

#### 1. 完整除數
```
800×480 螢幕:
- 480 ÷ 160 = 3 (完美除數)
- 480 ÷ 210 = 2.29 (無法完整分割)
```

#### 2. 安全餘量
```
16KB < 21KB (測試上限)
留有 5KB 安全餘量 (31%)
```

#### 3. 效能提升
```
原配置: 4 個 800×120 (12KB) = 4 次傳輸
新配置: 3 個 800×160 (16KB) = 3 次傳輸
節省時間: 25%
```

---

## 📝 配置詳情

### ESP8266 端 (client_esp8266)

#### config.h
```cpp
#define TILE_WIDTH 800
#define TILE_HEIGHT 160              // 480÷3 = 160
#define TILE_BUFFER_SIZE 16000       // 16KB per tile
```

#### protocol.h
```cpp
#define TILE_INDEX_BAND_0  0  // Y: 0-160   (上)
#define TILE_INDEX_BAND_1  1  // Y: 160-320 (中)
#define TILE_INDEX_BAND_2  2  // Y: 320-480 (下)
#define TILE_COUNT         3
```

#### 函式庫修改
```cpp
// libraries/arduinoWebSockets/src/WebSocketsClient.h
#define WEBSOCKETS_MAX_DATA_SIZE (16 * 1024)  // 16KB
```

### Server 端

#### image_processor.py
```python
def split_image_to_tiles(self, image):
    tiles = {}
    tiles[0] = image.crop((0, 0, 800, 160))    # 上
    tiles[1] = image.crop((0, 160, 800, 320))  # 中
    tiles[2] = image.crop((0, 320, 800, 480))  # 下
    return tiles
```

---

## 📊 效能比較

| 配置 | 封包大小 | 區塊數 | 總時間 (估算) | 穩定性 |
|------|---------|--------|--------------|--------|
| **舊配置** | 12KB | 4 | ~72秒 (4×18秒) | ✅ 非常穩定 |
| **新配置** | 16KB | 3 | ~54秒 (3×18秒) | ✅ 穩定 |
| 理論極限 | 21KB | 3 | ~54秒 (3×18秒) | ⚠️ 接近上限 |

**效能提升**: 25% (18秒)

---

## ⚙️ 如何修改 WEBSOCKETS_MAX_DATA_SIZE

### 方法 1: 修改函式庫檔案 (推薦)

1. 找到函式庫位置：
```
Windows: %USERPROFILE%\Documents\Arduino\libraries\arduinoWebSockets\
Linux/Mac: ~/Arduino/libraries/arduinoWebSockets/
```

2. 編輯檔案：
```
src/WebSocketsClient.h
```

3. 修改定義：
```cpp
#ifndef WEBSOCKETS_MAX_DATA_SIZE
#define WEBSOCKETS_MAX_DATA_SIZE (16 * 1024)  // 改為 16KB
#endif
```

### 方法 2: 在專案中定義 (不推薦)

在 `.ino` 檔案開頭加入：
```cpp
#define WEBSOCKETS_MAX_DATA_SIZE (16 * 1024)
#include <WebSocketsClient.h>
```

**注意**: 此方法可能無效，因為函式庫已經定義了預設值。

---

## ✅ 測試驗證

### 測試步驟

1. **上傳測試韌體** (PACKET_SIZE_TEST_MODE = 1)
2. **執行測試程式**: `python packet_size_test.py`
3. **觀察結果**: 找出最大穩定封包大小
4. **切換正常模式** (PACKET_SIZE_TEST_MODE = 0)
5. **測試顯示**: 發送實際圖片驗證

### 測試結果 (2025-10-10)

✅ **3×16KB 配置測試成功**
- 封包傳輸: 穩定無斷線
- 顯示效果: 完整覆蓋 800×480 螢幕
- 記憶體使用: 正常 (~23KB 可用)
- 更新時間: ~54秒 (比原配置快 25%)

---

## 📚 參考資料

### ESP8266 規格
- **RAM**: 80KB (系統佔用 ~50KB)
- **可用堆疊**: ~30KB (運行時)
- **TCP MSS**: 1460 bytes
- **TCP 緩衝**: ~14-15 packets

### WebSocket 協議
- [RFC 6455](https://tools.ietf.org/html/rfc6455)
- 理論上無大小限制
- 實際受 TCP/IP 協議棧和記憶體限制

### arduinoWebSockets 函式庫
- [GitHub Repository](https://github.com/Links2004/arduinoWebSockets)
- 預設 `WEBSOCKETS_MAX_DATA_SIZE = 15KB`
- 可透過修改源碼提升上限

---

## 📅 測試記錄

- **測試日期**: 2025-10-10
- **測試者**: System
- **測試環境**: 
  - ESP8266: WeMos D1 Mini
  - WiFi: 802.11n, 2.4GHz
  - Server: Python 3.13, Windows
- **測試工具版本**: 
  - arduinoWebSockets: 2.3.7
  - websockets (Python): 13.1

---

## 🎉 結論

通過系統性測試，找出了 ESP8266 WebSocket 的穩定上限 (21KB)，並選擇了最佳實用配置 (16KB)。

**最終配置: 3×16KB (800×160)**
- ✅ 完整覆蓋 800×480 螢幕
- ✅ 穩定性極佳 (留有 31% 安全餘量)
- ✅ 效能提升 25%
- ✅ 記憶體使用合理

這是 **穩定性** 和 **效能** 的最佳平衡點！
