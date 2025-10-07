# Release Notes v1.5.0

## 發布日期
2025-10-08

## 版本摘要
**顯示性能重大優化版本** - 修正顯示更新速度問題，將更新時間從 60 秒降至 18 秒，提升 3.3 倍速度。

## 主要改進

### 🚀 性能優化
1. **顯示更新速度提升 3.3 倍**
   - 修正前：每次更新 ~60 秒（包含完整刷新）
   - 修正後：每次更新 ~18 秒（快速部分更新）
   - 提升幅度：**70% 時間減少**

2. **WebSocket 連線穩定性改善**
   - 修正前：更新期間 WebSocket 經常斷線
   - 修正後：更新期間保持連線穩定
   - 原因：更新時間縮短，不再超過 WebSocket 超時時間

### 🔧 技術修正

#### 1. 智能未壓縮資料檢測
**問題**：當伺服器發送未壓縮資料時，客戶端仍嘗試 RLE 解壓縮，導致緩衝區溢位錯誤。

**解決方案**：
```cpp
// 智能判斷：如果資料長度等於緩衝區大小，表示未壓縮
bool isCompressed = (length != DISPLAY_BUFFER_SIZE);

if (isCompressed) {
  // 資料已壓縮，需要解壓縮
  decompressedSize = RLEDecoder::decode(payload, length, targetBuffer, DISPLAY_BUFFER_SIZE);
} else {
  // 資料未壓縮，直接使用
  memcpy(targetBuffer, payload, length);
}
```

**效果**：
- ✅ 修正緩衝區溢位錯誤
- ✅ 支援壓縮和未壓縮資料
- ✅ 自動選擇最佳傳輸方式

#### 2. 顯示反相修正
**問題**：畫面顯示黑底白字（應該是白底黑字）。

**解決方案**：
```cpp
display.writeImage(frame, 0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, false, false, true);
// 最後參數 true = 反相，修正黑底白字問題
```

**效果**：
- ✅ 正確顯示白底黑字
- ✅ 與實際內容一致

#### 3. 快速部分更新優化
**問題**：
- `#if ENABLE_CHUNKED_DISPLAY` 分支使用 `refresh(false)` ✅
- `#else` 分支使用 `refresh()` ❌（預設 = true = 完整刷新）
- 導致每次更新都執行完整刷新（包含清除螢幕）

**解決方案**：
```cpp
#else
  // 傳統模式：使用部分窗口快速更新
  display.setPartialWindow(DISPLAY_OFFSET_X, DISPLAY_OFFSET_Y, DISPLAY_WIDTH, DISPLAY_HEIGHT);
  display.writeImage(frame, 0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, false, false, true);
  display.refresh(false);  // false = 快速部分更新
#endif
```

**效果**：
- ✅ 所有更新都使用快速部分更新
- ✅ 不再重複清除螢幕
- ✅ 更新時間從 60 秒降至 18 秒

#### 4. 啟動時螢幕清除優化
**問題**：
- 原本在第一次 `displayFrame()` 調用時才清除螢幕
- 導致第一張圖片更新時仍會清除螢幕

**解決方案**：
```cpp
void setup() {
  // ... WiFi 和 WebSocket 初始化 ...
  
  // 初始化電子紙顯示器並清除螢幕
  display.init(SERIAL_BAUD);
  display.setFullWindow();
  display.clearScreen();
  display.refresh(true);  // 完整刷新以確保清除乾淨
  
  // 之後所有更新都使用快速部分更新
}
```

**效果**：
- ✅ Reset/上電時立即清除螢幕
- ✅ 第一張圖片使用快速更新（~18 秒）
- ✅ 所有圖片更新速度一致

## 性能對比

### 更新時間對比
| 更新類型 | v1.4 及以前 | v1.5 | 改善幅度 |
|---------|------------|------|---------|
| 第一次更新 | ~60 秒 | ~18 秒 | **70% ↓** |
| 後續更新 | ~60 秒 | ~18 秒 | **70% ↓** |
| Reset 清屏 | 第一次更新時 | 啟動時 | 體驗優化 |

### WebSocket 穩定性
| 指標 | v1.4 及以前 | v1.5 | 改善幅度 |
|------|------------|------|---------|
| 更新期間斷線 | 經常發生 | 不再發生 | **100% ↑** |
| 連線保持 | 不穩定 | 穩定 | 大幅改善 |

### GxEPD2 Refresh 模式說明
| 調用方式 | 模式 | 執行階段 | 時間 | 適用場景 |
|---------|------|---------|------|---------|
| `refresh()` 或 `refresh(true)` | 完整刷新 | _Update_Full + _Update_Part | ~60 秒 | 清除殘影、初始化 |
| `refresh(false)` | 快速部分更新 | _Update_Part 只 | ~18 秒 | 日常更新 |

## 技術細節

### 顯示更新流程

#### v1.4 及以前（問題版本）
```
[圖片更新請求]
  ↓
displayFrame()
  ↓
setFullWindow() 或 setPartialWindow()
  ↓
writeImage()
  ↓
refresh() 或 refresh(true)  ← 問題：完整刷新
  ↓
_Update_Full (~42 秒)
  ↓
_Update_Part (~18 秒)
  ↓
總計：~60 秒
WebSocket 斷線 ❌
```

