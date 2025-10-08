# 800×480 分區顯示實作方案

## 概念說明

### 顯示器分區
```
800×480 顯示器分成 4 個 400×240 區域：

┌─────────────┬─────────────┐
│   區域 0    │   區域 1    │
│  左上 (LT)  │  右上 (RT)  │
│   400×240   │   400×240   │
│  (0,0)      │  (400,0)    │
├─────────────┼─────────────┤
│   區域 2    │   區域 3    │
│  左下 (LB)  │  右下 (RB)  │
│   400×240   │   400×240   │
│  (0,240)    │  (400,240)  │
└─────────────┴─────────────┘
```

### 區域索引定義
```cpp
#define TILE_INDEX_LEFT_TOP     0  // 左上
#define TILE_INDEX_RIGHT_TOP    1  // 右上
#define TILE_INDEX_LEFT_BOTTOM  2  // 左下
#define TILE_INDEX_RIGHT_BOTTOM 3  // 右下
```

### 區域座標計算
```cpp
// 每個區域的尺寸
#define TILE_WIDTH  400
#define TILE_HEIGHT 240

// 區域座標對應表
區域 0 (左上):  x=0,   y=0
區域 1 (右上):  x=400, y=0
區域 2 (左下):  x=0,   y=240
區域 3 (右下):  x=400, y=240
```

## 通訊協定擴展

### 封包類型新增
```cpp
#define PROTO_TYPE_TILE_UPDATE  0x02  // 分區更新
```

### 封包格式
```
┌──────────────────────────────────────┐
│ PacketHeader (7 bytes)               │
├──────────────────────────────────────┤
│ - type (1 byte): PROTO_TYPE_TILE_UPDATE
│ - seq_id (2 bytes): 序列號           │
│ - length (4 bytes): 資料長度         │
├──────────────────────────────────────┤
│ TileInfo (1 byte)                    │
├──────────────────────────────────────┤
│ - tile_index (1 byte): 0~3           │
├──────────────────────────────────────┤
│ Payload (variable)                   │
├──────────────────────────────────────┤
│ - 壓縮或未壓縮的圖片資料 (最多12KB)  │
└──────────────────────────────────────┘
```

## 實作細節

### 1. 客戶端 (ESP8266) 修改

#### config.h 新增定義
```cpp
// 分區顯示設定
#define ENABLE_TILE_DISPLAY 1         // 啟用分區顯示
#define TILE_WIDTH  400               // 區塊寬度
#define TILE_HEIGHT 240               // 區塊高度
#define TILE_COUNT  4                 // 區塊數量

// 區域索引
#define TILE_INDEX_LEFT_TOP     0
#define TILE_INDEX_RIGHT_TOP    1
#define TILE_INDEX_LEFT_BOTTOM  2
#define TILE_INDEX_RIGHT_BOTTOM 3
```

#### protocol.h 擴展
```cpp
// 封包類型
#define PROTO_TYPE_FULL_UPDATE  0x01  // 完整更新（原有）
#define PROTO_TYPE_TILE_UPDATE  0x02  // 分區更新（新增）

// 分區資訊結構
struct TileInfo {
  uint8_t tile_index;  // 區域索引 (0~3)
} __attribute__((packed));
```

