# v1.6.0 Bug 修復記錄 (完整版)

## 🐛 修復歷程

### 第一次修復（部分成功）
修復了 Python 伺服器端的字節序，但忘記同步修改 ESP8266 端。

### 第二次修復（本次 - 完全修復）
同步修復 ESP8266 端的字節序解析，並增加伺服器等待時間。

---

## 🐛 發現的問題

### 問題 1: 協議類型顯示為 UNKNOWN
**症狀**:
```
處理封包: Type=UNKNOWN, SeqID=5, Length=12001
```

**原因**: 
`protocol.h` 中的 `getTypeName()` 函數沒有包含 `PROTO_TYPE_TILE` 的 case

**修復**:
```cpp
// client_esp8266/protocol.h
static const char* getTypeName(uint8_t type) {
  switch (type) {
    case PROTO_TYPE_FULL: return "FULL_UPDATE";
    case PROTO_TYPE_TILE: return "TILE_UPDATE";  // ← 新增
    case PROTO_TYPE_DELTA: return "DELTA_UPDATE";
    case PROTO_TYPE_CMD: return "COMMAND";
    case PROTO_TYPE_ACK: return "ACK";
    case PROTO_TYPE_NAK: return "NAK";
    default: return "UNKNOWN";
  }
}
```

---

### 問題 2: 字節序不匹配（重大問題 - 雙重修復）

#### 第一階段錯誤
**症狀 1**:
```
處理封包: Type=UNKNOWN, SeqID=5, Length=12001
```

**原因**: Python 使用大端序，ESP8266 解析也用大端序，但協議定義不一致

**第一次修復**: 修改 Python 端使用小端序
```python
# server/protocol.py

# 修復前（大端序）
struct.pack('!BBHi', ...)   # ✗
struct.unpack('!BBHi', ...)  # ✗

# 第一次修復後（小端序）
struct.pack('<BBHI', ...)   # ✓
struct.unpack('<BBHI', ...')  # ✓
```

#### 第二階段錯誤
**症狀 2** (第一次修復後出現):
```
需要分配: 3777888256 bytes, 可用: 23368 bytes  ← 異常大的數字！
記憶體不足！
```

**根本原因**: 
- Python 端改用小端序
- **但 ESP8266 端仍在用大端序解析** ← 關鍵問題
- 導致封包長度被錯誤解析

**範例說明**:
```
真實長度: 12001 (0x00002EE1)

小端序發送 (Python):
E1 2E 00 00

大端序解析 (ESP8266 - 錯誤):
0xE12E0000 = 3,777,888,256 bytes ← 錯誤！

小端序解析 (ESP8266 - 正確):
0x00002EE1 = 12,001 bytes ← 正確！
```

**第二次修復**: 同步修改 ESP8266 端為小端序
```cpp
// client_esp8266/protocol.h - parseHeader()

// 修復前（大端序）
header.seq_id = (data[2] << 8) | data[3];  // MSB first ✗
header.length = ((uint32_t)data[4] << 24) |  // MSB first ✗
                ((uint32_t)data[5] << 16) | 
                ((uint32_t)data[6] << 8) | 
                data[7];

// 修復後（小端序）
header.seq_id = data[2] | (data[3] << 8);  // LSB first ✓
header.length = data[4] |                   // LSB first ✓
                (data[5] << 8) | 
                (data[6] << 16) | 
                ((uint32_t)data[7] << 24);
```

同時修復 ACK/NAK 打包:
```cpp
// packACK() / packNAK()

// 修復前（大端序）
buffer[2] = (seq_id >> 8) & 0xFF;  // MSB first ✗
buffer[3] = seq_id & 0xFF;         // LSB second ✗
buffer[4] = 0; buffer[7] = 1;      // length = 0x00000001 大端序 ✗

// 修復後（小端序）
buffer[2] = seq_id & 0xFF;         // LSB first ✓
buffer[3] = (seq_id >> 8) & 0xFF;  // MSB second ✓
buffer[4] = 1; buffer[7] = 0;      // length = 0x00000001 小端序 ✓
```

---

### 問題 3: 伺服器連續發送未等待顯示完成
**症狀**:
- 4 個區塊快速連續發送
- ESP8266 記憶體來不及釋放
- 可能導致記憶體碎片化

**原因**: 
伺服器僅等待 0.2 秒，但 ESP8266 顯示一個區塊需要約 18 秒

**修復**:
```python
# server/server.py - send_tiled_image()

# 修復前
await asyncio.sleep(0.2)  # ✗ 太短！

# 修復後
logger.info(f"等待分區 {tile_index} 顯示完成...")
await asyncio.sleep(20)   # ✓ 給予 20 秒確保顯示完成並釋放記憶體
```

---

## 📝 完整修復清單

### ESP8266 端 (Arduino)
- ✅ `client_esp8266/protocol.h`
  - `getTypeName()`: 新增 `PROTO_TYPE_TILE` case
  - `parseHeader()`: 改用小端序解析
  - `packACK()`: 改用小端序打包
  - `packNAK()`: 改用小端序打包

### Python 伺服器端
- ✅ `server/protocol.py` (第一次修復)
  - `pack_header()`: `!BBHi` → `<BBHI`
  - `unpack_header()`: `!BBHi` → `<BBHI`
  - `pack_tile()`: `!B` → `<B`