#### v1.5（修正版本）
```
[上電/Reset]
  ↓
setup()
  ↓
display.init()
  ↓
clearScreen() + refresh(true)  ← 只在啟動時執行一次
  ↓
[圖片更新請求]
  ↓
displayFrame()
  ↓
setPartialWindow()
  ↓
writeImage()
  ↓
refresh(false)  ← 快速部分更新
  ↓
_Update_Part (~18 秒)
  ↓
完成
WebSocket 保持連線 ✅
```

## 解析度評估

### 記憶體分析
- **ESP8266 總 RAM**: 80,192 bytes
- **目前使用**: 30,428 bytes (37%)
- **可用記憶體**: ~49,764 bytes
- **安全顯示緩衝**: ~32,000 bytes (扣除網路和系統開銷)

### 推薦解析度
| 解析度 | 緩衝區 | 記憶體使用率 | 顯示面積比 | 狀態 | 建議 |
|--------|--------|-------------|-----------|------|------|
| **400×240** | 12,000 bytes | 36.6% | 25.0% | ✅ 非常安全 | **目前** |
| **480×320** | 19,200 bytes | 58.6% | 40.0% | ✅ 安全 | **推薦升級** |
| **560×336** | 23,520 bytes | 71.8% | 49.0% | ✅ 可行 | 進階選項 |
| 600×400 | 30,000 bytes | 91.6% | 62.5% | ⚠️ 極限 | 不建議 |
| 800×480 | 48,000 bytes | 146.5% | 100% | ❌ 不可行 | 超出限制 |

詳細分析請參閱：[RESOLUTION_ANALYSIS.md](RESOLUTION_ANALYSIS.md)

## 文件更新

### 新增文件
- ✅ `DISPLAY_SPEED_FIX.md` - 顯示速度優化詳細說明
- ✅ `RESOLUTION_ANALYSIS.md` - 解析度評估報告
- ✅ `RELEASE_NOTES_V1.5.md` - 本版本說明

### 更新文件
- ✅ `README.md` - 更新版本號和功能說明
- ✅ `VERSION_HISTORY.md` - 新增 v1.5 記錄

## 升級指南

### 從 v1.4 升級到 v1.5
1. 下載最新固件
2. 上傳到 ESP8266
3. 按 Reset 鍵重新啟動
4. 觀察螢幕立即清除
5. 發送圖片測試（應在 ~18 秒內完成）

### 升級注意事項
- ✅ 無需修改伺服器代碼
- ✅ 無需修改配置檔案
- ✅ 向下相容 v1.3 和 v1.4 的壓縮格式
- ✅ 自動檢測壓縮/未壓縮資料

## 已知問題與限制

### 殘影問題
- **現象**：長期使用快速部分更新可能累積殘影
- **建議**：每 20-50 次更新後執行一次完整刷新
- **未來版本**：將實作自動定期完整刷新功能

### 解析度限制
- **目前**：400×240（只使用 25% 顯示器面積）
- **推薦**：升級到 480×320（40% 面積，記憶體安全）
- **限制**：800×480 無法實現（需要 48KB，超出 ESP8266 限制）

## 測試驗證

### 測試環境
- ESP8266 (WeMos D1 Mini)
- GDEQ0426T82 4.26" E-Paper Display
- Arduino CLI 0.35.3
- GxEPD2 Library 1.5.7
- Python 3.x Server

### 測試結果
| 測試項目 | 結果 | 說明 |
|---------|------|------|
| 編譯成功 | ✅ | RAM 37%, Flash 35% |
| 上傳成功 | ✅ | COM5, 460800 baud |
| Reset 清屏 | ✅ | 啟動時立即清除 |
| 第一張圖片 | ✅ | ~18 秒快速更新 |
| 第二張圖片 | ✅ | ~18 秒快速更新 |
| 文字顯示 | ✅ | 正確顯示白底黑字 |
| 圖片顯示 | ✅ | 正確顯示無殘影 |
| 未壓縮資料 | ✅ | 自動檢測並處理 |
| 壓縮資料 | ✅ | 正常 RLE 解壓縮 |
| WebSocket 穩定性 | ✅ | 更新期間保持連線 |

## 貢獻者
- Daniel YJ Hsieh (@DanielYJHsieh)

## 下一步計劃

### v1.6 規劃
- [ ] 解析度升級到 480×320
- [ ] 自動定期完整刷新（每 N 次更新）
- [ ] 壓縮率統計和日誌
- [ ] OTA 固件更新支援

### v2.0 規劃
- [ ] 支援多種圖片格式（JPEG, PNG）
- [ ] 本地圖片緩存
- [ ] 電池供電模式優化
- [ ] 深度睡眠支援

## 相關資源
- [GitHub Repository](https://github.com/DanielYJHsieh/epaper_display)
- [顯示速度優化說明](DISPLAY_SPEED_FIX.md)
- [解析度評估報告](RESOLUTION_ANALYSIS.md)
- [版本歷史](VERSION_HISTORY.md)
- [快速開始](QUICKSTART.md)

## 授權
MIT License

---

**版本**: v1.5.0  
**發布日期**: 2025-10-08  
**狀態**: ✅ 穩定版本
