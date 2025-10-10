# Android App 選項 - 現有 APK 與自行開發

## 🔍 現有 APK 分析

### ❌ 直接可用的 APK：**沒有**

很遺憾，**目前沒有現成的 Android App** 可以直接用於此專案，原因如下：

1. **協議特殊性**
   - 本專案使用自定義的二進位協議（Header + tile_index + data）
   - 需要特定的封包格式：`0xAA 0x55 [Type] [SeqID] [Length] [Data]`
   - 需要處理 READY 文字訊息回饋

2. **影像處理需求**
   - 需要分割為 3×16KB 的特定格式
   - 需要 Floyd-Steinberg 抖動演算法
   - 需要打包成 1-bit 單色格式

3. **WebSocket Server 模式**
   - 大多數 App 是 Client，不是 Server
   - 需要監聽特定 port (8266)

---

## 📱 可考慮的替代方案

雖然沒有完全符合的 APK，但有幾種**部分可用**的方案：

### 方案 1：通用 WebSocket Server App + 自訂處理（⭐⭐）

**可能的 App**：
- **Simple HTTP Server** (Google Play)
- **WebSocket Server** (GitHub 開源專案)

**問題**：
- ❌ 只能傳送原始資料，無法處理影像
- ❌ 無法自動分割成 3 個條帶
- ❌ 無法實作自訂協議

**結論**：不實用

---

### 方案 2：使用 Termux + Python Server（⭐⭐⭐⭐）

**這是最接近「現成可用」的方案！**

#### 優點
- ✅ **可以直接使用現有的 Python Server**
- ✅ 無需開發新 App
- ✅ 完全相容現有協議
- ✅ 快速部署

#### 步驟

**Step 1: 安裝 Termux**
```
Google Play Store / F-Droid
搜尋：Termux
安裝免費版
```

**Step 2: 在 Termux 中安裝 Python 和依賴**
```bash
# 更新套件
pkg update && pkg upgrade

# 安裝 Python
pkg install python

# 安裝 pip
pkg install python-pip

# 安裝依賴
pip install websockets pillow numpy
```

**Step 3: 複製 Server 程式到手機**

方法 A - 使用 Git：
```bash
# 安裝 git
pkg install git

# Clone 專案
cd ~
git clone https://github.com/DanielYJHsieh/epaper_display.git
cd epaper_display/wifi_spi_display/server
```

方法 B - 手動複製：
```bash
# 建立目錄
mkdir -p ~/epaper_server
cd ~/epaper_server

# 使用檔案管理器複製以下檔案到此目錄：
# - server.py
# - image_processor.py
# - compressor.py
# - protocol.py
# - 測試圖片資料夾
```

**Step 4: 執行 Server**
```bash
cd ~/epaper_server
python server.py
```

**Step 5: 查看手機 IP**
```bash
# 在 Termux 中執行
ifconfig wlan0 | grep "inet "
# 或
ip addr show wlan0 | grep "inet "
```

**Step 6: 修改 ESP8266 設定**
```cpp
// config.h
#define SERVER_HOST "192.168.0.XXX"  // 手機的 IP
```

#### 使用方式

**傳送圖片**：
```bash
# 在 Termux 中
cd ~/epaper_server

# 方法 1: 使用指令傳送
python -c "
from server import DisplayServer
import asyncio

async def send():
    server = DisplayServer()
    # 讀取圖片並傳送
    await server.send_tiled_image_800('/path/to/image.png')

asyncio.run(send())
"

# 方法 2: 互動式命令
python
>>> from PIL import Image
>>> # 處理並傳送圖片
```

**在背景執行**：
```bash
# 安裝 tmux
pkg install tmux

# 啟動 tmux session
tmux new -s epaper

# 執行 server
python server.py

# 按 Ctrl+B 然後 D 離開（Server 繼續運行）

# 重新連接
tmux attach -t epaper
```

#### 優點總結
- ✅ **無需開發** - 直接用現有 Python Server
- ✅ **100% 相容** - 協議完全一致
- ✅ **快速部署** - 30 分鐘內完成
- ✅ **可在背景執行** - 使用 tmux

#### 缺點
- ⚠️ 需要使用命令列（對非技術用戶不友善）
- ⚠️ 無圖形界面
- ⚠️ 選圖片需要知道檔案路徑

---

### 方案 3：Termux + Python + 簡易 Web UI（⭐⭐⭐⭐⭐）

**最佳方案：在 Termux 執行 Python Server，並加上簡單的網頁界面！**

#### 改進 Server 加入 Web UI

**修改 `server.py` 加入 HTTP 介面**：

