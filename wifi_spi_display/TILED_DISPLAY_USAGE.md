# 800×480 分區顯示功能使用指南

## 版本資訊
- **版本**: v1.6.0
- **日期**: 2025-10-08
- **功能**: 800×480 全螢幕分 4 區塊顯示

## 功能概述

### 新增功能
- ✅ 支援 800×480 全螢幕顯示
- ✅ 自動分割成 4 個 400×240 區塊
- ✅ 依序更新：左上 → 右上 → 左下 → 右下
- ✅ 每個區塊獨立壓縮傳輸
- ✅ 向後兼容原有 400×240 中央顯示

### 分區排列
```
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

## 快速開始

### 1. 編譯並上傳固件

```bash
cd client_esp8266
arduino-cli compile --fqbn esp8266:esp8266:d1_mini .
arduino-cli upload -p COM5 --fqbn esp8266:esp8266:d1_mini .
```

**記憶體使用情況**：
- RAM: 37% (30,472 / 80,192 bytes)
- Flash: 36% (377,928 / 1,048,576 bytes)
- IRAM: 92% (27,969 / 32,768 bytes)

### 2. 創建測試圖片

```bash
cd server
python create_test_images.py
```

**生成的測試圖片**：
- `test_images_800x480/tile_test_800x480.png` - 4 色分區標示圖
- `test_images_800x480/gradient_800x480.png` - 漸變測試
- `test_images_800x480/checkerboard_800x480.png` - 棋盤格測試

### 3. 啟動伺服器

```bash
cd server
python server.py
```

伺服器會監聽 `0.0.0.0:8266`。

### 4. 發送圖片

等待 ESP8266 連線後（會顯示 "客戶端連接" 訊息），在互動模式中輸入：

#### 方式 1：分區顯示（800×480 全螢幕）
```
tile test_images_800x480/tile_test_800x480.png
```

#### 方式 2：中央顯示（400×240 原方式）
```
image your_image.png
```

## 使用指南

### 伺服器端指令

| 指令 | 說明 | 範例 |
|------|------|------|
| `tile <檔案>` | 發送 800×480 圖片（分區顯示） | `tile test.png` |
| `image <檔案>` | 發送 400×240 圖片（中央顯示） | `image logo.png` |
| `text <文字>` | 發送文字（中央顯示） | `text Hello` |
| `test` | 發送測試圖案 | `test` |
| `clear` | 清空螢幕 | `clear` |
| `clients` | 顯示連接的客戶端 | `clients` |
| `quit` | 結束程式 | `quit` |

### 圖片要求

#### 分區顯示（tile 指令）
- **尺寸**: 800×480 像素（會自動縮放）
- **格式**: PNG, JPG, BMP 等 PIL 支援的格式
- **顏色**: 任意（會自動轉換為黑白）
- **處理**: 自動分割成 4 個 400×240 區塊

#### 中央顯示（image 指令）
- **尺寸**: 400×240 像素（會自動縮放）
- **格式**: PNG, JPG, BMP 等 PIL 支援的格式
- **顏色**: 任意（會自動轉換為黑白）
- **位置**: 顯示在 800×480 螢幕中央

## 技術細節

### 通訊協定

#### 封包類型
```cpp
#define PROTO_TYPE_FULL 0x01  // 完整更新（400×240 中央）
#define PROTO_TYPE_TILE 0x02  // 分區更新（400×240 區塊）
#define PROTO_TYPE_DELTA 0x03 // 差分更新
#define PROTO_TYPE_CMD 0x04   // 控制指令
```

#### 分區封包格式
```
┌──────────────────────────────────────┐
│ PacketHeader (8 bytes)               │
├──────────────────────────────────────┤
│ - header (1 byte): 0xA5              │
│ - type (1 byte): PROTO_TYPE_TILE     │
│ - seq_id (2 bytes): 序列號           │
│ - length (4 bytes): 資料長度         │
├──────────────────────────────────────┤
│ TileInfo (1 byte)                    │
├──────────────────────────────────────┤
│ - tile_index (1 byte): 0~3           │
├──────────────────────────────────────┤
│ Payload (variable)                   │
├──────────────────────────────────────┤
│ - 壓縮或未壓縮的圖片資料             │
└──────────────────────────────────────┘
```

### 分區索引
```cpp
#define TILE_INDEX_LEFT_TOP     0  // 左上 (0, 0)
#define TILE_INDEX_RIGHT_TOP    1  // 右上 (400, 0)
#define TILE_INDEX_LEFT_BOTTOM  2  // 左下 (0, 240)
#define TILE_INDEX_RIGHT_BOTTOM 3  // 右下 (400, 240)
```

### 處理流程

#### 伺服器端（Python）
1. 載入 800×480 圖片
2. 轉換為 1-bit 黑白
3. 分割成 4 個 400×240 區塊
4. 逐一壓縮（RLE 智能壓縮）
5. 創建分區封包
6. 依序發送（0 → 1 → 2 → 3）

#### 客戶端（ESP8266）
1. 接收分區封包
2. 解析分區索引（0~3）
3. 計算座標（x, y）
4. 解壓縮資料
5. 設定部分窗口
6. 寫入圖像
7. 快速刷新（refresh(false)）

## 性能分析

### 更新時間
- **單個區塊**: ~18 秒（部分更新）
- **4 個區塊**: ~72 秒（4 × 18秒）
- **對比**: 完整刷新需 ~60 秒

### 記憶體使用
- **區塊緩衝**: 12KB (400×240÷8)
- **總 RAM**: 37% (~30KB)
- **可用空間**: 充足

### 壓縮效率
取決於圖片內容：
- **純色圖**: 壓縮率 >90%
- **複雜圖**: 可能無壓縮（使用原始資料）
- **智能選擇**: 自動選擇較小的方案

## 範例程式碼

### Python: 發送分區圖片
```python
# 在 server.py 的互動模式中
await server.send_tiled_image("my_image.png")
```

### Arduino: 處理分區更新
```cpp
void handleTileUpdate(uint8_t* payload, uint32_t length, uint16_t seqId) {
  // 1. 解析分區索引
  uint8_t tileIndex = payload[0];
  
  // 2. 計算座標
  uint16_t tile_x = (tileIndex % 2) * TILE_WIDTH;
  uint16_t tile_y = (tileIndex / 2) * TILE_HEIGHT;
  
  // 3. 解壓縮
  uint8_t* tileBuffer = (uint8_t*)malloc(TILE_BUFFER_SIZE);
  int size = RLEDecoder::decode(payload + 1, length - 1, tileBuffer, TILE_BUFFER_SIZE);
  
  // 4. 顯示
  display.setPartialWindow(tile_x, tile_y, TILE_WIDTH, TILE_HEIGHT);
  display.writeImage(tileBuffer, 0, 0, TILE_WIDTH, TILE_HEIGHT, false, false, true);
  display.refresh(false);
  
  free(tileBuffer);
  sendACK(seqId);
}
```

## 故障排除

### 問題 1: 編譯錯誤
**症狀**: 編譯時出現記憶體不足
**解決**: 
- 檢查 `TILE_BUFFER_SIZE` 定義
- 確認 RLE 解碼器正確引入

### 問題 2: 顯示不完整
**症狀**: 某些區塊沒有顯示
**解決**:
- 檢查 Serial Monitor 確認 4 個區塊都收到
- 確認座標計算正確
- 檢查記憶體是否充足

### 問題 3: 顯示速度慢
**症狀**: 更新時間超過 90 秒
**解決**:
- 確認使用 `refresh(false)` 而非 `refresh(true)`
- 檢查 WiFi 信號強度
- 減少等待時間（server.py 中的 `asyncio.sleep(0.2)`）

### 問題 4: 圖片模糊
**症狀**: 顯示效果不清晰
**解決**:
- 使用高對比度圖片
- 啟用抖動（dither=True）
- 調整圖片尺寸為 800×480

## 進階使用

### 選擇性更新特定區塊
修改 `server.py` 中的 `send_tiled_image()`：
```python
# 只更新左上和右下
for tile_index in [0, 3]:  # 原本是 range(4)
    # ... 處理和發送