#### client_esp8266.ino 新增函數
```cpp
// 處理分區更新
void handleTileUpdate(uint8_t* payload, uint32_t length, uint16_t seqId) {
  // 解析分區索引
  if (length < 1) {
    Serial.println(F("分區資料長度不足"));
    sendNAK(seqId);
    return;
  }
  
  uint8_t tileIndex = payload[0];
  payload++;  // 跳過 tile_index
  length--;   // 減去 tile_index 長度
  
  if (tileIndex >= TILE_COUNT) {
    Serial.print(F("無效的分區索引: "));
    Serial.println(tileIndex);
    sendNAK(seqId);
    return;
  }
  
  // 計算分區座標
  uint16_t tile_x = (tileIndex % 2) * TILE_WIDTH;   // 0 or 400
  uint16_t tile_y = (tileIndex / 2) * TILE_HEIGHT;  // 0 or 240
  
  Serial.print(F("分區更新: 索引="));
  Serial.print(tileIndex);
  Serial.print(F(", 座標=("));
  Serial.print(tile_x);
  Serial.print(F(","));
  Serial.print(tile_y);
  Serial.println(F(")"));
  
  // 分配緩衝區
  uint8_t* tileBuffer = (uint8_t*)malloc(DISPLAY_BUFFER_SIZE);
  if (!tileBuffer) {
    Serial.println(F("無法分配分區緩衝區"));
    sendNAK(seqId);
    return;
  }
  
  // 智能解壓縮
  bool isCompressed = (length != DISPLAY_BUFFER_SIZE);
  int decompressedSize;
  
  if (isCompressed) {
    Serial.println(F("解壓縮分區資料..."));
    decompressedSize = RLEDecoder::decode(payload, length, tileBuffer, DISPLAY_BUFFER_SIZE);
  } else {
    Serial.println(F("直接使用分區資料..."));
    memcpy(tileBuffer, payload, length);
    decompressedSize = length;
  }
  
  if (decompressedSize != DISPLAY_BUFFER_SIZE) {
    Serial.print(F("分區資料大小錯誤: "));
    Serial.println(decompressedSize);
    free(tileBuffer);
    sendNAK(seqId);
    return;
  }
  
  // 顯示分區
  Serial.println(F("更新分區顯示..."));
  display.setPartialWindow(tile_x, tile_y, TILE_WIDTH, TILE_HEIGHT);
  display.writeImage(tileBuffer, 0, 0, TILE_WIDTH, TILE_HEIGHT, false, false, true);
  display.refresh(false);  // 快速部分更新
  
  free(tileBuffer);
  sendACK(seqId);
  
  Serial.print(F("分區 "));
  Serial.print(tileIndex);
  Serial.println(F(" 更新完成"));
}

// 在 webSocketEvent 中處理
void webSocketEvent(WStype_t type, uint8_t* payload, size_t length) {
  switch(type) {
    case WStype_BIN: {
      // ... 解析 header ...
      
      uint8_t packetType = header.type;
      
      if (packetType == PROTO_TYPE_FULL_UPDATE) {
        handleFullUpdate(dataPayload, dataLength, header.seq_id);
      } else if (packetType == PROTO_TYPE_TILE_UPDATE) {
        handleTileUpdate(dataPayload, dataLength, header.seq_id);
      } else {
        Serial.print(F("未知的封包類型: "));
        Serial.println(packetType);
      }
      break;
    }
  }
}
```

### 2. 伺服器端 (Python) 修改

#### protocol.py 擴展
```python
# 封包類型
PROTO_TYPE_FULL_UPDATE = 0x01
PROTO_TYPE_TILE_UPDATE = 0x02

# 分區索引
TILE_INDEX_LEFT_TOP = 0
TILE_INDEX_RIGHT_TOP = 1
TILE_INDEX_LEFT_BOTTOM = 2
TILE_INDEX_RIGHT_BOTTOM = 3

def create_tile_packet(tile_index: int, tile_data: bytes, seq_id: int) -> bytes:
    """
    創建分區更新封包
    
    Args:
        tile_index: 分區索引 (0~3)
        tile_data: 分區圖片資料 (壓縮或未壓縮)
        seq_id: 序列號
    
    Returns:
        完整的封包 bytes
    """
    packet_type = PROTO_TYPE_TILE_UPDATE
    length = len(tile_data) + 1  # +1 for tile_index
    
    # PacketHeader: type(1) + seq_id(2) + length(4)
    header = struct.pack('<BHI', packet_type, seq_id, length)
    
    # TileInfo: tile_index(1)
    tile_info = struct.pack('<B', tile_index)
    
    # 完整封包
    packet = header + tile_info + tile_data
    return packet
```

#### image_processor.py 新增分區處理
```python
def split_image_to_tiles(self, image: Image.Image) -> dict:
    """
    將 800×480 圖片分割成 4 個 400×240 區塊
    
    Returns:
        {
            0: tile_left_top,
            1: tile_right_top,
            2: tile_left_bottom,
            3: tile_right_bottom
        }
    """
    # 確保圖片是 800×480
    if image.size != (800, 480):
        image = image.resize((800, 480), Image.LANCZOS)
    
    tiles = {}
    
    # 左上 (0, 0, 400, 240)
    tiles[0] = image.crop((0, 0, 400, 240))
    
    # 右上 (400, 0, 800, 240)
    tiles[1] = image.crop((400, 0, 800, 240))
    
    # 左下 (0, 240, 400, 480)
    tiles[2] = image.crop((0, 240, 400, 480))
    
    # 右下 (400, 240, 800, 480)
    tiles[3] = image.crop((400, 240, 800, 480))
    
    return tiles

def process_tile(self, tile: Image.Image) -> bytes:
    """
    處理單個 400×240 區塊
    """
    # 轉換為 1-bit 黑白
    if tile.mode != '1':
        tile = tile.convert('1')
    
    # 轉換為 bytes
    pixels = np.array(tile, dtype=np.uint8)
    pixels = (pixels > 0).astype(np.uint8)
    packed = np.packbits(pixels, axis=1)
    
    return packed.tobytes()
```