```python
from aiohttp import web
import aiofiles
import base64

class DisplayServer:
    def __init__(self, host: str = "0.0.0.0", port: int = 8266):
        # ... 原有程式碼 ...
        
        # 加入 HTTP Server
        self.http_app = web.Application()
        self.http_app.router.add_get('/', self.handle_index)
        self.http_app.router.add_post('/upload', self.handle_upload)
        self.http_app.router.add_static('/static', path='./static')
    
    async def handle_index(self, request):
        """提供網頁界面"""
        html = """
        <!DOCTYPE html>
        <html>
        <head>
            <meta charset="utf-8">
            <meta name="viewport" content="width=device-width, initial-scale=1">
            <title>ESP8266 Display Control</title>
            <style>
                body { font-family: Arial; max-width: 600px; margin: 50px auto; padding: 20px; }
                button { padding: 15px 30px; font-size: 18px; margin: 10px; cursor: pointer; }
                #preview { max-width: 100%; border: 1px solid #ccc; margin: 20px 0; }
                .status { padding: 10px; background: #e8f5e9; border-radius: 5px; margin: 10px 0; }
            </style>
        </head>
        <body>
            <h1>📱 ESP8266 顯示器控制</h1>
            <div class="status">
                <strong>Server 狀態：</strong> 運行中<br>
                <strong>已連接設備：</strong> <span id="clients">0</span>
            </div>
            
            <h2>選擇圖片</h2>
            <input type="file" id="imageFile" accept="image/*">
            <br><br>
            <img id="preview" style="display:none;">
            <br>
            <button onclick="sendImage()">📤 傳送到顯示器</button>
            
            <div id="status"></div>
            
            <script>
                document.getElementById('imageFile').onchange = function(e) {
                    const file = e.target.files[0];
                    const reader = new FileReader();
                    reader.onload = function(event) {
                        const img = document.getElementById('preview');
                        img.src = event.target.result;
                        img.style.display = 'block';
                    };
                    reader.readAsDataURL(file);
                };
                
                async function sendImage() {
                    const fileInput = document.getElementById('imageFile');
                    if (!fileInput.files[0]) {
                        alert('請先選擇圖片');
                        return;
                    }
                    
                    const formData = new FormData();
                    formData.append('image', fileInput.files[0]);
                    
                    document.getElementById('status').innerHTML = '⏳ 處理中...';
                    
                    try {
                        const response = await fetch('/upload', {
                            method: 'POST',
                            body: formData
                        });
                        
                        const result = await response.json();
                        if (result.success) {
                            document.getElementById('status').innerHTML = '✅ ' + result.message;
                        } else {
                            document.getElementById('status').innerHTML = '❌ ' + result.error;
                        }
                    } catch (error) {
                        document.getElementById('status').innerHTML = '❌ 錯誤: ' + error;
                    }
                }
                
                // 定期更新連接數
                setInterval(async () => {
                    try {
                        const response = await fetch('/status');
                        const data = await response.json();
                        document.getElementById('clients').textContent = data.clients;
                    } catch (e) {}
                }, 2000);
            </script>
        </body>
        </html>
        """
        return web.Response(text=html, content_type='text/html')
    
    async def handle_upload(self, request):
        """處理圖片上傳"""
        try:
            data = await request.post()
            image_field = data['image']
            
            # 讀取上傳的圖片
            image_data = image_field.file.read()
            
            # 儲存到臨時檔案
            temp_path = '/tmp/uploaded_image.png'
            with open(temp_path, 'wb') as f:
                f.write(image_data)
            
            # 處理並傳送
            await self.send_tiled_image_800(temp_path)
            
            return web.json_response({
                'success': True,
                'message': '圖片已傳送到顯示器'
            })
        except Exception as e:
            return web.json_response({
                'success': False,
                'error': str(e)
            })
    
    async def start(self):
        """啟動 WebSocket Server 和 HTTP Server"""
        # WebSocket Server (port 8266)
        ws_server = await serve(self.handler, self.host, self.port)
        logger.info(f"WebSocket Server 啟動: ws://{self.host}:{self.port}")
        
        # HTTP Server (port 8080)
        runner = web.AppRunner(self.http_app)
        await runner.setup()
        site = web.TCPSite(runner, self.host, 8080)
        await site.start()
        logger.info(f"Web UI 啟動: http://{self.host}:8080")
        
        # 保持運行
        await asyncio.Future()
```

#### 使用方式

**Step 1: 在 Termux 安裝額外依賴**
```bash
pip install aiohttp aiofiles
```

**Step 2: 啟動 Server**
```bash
cd ~/epaper_server
python server.py
```

**Step 3: 在手機瀏覽器開啟**
```
http://localhost:8080
或
http://192.168.0.XXX:8080
```

**Step 4: 使用網頁界面**
1. 點擊「選擇圖片」
2. 從相簿選擇照片
3. 預覽圖片
4. 點擊「傳送到顯示器」
5. 等待完成（約 1 分鐘）