```

### 調整更新順序
```python
# 從中間往外更新
for tile_index in [0, 1, 2, 3]:  # 改為 [1, 0, 3, 2]
    # ... 處理和發送
```

### 添加進度顯示
```python
# 在發送時顯示進度
progress = (tile_index + 1) / 4 * 100
print(f"進度: {progress:.0f}%")
```

## 未來擴展

### 1. 動態分區策略
- 2×2 = 4 區塊（當前實作）
- 4×2 = 8 區塊（更細緻）
- 自適應分區

### 2. 智能更新
- 只更新變化的區塊
- 差分壓縮
- 快取機制

### 3. 並行傳輸
- 多客戶端同時發送
- 非同步處理
- 優先級隊列

## 相關文件
- `TILED_DISPLAY_DESIGN.md` - 詳細設計文檔
- `protocol.h` - 通訊協議定義
- `config.h` - 配置說明
- `README.md` - 專案總覽

## 版本歷史

### v1.6.0 (2025-10-08)
- ✅ 實作 800×480 分區顯示
- ✅ 新增 `PROTO_TYPE_TILE` 封包類型
- ✅ 新增 `handleTileUpdate()` 函數
- ✅ 新增 `send_tiled_image()` 伺服器方法
- ✅ 新增測試圖片生成工具
- ✅ 更新文檔

### v1.5.1 (之前版本)
- 修正第一張圖顯示問題
- 新增測試圖案功能

### v1.5.0 (之前版本)
- 顯示速度優化（60s → 18s）
- 移除完整刷新邏輯

## 授權
MIT License

## 聯絡方式
- GitHub Issues: 報告問題和功能請求
- Pull Requests: 歡迎貢獻代碼
