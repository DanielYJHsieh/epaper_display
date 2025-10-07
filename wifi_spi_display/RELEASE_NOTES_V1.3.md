# WiFi E-Paper Display - Release Notes v1.3

**發布日期**: 2025-10-07  
**版本**: v1.3.0  
**狀態**: ✅ 穩定版本

---

## 🎉 重大更新

### 1. 解析度優化 (v1.2)
- **從 800×480 降至 400×240**
- 顯示在 800×480 螢幕中央
- 緩衝區需求從 48KB 降至 12KB
- ESP8266 記憶體使用率從 120% 降至 25%

### 2. 智能壓縮系統 (v1.3)
- **自動選擇最優壓縮方式**
- 比較 RLE 和無壓縮，選擇較小的
- 避免壓縮率變負（壓縮後變大）
- 支援複雜圖像傳輸

### 3. 顯示修正 (v1.3)
- **修正黑白反相問題**
- 圖像正確顯示（不再黑底白字）
- writeImage invert 參數設為 false

### 4. 記憶體保護 (v1.2)
- 分配前檢查可用記憶體
- 保留 8KB 安全餘量
- 防止看門狗重啟

---

## 📊 技術規格

### 硬體配置
| 項目 | 規格 |
|------|------|
| **微控制器** | ESP8266 (WeMos D1 Mini) |
| **RAM** | ~96KB 總量, ~45KB 可用 |
| **顯示器** | GDEQ0426T82 4.26" E-Paper |
| **實體解析度** | 800×480 pixels |
| **使用解析度** | 400×240 pixels (中央區域) |
| **緩衝區** | 12KB (400×240÷8) |

### 記憶體使用
| 項目 | 大小 | 佔比 |
|------|-----|------|
| **程式 RAM** | 30,428 bytes | 37% |
| **Flash** | 375,768 bytes | 35% |
| **顯示緩衝** | 12,000 bytes | ~27% 可用記憶體 |
| **可用餘量** | ~15,000 bytes | 安全充足 |

---

## 🚀 功能特點

### 支援的內容類型

#### ✅ 高效壓縮 (70-95%)
- **文字顯示**：標題、訊息、通知
- **QR Code**：二維碼、條碼
- **簡單圖形**：Logo、圖標、UI 元素
- **測試圖案**：邊框、線條、十字

#### ⚠️ 中等效果 (30-70%)
- **簡單照片**：高對比度、低細節
- **卡通圖像**：大色塊、少紋理

#### ❌ 不適合 (壓縮失效)
- **複雜照片**：漸層、紋理、噪點
- **彩色圖片**（轉換後細節丟失）

### 智能壓縮邏輯

```python
# 自動選擇最佳方式
compressed, is_compressed = compressor.compress_smart(data)

if len(compressed) < len(data):
    # 使用 RLE 壓縮
    send(compressed, mode="RLE")
else:
    # 使用原始資料
    send(data, mode="RAW")
```

**效果**：
- 簡單圖像：壓縮率 90%，傳輸 1.2KB
- 複雜圖像：不壓縮，傳輸 12KB（避免 21KB）

---

## 🔧 修正問題

### v1.3.0 (2025-10-07)

#### 🐛 修正
1. **黑白反相**：
   - 問題：圖像顯示為黑底白字
   - 原因：writeImage 的 invert 參數為 true
   - 修正：改為 false

2. **壓縮失效**：
   - 問題：複雜圖像壓縮後變大 82%
   - 原因：RLE 對複雜資料無效
   - 修正：智能選擇，比較後使用較小方式

3. **記憶體不足**：
   - 問題：接收 21KB 資料導致 ESP8266 重啟
   - 原因：壓縮後資料 + 解壓緩衝 = 33KB
   - 修正：智能壓縮避免資料變大

#### ✨ 新功能
1. **compress_smart()** 方法
2. 壓縮方式自動選擇
3. 詳細日誌（顯示使用的壓縮方式）

### v1.2.0 (2025-10-07)

#### 🔄 重大變更
1. 解析度從 800×480 降至 400×240
2. 關閉分塊模式（改用完整緩衝）
3. 顯示位置調整到螢幕中央

#### ✨ 改進
1. 記憶體檢查和保護
2. 看門狗餵食（yield）
3. 詳細記憶體日誌

---

## 📝 使用指南

### 伺服器端

#### 啟動
```bash
cd server
python server.py
```

#### 指令

**發送測試圖案**：
```
>>> t
```

**發送文字**：
```
>>> text Hello World
>>> text "多行文字\n第二行" 48
```

**發送圖像**：
```
>>> image simple.png
>>> image test.jpg
```

**退出**：
```
>>> q
```

