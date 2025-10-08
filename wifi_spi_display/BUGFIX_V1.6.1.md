# Bug Fix Release v1.6.1

## 發布日期
2025-10-09

## 修正問題

### 1. 座標偏移錯誤
**問題描述：**
- 所有分區（左上、右上、左下、右下）都顯示在螢幕左上角
- 分區座標計算沒有考慮顯示偏移量 `DISPLAY_OFFSET_X` 和 `DISPLAY_OFFSET_Y`

**原因分析：**
```cpp
// 錯誤的座標計算（v1.6.0）
uint16_t tile_x = (tileIndex % 2) * TILE_WIDTH;   // 0 or 400
uint16_t tile_y = (tileIndex / 2) * TILE_HEIGHT;  // 0 or 240
```
這樣計算的座標是相對於螢幕左上角的，但 GDEQ0426T82 顯示器需要使用中央對齊，必須加上 `DISPLAY_OFFSET_X` 和 `DISPLAY_OFFSET_Y`。

**修正方法：**
```cpp
// 正確的座標計算（v1.6.1）
uint16_t tile_x = DISPLAY_OFFSET_X + (tileIndex % 2) * TILE_WIDTH;   // 0+offset or 400+offset
uint16_t tile_y = DISPLAY_OFFSET_Y + (tileIndex / 2) * TILE_HEIGHT;  // 0+offset or 240+offset
```

**修正結果：**
- 左上分區 (0): (0+offset, 0+offset)
- 右上分區 (1): (400+offset, 0+offset)
- 左下分區 (2): (0+offset, 240+offset)
- 右下分區 (3): (400+offset, 240+offset)

### 2. 記憶體分配失敗（間歇性）
**問題描述：**
- 更新左下分區時出現記憶體分配失敗
- 錯誤訊息：`❌ 無法分配分區緩衝區 (12000 bytes)`
- 雖然顯示有 35888 bytes 可用，但 `malloc(12000)` 失敗

**原因分析：**
- ESP8266 記憶體碎片化問題
- 前一個分區的緩衝區可能還沒完全釋放
- 系統需要時間整理記憶體堆

**修正方法：**
1. 添加記憶體診斷輸出：
```cpp
Serial.print(F("🔍 分配前記憶體: 可用="));
Serial.print(ESP.getFreeHeap());
Serial.print(F(" bytes, 最大塊="));
Serial.print(ESP.getMaxFreeBlockSize());
Serial.println(F(" bytes"));
```

2. 強制垃圾收集和記憶體整理：
```cpp
yield();
delay(10);  // 給系統時間整理記憶體
```

3. 失敗時顯示詳細記憶體資訊：
```cpp
Serial.print(F("   總可用: "));
Serial.print(ESP.getFreeHeap());
Serial.print(F(" bytes, 碎片化程度: "));
uint32_t freeHeap = ESP.getFreeHeap();
uint32_t maxBlock = ESP.getMaxFreeBlockSize();
Serial.print((freeHeap - maxBlock) * 100 / freeHeap);
Serial.println(F("%"));
```

**修正結果：**
- 更穩定的記憶體分配
- 詳細的診斷資訊幫助追蹤問題
- 給系統時間整理記憶體減少碎片化影響

## 修改文件

### client_esp8266/client_esp8266.ino
- **第 470-472 行**：修正分區座標計算（添加 `DISPLAY_OFFSET_X` 和 `DISPLAY_OFFSET_Y`）
- **第 494-521 行**：添加記憶體診斷和整理邏輯

## 編譯資訊
```
RAM: 30476/80192 bytes (38%)
├─ DATA: 1500 bytes
├─ RODATA: 2144 bytes
└─ BSS: 26832 bytes

IRAM: 60737/65536 bytes (92%)
├─ ICACHE: 32768 bytes
└─ IRAM: 27969 bytes

Flash: 378220/1048576 bytes (36%)
└─ IROM: 378220 bytes
```

## 測試步驟
1. 上傳固件到 ESP8266
2. 啟動伺服器：`python server.py`
3. 等待 WebSocket 連接
4. 測試分區顯示：`tile test_images_800x480/tile_test_800x480.png`
5. 觀察 Serial Monitor：
   - 確認座標正確（應該看到 offset 值）
   - 確認記憶體分配成功
   - 確認 4 個分區都正確顯示

## 已知限制
- 記憶體碎片化問題可能在長時間運行後仍會出現
- 建議在分區更新之間保持 20 秒間隔（伺服器端已實現）

## 版本資訊
- **版本**：v1.6.1
- **基於**：v1.6.0（800×480 分塊顯示功能）
- **更新日期**：2025-10-09
- **變更類型**：Bug Fix

## 相關問題
- Issue #1：所有分區顯示在左上角
- Issue #2：左下分區記憶體分配失敗
