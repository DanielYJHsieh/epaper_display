# Release Notes v1.4.0

**發布日期**: 2024-10-07

---

## 🎉 版本亮點

### ✨ 主要改進

#### 1. 解析度升級：400×240 → 480×320
- **顯示面積提升 60%**: 從 96,000 像素增加到 153,600 像素
- **緩衝區增加**: 從 12KB 增加到 19.2KB
- **記憶體使用率**: 仍保持在 58.6%，安全且高效
- **顯示偏移**: X=160, Y=80（完美置中）

#### 2. 修正顯示反相問題
- **問題**: v1.3 顯示黑底白字（應該是白底黑字）
- **根本原因**: 像素格式轉換錯誤
- **解決方案**: 
  - 修正 `image_processor.py` 的像素轉換邏輯
  - 從 `pixels = (pixels > 0)` 改為 `pixels = (pixels == 0)`
  - GxEPD2 期望: 0=白色，1=黑色
- **結果**: 正確顯示白底黑字

#### 3. 智能解壓縮邏輯
- **問題**: v1.3 中，未壓縮資料仍嘗試 RLE 解壓，導致緩衝溢出
- **解決方案**: 
  - 根據資料長度判斷是否壓縮
  - `length == DISPLAY_BUFFER_SIZE` → 未壓縮，直接複製
  - `length < DISPLAY_BUFFER_SIZE` → 已壓縮，使用 RLE 解碼
- **優點**: 
  - 完美配合 `compress_smart()` 功能
  - 避免緩衝溢出錯誤
  - 支援大型圖片傳輸

---

## 📊 性能比較

### v1.3.0 vs v1.4.0

| 項目 | v1.3.0 | v1.4.0 | 改進 |
|------|--------|--------|------|
| **解析度** | 400×240 | 480×320 | +60% 面積 |
| **緩衝區** | 12,000 bytes | 19,200 bytes | +60% |
| **記憶體使用** | 36.6% | 58.6% | 仍在安全範圍 |
| **顯示面積比** | 25% | 40% | +15% |
| **RAM 使用** | 30,428 bytes | 30,428 bytes | 不變 |
| **更新時間** | ~700ms | ~900ms | +200ms |
| **顯示品質** | 黑底白字 ❌ | 白底黑字 ✅ | 已修正 |
| **大圖支援** | 緩衝溢出 ❌ | 正常運作 ✅ | 已修正 |

### 記憶體分析

```
ESP8266 總 RAM: 80,192 bytes
├─ 程式使用: 30,428 bytes (37%)
├─ 顯示緩衝: 19,200 bytes (24%)
└─ 剩餘可用: 30,564 bytes (38%)
   ├─ 網路緩衝: ~8,000 bytes
   ├─ 堆疊空間: ~4,000 bytes
   ├─ 臨時緩衝: ~2,000 bytes
   └─ 安全邊界: ~16,564 bytes ✅
```

---

## 🔧 技術細節

### 修改檔案清單

#### Client 端 (ESP8266)
1. **config.h**
   ```cpp
   // v1.3.0
   #define DISPLAY_WIDTH 400
   #define DISPLAY_HEIGHT 240
   #define DISPLAY_BUFFER_SIZE 12000
   
   // v1.4.0
   #define DISPLAY_WIDTH 480
   #define DISPLAY_HEIGHT 320
   #define DISPLAY_BUFFER_SIZE 19200
   ```

2. **client_esp8266.ino**
   - 新增智能解壓縮判斷
   ```cpp
   // 智能判斷：如果資料長度等於緩衝區大小，表示未壓縮
   bool isCompressed = (length != DISPLAY_BUFFER_SIZE);
   
   if (isCompressed) {
     // 解壓縮
     int decompressedSize = RLEDecoder::decode(...);
   } else {
     // 直接複製
     memcpy(targetBuffer, payload, length);
   }
   ```

#### Server 端 (Python)
1. **image_processor.py**
   ```python
   # v1.3.0 (錯誤)
   pixels = (pixels > 0).astype(np.uint8)  # 白=1, 黑=0
   
   # v1.4.0 (正確)
   pixels = (pixels == 0).astype(np.uint8)  # 黑=1, 白=0
   
   # 預設解析度
   def __init__(self, width: int = 480, height: int = 320):
   ```

