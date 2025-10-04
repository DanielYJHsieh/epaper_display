# WiFi SPI Display 專案開發規劃

**專案名稱**: WiFi SPI Display - 高速電子紙無線顯示系統  
**版本**: v1.0 (規劃階段)  
**日期**: 2025-10-04  
**架構**: Server/Client 模式

---

## 📋 目錄

1. [專案概述](#專案概述)
2. [系統架構](#系統架構)
3. [技術選型](#技術選型)
4. [通訊協議設計](#通訊協議設計)
5. [記憶體優化策略](#記憶體優化策略)
6. [Server 端規劃](#server-端規劃)
7. [Client 端規劃](#client-端規劃)
8. [開發流程](#開發流程)
9. [測試計畫](#測試計畫)
10. [風險評估](#風險評估)

---

## 🎯 專案概述

### 目標
建立一個高效能的無線電子紙顯示系統，將運算和資料處理放在 PC/手機端，ESP8266 專注於顯示控制。

### 核心需求
- ✅ **反向架構**: PC/手機為 Server，ESP8266 為 Client
- ✅ **高速傳輸**: 使用最快的通訊協議
- ✅ **低記憶體**: ESP8266 RAM 使用最小化
- ✅ **運算外移**: 圖像處理、編碼等在 Server 端完成
- ✅ **大螢幕支援**: 800×480 GDEQ0426T82 電子紙

### 硬體規格
- **ESP8266**: WeMos D1 Mini (~80KB 可用 RAM, 160MHz CPU)
- **電子紙**: GDEQ0426T82 (800×480, 黑白, SPI 介面)
- **解析度**: 800×480 = 384,000 像素 = **48,000 bytes** (黑白 1-bit)

---

## 🏗️ 系統架構

### 架構圖

```
┌─────────────────────────────────────────────────────────────────┐
│                         Server 端                                │
│                    (PC / 手機 / 平板)                            │
│                                                                   │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐          │
│  │  圖像處理    │  │  資料壓縮    │  │  WebSocket   │          │
│  │  - 縮放      │  │  - RLE       │  │  Server      │          │
│  │  - 轉黑白    │  │  - Delta     │  │  Port 8266   │          │
│  │  - 抖動      │  │  - LZ77      │  │              │          │
│  └──────────────┘  └──────────────┘  └──────────────┘          │
│           │                 │                 │                  │
└───────────┼─────────────────┼─────────────────┼──────────────────┘
            │                 │                 │
            └─────────────────┴─────────────────┘
                              │
                         WiFi Network
                         (802.11n)
                              │
            ┌─────────────────┴─────────────────┐
            │                                    │
┌───────────┼────────────────────────────────────┼──────────────────┐
│           │          Client 端                 │                  │
│           │        (ESP8266)                   │                  │
│           ▼                                    ▼                  │
│  ┌──────────────┐                    ┌──────────────┐            │
│  │  WebSocket   │────────────────────>│  數據接收    │            │
│  │  Client      │    接收壓縮數據     │  緩衝區      │            │
│  │              │                     │  (4KB)       │            │
│  └──────────────┘                    └──────────────┘            │
│                                             │                     │
│                                             ▼                     │
│                                    ┌──────────────┐              │
│                                    │  解壓縮模組  │              │
│                                    │  (即時解壓)  │              │
│                                    └──────────────┘              │
│                                             │                     │
│                                             ▼                     │
│                                    ┌──────────────┐              │
│                                    │  SPI 顯示    │              │
│                                    │  控制        │              │
│                                    └──────────────┘              │
│                                             │                     │
│                                             ▼                     │
│                               ┌────────────────────────┐         │
│                               │  GDEQ0426T82 E-Paper   │         │
│                               │  800×480 Display       │         │
│                               └────────────────────────┘         │
└──────────────────────────────────────────────────────────────────┘
```

### 資料流程

```
Server 端:
圖片/文字 → 轉黑白 → 壓縮 → WebSocket → WiFi
                ↓
              預處理
            (抖動演算法)

Client 端:
WiFi → WebSocket → 解壓縮 → SPI → 電子紙
              ↓
         4KB 緩衝區
        (流式處理)
```

---

## 🔧 技術選型

### 1. 通訊協議比較

| 協議 | 速度 | 雙向 | 延遲 | RAM 使用 | 適合度 | 備註 |
|------|------|------|------|---------|--------|------|
| **WebSocket** | ⭐⭐⭐⭐⭐ | ✅ | 極低 | 中等 (~8KB) | **推薦** | 雙向即時，支援二進位 |
| Raw TCP | ⭐⭐⭐⭐⭐ | ✅ | 極低 | 低 (~4KB) | **推薦** | 最快但需自行實作協議 |
| HTTP POST | ⭐⭐⭐ | ❌ | 高 | 低 (~3KB) | 不推薦 | 每次建立連線，慢 |
| MQTT | ⭐⭐⭐⭐ | ✅ | 低 | 高 (~12KB) | 不推薦 | 需 Broker |
| UDP | ⭐⭐⭐⭐⭐ | ✅ | 極低 | 極低 (~2KB) | 考慮 | 不可靠，需自行實作重傳 |

**最終選擇**: **WebSocket (二進位模式)** 或 **Raw TCP Socket**

**理由**:
- WebSocket: 易於實作，支援瀏覽器測試，二進位模式速度快
- Raw TCP: 最快，完全控制，但需自行設計協議

**建議**: 先用 WebSocket 開發，後期可優化為 Raw TCP

---

### 2. 資料壓縮方案

#### 方案 A: RLE (Run-Length Encoding) ⭐⭐⭐⭐⭐ **推薦**

**優點**:
- 實作簡單（~100 行程式碼）
- RAM 使用極低（幾乎不需額外記憶體）
- 解壓速度快（~1ms/KB）
- 適合大面積相同顏色（電子紙常見）

**壓縮率**:
- 文字顯示: 85-95% 壓縮
- 複雜圖片: 40-60% 壓縮

**實作範例**:
```
原始: [0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00]
壓縮: [0x04, 0xFF, 0x02, 0x00]
      (重複4次0xFF, 重複2次0x00)
```

#### 方案 B: Delta Encoding (差分編碼) ⭐⭐⭐⭐

**優點**:
- 只傳輸改變的部分
- 適合局部更新
- 第一次傳完整，後續只傳差異

**壓縮率**:
- 文字更新: 95-99% 減少
- 動畫: 70-90% 減少

#### 方案 C: LZ77 / DEFLATE ⭐⭐⭐

**優點**:
- 壓縮率高（70-90%）

**缺點**:
- RAM 需求大（16-32KB 字典）
- 解壓慢
- 不適合 ESP8266

#### 方案 D: 混合方案 ⭐⭐⭐⭐⭐ **最佳**

**RLE + Delta Encoding**:
1. 首次傳輸: RLE 壓縮的完整畫面
2. 更新: Delta (只傳變化) + RLE 壓縮

**預期效果**:
- 首次: 48KB → 5-10KB
- 更新: 48KB → 0.5-2KB

---

### 3. Server 端技術選型

#### 選項 A: Python + WebSocket Server ⭐⭐⭐⭐⭐ **推薦**

**優點**:
- 快速開發
- 豐富的圖像處理庫 (Pillow, OpenCV)
- 跨平台 (PC/Mac/Linux)
- WebSocket 庫成熟 (websockets, Flask-SocketIO)

**範例技術棧**:
```
- Python 3.8+
- Pillow (圖像處理)
- websockets (WebSocket server)
- asyncio (異步處理)
```

#### 選項 B: Node.js + Socket.io ⭐⭐⭐⭐

**優點**:
- 異步處理天生支援
- npm 生態系豐富
- 可打包為桌面應用 (Electron)

**缺點**:
- 圖像處理庫較少
- 效能略低於 Python

#### 選項 C: 手機 App (Android/iOS) ⭐⭐⭐

**優點**:
- 行動化
- 可使用相機

**缺點**:
- 開發週期長
- 需要兩個平台

**建議**: **先開發 Python 版本**，確認可行後再考慮手機 App

---

### 4. Client 端技術選型

#### ESP8266 函式庫

| 功能 | 函式庫 | RAM 使用 | 備註 |
|------|--------|---------|------|
| WiFi | ESP8266WiFi (內建) | ~3KB | 必要 |
| WebSocket | WebSocketsClient | ~8KB | 推薦 (arduinoWebSockets) |
| TCP Socket | WiFiClient (內建) | ~2KB | 最省記憶體 |
| Display | GxEPD2 | ~1KB | 已有經驗 |

**記憶體預算**:
```
系統基礎:     ~25KB
WiFi Stack:   ~3KB
WebSocket:    ~8KB
接收緩衝:     ~4KB
顯示控制:     ~1KB
────────────────────
總計:         ~41KB

剩餘可用:     ~39KB (安全範圍)
```

---

## 📡 通訊協議設計

### WebSocket 二進位協議

#### 封包格式

```
┌──────────┬──────────┬──────────┬──────────┬────────────────┐
│  Header  │   Type   │  Seq ID  │  Length  │     Payload    │
│  (1B)    │  (1B)    │  (2B)    │  (4B)    │   (Variable)   │
└──────────┴──────────┴──────────┴──────────┴────────────────┘

Header:  0xA5 (固定魔術數字)
Type:    0x01=完整畫面, 0x02=差分更新, 0x03=指令
Seq ID:  封包序號 (用於重傳檢測)
Length:  Payload 長度 (bytes)
Payload: 壓縮後的圖像資料
```

#### 訊息類型

**1. 完整畫面更新 (Type 0x01)**
```
Server → Client:
[0xA5][0x01][SeqID][Length][RLE壓縮的48KB資料]

Client → Server:
[0xA5][0x10][SeqID][0x01] (ACK: 成功)
[0xA5][0x11][SeqID][0x00] (NAK: 失敗，重傳)
```

**2. 差分更新 (Type 0x02)**
```
Server → Client:
[0xA5][0x02][SeqID][Length][改變區域列表]

改變區域格式:
[X座標(2B)][Y座標(2B)][寬度(2B)][高度(2B)][RLE壓縮資料]
```

**3. 控制指令 (Type 0x03)**
```
0x10: 清空螢幕
0x11: 休眠模式
0x12: 喚醒
0x13: 局部更新模式
0x14: 全螢幕更新模式
```

---

## 💾 記憶體優化策略

### 1. 流式處理 (Streaming)

**問題**: 完整畫面需要 48KB，但只有 80KB RAM

**解決方案**: 分塊接收、解壓、顯示

```cpp
// 不儲存完整畫面，即時處理
void processImageStream() {
  uint8_t buffer[512];  // 512 bytes 緩衝
  
  while (hasMoreData()) {
    // 接收一塊
    websocket.receive(buffer, 512);
    
    // 立即解壓縮
    decompressRLE(buffer, outputBuffer, 512);
    
    // 立即寫入顯示
    display.writePixels(outputBuffer, 512);
  }
}
```

**記憶體使用**: 僅需 1-2KB 緩衝

---

### 2. RLE 解壓縮（零額外記憶體）

```cpp
void decompressRLE(uint8_t* compressed, uint8_t* output, size_t maxSize) {
  size_t outIdx = 0;
  size_t inIdx = 0;
  
  while (inIdx < compressedSize && outIdx < maxSize) {
    uint8_t count = compressed[inIdx++];
    uint8_t value = compressed[inIdx++];
    
    // 直接寫入輸出緩衝
    for (uint8_t i = 0; i < count; i++) {
      output[outIdx++] = value;
    }
  }
}
```

**記憶體使用**: 0 bytes (in-place)

---

### 3. 差分更新記憶體

```cpp
// 只儲存改變的座標
struct DeltaRegion {
  uint16_t x, y, w, h;
  uint8_t* data;  // 動態分配
};

// 最多 10 個改變區域
DeltaRegion regions[10];
```

**記憶體使用**: ~100 bytes (不含 data)

---

### 4. WiFi 緩衝優化

```cpp
// 設定 TCP 接收緩衝大小
WiFiClient client;
client.setNoDelay(true);  // 禁用 Nagle 演算法
client.setTimeout(5000);   // 5 秒逾時

// 使用較小的接收緩衝
#define RX_BUFFER_SIZE 2048  // 預設是 5744
```

---

## 🖥️ Server 端規劃

### 專案結構

```
wifi_spi_display/
└── server/
    ├── README.md                 # Server 端說明
    ├── requirements.txt          # Python 依賴
    ├── server.py                # 主程式
    ├── image_processor.py       # 圖像處理
    ├── compressor.py            # 壓縮模組
    ├── protocol.py              # 協議處理
    ├── config.json              # 設定檔
    ├── static/                  # 網頁資源
    │   ├── index.html          # 測試網頁
    │   └── style.css
    └── tests/                   # 測試程式
        ├── test_image.py
        └── test_compress.py
```

### 核心模組

#### 1. image_processor.py - 圖像處理

```python
from PIL import Image, ImageDraw, ImageFont
import numpy as np

class ImageProcessor:
    def __init__(self, width=800, height=480):
        self.width = width
        self.height = height
    
    def convert_to_1bit(self, image):
        """轉換為 1-bit 黑白"""
        # 縮放到目標尺寸
        image = image.resize((self.width, self.height))
        
        # 轉灰階
        image = image.convert('L')
        
        # 抖動演算法 (Floyd-Steinberg)
        image = image.convert('1', dither=Image.FLOYDSTEINBERG)
        
        return image
    
    def image_to_bytes(self, image):
        """轉換為 byte array"""
        pixels = np.array(image)
        # 打包為 1-bit (8 pixels per byte)
        packed = np.packbits(pixels)
        return packed.tobytes()
    
    def create_text_image(self, text, font_size=48):
        """從文字建立圖像"""
        img = Image.new('1', (self.width, self.height), 1)
        draw = ImageDraw.Draw(img)
        font = ImageFont.truetype("arial.ttf", font_size)
        
        # 置中文字
        bbox = draw.textbbox((0, 0), text, font=font)
        x = (self.width - bbox[2]) // 2
        y = (self.height - bbox[3]) // 2
        
        draw.text((x, y), text, font=font, fill=0)
        return img
```

#### 2. compressor.py - 壓縮模組

```python
class RLECompressor:
    @staticmethod
    def compress(data):
        """RLE 壓縮"""
        compressed = []
        i = 0
        
        while i < len(data):
            count = 1
            value = data[i]
            
            # 計算重複次數 (最多 255)
            while i + count < len(data) and \
                  data[i + count] == value and \
                  count < 255:
                count += 1
            
            compressed.append(count)
            compressed.append(value)
            i += count
        
        return bytes(compressed)
    
    @staticmethod
    def compress_ratio(original_size, compressed_size):
        """計算壓縮率"""
        return (1 - compressed_size / original_size) * 100

class DeltaCompressor:
    def __init__(self):
        self.last_frame = None
    
    def compress(self, current_frame):
        """差分壓縮"""
        if self.last_frame is None:
            # 第一幀，回傳完整資料
            self.last_frame = current_frame.copy()
            return None, current_frame
        
        # 找出差異
        diff = []
        for i in range(len(current_frame)):
            if current_frame[i] != self.last_frame[i]:
                diff.append((i, current_frame[i]))
        
        self.last_frame = current_frame.copy()
        return diff, None
```

#### 3. protocol.py - 協議處理

```python
import struct

class Protocol:
    HEADER = 0xA5
    TYPE_FULL = 0x01
    TYPE_DELTA = 0x02
    TYPE_CMD = 0x03
    
    @staticmethod
    def pack_full_frame(seq_id, data):
        """打包完整畫面"""
        header = struct.pack('BBH', 
                            Protocol.HEADER,
                            Protocol.TYPE_FULL,
                            seq_id)
        length = struct.pack('I', len(data))
        return header + length + data
    
    @staticmethod
    def pack_delta(seq_id, regions):
        """打包差分更新"""
        header = struct.pack('BBH',
                            Protocol.HEADER,
                            Protocol.TYPE_DELTA,
                            seq_id)
        
        payload = b''
        for x, y, w, h, data in regions:
            region_header = struct.pack('HHHH', x, y, w, h)
            payload += region_header + data
        
        length = struct.pack('I', len(payload))
        return header + length + payload
```

#### 4. server.py - 主程式

```python
import asyncio
import websockets
from image_processor import ImageProcessor
from compressor import RLECompressor
from protocol import Protocol

class DisplayServer:
    def __init__(self, port=8266):
        self.port = port
        self.clients = set()
        self.processor = ImageProcessor(800, 480)
        self.compressor = RLECompressor()
        self.seq_id = 0
    
    async def handler(self, websocket, path):
        """處理客戶端連接"""
        self.clients.add(websocket)
        print(f"Client connected: {websocket.remote_address}")
        
        try:
            async for message in websocket:
                # 處理來自 ESP8266 的訊息 (ACK/NAK)
                await self.handle_message(websocket, message)
        finally:
            self.clients.remove(websocket)
            print(f"Client disconnected")
    
    async def send_image(self, image_path):
        """發送圖片到所有客戶端"""
        # 處理圖像
        from PIL import Image
        img = Image.open(image_path)
        processed = self.processor.convert_to_1bit(img)
        raw_data = self.processor.image_to_bytes(processed)
        
        # 壓縮
        compressed = self.compressor.compress(raw_data)
        
        print(f"Original: {len(raw_data)} bytes")
        print(f"Compressed: {len(compressed)} bytes")
        print(f"Ratio: {self.compressor.compress_ratio(len(raw_data), len(compressed)):.1f}%")
        
        # 打包協議
        packet = Protocol.pack_full_frame(self.seq_id, compressed)
        self.seq_id += 1
        
        # 發送到所有客戶端
        if self.clients:
            await asyncio.gather(
                *[client.send(packet) for client in self.clients]
            )
    
    async def send_text(self, text):
        """發送文字到所有客戶端"""
        img = self.processor.create_text_image(text)
        raw_data = self.processor.image_to_bytes(img)
        compressed = self.compressor.compress(raw_data)
        packet = Protocol.pack_full_frame(self.seq_id, compressed)
        self.seq_id += 1
        
        if self.clients:
            await asyncio.gather(
                *[client.send(packet) for client in self.clients]
            )
    
    def start(self):
        """啟動 Server"""
        start_server = websockets.serve(self.handler, "0.0.0.0", self.port)
        print(f"WebSocket server started on port {self.port}")
        asyncio.get_event_loop().run_until_complete(start_server)
        asyncio.get_event_loop().run_forever()

if __name__ == "__main__":
    server = DisplayServer(port=8266)
    server.start()
```

---

## 📱 Client 端規劃 (ESP8266)

### 專案結構

```
wifi_spi_display/
└── client_esp8266/
    ├── README.md                    # Client 端說明
    ├── wifi_spi_display.ino        # 主程式
    ├── config.h                    # 設定檔
    ├── protocol.h                  # 協議定義
    ├── websocket_client.h          # WebSocket 處理
    ├── rle_decoder.h               # RLE 解壓縮
    ├── display_driver.h            # 顯示驅動
    └── memory_monitor.h            # 記憶體監控
```

### 核心模組

#### 1. config.h - 設定檔

```cpp
#ifndef CONFIG_H
#define CONFIG_H

// WiFi 設定
#define WIFI_SSID "your_wifi_ssid"
#define WIFI_PASSWORD "your_wifi_password"

// Server 設定
#define SERVER_HOST "192.168.1.100"  // PC IP
#define SERVER_PORT 8266

// 顯示設定
#define DISPLAY_WIDTH 800
#define DISPLAY_HEIGHT 480
#define DISPLAY_BUFFER_SIZE 512  // 接收緩衝

// 記憶體設定
#define MAX_RETRY 3
#define CONNECT_TIMEOUT 30000  // 30 秒

#endif
```

#### 2. protocol.h - 協議定義

```cpp
#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <Arduino.h>

#define PROTO_HEADER 0xA5
#define PROTO_TYPE_FULL 0x01
#define PROTO_TYPE_DELTA 0x02
#define PROTO_TYPE_CMD 0x03

struct PacketHeader {
  uint8_t header;
  uint8_t type;
  uint16_t seq_id;
  uint32_t length;
} __attribute__((packed));

class ProtocolParser {
public:
  static bool parseHeader(uint8_t* data, PacketHeader& header) {
    if (data[0] != PROTO_HEADER) return false;
    
    header.header = data[0];
    header.type = data[1];
    header.seq_id = (data[2] << 8) | data[3];
    header.length = (data[4] << 24) | (data[5] << 16) | 
                    (data[6] << 8) | data[7];
    return true;
  }
};

#endif
```

#### 3. rle_decoder.h - RLE 解壓縮

```cpp
#ifndef RLE_DECODER_H
#define RLE_DECODER_H

#include <Arduino.h>

class RLEDecoder {
public:
  static size_t decode(uint8_t* input, size_t inputSize, 
                      uint8_t* output, size_t maxOutputSize) {
    size_t inIdx = 0;
    size_t outIdx = 0;
    
    while (inIdx < inputSize && outIdx < maxOutputSize) {
      uint8_t count = input[inIdx++];
      uint8_t value = input[inIdx++];
      
      for (uint8_t i = 0; i < count && outIdx < maxOutputSize; i++) {
        output[outIdx++] = value;
      }
    }
    
    return outIdx;
  }
  
  // 流式解壓（節省記憶體）
  static void decodeStream(uint8_t* input, size_t inputSize,
                          void (*callback)(uint8_t*, size_t)) {
    uint8_t buffer[256];  // 小緩衝
    size_t bufIdx = 0;
    size_t inIdx = 0;
    
    while (inIdx < inputSize) {
      uint8_t count = input[inIdx++];
      uint8_t value = input[inIdx++];
      
      for (uint8_t i = 0; i < count; i++) {
        buffer[bufIdx++] = value;
        
        // 緩衝滿了就回調
        if (bufIdx >= 256) {
          callback(buffer, bufIdx);
          bufIdx = 0;
        }
      }
    }
    
    // 處理剩餘
    if (bufIdx > 0) {
      callback(buffer, bufIdx);
    }
  }
};

#endif
```

#### 4. wifi_spi_display.ino - 主程式架構

```cpp
#include <ESP8266WiFi.h>
#include <WebSocketsClient.h>
#include <GxEPD2_BW.h>
#include "config.h"
#include "protocol.h"
#include "rle_decoder.h"

// 顯示器
GxEPD2_BW<GxEPD2_426_GDEQ0426T82, GxEPD2_426_GDEQ0426T82::HEIGHT> display(
  GxEPD2_426_GDEQ0426T82(/*CS=*/ SS, /*DC=*/ 0, /*RST=*/ 5, /*BUSY=*/ 4)
);

// WebSocket 客戶端
WebSocketsClient webSocket;

// 接收緩衝
uint8_t rxBuffer[DISPLAY_BUFFER_SIZE];
size_t rxIndex = 0;

// 當前封包資訊
PacketHeader currentPacket;
uint8_t* packetPayload = nullptr;
size_t payloadReceived = 0;

void setup() {
  Serial.begin(115200);
  Serial.println(F("\n\n=== WiFi SPI Display Client ==="));
  
  // 初始化顯示器
  display.init(115200);
  display.setRotation(0);
  display.setTextColor(GxEPD_BLACK);
  
  // 連接 WiFi
  setupWiFi();
  
  // 連接 WebSocket Server
  setupWebSocket();
}

void loop() {
  webSocket.loop();
  
  // 記憶體監控
  static unsigned long lastCheck = 0;
  if (millis() - lastCheck > 10000) {
    Serial.printf("Free Heap: %d bytes\n", ESP.getFreeHeap());
    lastCheck = millis();
  }
}

void setupWiFi() {
  Serial.printf("Connecting to WiFi: %s\n", WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED) {
    if (millis() - start > CONNECT_TIMEOUT) {
      Serial.println("WiFi connection timeout!");
      ESP.restart();
    }
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("\nWiFi connected!");
  Serial.printf("IP: %s\n", WiFi.localIP().toString().c_str());
}

void setupWebSocket() {
  webSocket.begin(SERVER_HOST, SERVER_PORT, "/");
  webSocket.onEvent(webSocketEvent);
  webSocket.setReconnectInterval(3000);
  
  Serial.printf("Connecting to Server: %s:%d\n", SERVER_HOST, SERVER_PORT);
}

void webSocketEvent(WStype_t type, uint8_t* payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      Serial.println("WebSocket Disconnected");
      break;
      
    case WStype_CONNECTED:
      Serial.println("WebSocket Connected");
      Serial.printf("Server: %s\n", payload);
      break;
      
    case WStype_BIN:
      // 接收二進位資料
      handleBinaryData(payload, length);
      break;
  }
}

void handleBinaryData(uint8_t* data, size_t length) {
  Serial.printf("Received %d bytes\n", length);
  
  // 解析封包頭
  if (payloadReceived == 0) {
    if (length < sizeof(PacketHeader)) {
      Serial.println("Invalid packet header");
      return;
    }
    
    if (!ProtocolParser::parseHeader(data, currentPacket)) {
      Serial.println("Invalid header magic");
      return;
    }
    
    Serial.printf("Packet Type: 0x%02X, SeqID: %d, Length: %d\n",
                  currentPacket.type, currentPacket.seq_id, currentPacket.length);
    
    // 分配 payload 記憶體
    packetPayload = (uint8_t*)malloc(currentPacket.length);
    if (!packetPayload) {
      Serial.println("Failed to allocate payload memory!");
      return;
    }
    
    // 複製 payload（跳過 header）
    size_t headerSize = sizeof(PacketHeader);
    size_t payloadSize = length - headerSize;
    memcpy(packetPayload, data + headerSize, payloadSize);
    payloadReceived = payloadSize;
  } else {
    // 繼續接收 payload
    memcpy(packetPayload + payloadReceived, data, length);
    payloadReceived += length;
  }
  
  // 檢查是否接收完整
  if (payloadReceived >= currentPacket.length) {
    processPacket();
    
    // 釋放記憶體
    free(packetPayload);
    packetPayload = nullptr;
    payloadReceived = 0;
  }
}

void processPacket() {
  Serial.println("Processing packet...");
  
  unsigned long startTime = millis();
  
  if (currentPacket.type == PROTO_TYPE_FULL) {
    // 完整畫面更新
    updateFullDisplay(packetPayload, currentPacket.length);
  } else if (currentPacket.type == PROTO_TYPE_DELTA) {
    // 差分更新
    updateDeltaDisplay(packetPayload, currentPacket.length);
  }
  
  unsigned long elapsed = millis() - startTime;
  Serial.printf("Display updated in %lu ms\n", elapsed);
  
  // 發送 ACK
  uint8_t ack[] = {PROTO_HEADER, 0x10, 
                   (uint8_t)(currentPacket.seq_id >> 8),
                   (uint8_t)(currentPacket.seq_id & 0xFF),
                   0x01};
  webSocket.sendBIN(ack, sizeof(ack));
}

void updateFullDisplay(uint8_t* compressed, size_t compressedSize) {
  Serial.println("Full display update");
  
  display.setFullWindow();
  display.firstPage();
  
  do {
    display.fillScreen(GxEPD_WHITE);
    
    // 流式解壓並顯示
    size_t pixelIndex = 0;
    
    RLEDecoder::decodeStream(compressed, compressedSize, 
      [&](uint8_t* buffer, size_t size) {
        // 直接寫入顯示緩衝
        for (size_t i = 0; i < size; i++) {
          uint8_t byte = buffer[i];
          
          // 每個 byte 包含 8 個像素
          for (int bit = 7; bit >= 0; bit--) {
            int x = pixelIndex % DISPLAY_WIDTH;
            int y = pixelIndex / DISPLAY_WIDTH;
            
            if ((byte & (1 << bit)) == 0) {
              display.drawPixel(x, y, GxEPD_BLACK);
            }
            
            pixelIndex++;
          }
        }
      }
    );
    
  } while (display.nextPage());
  
  Serial.println("Display updated");
}

void updateDeltaDisplay(uint8_t* data, size_t dataSize) {
  Serial.println("Delta display update");
  // TODO: 實作差分更新
}
```

---

## 🔄 開發流程

### Phase 1: 基礎架構 (第 1-2 週)

**目標**: 建立基本的 Server-Client 通訊

**Server 端**:
- [x] 建立 WebSocket Server
- [x] 實作圖像處理（轉黑白、縮放）
- [x] 實作 RLE 壓縮
- [x] 實作協議打包

**Client 端**:
- [x] WiFi 連接
- [x] WebSocket Client 連接
- [x] 協議解析
- [x] RLE 解壓縮

**測試**:
- [ ] Server 可以啟動並等待連接
- [ ] Client 可以連接到 Server
- [ ] 可以傳送簡單的測試資料

---

### Phase 2: 圖像傳輸 (第 3-4 週)

**目標**: 實現完整畫面傳輸

**Server 端**:
- [ ] 載入圖片並轉換
- [ ] 壓縮並發送
- [ ] 處理 ACK/NAK

**Client 端**:
- [ ] 接收完整封包
- [ ] 流式解壓縮
- [ ] 顯示到電子紙
- [ ] 發送 ACK

**測試**:
- [ ] 傳輸純黑色圖片
- [ ] 傳輸純白色圖片
- [ ] 傳輸簡單文字
- [ ] 傳輸複雜圖片
- [ ] 測量傳輸時間和壓縮率

---

### Phase 3: 優化與功能 (第 5-6 週)

**目標**: 提升速度和功能

**Server 端**:
- [ ] 實作差分更新
- [ ] 文字轉圖像
- [ ] 網頁測試介面

**Client 端**:
- [ ] 差分更新支援
- [ ] 局部刷新優化
- [ ] 記憶體優化

**測試**:
- [ ] 連續更新測試
- [ ] 記憶體壓力測試
- [ ] 長時間穩定性測試

---

### Phase 4: 完善與測試 (第 7-8 週)

**目標**: 完整功能和穩定性

**功能**:
- [ ] 錯誤處理和重傳
- [ ] 斷線重連
- [ ] 多客戶端支援
- [ ] 設定介面

**測試**:
- [ ] 各種網路環境測試
- [ ] 邊界條件測試
- [ ] 效能基準測試
- [ ] 用戶測試

---

## 🧪 測試計畫

### 單元測試

**Server 端**:
```python
# test_image_processor.py
def test_convert_to_1bit():
    processor = ImageProcessor(800, 480)
    img = Image.new('RGB', (1920, 1080))
    result = processor.convert_to_1bit(img)
    assert result.size == (800, 480)
    assert result.mode == '1'

# test_compressor.py
def test_rle_compression():
    data = bytes([0xFF] * 100 + [0x00] * 100)
    compressed = RLECompressor.compress(data)
    assert len(compressed) < len(data)
    assert compressed == bytes([100, 0xFF, 100, 0x00])
```

**Client 端**:
```cpp
// Arduino IDE Serial Monitor 測試
void test_rle_decoder() {
  uint8_t input[] = {4, 0xFF, 2, 0x00};
  uint8_t output[6];
  
  size_t size = RLEDecoder::decode(input, 4, output, 6);
  
  assert(size == 6);
  assert(output[0] == 0xFF && output[3] == 0xFF);
  assert(output[4] == 0x00 && output[5] == 0x00);
}
```

### 整合測試

**測試案例**:
1. **簡單文字**: "Hello World" → 測試基本功能
2. **大面積單色**: 純黑/純白 → 測試壓縮率
3. **複雜圖片**: 照片 → 測試處理能力
4. **連續更新**: 10 次更新 → 測試穩定性
5. **網路異常**: 斷線重連 → 測試容錯

### 效能基準

**目標**:
- 完整畫面傳輸: < 3 秒
- 差分更新: < 1 秒
- 壓縮率: > 60%
- RAM 使用: < 50KB
- 連續運行: > 24 小時

---

## ⚠️ 風險評估

### 高風險

| 風險 | 影響 | 機率 | 緩解策略 |
|------|------|------|----------|
| **RAM 不足** | 專案失敗 | 中 | 流式處理、最小緩衝、記憶體監控 |
| **WiFi 不穩定** | 顯示中斷 | 中 | 自動重連、斷點續傳 |
| **傳輸太慢** | 用戶體驗差 | 低 | 壓縮、TCP 優化、802.11n |

### 中風險

| 風險 | 影響 | 機率 | 緩解策略 |
|------|------|------|----------|
| **電子紙刷新慢** | 更新延遲 | 高 | 局部更新、差分編碼 |
| **圖像品質差** | 顯示不佳 | 中 | 抖動演算法、預覽功能 |
| **跨平台問題** | 開發困難 | 低 | Python 跨平台、詳細文件 |

### 低風險

| 風險 | 影響 | 機率 | 緩解策略 |
|------|------|------|----------|
| **協議錯誤** | 通訊失敗 | 低 | CRC 校驗、版本號 |
| **記憶體洩漏** | 長期運行失敗 | 低 | 定期重啟、監控 |

---

## 📅 時程規劃

```
第 1-2 週: Phase 1 (基礎架構)
  ├─ Server WebSocket 實作
  ├─ Client WiFi + WebSocket
  └─ 基本通訊測試

第 3-4 週: Phase 2 (圖像傳輸)
  ├─ 圖像處理流程
  ├─ 壓縮/解壓實作
  └─ 完整畫面顯示

第 5-6 週: Phase 3 (優化功能)
  ├─ 差分更新
  ├─ 效能優化
  └─ 網頁介面

第 7-8 週: Phase 4 (測試完善)
  ├─ 整合測試
  ├─ 穩定性測試
  └─ 文件完善
```

---

## 📚 參考資源

### 技術文件
- **WebSocket Protocol**: RFC 6455
- **ESP8266 Arduino Core**: https://github.com/esp8266/Arduino
- **GxEPD2 Library**: https://github.com/ZinggJM/GxEPD2
- **Pillow Documentation**: https://pillow.readthedocs.io/

### 範例專案
- **ESP8266 WebSocket**: https://github.com/Links2004/arduinoWebSockets
- **Python WebSocket Server**: https://github.com/Pithikos/python-websocket-server

---

## ✅ 確認清單

在開始開發前，請確認：

- [ ] 已詳細閱讀本規劃文件
- [ ] 理解 Server/Client 架構
- [ ] 同意使用 WebSocket 二進位協議
- [ ] 同意使用 RLE + Delta 壓縮方案
- [ ] 確認 Python 作為 Server 端技術
- [ ] 確認 ESP8266 記憶體限制可接受
- [ ] 理解流式處理的必要性
- [ ] 同意 8 週開發時程

**請審核後提供意見，確認無誤後開始開發！**

---

**文件版本**: 1.0  
**作者**: Daniel YJ Hsieh  
**日期**: 2025-10-04  
**狀態**: 待審核
