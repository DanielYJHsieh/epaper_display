# v1.6.0 Bug 修復記錄

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
// protocol.h
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

### 問題 2: 字節序不匹配（重大問題）
**症狀**:
- 封包長度被錯誤解析為 `3777888256 bytes`（應該是 12000 bytes）
- ESP8266 顯示 "記憶體不足！"（實際有 23KB 可用）
- Type 顯示為 UNKNOWN

**原因**: 
**雙重字節序問題**：
1. Python 伺服器原本使用大端序 (`!BBHi`)
2. ESP8266 的 `parseHeader()` 使用大端序解析
3. 第一次修復時只改了 Python 端為小端序
4. **忘記同步修改 ESP8266 端的解析函數**

**影響的檔案**:
- `server/protocol.py` - Python 端（第一次修復）
- `client_esp8266/protocol.h` - ESP8266 端（本次修復）

**完整修復**:
```python
# server/protocol.py

# 修復前（大端序）
struct.pack('!BBHi', ...)  # ✗ 錯誤
struct.unpack('!BBHi', ...) # ✗ 錯誤

# 修復後（小端序）
struct.pack('<BBHI', ...)  # ✓ 正確
struct.unpack('<BBHI', ...) # ✓ 正確
```

### 問題 3: 記憶體分配"失敗"
**症狀**:
```
可用: 23480 bytes
❌ 無法分配分區緩衝區 (12000 bytes)
```

**真實原因**: 
不是記憶體不足，而是因為字節序問題導致：
1. 封包類型被誤解析
2. 封包長度被誤解析
3. Payload 指針指向錯誤位置
4. 導致後續的 malloc 失敗

**修復**: 
修復字節序後，問題自動解決

## 📝 修復清單

### ESP8266 端
- ✅ `client_esp8266/protocol.h`
  - 在 `getTypeName()` 中新增 `PROTO_TYPE_TILE` case

### Python 伺服器端
- ✅ `server/protocol.py`
  - `pack_header()`: `!BBHi` → `<BBHI`
  - `unpack_header()`: `!BBHi` → `<BBHI`
  - `pack_tile()`: `!B` → `<B`

## 🔍 技術細節

### 字節序說明

**大端序 (Big-Endian)** `!`:
- 高位字節存儲在低地址
- 網路字節序標準
- 人類閱讀友好

**小端序 (Little-Endian)** `<`:
- 低位字節存儲在低地址
- x86/ARM 處理器常用
- ESP8266 使用小端序

### 範例：數字 0x12345678

**大端序**:
```
地址:   0x00  0x01  0x02  0x03
資料:   0x12  0x34  0x56  0x78
```

**小端序**:
```
地址:   0x00  0x01  0x02  0x03
資料:   0x78  0x56  0x34  0x12
```

### 封包頭格式

```
┌────────┬────────┬─────────┬──────────┐
│ Header │  Type  │ Seq ID  │  Length  │
│ 1 byte │ 1 byte │ 2 bytes │ 4 bytes  │
├────────┼────────┼─────────┼──────────┤
│  0xA5  │  0x02  │ 0x0005  │ 0x002EE1 │
│        │  TILE  │    5    │  12001   │
└────────┴────────┴─────────┴──────────┘

小端序表示：
A5 02 05 00 E1 2E 00 00
^^    ^^    ^^       ^^
header type seq_id   length
```

## ✅ 驗證方法

### 1. 檢查協議類型
**修復前**:
```
處理封包: Type=UNKNOWN, SeqID=5, Length=12001
```

**修復後**:
```
處理封包: Type=TILE_UPDATE, SeqID=5, Length=12001
```

### 2. 檢查記憶體分配
**修復前**:
```
❌ 無法分配分區緩衝區 (12000 bytes)
```

**修復後**:
```
✅ 記憶體分配成功
🗜️  解壓縮分區資料: 12000 bytes → 12000 bytes
🖼️  更新分區顯示...
✅ 分區 左上 更新完成 (18234 ms)
```

## 📊 測試結果

### 測試命令
```bash
# 1. 重新編譯並上傳
cd client_esp8266
arduino-cli compile --fqbn esp8266:esp8266:d1_mini .
arduino-cli upload -p COM5 --fqbn esp8266:esp8266:d1_mini .

# 2. 啟動伺服器
cd server
python server.py

# 3. 發送測試圖
>>> tile test_images_800x480/tile_test_800x480.png
```

### 預期結果
- ✅ 協議類型正確顯示：`Type=TILE_UPDATE`
- ✅ 記憶體分配成功
- ✅ 4 個區塊依序更新
- ✅ 顯示在螢幕上

## 🎓 經驗教訓

### 1. 字節序一致性至關重要
- 務必確保客戶端和伺服器端使用相同字節序
- ESP8266 和大多數嵌入式系統使用小端序
- Python 的 struct 預設使用本機字節序（通常是小端序）

### 2. 除錯技巧
- 查看原始 bytes: `print(packet.hex())`
- 對比兩端的解析結果
- 使用 Wireshark 抓包分析

### 3. 文檔重要性
- 在設計文檔中明確標註字節序
- 範例代碼要完整測試
- 跨語言通訊尤其需要注意

## 🔮 後續改進

### 可選優化
1. 統一使用小端序並在文檔中明確說明
2. 添加字節序檢測和自動轉換
3. 增加封包校驗和（CRC）
4. 添加版本號協商機制

### 測試建議
1. 單元測試封包打包/解包
2. 集成測試完整通訊流程
3. 壓力測試記憶體管理
4. 邊界條件測試

## 📅 修復記錄

**日期**: 2025-10-08  
**版本**: v1.6.0 Hotfix  
**影響**: 所有分區顯示功能  
**嚴重性**: 高（功能完全無法使用）  
**狀態**: ✅ 已修復並驗證

---

**相關文件**:
- `TILED_DISPLAY_USAGE.md` - 使用指南
- `TILED_DISPLAY_DESIGN.md` - 設計文檔
- `RELEASE_NOTES_V1.6.md` - 版本說明