### 客戶端

#### 上傳韌體
```bash
cd client_esp8266
arduino-cli compile --fqbn esp8266:esp8266:d1_mini client_esp8266.ino
arduino-cli upload --fqbn esp8266:esp8266:d1_mini --port COM5 client_esp8266.ino
```

#### 設定檔

編輯 `config.h`：
```cpp
#define WIFI_SSID "你的WiFi名稱"
#define WIFI_PASSWORD "你的WiFi密碼"
#define SERVER_HOST "192.168.x.x"
#define SERVER_PORT 8266
```

---

## 📈 效能數據

### 測試結果

| 內容類型 | 原始大小 | 壓縮後 | 壓縮率 | 傳輸時間 | 顯示時間 |
|---------|---------|--------|--------|----------|----------|
| **測試圖案** | 12KB | 1.3KB | 89% | ~50ms | ~700ms |
| **文字** | 12KB | 1.2KB | 90% | ~50ms | ~700ms |
| **QR Code** | 12KB | 0.6KB | 95% | ~30ms | ~700ms |
| **簡單圖** | 12KB | 3-4KB | 70% | ~150ms | ~700ms |
| **複雜圖** | 12KB | 12KB | 0% | ~500ms | ~700ms |

### 記憶體峰值

| 階段 | 記憶體使用 | 說明 |
|------|-----------|------|
| **啟動後** | ~45KB 可用 | 系統就緒 |
| **接收資料** | 1-12KB | 壓縮資料緩衝 |
| **解壓縮** | +12KB | 顯示緩衝 |
| **總需求** | 13-24KB | 安全範圍內 |

---

## 🎯 適用場景

### ✅ 推薦用途

1. **資訊顯示**
   - 天氣資訊
   - 時間日期
   - 通知訊息

2. **QR Code 顯示**
   - WiFi QR code
   - 網址連結
   - 電子票券

3. **簡單 UI**
   - 選單介面
   - 圖標導航
   - 狀態指示

4. **文字內容**
   - 新聞標題
   - 提醒事項
   - 標籤標示

### ❌ 不適合

1. 照片展示
2. 複雜圖像顯示
3. 高頻率更新（E-Paper 限制）
4. 即時影片

---

## 🔮 未來計劃

### 短期 (1 週內)

- [ ] 添加容許失真的壓縮選項
- [ ] 圖像預處理工具（自動優化）
- [ ] 壓縮率預測

### 中期 (1 個月內)

- [ ] 分塊傳輸協議（支援更大圖像）
- [ ] 差分更新（只傳輸變化部分）
- [ ] 多重壓縮算法（LZ4, PNG等）

### 長期

- [ ] ESP32 版本（520KB RAM，支援全解析度）
- [ ] 雙色/三色E-Paper 支援
- [ ] Web UI 管理介面

---

## 📚 文件清單

### 客戶端 (client_esp8266/)

- `README.md` - 快速開始指南
- `config.h` - 配置文件
- `client_esp8266.ino` - 主程式
- `protocol.h` - 通訊協議
- `rle_decoder.h` - RLE 解碼器
- `GxEPD2_display_selection_GDEQ0426T82.h` - 顯示器配置

### 伺服器 (server/)

- `README.md` - 伺服器說明
- `server.py` - WebSocket 伺服器
- `image_processor.py` - 圖像處理
- `compressor.py` - 壓縮算法
- `protocol.py` - 協議定義
- `requirements.txt` - Python 依賴

### 文檔 (client_esp8266/)

- `RESOLUTION_OPTIMIZATION_V1.2.md` - 解析度優化說明
- `ESP8266_HARDWARE_LIMITATIONS.md` - 硬體限制分析
- `IMAGE_TRANSFER_ISSUE.md` - 圖像傳輸問題診斷
- `CHUNKED_MODE_FIX.md` - 分塊模式修正
- `MEMORY_OPTIMIZATION_REPORT.md` - 記憶體優化報告
- `VERSION_HISTORY.md` - 完整版本歷史

---

## 🙏 致謝

- [GxEPD2](https://github.com/ZinggJM/GxEPD2) - E-Paper 顯示庫
- [ESP8266 Arduino Core](https://github.com/esp8266/Arduino)
- [WebSockets](https://github.com/Links2004/arduinoWebSockets)

---

## 📄 授權

MIT License

---

## 🐛 問題回報

如遇到問題，請提供：
1. ESP8266 序列輸出日誌
2. 伺服器端日誌
3. 使用的圖像類型和大小
4. config.h 配置

---

**版本**: v1.3.0  
**發布日期**: 2025-10-07  
**維護者**: Daniel  
**狀態**: ✅ 穩定運行