#### server.py 新增分區傳輸
```python
async def send_tiled_image(self, image_path: str):
    """
    發送 800×480 圖片（分 4 個區塊）
    """
    # 載入並分割圖片
    image = Image.open(image_path)
    tiles = self.image_processor.split_image_to_tiles(image)
    
    seq_id = self.get_next_seq_id()
    
    # 依序發送 4 個區塊
    for tile_index in range(4):
        tile_image = tiles[tile_index]
        
        # 處理區塊
        tile_data = self.image_processor.process_tile(tile_image)
        
        # 智能壓縮
        compressed_data, is_compressed = self.compressor.compress_smart(tile_data)
        
        # 創建封包
        packet = create_tile_packet(tile_index, compressed_data, seq_id + tile_index)
        
        # 發送
        await self.websocket.send(packet)
        print(f"分區 {tile_index} 已發送: {len(compressed_data)} bytes "
              f"({'壓縮' if is_compressed else '未壓縮'})")
        
        # 等待 ACK（可選）
        await asyncio.sleep(0.1)
    
    print("800×480 圖片發送完成（4 個區塊）")
```

## 使用方式

### 發送 400×240 圖片（中央顯示）
```python
await server.send_image("test_400x240.png")  # 原有方式
```

### 發送 800×480 圖片（分區顯示）
```python
await server.send_tiled_image("test_800x480.png")  # 新方式
```

## 優點

1. **記憶體效率**: 每次只需 12KB 緩衝區
2. **更新速度**: 4 × 18秒 = 72秒（vs 單次60秒完整刷新）
3. **靈活性**: 可選擇性更新特定區域
4. **擴展性**: 容易擴展到更多分區

## 進階功能

### 1. 選擇性更新
只更新變化的區域：
```python
# 只更新左上和右下
await server.send_tiles([0, 3], image)
```

### 2. 快取機制
```python
# 記住上次顯示的內容
self.last_tiles = [None] * 4

# 只發送變化的區塊
for i in range(4):
    if tiles[i] != self.last_tiles[i]:
        await send_tile(i, tiles[i])
```

### 3. 並行更新
```python
# 同時發送多個區塊（ESP8266 處理）
await asyncio.gather(
    send_tile(0, tiles[0]),
    send_tile(1, tiles[1]),
    send_tile(2, tiles[2]),
    send_tile(3, tiles[3])
)
```

## 注意事項

### 1. 更新順序
建議順序：左上 → 右上 → 左下 → 右下
- 從上到下，從左到右
- 符合視覺習慣

### 2. 邊界處理
確保區塊邊界對齊：
- 每個區塊必須是 400×240
- 座標必須準確

### 3. 錯誤處理
- 區塊索引驗證
- 資料大小驗證
- 記憶體分配失敗處理

### 4. 性能考量
- 每個區塊 ~18 秒
- 總時間約 72 秒
- 可透過智能更新減少時間

## 測試計劃

### 快速測試步驟

1. **創建測試圖片**
```bash
cd server
python create_test_images.py
```
這會在 `server/test_images_800x480/` 目錄下創建 3 個測試圖片：
- `tile_test_800x480.png` - 分區標示圖（4 色區塊）
- `gradient_800x480.png` - 漸變測試圖
- `checkerboard_800x480.png` - 棋盤格測試圖

2. **編譯並上傳固件**
```bash
cd client_esp8266
arduino-cli compile --fqbn esp8266:esp8266:d1_mini .
arduino-cli upload -p COM5 --fqbn esp8266:esp8266:d1_mini .
```

3. **啟動伺服器**
```bash
cd server
python server.py
```

4. **發送分區測試圖**
等待 ESP8266 連線後，在伺服器輸入：
```
tile test_images_800x480/tile_test_800x480.png
```

### 詳細測試項目

1. 測試單一區塊更新
2. 測試 4 區塊依序更新
3. 測試部分區塊更新
4. 測試錯誤處理
5. 測試記憶體穩定性

## 未來擴展

### 動態分區
```cpp
// 支援不同分區策略
#define TILE_STRATEGY_2X2  0  // 2×2 = 4 區塊 (400×240)
#define TILE_STRATEGY_4X2  1  // 4×2 = 8 區塊 (200×240)
#define TILE_STRATEGY_2X4  2  // 2×4 = 8 區塊 (400×120)
```

### 自適應分區
根據圖片複雜度和記憶體狀況動態調整分區策略。
