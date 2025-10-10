# Server with Web UI - 使用說明

## ✅ 特點

**完全向後相容** + **新增 Web UI**

- ✅ **在 PC 上執行** - Windows/Linux/Mac 都可以
- ✅ **在手機上執行** - Android (Termux) / iOS (iSH)
- ✅ **圖形界面** - 網頁控制（port 8080）
- ✅ **命令列** - 原有互動模式保留
- ✅ **無需額外安裝** - 只需 `aiohttp` 套件

---

## 📦 安裝依賴

### PC (Windows/Linux/Mac)

```bash
# 已有的依賴
pip install websockets pillow numpy

# 新增：Web UI 需要
pip install aiohttp
```

### 手機 (Termux)

```bash
pkg install python
pip install websockets pillow numpy aiohttp
```

---

## 🚀 使用方式

### 方法 1：使用 Web UI（推薦）

**Step 1: 啟動 Server**
```bash
cd server
python server_with_webui.py
```

**Step 2: 查看輸出**
```
WebSocket Server 啟動: ws://0.0.0.0:8266
HTTP Server 啟動: http://0.0.0.0:8080

🌐 網頁控制介面:
   本機存取: http://localhost:8080
   網路存取: http://192.168.0.100:8080

💡 ESP8266 設定:
   #define SERVER_HOST "192.168.0.100"
   #define SERVER_PORT 8266
```

**Step 3: 開啟瀏覽器**

- **PC**: 開啟 `http://localhost:8080`
- **手機**: 開啟 `http://192.168.0.100:8080` (使用顯示的 IP)
- **同網段其他設備**: 使用網路 IP

**Step 4: 使用網頁介面**

1. 查看 ESP8266 連接狀態
2. 點擊「選擇圖片」
3. 選擇要顯示的圖片
4. 點擊「傳送到顯示器」
5. 等待完成（約 1 分鐘）

**Step 5: 其他功能**

- **傳送測試圖案**: 點擊「傳送測試圖案」
- **清除螢幕**: 點擊「清除螢幕」

---

### 方法 2：使用命令列（原有方式）

Server 啟動後，在命令列輸入：

```bash
>>> clients          # 查看連接的設備
>>> test             # 發送測試圖案
>>> clear            # 清除螢幕
>>> tile image.png   # 發送圖片
>>> web              # 顯示網頁連結
>>> quit             # 結束程式
```

---

## 📱 手機 (Termux) 完整流程

### Step 1: 安裝 Termux
```
Google Play Store → 搜尋 "Termux" → 安裝（免費）
```

### Step 2: 設定環境
```bash
# 更新套件
pkg update && pkg upgrade

# 安裝依賴
pkg install python git
pip install websockets pillow numpy aiohttp
```

### Step 3: 下載專案
```bash
# 方法 A: 使用 Git
cd ~
git clone https://github.com/DanielYJHsieh/epaper_display.git
cd epaper_display/wifi_spi_display/server

# 方法 B: 手動複製
# 將 server 資料夾複製到手機
# 使用檔案管理器放到 Termux 可存取的位置
```

### Step 4: 執行
```bash
python server_with_webui.py
```

### Step 5: 使用

**在手機上：**
- 開啟瀏覽器（Chrome/Firefox）
- 輸入 `http://localhost:8080`
- 從相簿選擇圖片並傳送

**從其他設備：**
- 使用 Termux 顯示的 IP（例如 `http://192.168.0.100:8080`）
- 可以從電腦瀏覽器控制手機上的 Server

### Step 6: 背景執行（可選）

安裝 tmux 讓 Server 在背景運行：

```bash
# 安裝 tmux
pkg install tmux

# 啟動 tmux session
tmux new -s epaper

# 執行 server
python server_with_webui.py

# 離開但保持運行：按 Ctrl+B 然後按 D

# 重新連接
tmux attach -t epaper
```

---

## 🖥️ PC 使用

### Windows

```powershell
# 啟動
cd D:\1_myproject\epaper_display\epaper_display\wifi_spi_display\server
python server_with_webui.py

# 開啟瀏覽器
start http://localhost:8080
```

### Linux/Mac

