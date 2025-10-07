# GitHub Upload Guide - v1.3

**日期**: 2025-10-07  
**版本**: v1.3.0

---

## 📦 準備上傳內容

### 核心檔案

#### Client 端 (client_esp8266/)
✅ **程式檔案**:
- `client_esp8266.ino` - 主程式 (已更新: 修正反相, 記憶體保護)
- `config.h` - 配置 (已更新: 400x240 解析度)
- `protocol.h` - 協議 (已更新: 記憶體檢查, yield())
- `rle_decoder.h` - 解碼器 (已更新: yield())
- `GxEPD2_display_selection_GDEQ0426T82.h` - 顯示器配置

✅ **文檔檔案**:
- `README.md` - 客戶端說明
- `RESOLUTION_OPTIMIZATION_V1.2.md` - 解析度優化
- `ESP8266_HARDWARE_LIMITATIONS.md` - 硬體限制分析
- `IMAGE_TRANSFER_ISSUE.md` - 圖像傳輸問題診斷
- `CHUNKED_MODE_FIX.md` - 分塊模式修正
- `MEMORY_OPTIMIZATION_REPORT.md` - 記憶體優化報告
- `VERSION_HISTORY.md` - 版本歷史

#### Server 端 (server/)
✅ **程式檔案**:
- `server.py` - WebSocket 伺服器 (已更新: 智能壓縮)
- `image_processor.py` - 圖像處理 (已更新: 400x240 預設)
- `compressor.py` - 壓縮算法 (已更新: compress_smart())
- `protocol.py` - 協議定義
- `requirements.txt` - Python 依賴

✅ **文檔檔案**:
- `README.md` - 伺服器說明
- `INSTALL.md` - 安裝指南

#### 專案根目錄 (wifi_spi_display/)
✅ **主要文檔**:
- `README.md` - 專案總覽 (已更新: v1.3 特性)
- `RELEASE_NOTES_V1.3.md` - 版本說明 (新建)

---

## 🎯 版本重點

### v1.3.0 主要更新

1. **智能壓縮系統**
   - `compressor.py`: 新增 `compress_smart()` 方法
   - `server.py`: 所有壓縮點改用智能壓縮
   - 效果: 避免壓縮率變負，支援複雜圖像

2. **顯示修正**
   - `client_esp8266.ino`: writeImage invert 參數 true → false
   - 效果: 修正黑白反相問題

3. **記憶體保護**
   - `protocol.h`: 分配前檢查記憶體 + yield()
   - `rle_decoder.h`: 解壓時 yield()
   - 效果: 防止看門狗重啟

### v1.2.0 基礎優化

1. **解析度調整**
   - 從 800×480 降至 400×240
   - 緩衝區從 48KB 降至 12KB
   - 記憶體使用率從 120% 降至 25%

2. **關閉分塊模式**
   - `config.h`: ENABLE_CHUNKED_DISPLAY = 0
   - 使用完整緩衝，速度更快

---

## 📝 Commit 訊息建議

### 主要 Commit
```bash
git add .
git commit -m "Release v1.3.0 - 智能壓縮系統與顯示修正

主要更新:
- 新增智能壓縮: 自動選擇 RLE 或無壓縮，避免壓縮失效
- 修正黑白反相: writeImage invert 參數改為 false
- 記憶體保護: 分配前檢查 + 看門狗餵食
- 文檔完善: 新增 RELEASE_NOTES_V1.3.md

基於 v1.2.0:
- 解析度優化至 400×240 (中央顯示)
- 緩衝區從 48KB 降至 12KB
- ESP8266 記憶體使用率 37%

詳細變更請參閱 RELEASE_NOTES_V1.3.md"
```

### 檔案變更摘要
```
修改的檔案:
  client_esp8266/client_esp8266.ino  (反相修正)
  client_esp8266/protocol.h          (記憶體檢查)
  client_esp8266/rle_decoder.h       (yield 餵食)
  server/compressor.py               (智能壓縮)
  server/server.py                   (使用智能壓縮)
  wifi_spi_display/README.md         (更新說明)

新增的檔案:
  wifi_spi_display/RELEASE_NOTES_V1.3.md

從 v1.2.0 延續:
  client_esp8266/config.h            (400x240 解析度)
  client_esp8266/RESOLUTION_OPTIMIZATION_V1.2.md
  client_esp8266/ESP8266_HARDWARE_LIMITATIONS.md
  client_esp8266/IMAGE_TRANSFER_ISSUE.md
```

---

## 🚀 GitHub 上傳步驟

### 1. 檢查狀態
```bash
cd d:\1_myproject\epaper_display\epaper_display\wifi_spi_display
git status
```

### 2. 添加所有變更
```bash
git add .
```

### 3. 確認變更
```bash
git status
git diff --cached
```

