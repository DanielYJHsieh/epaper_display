# 🚀 快速啟動指南

## ✅ 已完成

1. ✅ 創建 `server_with_webui.py` - 包含 Web UI 的完整 Server
2. ✅ 安裝 `aiohttp` - Web UI 所需套件
3. ✅ 完全向後相容 - 原 `server.py` 仍可使用

---

## 🎯 立即使用

### PC (Windows) - 現在就能用！

```powershell
# Step 1: 啟動 Server
cd D:\1_myproject\epaper_display\epaper_display\wifi_spi_display\server
python server_with_webui.py

# Step 2: 開啟瀏覽器
start http://localhost:8080
```

### 手機 (Termux) - 需要先安裝

```bash
# Step 1: 安裝 Termux (Google Play Store)

# Step 2: 設定環境
pkg update && pkg upgrade
pkg install python git
pip install websockets pillow numpy aiohttp

# Step 3: 下載專案
cd ~
git clone https://github.com/DanielYJHsieh/epaper_display.git
cd epaper_display/wifi_spi_display/server

# Step 4: 執行
python server_with_webui.py

# Step 5: 在手機瀏覽器開啟
http://localhost:8080
```

---

## 🌐 網頁界面功能

開啟瀏覽器後，你會看到：

```
┌─────────────────────────────────┐
│ 📱 ESP8266 電子紙顯示器         │
├─────────────────────────────────┤
│ 狀態                            │
│ • Server: 運行中                │
│ • 已連接設備: 1                 │
│ • 狀態: 待命中                  │
├─────────────────────────────────┤
│ 📸 選擇圖片                     │
│ [點擊選擇]                      │
│ [預覽圖片]                      │
├─────────────────────────────────┤
│ 🎮 操作                         │
│ [📤 傳送到顯示器]               │
│ [🎨 測試圖案]                   │
│ [🗑️ 清除螢幕]                   │
└─────────────────────────────────┘
```

### 使用步驟

1. **查看狀態** - 確認 ESP8266 已連接
2. **選擇圖片** - 點擊上傳區域
3. **預覽** - 查看選擇的圖片
4. **傳送** - 點擊「傳送到顯示器」
5. **等待** - 約 52 秒完成（3 個條帶）

---

## 📊 兩種模式

### 模式 1: Web UI（圖形界面）

**適合**：
- 日常使用
- 手機操作
- 圖片瀏覽

**優點**：
- ✅ 視覺化
- ✅ 簡單易用
- ✅ 即時狀態

### 模式 2: 命令列（原有方式）

**適合**：
- 自動化腳本
- 批次處理
- 開發測試

**使用**：
Server 啟動後，在命令列輸入：

```
>>> clients    # 查看連接
>>> test       # 測試圖案
>>> tile img.png  # 傳送圖片
>>> web        # 顯示網頁連結
>>> quit       # 結束
```

**兩種模式可以同時使用！**

---

## 🔧 ESP8266 設定

確保 ESP8266 的 `config.h` 設定正確：

```cpp
// 修改為 Server 的 IP
#define SERVER_HOST "192.168.0.100"  // Server 啟動時會顯示
#define SERVER_PORT 8266              // 不變
```

### 如何取得 Server IP？

**方法 1**: Server 啟動時自動顯示
```
🌐 網頁控制介面:
   本機存取: http://localhost:8080
   網路存取: http://192.168.0.100:8080  ← 這個 IP

💡 ESP8266 設定:
   #define SERVER_HOST "192.168.0.100"  ← 使用這個
```

**方法 2**: 網頁界面底部顯示
```
本機 IP: 192.168.0.100
```

---

## 📁 檔案說明

```
server/
├── server.py                # 原版（命令列）
├── server_with_webui.py     # 新版（Web UI + 命令列）✨
├── SERVER_WEBUI_GUIDE.md    # 完整使用說明
├── install_webui_deps.bat   # Windows 快速安裝
└── ...（其他檔案）
```

### 選擇使用

- **推薦**: `server_with_webui.py` - 功能更多
- **輕量**: `server.py` - 純命令列

---

## 💡 使用場景

### 場景 A: PC 開發測試
```
1. PC 執行 server_with_webui.py
2. ESP8266 連接到 PC
3. PC 瀏覽器控制 (localhost:8080)
```

### 場景 B: 手機獨立使用
```
1. 手機 Termux 執行 server_with_webui.py
2. ESP8266 連接到手機熱點
3. 手機瀏覽器控制 (localhost:8080)
4. 從相簿選圖傳送
```

### 場景 C: 遠端控制
```
1. 任一設備執行 server_with_webui.py
2. 其他設備用瀏覽器控制
3. http://192.168.0.XXX:8080
```

---

## ⚡ 快速測試

想馬上測試？

```powershell
# 1. 確認 ESP8266 已上傳最新韌體並連接
# 2. 確認 ESP8266 config.h 的 IP 正確
# 3. 執行

cd D:\1_myproject\epaper_display\epaper_display\wifi_spi_display\server
python server_with_webui.py

# 4. 瀏覽器開啟 http://localhost:8080
# 5. 等待 ESP8266 連接（看到「已連接設備: 1」）
# 6. 點擊「傳送測試圖案」
# 7. 觀察 ESP8266 顯示棋盤格
```

---

## ❓ 常見問題

**Q: 可以在 PC 和手機同時執行嗎？**
A: 不行，同時只能有一個 Server。但可以從多個瀏覽器存取同一個 Server。

**Q: Web UI 會取代命令列嗎？**
A: 不會，兩者並存。命令列仍可使用。

**Q: 需要重新編譯 ESP8266 嗎？**
A: 不需要！ESP8266 端完全不需改動。

**Q: 網頁可以從其他電腦存取嗎？**
A: 可以！使用 `http://192.168.0.XXX:8080`

**Q: 手機瀏覽器可以嗎？**
A: 可以！Chrome、Firefox、Safari 都支援。

---

## 📚 詳細文檔

- `SERVER_WEBUI_GUIDE.md` - 完整使用說明
- `ANDROID_APP_OPTIONS.md` - Android App 選項
- `MOBILE_INTEGRATION_PLAN.md` - 手機整合方案

---

## 🎉 總結

### ✅ 你現在擁有

1. **Web UI** - 現代化圖形界面
2. **命令列** - 原有功能保留
3. **跨平台** - PC 和手機都能用
4. **向後相容** - 無需修改 ESP8266

### 🚀 立即開始

```bash
python server_with_webui.py
```

然後開啟瀏覽器：`http://localhost:8080`

享受你的 ESP8266 電子紙顯示器！🎨