#### 優點
- ✅ **圖形界面** - 手機瀏覽器操作
- ✅ **簡單易用** - 選圖、點擊、完成
- ✅ **無需額外 App** - 用瀏覽器即可
- ✅ **相容現有協議** - 100% 相容
- ✅ **快速部署** - 1 小時內完成

---

## 🎯 最終建議

### 推薦方案排序

| 方案 | 難度 | 時間 | 用戶體驗 | 推薦度 |
|-----|-----|------|---------|--------|
| **方案 3: Termux + Web UI** | ⭐⭐ | 1 小時 | ⭐⭐⭐⭐⭐ | ⭐⭐⭐⭐⭐ |
| 方案 2: Termux + CLI | ⭐ | 30 分鐘 | ⭐⭐ | ⭐⭐⭐⭐ |
| 自行開發 Native App | ⭐⭐⭐⭐⭐ | 2-4 週 | ⭐⭐⭐⭐⭐ | ⭐⭐⭐ |

### 🏆 最佳選擇：**Termux + Python Server + Web UI**

**為什麼？**
1. ✅ **快速** - 1 小時內可用
2. ✅ **簡單** - 複製程式碼即可
3. ✅ **完全相容** - 使用現有 Python Server
4. ✅ **用戶友善** - 網頁界面操作
5. ✅ **無需 APK** - 不需要安裝額外 App

---

## 📋 快速部署指南（方案 3）

### 預計時間：60 分鐘

**Step 1: 安裝 Termux（5 分鐘）**
```
1. 打開 Google Play Store
2. 搜尋 "Termux"
3. 安裝（免費）
```

**Step 2: 設定環境（15 分鐘）**
```bash
# 打開 Termux，執行：
pkg update && pkg upgrade
pkg install python git
pip install websockets pillow numpy aiohttp aiofiles
```

**Step 3: 下載專案（5 分鐘）**
```bash
cd ~
git clone https://github.com/DanielYJHsieh/epaper_display.git
cd epaper_display/wifi_spi_display/server
```

**Step 4: 修改 Server（20 分鐘）**
```bash
# 編輯 server.py
nano server.py

# 加入 Web UI 程式碼（如上）
# Ctrl+X 儲存
```

**Step 5: 執行（5 分鐘）**
```bash
python server.py
```

**Step 6: 使用（10 分鐘）**
```
1. 查看 Termux 顯示的 IP (例如: 192.168.0.100)
2. 修改 ESP8266 的 SERVER_HOST
3. 在手機瀏覽器開啟: http://localhost:8080
4. 選圖、傳送、完成！
```

---

## 💡 進階：打包成 APK（可選）

如果想要更方便，可以使用 **QPython** 或 **Pydroid 3** 打包成獨立 APK：

### 使用 Pydroid 3

**Step 1: 安裝 Pydroid 3**
```
Google Play Store → Pydroid 3
```

**Step 2: 複製程式碼**
- 將 server.py 和相關檔案複製到 Pydroid 3 目錄

**Step 3: 執行**
- 在 Pydroid 3 中打開 server.py
- 點擊執行按鈕

**Step 4: 背景執行**
- Pydroid 3 支援背景服務

### 優點
- ✅ 圖形界面
- ✅ 無需命令列
- ✅ 更像原生 App

---

## ❓ FAQ

**Q1: Termux 可以在背景執行嗎？**
A: 可以！使用 `tmux` 或 Termux 的 wake lock 功能。

**Q2: 關閉 Termux 後 Server 會停止嗎？**
A: 會。需要使用 tmux 或 Termux:Boot 保持運行。

**Q3: 需要 Root 嗎？**
A: 完全不需要！

**Q4: 可以設定開機自動啟動嗎？**
A: 可以！安裝 Termux:Boot 插件。

**Q5: 耗電嗎？**
A: 待機耗電極低（< 1%/小時），傳輸時略增。

---

## 📚 相關資源

- [Termux 官方網站](https://termux.dev/)
- [Pydroid 3](https://play.google.com/store/apps/details?id=ru.iiec.pydroid3)
- [專案 GitHub](https://github.com/DanielYJHsieh/epaper_display)

---

## 🎯 結論

**沒有現成可用的 APK**，但有 3 種實用替代方案：

1. ⭐⭐⭐⭐⭐ **Termux + Python + Web UI** - 最推薦
2. ⭐⭐⭐⭐ **Termux + Python CLI** - 最快速
3. ⭐⭐⭐ **自行開發 Native App** - 最完美但耗時

**建議從方案 3 開始**，1 小時內即可使用！

需要我提供：
1. 完整修改後的 `server.py`（含 Web UI）？
2. 詳細的 Termux 安裝步驟截圖？
3. 測試用的簡易圖片？

請告訴我需要什麼協助！🚀