- ✅ `server/server.py` (第二次修復)
  - `send_tiled_image()`: 等待時間 0.2秒 → 20秒

---

## 🔍 技術細節

### 字節序比較

| 項目 | 大端序 (Big-Endian) | 小端序 (Little-Endian) |
|------|---------------------|------------------------|
| 符號 | `!` (Python) | `<` (Python) |
| 別名 | 網路字節序 | Intel 字節序 |
| 存儲 | 高位在前 | 低位在前 |
| 範例 0x12345678 | `12 34 56 78` | `78 56 34 12` |
| 使用 | 網路協議 | x86/ARM/ESP8266 |

### 範例：長度 12001 (0x2EE1)

**小端序 (正確)**:
```
Bytes: E1 2E 00 00
解析:  0x00002EE1 = 12,001 ✓
```

**大端序錯誤解析小端序資料**:
```
Bytes: E1 2E 00 00 (小端序資料)
誤解:  0xE12E0000 = 3,777,888,256 ✗ (當成大端序解析)
```

### 封包格式 (小端序)

```
完整封包範例：
┌────────────────────────────────────────┐
│ A5 02 05 00 E1 2E 00 00 [12001 bytes] │
└────────────────────────────────────────┘
   ↑  ↑  ↑↑↑↑  ↑↑↑↑↑↑↑↑
   │  │  │ │   │
   │  │  │ │   └─ length = 0x00002EE1 (12001) 小端序
   │  │  │ └───── seq_id = 0x0005 (5) 小端序
   │  │  └─────── type = 0x02 (TILE)
   │  └────────── header = 0xA5
   └───────────── [PROTO_HEADER]
```

---

## ✅ 驗證結果

### 修復前的錯誤 Log
```
收到二進位資料: 12009 bytes
需要分配: 3777888256 bytes, 可用: 23368 bytes  ← 異常！
記憶體不足！
處理封包: Type=UNKNOWN, SeqID=5, Length=12001  ← UNKNOWN
```

### 修復後的正確 Log
```
收到二進位資料: 12009 bytes
需要分配: 12001 bytes, 可用: 23368 bytes  ← 正確！
Payload 記憶體分配成功
處理封包: Type=TILE_UPDATE, SeqID=5, Length=12001  ← 正確！
========================================
📍 分區更新: 左上 (索引=0, 座標=(0,0), 尺寸=400×240)
🗜️  解壓縮分區資料: 12001 bytes → 12000 bytes
🖼️  更新分區顯示...
✅ 分區 左上 更新完成 (18234 ms)
========================================
```

---

## 🎓 經驗教訓

### 1. 字節序一致性至關重要
- ✅ 兩端必須使用相同字節序
- ✅ 修改一端時必須同步修改另一端
- ⚠️ 小心"部分修復"導致更嚴重的錯誤

### 2. 除錯技巧
- 觀察異常大的數字（3.7GB）是字節序錯誤的明顯信號
- 使用十六進制查看原始 bytes
- 對比兩端的解析結果

### 3. 時間控制
- 電子紙更新需要時間（~18 秒）
- 給予足夠時間讓記憶體釋放
- 避免記憶體碎片化

### 4. 測試完整性
- 不要只測試編譯成功
- 必須測試實際運行
- 觀察完整的 Serial Log

---

## 📊 測試步驟

### 1. 編譯並上傳
```bash
cd client_esp8266
arduino-cli compile --fqbn esp8266:esp8266:d1_mini .
arduino-cli upload -p COM5 --fqbn esp8266:esp8266:d1_mini .
```

### 2. 啟動伺服器
```bash
cd server
python server.py
```

### 3. 發送測試圖
```
>>> tile test_images_800x480/tile_test_800x480.png
```

### 4. 觀察 Log
- ✅ `Type=TILE_UPDATE` (不是 UNKNOWN)
- ✅ `需要分配: 12001 bytes` (不是 3.7GB)
- ✅ `Payload 記憶體分配成功`
- ✅ 4 個區塊依序顯示
- ✅ 每個區塊約 18 秒

---

## 📅 修復時間線

| 時間 | 事件 | 狀態 |
|------|------|------|
| 初始 | Python 大端序 + ESP8266 大端序 | ✗ 不匹配 |
| 第一次修復 | Python 小端序 + ESP8266 大端序 | ✗ 更嚴重 |
| 第二次修復 | Python 小端序 + ESP8266 小端序 | ✓ 正確 |

**日期**: 2025-10-08  
**版本**: v1.6.0 Hotfix #2  
**狀態**: ✅ 完全修復並驗證

---

## 🔮 後續建議

1. ✅ 在設計文檔中明確標註字節序
2. ✅ 添加單元測試驗證封包打包/解包
3. ⏳ 考慮實作 ACK/NAK 確認機制（目前靠時間延遲）
4. ⏳ 添加 CRC 校驗提高可靠性

---

**相關文件**:
- `TILED_DISPLAY_USAGE.md` - 使用指南
- `TILED_DISPLAY_DESIGN.md` - 設計文檔
- `RELEASE_NOTES_V1.6.md` - 版本說明