```bash
# 啟動
cd ~/epaper_display/wifi_spi_display/server
python server_with_webui.py

# 開啟瀏覽器
xdg-open http://localhost:8080  # Linux
open http://localhost:8080      # Mac
```

---

## 🌐 網頁界面功能

### 狀態顯示

- 🖥️ **Server 狀態**: 運行中/停止
- 📡 **WebSocket Port**: 8266
- 🌐 **HTTP Port**: 8080
- 📱 **已連接設備**: 顯示 ESP8266 數量
- ⚡ **當前狀態**: 待命/處理中/傳送中/完成

### 圖片上傳

- 📁 **選擇圖片**: 點擊區域選擇檔案
- 👁️ **預覽**: 自動顯示選擇的圖片
- 📤 **傳送**: 處理並傳送到 ESP8266

### 操作按鈕

- 🎨 **傳送測試圖案**: 棋盤格測試圖
- 🗑️ **清除螢幕**: 清空顯示器
- 📊 **即時更新**: 每 2 秒更新狀態

---

## 🔧 設定 ESP8266

修改 `client_esp8266/config.h`：

```cpp
// 使用 PC 的 IP
#define SERVER_HOST "192.168.0.100"  // PC 的 IP

// 使用手機的 IP
#define SERVER_HOST "192.168.0.200"  // 手機的 IP

// Port 不變
#define SERVER_PORT 8266
```

### 如何取得 IP？

**PC (Windows)**:
```cmd
ipconfig
# 查看 "IPv4 位址"
```

**PC (Linux/Mac)**:
```bash
ifconfig
# 或
ip addr show
```

**手機 (Termux)**:
```bash
ifconfig wlan0
# 或
ip addr show wlan0
```

**自動顯示**:
Server 啟動後會自動顯示本機 IP

---

## 📊 功能對比

| 功能 | 原 server.py | server_with_webui.py |
|-----|-------------|---------------------|
| WebSocket Server | ✅ | ✅ |
| 命令列操作 | ✅ | ✅ |
| 網頁界面 | ❌ | ✅ |
| 圖片上傳 | ❌ | ✅ |
| 狀態監控 | ❌ | ✅ |
| 手機友善 | ❌ | ✅ |
| PC 執行 | ✅ | ✅ |
| Termux 執行 | ✅ | ✅ |

---

## 💡 使用場景

### 場景 1: PC 開發測試
```
在 PC 執行 server_with_webui.py
ESP8266 連接到 PC
用瀏覽器控制 (http://localhost:8080)
```

### 場景 2: 手機獨立運作
```
手機 Termux 執行 server_with_webui.py
ESP8266 連接到手機
用手機瀏覽器控制 (http://localhost:8080)
從相簿選圖並傳送
```

### 場景 3: 遠端控制
```
手機執行 server_with_webui.py
ESP8266 連接到手機
電腦瀏覽器控制 (http://192.168.0.200:8080)
```

### 場景 4: 展示/Demo
```
手機熱點模式
手機執行 Server
ESP8266 連接手機熱點
用手機或平板控制
完全獨立，無需 WiFi 路由器
```

---

## 🎨 網頁界面截圖說明

### 主畫面
```
┌─────────────────────────────────┐
│ 📱 ESP8266 電子紙顯示器         │
│ Web 控制介面 v1.7.1             │
├─────────────────────────────────┤
│ 狀態卡片                         │
│ • Server 狀態: 運行中           │
│ • WebSocket: Port 8266          │
│ • HTTP Server: Port 8080        │
│ • 已連接設備: 1                 │
│ • 狀態: 待命中                  │
├─────────────────────────────────┤
│ 選擇圖片                         │
│ [點擊選擇圖片區域]              │
│ [圖片預覽]                      │
├─────────────────────────────────┤
│ 操作                            │
│ [📤 傳送到顯示器]               │
│ [🎨 傳送測試圖案]               │
│ [🗑️ 清除螢幕]                   │
├─────────────────────────────────┤
│ [進度條]                        │
│ [狀態訊息]                      │
└─────────────────────────────────┘
```

---

## ⚠️ 注意事項