### 4. 提交變更
```bash
git commit -m "Release v1.3.0 - 智能壓縮系統與顯示修正

主要更新:
- 新增智能壓縮: 自動選擇 RLE 或無壓縮
- 修正黑白反相問題
- 記憶體保護與看門狗餵食
- 完善文檔說明

技術規格:
- 解析度: 400×240 (中央顯示)
- 緩衝區: 12KB
- RAM 使用: 37%
- 壓縮率: 自動優化

詳見 RELEASE_NOTES_V1.3.md"
```

### 5. 推送到 GitHub
```bash
git push origin main
```

或如果使用 master 分支:
```bash
git push origin master
```

### 6. 建立 Release Tag
```bash
git tag -a v1.3.0 -m "Version 1.3.0 - 智能壓縮與顯示修正"
git push origin v1.3.0
```

---

## 📋 GitHub Release 說明

### Release Title
```
v1.3.0 - 智能壓縮系統與顯示修正
```

### Release Description
```markdown
# WiFi E-Paper Display v1.3.0

## 🎉 主要更新

### 智能壓縮系統
- 自動比較 RLE 和無壓縮，選擇較小的方式
- 避免壓縮率變負（壓縮後變大）
- 支援複雜圖像傳輸

### 顯示修正
- 修正黑白反相問題
- 圖像現在正確顯示（白底黑字）

### 記憶體保護
- 分配前檢查可用記憶體
- 看門狗餵食防止重啟
- 保留 8KB 安全餘量

## 📊 技術規格

- **解析度**: 400×240 pixels (螢幕中央)
- **緩衝區**: 12KB
- **RAM 使用**: 37% (30.4KB / 80KB)
- **Flash 使用**: 35% (375KB / 1MB)
- **壓縮率**: 70-95% (視內容而定)

## 📦 檔案說明

### 必要檔案
- `client_esp8266/` - ESP8266 客戶端程式
- `server/` - Python WebSocket 伺服器
- `RELEASE_NOTES_V1.3.md` - 完整版本說明

### 快速開始
1. 安裝 Python 依賴: `pip install -r server/requirements.txt`
2. 編輯 `client_esp8266/config.h` 設定 WiFi
3. 上傳韌體到 ESP8266
4. 啟動伺服器: `python server/server.py`
5. 測試: 輸入 `t` 發送測試圖案

## 🐛 已知問題
- 複雜照片可能無壓縮效果（會使用原始 12KB 傳輸）
- E-Paper 刷新時間約 700ms

## 📚 文檔
- [專案 README](README.md)
- [版本說明](RELEASE_NOTES_V1.3.md)
- [硬體限制分析](client_esp8266/ESP8266_HARDWARE_LIMITATIONS.md)
- [圖像傳輸診斷](client_esp8266/IMAGE_TRANSFER_ISSUE.md)

## 🙏 致謝
- GxEPD2 Library
- ESP8266 Arduino Core
- WebSockets Library
```

---

## ✅ 檢查清單

上傳前確認:

### 程式碼
- [ ] client_esp8266.ino 編譯成功
- [ ] server.py 可正常執行
- [ ] 測試圖案顯示正常
- [ ] 文字顯示正常
- [ ] 圖像顯示正確（無反相）

### 文檔
- [ ] README.md 更新至 v1.3
- [ ] RELEASE_NOTES_V1.3.md 完整
- [ ] 版本號一致 (v1.3.0)
- [ ] 技術規格正確

### Git
- [ ] .gitignore 包含臨時檔案
- [ ] 沒有敏感資料 (WiFi 密碼等)
- [ ] Commit 訊息清楚
- [ ] Tag 版本正確

---

## 🔒 注意事項

### 不要上傳
```
client_esp8266/build/          # 編譯輸出
server/__pycache__/            # Python 快取
*.pyc                          # Python 編譯檔
.vscode/                       # IDE 設定
*.log                          # 日誌檔案
config.h (如有私密資料)        # WiFi 密碼
```

### .gitignore 建議
```gitignore
# Build output
build/
*.bin
*.elf
*.map

# Python
__pycache__/
*.pyc
*.pyo
*.egg-info/

# IDE
.vscode/
.idea/
*.swp

# Logs
*.log

# Test files
test_*.png
test_*.jpg
simple_test.*

# Private config (optional)
# config.h
```

---

## 📊 上傳後驗證

1. **GitHub 頁面**
   - [ ] README.md 正確顯示
   - [ ] 檔案結構完整
   - [ ] Release 創建成功

2. **下載測試**
   ```bash
   git clone https://github.com/你的帳號/epaper_display.git
   cd epaper_display/wifi_spi_display
   # 測試編譯和執行
   ```

3. **文檔鏈結**
   - [ ] 所有內部連結有效
   - [ ] 圖片正確顯示
   - [ ] 程式碼區塊格式正確

---

## 🎯 完成

上傳完成後:
1. 在 GitHub 建立 Release v1.3.0
2. 更新專案描述
3. 添加 Topics (標籤): `esp8266`, `e-paper`, `websocket`, `iot`, `epd`
4. 考慮添加 Screenshots

---

**準備就緒！可以開始上傳了！** 🚀