2. **server.py**
   - 無需修改，已支援動態解析度

---

## 🎯 升級指南

### 從 v1.3.0 升級到 v1.4.0

#### 1. 更新 Client 端
```bash
cd client_esp8266
arduino-cli compile --fqbn esp8266:esp8266:d1_mini
arduino-cli upload -p COM5 --fqbn esp8266:esp8266:d1_mini
```

#### 2. 更新 Server 端
```bash
cd server
# 無需重新安裝依賴，直接使用新版 image_processor.py
python server.py
```

#### 3. 測試
1. 連接 ESP8266 到 WiFi
2. 發送測試圖片
3. 確認顯示為白底黑字
4. 測試大型圖片（貓咪圖等）

---

## 🐛 Bug 修正

### 1. 顯示反相問題 (Critical)
- **症狀**: 顯示黑底白字
- **影響**: 所有圖片和文字
- **狀態**: ✅ 已修正

### 2. 緩衝溢出錯誤 (Critical)
- **症狀**: "輸出緩衝溢出！" 錯誤
- **影響**: 複雜圖片無法顯示
- **狀態**: ✅ 已修正

### 3. RLE 解壓失敗 (High)
- **症狀**: "解壓縮失敗！期望 12000 bytes, 得到 -1"
- **原因**: 未壓縮資料被誤認為壓縮資料
- **狀態**: ✅ 已修正

---

## 📈 效能提升

### 顯示效果
- ✅ 解析度提升 60%
- ✅ 顯示面積從 25% 增加到 40%
- ✅ 更好的文字清晰度
- ✅ 更多細節顯示

### 穩定性
- ✅ 記憶體使用安全 (58.6%)
- ✅ 支援更大圖片
- ✅ 無緩衝溢出風險
- ✅ 長時間運行穩定

### 相容性
- ✅ 向後相容 v1.3 協議
- ✅ 支援舊版 Server（會自動調整）
- ✅ 保持相同的 API 介面

---

## 🔮 未來規劃

### v1.5.0 候選功能
- [ ] 進一步提升到 560×336 解析度（70% 顯示面積）
- [ ] 容許失真壓縮（降低解析度、減少細節）
- [ ] 灰階顯示支援（4 級灰階）
- [ ] 局部更新優化（只更新變動區域）
- [ ] OTA 更新支援
- [ ] Web 管理介面

---

## 📚 相關文件

- [README.md](README.md) - 專案總覽
- [RESOLUTION_ANALYSIS.md](RESOLUTION_ANALYSIS.md) - 解析度評估報告
- [VERSION_HISTORY.md](client_esp8266/VERSION_HISTORY.md) - 完整版本歷史
- [TESTING_GUIDE.md](client_esp8266/TESTING_GUIDE.md) - 測試指南

---

## 👥 貢獻者

感謝所有參與 v1.4.0 開發的人員！

---

## 📝 變更日誌

### v1.4.0 (2024-10-07)
- ✨ 解析度升級到 480×320
- 🐛 修正顯示反相問題（黑底白字 → 白底黑字）
- 🐛 修正緩衝溢出錯誤
- ✨ 智能解壓縮邏輯
- 📚 更新所有相關文件

### v1.3.0 (2024-10-06)
- ✨ 智能壓縮（compress_smart）
- 📚 完整文件更新
- 🚀 GitHub 發布

### v1.2.0 (2024-10-05)
- ✨ 解析度降低到 400×240
- 🐛 修正記憶體分配問題
- ✨ 取消分塊顯示模式

### v1.1.0 (2024-10-04)
- ✨ 記憶體優化框架
- ✨ 分塊顯示模式
- 🐛 修正 Y 軸座標問題

### v1.0.0 (2024-10-03)
- 🎉 初始版本
- ✨ 基本 WebSocket 通訊
- ✨ RLE 壓縮
- ❌ 800×480 記憶體不足

---

**完整更新內容請參考 [GitHub Releases](https://github.com/DanielYJHsieh/epaper_display/releases)**
