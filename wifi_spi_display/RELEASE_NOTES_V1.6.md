# WiFi SPI Display v1.6.0 Release Notes

## 🎉 新功能：800×480 全螢幕分區顯示

**發佈日期**: 2025-10-08

### ✨ 主要更新

#### 1. 全螢幕支援
- ✅ 支援 800×480 全螢幕顯示（之前僅支援 400×240 中央區域）
- ✅ 自動分割成 4 個 400×240 區塊
- ✅ 記憶體高效：每次僅需 12KB 緩衝區

#### 2. 分區傳輸機制
- ✅ 新增 `PROTO_TYPE_TILE` (0x02) 封包類型
- ✅ 支援分區索引 (0~3)
- ✅ 依序更新：左上 → 右上 → 左下 → 右下

#### 3. 向後兼容
- ✅ 保留原有 400×240 中央顯示功能（`image` 指令）
- ✅ 新增 800×480 全螢幕功能（`tile` 指令）
- ✅ 兩種模式可自由切換

### 📊 分區排列

```
┌─────────────┬─────────────┐
│   區域 0    │   區域 1    │
│   左上      │   右上      │
│  (0,0)      │  (400,0)    │
│  400×240    │  400×240    │
├─────────────┼─────────────┤
│   區域 2    │   區域 3    │
│   左下      │   右下      │
│  (0,240)    │  (400,240)  │
│  400×240    │  400×240    │
└─────────────┴─────────────┘
```

### 🚀 快速使用

#### 編譯上傳
```bash
cd client_esp8266
arduino-cli compile --fqbn esp8266:esp8266:d1_mini .
arduino-cli upload -p COM5 --fqbn esp8266:esp8266:d1_mini .
```

#### 創建測試圖
```bash
cd server
python create_test_images.py
```

#### 啟動伺服器
```bash
cd server
python server.py
```

#### 發送圖片
```
# 800×480 全螢幕（新功能）
tile test_images_800x480/tile_test_800x480.png

# 400×240 中央顯示（原功能）
image your_image.png
```

### 📈 性能指標

| 項目 | 數值 | 說明 |
|------|------|------|
| 單區塊更新 | ~18 秒 | 使用快速部分更新 |
| 全螢幕更新 | ~72 秒 | 4 區塊依序更新 |
| RAM 使用 | 37% | 30,472 / 80,192 bytes |
| Flash 使用 | 36% | 377,928 / 1,048,576 bytes |
| 區塊緩衝 | 12 KB | 每個 400×240 區塊 |

### 🔧 技術實作

#### ESP8266 端 (Arduino)
- `protocol.h`: 新增 `PROTO_TYPE_TILE` 和 `TileInfo` 結構
- `config.h`: 新增 `TILE_WIDTH`, `TILE_HEIGHT`, `TILE_BUFFER_SIZE`
- `client_esp8266.ino`: 新增 `handleTileUpdate()` 函數

#### 伺服器端 (Python)
- `protocol.py`: 新增 `pack_tile()` 方法
- `image_processor.py`: 新增 `split_image_to_tiles()` 和 `process_tile()`
- `server.py`: 新增 `send_tiled_image()` 方法

### 📦 新增檔案

1. **測試工具**
   - `server/create_test_images.py` - 生成 800×480 測試圖片

2. **文檔**
   - `TILED_DISPLAY_DESIGN.md` - 詳細設計文檔
   - `TILED_DISPLAY_USAGE.md` - 使用指南
   - `RELEASE_NOTES_V1.6.md` - 本文件

3. **測試圖片**（自動生成）
   - `test_images_800x480/tile_test_800x480.png` - 分區標示圖
   - `test_images_800x480/gradient_800x480.png` - 漸變測試
   - `test_images_800x480/checkerboard_800x480.png` - 棋盤格

### 🎯 使用場景

#### 場景 1: 全螢幕圖片展示
```bash
# 顯示 800×480 風景照片
tile landscape.png
```

#### 場景 2: 資訊儀表板
```
┌──────────────┬──────────────┐
│  時間/日期   │  天氣資訊    │
├──────────────┼──────────────┤
│  溫濕度數據  │  系統狀態    │
└──────────────┴──────────────┘
```

#### 場景 3: Logo 中央顯示（原功能）
```bash
# 保持原有 400×240 中央顯示
image logo.png
```

### 📝 API 變更

#### 新增伺服器指令
```python
# Python API
await server.send_tiled_image("image.png")  # 新增

# 互動模式
tile <檔案路徑>  # 新增
```

#### 新增封包類型
```cpp
// Arduino
#define PROTO_TYPE_TILE 0x02  // 新增

// Python
PacketType.TILE_UPDATE = 0x02  # 新增
```

### 🐛 已知問題

1. **更新時間較長**: 4 區塊需 ~72 秒
   - **原因**: 電子紙更新速度限制
   - **緩解**: 使用部分更新（已實作）
   - **未來**: 支援選擇性區塊更新

2. **記憶體限制**: ESP8266 RAM 有限
   - **原因**: 硬體限制（80KB RAM）
   - **緩解**: 分區處理，每次僅 12KB
   - **狀態**: 已優化，RAM 使用 37%

### 🔮 未來計劃

#### v1.7.0 計劃
- [ ] 選擇性區塊更新（只更新變化的區塊）
- [ ] 智能快取機制
- [ ] 動態分區策略（2×2, 4×2, 2×4）

#### v2.0.0 願景
- [ ] 多客戶端管理
- [ ] 圖片隊列系統
- [ ] Web 管理介面
- [ ] OTA 固件更新

### 📚 相關文件

- **使用指南**: `TILED_DISPLAY_USAGE.md`
- **設計文檔**: `TILED_DISPLAY_DESIGN.md`
- **專案總覽**: `README.md`
- **之前版本**: `RELEASE_NOTES_V1.5.md`

### 🙏 致謝

感謝所有測試和反饋的使用者！

### 📥 下載

#### Git Tag
```bash
git clone https://github.com/YourUsername/epaper_display.git
git checkout v1.6.0
```

#### 直接使用
```bash
cd wifi_spi_display
# 按照 TILED_DISPLAY_USAGE.md 的說明操作
```

---

**完整變更記錄**: 見 Git commits
**問題回報**: GitHub Issues
**功能請求**: GitHub Discussions