### 1. 防火牆設定

**Windows**:
- 首次執行可能會彈出防火牆警告
- 點擊「允許存取」
- 或手動開放 port 8266 和 8080

**Linux**:
```bash
sudo ufw allow 8266
sudo ufw allow 8080
```

### 2. 網路設定

- ESP8266、Server 必須在**同一網段**
- 如果使用手機熱點，ESP8266 要連接到手機熱點
- 確認 IP 位址正確

### 3. 瀏覽器相容性

- ✅ Chrome/Edge (推薦)
- ✅ Firefox
- ✅ Safari
- ⚠️ 舊版 IE 不支援

### 4. 手機效能

- Termux 執行 Python 會消耗電力
- 建議在充電時使用
- 可使用 tmux 背景執行

---

## ❓ 常見問題

**Q1: Web UI 可以和命令列同時使用嗎？**
A: 可以！兩者完全獨立，互不影響。

**Q2: 可以從多個瀏覽器同時存取嗎？**
A: 可以！支援多個瀏覽器同時開啟。

**Q3: 手機執行會不會很慢？**
A: 影像處理需要幾秒鐘，但整體體驗流暢。

**Q4: 可以關閉網頁界面只用命令列嗎？**
A: 可以！使用原本的 `server.py` 即可。

**Q5: Termux 關閉後 Server 會停止嗎？**
A: 會。使用 tmux 或 Termux:Boot 可保持運行。

**Q6: 可以用 https 嗎？**
A: 目前是 http。如需 https 需要設定 SSL 憑證。

**Q7: 可以設定密碼保護嗎？**
A: 目前沒有。建議只在信任的網路使用。

---

## 🔄 更新

### 從原 server.py 遷移

1. **備份**
   ```bash
   cp server.py server_backup.py
   ```

2. **使用新版**
   ```bash
   # 直接使用 server_with_webui.py
   python server_with_webui.py
   ```

3. **回到舊版**
   ```bash
   # 使用原本的 server.py
   python server.py
   ```

### 版本區別

- `server.py` - 原版，命令列
- `server_with_webui.py` - 新版，命令列 + Web UI

**建議**：平常使用 `server_with_webui.py`，需要純命令列時用 `server.py`

---

## 📚 進階使用

### 自訂 Port

```python
# 修改 server_with_webui.py 最後幾行
server = DisplayServer(
    host="0.0.0.0", 
    port=8266,      # WebSocket port
    http_port=8080  # HTTP port（可改成其他）
)
```

### 開機自動啟動 (Termux)

```bash
# 安裝 Termux:Boot
# Google Play Store → Termux:Boot

# 建立啟動腳本
mkdir -p ~/.termux/boot
nano ~/.termux/boot/start_epaper.sh
```

內容：
```bash
#!/data/data/com.termux/files/usr/bin/sh
cd ~/epaper_display/wifi_spi_display/server
python server_with_webui.py
```

賦予執行權限：
```bash
chmod +x ~/.termux/boot/start_epaper.sh
```

### 建立桌面捷徑 (Termux)

```bash
# 建立啟動腳本
nano ~/start_epaper.sh
```

內容：
```bash
#!/data/data/com.termux/files/usr/bin/bash
cd ~/epaper_display/wifi_spi_display/server
python server_with_webui.py
```

建立捷徑：
```bash
chmod +x ~/start_epaper.sh
termux-create-shortcut ~/start_epaper.sh -n "E-Paper Server"
```

---

## 🎯 總結

### ✅ 優點

- **雙模式**: Web UI + 命令列
- **跨平台**: PC、手機都能用
- **簡單**: 點擊選圖即可
- **相容**: 完全向後相容
- **無依賴**: 只需 aiohttp

### 📌 使用建議

- **開發測試**: PC + Web UI
- **展示**: 手機 + Web UI
- **自動化**: 命令列腳本

---

需要協助嗎？

- 📧 提交 Issue 到 GitHub
- 💬 查看 `ANDROID_APP_OPTIONS.md`
- 📖 閱讀 `MOBILE_INTEGRATION_PLAN.md`

🚀 享受你的 ESP8266 電子紙顯示器！
