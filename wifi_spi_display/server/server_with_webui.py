"""
WiFi SPI Display Server - 含 Web UI
WebSocket 伺服器主程式 + HTTP 網頁控制介面

✅ 可在 PC 和手機（Termux）上執行
✅ 支援網頁界面（port 8080）和命令列
✅ 完全向後相容原有功能
"""

import asyncio
import websockets
from websockets.server import serve
from pathlib import Path
import logging
from typing import Set, Optional
from PIL import Image
import socket
import os
import tempfile

# HTTP Server
from aiohttp import web
import aiohttp

from image_processor import ImageProcessor
from compressor import RLECompressor, HybridCompressor
from protocol import Protocol, PacketType, Command

# 設定日誌
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(levelname)s - %(message)s'
)
logger = logging.getLogger(__name__)


def get_local_ip():
    """取得本機 IP 位址"""
    try:
        # 連接到外部 IP 以取得本機 IP（不實際發送資料）
        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        s.connect(("8.8.8.8", 80))
        ip = s.getsockname()[0]
        s.close()
        return ip
    except Exception:
        return "127.0.0.1"


class DisplayServer:
    """電子紙顯示伺服器（含 Web UI）"""
    
    def __init__(self, host: str = "0.0.0.0", port: int = 8266, http_port: int = 8080):
        """
        初始化伺服器
        
        Args:
            host: 監聽位址
            port: WebSocket 埠號
            http_port: HTTP 網頁介面埠號
        """
        self.host = host
        self.port = port
        self.http_port = http_port
        self.clients: Set[websockets.WebSocketServerProtocol] = set()
        self.seq_id = 0
        
        # 初始化模組
        self.processor = ImageProcessor(400, 240)
        self.processor_800 = ImageProcessor(800, 480)
        self.compressor = RLECompressor()
        self.hybrid = HybridCompressor()
        
        # 條帶顯示完成事件（用於同步）
        self.tile_ready_event = asyncio.Event()
        
        # 狀態追蹤
        self.last_image_path = None
        self.is_sending = False
        self.last_status = "待命中"
        
        # 建立 HTTP Server (增加上傳限制到 20MB)
        self.http_app = web.Application(client_max_size=20*1024*1024)
        self._setup_routes()
        
        logger.info(f"伺服器初始化完成")
        logger.info(f"顯示器: 400x240 (中央區域) / 800x480 (全螢幕分區)")
        logger.info(f"本機 IP: {get_local_ip()}")
    
    def _setup_routes(self):
        """設定 HTTP 路由"""
        self.http_app.router.add_get('/', self.handle_index)
        self.http_app.router.add_post('/upload', self.handle_upload)
        self.http_app.router.add_get('/status', self.handle_status)
        self.http_app.router.add_post('/send_test', self.handle_send_test)
        self.http_app.router.add_post('/clear', self.handle_clear)
    
    async def handle_index(self, request):
        """提供網頁界面"""
        html = """
<!DOCTYPE html>
<html lang="zh-TW">
<head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>ESP8266 電子紙顯示器控制</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        
        body {
            font-family: 'Segoe UI', Arial, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
            padding: 20px;
        }
        
        .container {
            max-width: 800px;
            margin: 0 auto;
            background: white;
            border-radius: 20px;
            box-shadow: 0 20px 60px rgba(0,0,0,0.3);
            overflow: hidden;
        }
        
        .header {
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
            padding: 30px;
            text-align: center;
        }
        
        .header h1 {
            font-size: 28px;
            margin-bottom: 10px;
        }
        
        .header p {
            opacity: 0.9;
            font-size: 14px;
        }
        
        .content {
            padding: 30px;
        }
        
        .status-card {
            background: #f8f9fa;
            border-left: 4px solid #667eea;
            padding: 20px;
            border-radius: 10px;
            margin-bottom: 30px;
        }
        
        .status-item {
            display: flex;
            justify-content: space-between;
            margin-bottom: 10px;
            font-size: 15px;
        }
        
        .status-item:last-child {
            margin-bottom: 0;
        }
        
        .status-label {
            color: #666;
            font-weight: 500;
        }
        
        .status-value {
            color: #333;
            font-weight: 600;
        }
        
        .status-value.online {
            color: #28a745;
        }
        
        .section {
            margin-bottom: 30px;
        }
        
        .section h2 {
            font-size: 20px;
            margin-bottom: 15px;
            color: #333;
        }
        
        .upload-area {
            border: 3px dashed #667eea;
            border-radius: 10px;
            padding: 40px;
            text-align: center;
            background: #f8f9ff;
            cursor: pointer;
            transition: all 0.3s;
        }
        
        .upload-area:hover {
            background: #f0f3ff;
            border-color: #764ba2;
        }
        
        .upload-area input[type="file"] {
            display: none;
        }
        
        .upload-icon {
            font-size: 48px;
            margin-bottom: 10px;
        }
        
        #preview {
            max-width: 100%;
            max-height: 400px;
            border-radius: 10px;
            margin: 20px 0;
            display: none;
            box-shadow: 0 4px 12px rgba(0,0,0,0.1);
        }
        
        .btn {
            display: inline-block;
            padding: 15px 30px;
            font-size: 16px;
            font-weight: 600;
            border: none;
            border-radius: 10px;
            cursor: pointer;
            transition: all 0.3s;
            margin: 5px;
            text-decoration: none;
            color: white;
        }
        
        .btn-primary {
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
        }
        
        .btn-primary:hover {
            transform: translateY(-2px);
            box-shadow: 0 8px 20px rgba(102, 126, 234, 0.4);
        }
        
        .btn-secondary {
            background: #6c757d;
        }
        
        .btn-secondary:hover {
            background: #5a6268;
            transform: translateY(-2px);
        }
        
        .btn-success {
            background: #28a745;
        }
        
        .btn-success:hover {
            background: #218838;
            transform: translateY(-2px);
        }
        
        .btn:disabled {
            opacity: 0.5;
            cursor: not-allowed;
            transform: none !important;
        }
        
        .button-group {
            display: flex;
            flex-wrap: wrap;
            gap: 10px;
            margin-top: 20px;
        }
        
        .alert {
            padding: 15px 20px;
            border-radius: 10px;
            margin-top: 20px;
            display: none;
            animation: slideIn 0.3s;
        }
        
        @keyframes slideIn {
            from {
                opacity: 0;
                transform: translateY(-10px);
            }
            to {
                opacity: 1;
                transform: translateY(0);
            }
        }
        
        .alert-info {
            background: #d1ecf1;
            color: #0c5460;
            border: 1px solid #bee5eb;
        }
        
        .alert-success {
            background: #d4edda;
            color: #155724;
            border: 1px solid #c3e6cb;
        }
        
        .alert-error {
            background: #f8d7da;
            color: #721c24;
            border: 1px solid #f5c6cb;
        }
        
        .progress-bar {
            height: 8px;
            background: #e9ecef;
            border-radius: 10px;
            overflow: hidden;
            margin-top: 15px;
            display: none;
        }
        
        .progress-fill {
            height: 100%;
            background: linear-gradient(90deg, #667eea 0%, #764ba2 100%);
            width: 0%;
            transition: width 0.3s;
            animation: pulse 1.5s infinite;
        }
        
        @keyframes pulse {
            0%, 100% { opacity: 1; }
            50% { opacity: 0.7; }
        }
        
        .footer {
            text-align: center;
            padding: 20px;
            color: #666;
            font-size: 14px;
            border-top: 1px solid #eee;
        }
        
        @media (max-width: 600px) {
            .container {
                border-radius: 0;
            }
            
            .content {
                padding: 20px;
            }
            
            .button-group {
                flex-direction: column;
            }
            
            .btn {
                width: 100%;
            }
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>📱 ESP8266 電子紙顯示器</h1>
            <p>Web 控制介面 v1.7.1</p>
        </div>
        
        <div class="content">
            <!-- 狀態卡片 -->
            <div class="status-card">
                <div class="status-item">
                    <span class="status-label">🖥️ Server 狀態</span>
                    <span class="status-value online" id="serverStatus">運行中</span>
                </div>
                <div class="status-item">
                    <span class="status-label">📡 WebSocket</span>
                    <span class="status-value" id="wsPort">Port 8266</span>
                </div>
                <div class="status-item">
                    <span class="status-label">🌐 HTTP Server</span>
                    <span class="status-value" id="httpPort">Port 8080</span>
                </div>
                <div class="status-item">
                    <span class="status-label">📱 已連接設備</span>
                    <span class="status-value" id="clients">0</span>
                </div>
                <div class="status-item">
                    <span class="status-label">⚡ 狀態</span>
                    <span class="status-value" id="lastStatus">待命中</span>
                </div>
            </div>
            
            <!-- 圖片上傳區 -->
            <div class="section">
                <h2>📸 選擇圖片</h2>
                <div class="upload-area" onclick="document.getElementById('imageFile').click()">
                    <div class="upload-icon">📁</div>
                    <p style="color: #667eea; font-weight: 600; margin-bottom: 5px;">點擊選擇圖片</p>
                    <p style="color: #999; font-size: 14px;">支援 JPG, PNG, BMP 等格式</p>
                    <input type="file" id="imageFile" accept="image/*">
                </div>
                <img id="preview" alt="圖片預覽">
            </div>
            
            <!-- 操作按鈕 -->
            <div class="section">
                <h2>🎮 操作</h2>
                <div class="button-group">
                    <button class="btn btn-primary" onclick="sendImage()" id="sendBtn" disabled>
                        📤 傳送到顯示器
                    </button>
                    <button class="btn btn-success" onclick="sendTest()">
                        🎨 傳送測試圖案
                    </button>
                    <button class="btn btn-secondary" onclick="clearScreen()">
                        🗑️ 清除螢幕
                    </button>
                </div>
            </div>
            
            <!-- 進度條 -->
            <div class="progress-bar" id="progressBar">
                <div class="progress-fill" id="progressFill"></div>
            </div>
            
            <!-- 訊息提示 -->
            <div class="alert" id="alert"></div>
        </div>
        
        <div class="footer">
            <p>💡 提示：確保 ESP8266 已連接到相同網路並設定正確的 Server IP</p>
            <p style="margin-top: 5px;">🔧 本機 IP: <strong id="localIP">載入中...</strong></p>
        </div>
    </div>
    
    <script>
        let selectedFile = null;
        
        // 載入本機 IP
        fetch('/status')
            .then(res => res.json())
            .then(data => {
                if (data.local_ip) {
                    document.getElementById('localIP').textContent = data.local_ip;
                }
            });
        
        // 圖片選擇
        document.getElementById('imageFile').onchange = function(e) {
            const file = e.target.files[0];
            if (!file) return;
            
            selectedFile = file;
            const reader = new FileReader();
            reader.onload = function(event) {
                const img = document.getElementById('preview');
                img.src = event.target.result;
                img.style.display = 'block';
                document.getElementById('sendBtn').disabled = false;
            };
            reader.readAsDataURL(file);
        };
        
        // 傳送圖片
        async function sendImage() {
            if (!selectedFile) {
                showAlert('請先選擇圖片', 'error');
                return;
            }
            
            const formData = new FormData();
            formData.append('image', selectedFile);
            
            showProgress(true);
            showAlert('⏳ 處理中，請稍候...', 'info');
            document.getElementById('sendBtn').disabled = true;
            
            try {
                const response = await fetch('/upload', {
                    method: 'POST',
                    body: formData
                });
                
                const result = await response.json();
                
                if (result.success) {
                    showAlert('✅ ' + result.message, 'success');
                } else {
                    showAlert('❌ ' + result.error, 'error');
                }
            } catch (error) {
                showAlert('❌ 發生錯誤: ' + error, 'error');
            } finally {
                showProgress(false);
                document.getElementById('sendBtn').disabled = false;
            }
        }
        
        // 傳送測試圖案
        async function sendTest() {
            showProgress(true);
            showAlert('⏳ 產生測試圖案...', 'info');
            
            try {
                const response = await fetch('/send_test', {
                    method: 'POST'
                });
                
                const result = await response.json();
                
                if (result.success) {
                    showAlert('✅ ' + result.message, 'success');
                } else {
                    showAlert('❌ ' + result.error, 'error');
                }
            } catch (error) {
                showAlert('❌ 發生錯誤: ' + error, 'error');
            } finally {
                showProgress(false);
            }
        }
        
        // 清除螢幕
        async function clearScreen() {
            if (!confirm('確定要清除螢幕嗎？')) return;
            
            showProgress(true);
            showAlert('⏳ 清除中...', 'info');
            
            try {
                const response = await fetch('/clear', {
                    method: 'POST'
                });
                
                const result = await response.json();
                
                if (result.success) {
                    showAlert('✅ ' + result.message, 'success');
                } else {
                    showAlert('❌ ' + result.error, 'error');
                }
            } catch (error) {
                showAlert('❌ 發生錯誤: ' + error, 'error');
            } finally {
                showProgress(false);
            }
        }
        
        // 顯示提示訊息
        function showAlert(message, type) {
            const alert = document.getElementById('alert');
            alert.textContent = message;
            alert.className = 'alert alert-' + type;
            alert.style.display = 'block';
            
            if (type === 'success' || type === 'error') {
                setTimeout(() => {
                    alert.style.display = 'none';
                }, 5000);
            }
        }
        
        // 顯示/隱藏進度條
        function showProgress(show) {
            const bar = document.getElementById('progressBar');
            const fill = document.getElementById('progressFill');
            
            if (show) {
                bar.style.display = 'block';
                fill.style.width = '100%';
            } else {
                setTimeout(() => {
                    bar.style.display = 'none';
                    fill.style.width = '0%';
                }, 300);
            }
        }
        
        // 定期更新狀態
        setInterval(async () => {
            try {
                const response = await fetch('/status');
                const data = await response.json();
                
                document.getElementById('clients').textContent = data.clients;
                document.getElementById('lastStatus').textContent = data.last_status;
                
                // 更新連接狀態顏色
                const statusEl = document.getElementById('serverStatus');
                if (data.clients > 0) {
                    statusEl.className = 'status-value online';
                    statusEl.textContent = '運行中 (' + data.clients + ' 個設備)';
                } else {
                    statusEl.className = 'status-value';
                    statusEl.textContent = '運行中 (等待連接)';
                }
            } catch (e) {
                console.error('無法更新狀態:', e);
            }
        }, 2000);
    </script>
</body>
</html>
        """
        return web.Response(text=html, content_type='text/html', charset='utf-8')
    
    async def handle_upload(self, request):
        """處理圖片上傳"""
        if not self.clients:
            return web.json_response({
                'success': False,
                'error': '沒有連接的 ESP8266 設備'
            })
        
        try:
            self.is_sending = True
            self.last_status = "處理圖片中..."
            
            # 讀取上傳的圖片
            data = await request.post()
            image_field = data['image']
            
            # 儲存到臨時檔案
            temp_dir = tempfile.gettempdir()
            temp_path = os.path.join(temp_dir, 'esp8266_uploaded_image.png')
            
            with open(temp_path, 'wb') as f:
                f.write(image_field.file.read())
            
            logger.info(f"收到上傳圖片，儲存至: {temp_path}")
            self.last_image_path = temp_path
            
            # 處理並傳送（使用 800×480 全螢幕分區模式）
            self.last_status = "傳送中..."
            await self.send_tiled_image_800(temp_path)
            
            self.last_status = "傳送完成"
            self.is_sending = False
            
            return web.json_response({
                'success': True,
                'message': '圖片已成功傳送到顯示器'
            })
        
        except Exception as e:
            self.last_status = f"錯誤: {str(e)}"
            self.is_sending = False
            logger.error(f"處理上傳時發生錯誤: {e}", exc_info=True)
            return web.json_response({
                'success': False,
                'error': str(e)
            })
    
    async def handle_status(self, request):
        """返回伺服器狀態"""
        return web.json_response({
            'clients': len(self.clients),
            'last_status': self.last_status,
            'is_sending': self.is_sending,
            'local_ip': get_local_ip()
        })
    
    async def handle_send_test(self, request):
        """發送測試圖案"""
        if not self.clients:
            return web.json_response({
                'success': False,
                'error': '沒有連接的 ESP8266 設備'
            })
        
        try:
            self.last_status = "傳送測試圖案..."
            await self.send_test_pattern_800()
            self.last_status = "測試圖案已傳送"
            
            return web.json_response({
                'success': True,
                'message': '測試圖案已傳送'
            })
        except Exception as e:
            self.last_status = f"錯誤: {str(e)}"
            logger.error(f"發送測試圖案時發生錯誤: {e}")
            return web.json_response({
                'success': False,
                'error': str(e)
            })
    
    async def handle_clear(self, request):
        """清除螢幕"""
        if not self.clients:
            return web.json_response({
                'success': False,
                'error': '沒有連接的 ESP8266 設備'
            })
        
        try:
            self.last_status = "清除螢幕中..."
            await self.send_command(Command.CLEAR_SCREEN)
            self.last_status = "螢幕已清除"
            
            return web.json_response({
                'success': True,
                'message': '螢幕已清除'
            })
        except Exception as e:
            self.last_status = f"錯誤: {str(e)}"
            logger.error(f"清除螢幕時發生錯誤: {e}")
            return web.json_response({
                'success': False,
                'error': str(e)
            })
    
    async def register(self, websocket):
        """註冊客戶端"""
        self.clients.add(websocket)
        logger.info(f"客戶端連接: {websocket.remote_address}")
        logger.info(f"目前連接數: {len(self.clients)}")
        self.last_status = f"設備已連接 ({len(self.clients)})"
    
    async def unregister(self, websocket):
        """註銷客戶端"""
        self.clients.discard(websocket)
        logger.info(f"客戶端斷開: {websocket.remote_address}")
        logger.info(f"目前連接數: {len(self.clients)}")
        self.last_status = f"設備已斷開 (剩餘 {len(self.clients)})"
    
    async def handler(self, websocket, path):
        """處理客戶端連接"""
        await self.register(websocket)
        
        try:
            async for message in websocket:
                await self.handle_message(websocket, message)
        except websockets.exceptions.ConnectionClosed:
            logger.info("連接已關閉")
        except Exception as e:
            logger.error(f"處理連接時發生錯誤: {e}")
        finally:
            await self.unregister(websocket)
    
    async def handle_message(self, websocket, message):
        """處理收到的訊息"""
        if isinstance(message, bytes):
            # 二進位訊息（ACK/NAK）
            if len(message) >= 8:
                info = Protocol.get_packet_info(message)
                if info['valid']:
                    logger.info(f"收到回應: {info['type']}, SeqID={info['seq_id']}")
                    
                    if info['type_code'] == PacketType.ACK:
                        logger.info(f"✓ ESP8266 確認收到 SeqID={info['seq_id']}")
                    elif info['type_code'] == PacketType.NAK:
                        logger.warning(f"✗ ESP8266 回報錯誤 SeqID={info['seq_id']}")
        else:
            # 文字訊息
            message_str = message.strip()
            logger.info(f"收到文字訊息: {message_str}")
            
            # 處理 READY 訊號
            if message_str == "READY":
                logger.info("✓ ESP8266 已準備好接收下一個條帶")
                self.tile_ready_event.set()
    
    async def send_command(self, cmd: int):
        """發送控制指令"""
        if not self.clients:
            logger.warning("沒有連接的客戶端")
            return
        
        self.seq_id += 1
        packet = Protocol.pack_command(self.seq_id, cmd)
        logger.info(f"發送指令: {cmd}")
        
        await asyncio.gather(
            *[client.send(packet) for client in self.clients],
            return_exceptions=True
        )
    
    async def send_test_pattern_800(self):
        """發送測試圖案 (800×480 分區模式)"""
        logger.info("生成測試圖案 (800×480)")
        test_img = self.processor_800.create_test_pattern()
        await self.send_tiled_image_800_from_image(test_img)
    
    async def send_tiled_image_800(self, image_path: str):
        """發送分區圖片 (800×480, 3個條帶)"""
        logger.info(f"處理圖片: {image_path}")
        img = Image.open(image_path)
        await self.send_tiled_image_800_from_image(img)
    
    async def send_tiled_image_800_from_image(self, img: Image.Image):
        """從 PIL Image 發送分區圖片"""
        if not self.clients:
            logger.warning("沒有連接的客戶端")
            return
        
        logger.info(f"原始圖片: {img.size}, 模式: {img.mode}")
        
        # 轉換為 1-bit (使用 800x480 處理器)
        processed = self.processor_800.convert_to_1bit(img, dither=True)
        logger.info(f"轉換為 1-bit: {processed.size}")
        
        # 分割成 3 個水平條帶（垂直分割：上、中、下）
        tiles = self.processor_800.split_image_to_tiles(processed)
        logger.info(f"分割為 {len(tiles)} 個條帶 (800×160, 垂直分割)")
        
        tile_names = ["條帶0 (上)", "條帶1 (中)", "條帶2 (下)"]
        total_raw = 0
        total_compressed = 0
        
        # 依序發送 3 個條帶
        for tile_index in range(3):
            logger.info(f"\n--- 處理條帶 {tile_index} ({tile_names[tile_index]}) ---")
            
            tile_image = tiles[tile_index]
            
            # 處理條帶（轉換為 bytes）- tile 已經是 1-bit
            tile_data = self.processor_800.process_tile(tile_image, dither=False)
            logger.info(f"條帶資料: {len(tile_data)} bytes (800x160)")
            total_raw += len(tile_data)
            
            # 不使用壓縮
            compressed_data = tile_data
            is_compressed = False
            logger.info(f"發送未壓縮資料: {len(compressed_data)} bytes（避免記憶體碎片化）")
            
            total_compressed += len(compressed_data)
            
            # 創建分區封包
            self.seq_id += 1
            packet = Protocol.pack_tile(self.seq_id, tile_index, compressed_data)
            logger.info(f"封包大小: {len(packet)} bytes (含標頭)")
            
            # 發送
            logger.info(f"發送條帶 {tile_index} 到 {len(self.clients)} 個客戶端...")
            await asyncio.gather(
                *[client.send(packet) for client in self.clients],
                return_exceptions=True
            )
            logger.info(f"✓ 條帶 {tile_index} ({tile_names[tile_index]}) 發送完成")
            
            # 等待 ESP8266 完成顯示並發送 READY 訊號
            if tile_index < 2:
                logger.info(f"等待條帶 {tile_index} 顯示完成...")
                self.tile_ready_event.clear()
                
                try:
                    await asyncio.wait_for(self.tile_ready_event.wait(), timeout=30.0)
                    logger.info(f"✓ 條帶 {tile_index} 顯示完成，準備發送下一個條帶")
                except asyncio.TimeoutError:
                    logger.warning(f"⚠️ 等待條帶 {tile_index} 完成超時，繼續發送下一個條帶")
            else:
                # 最後一個條帶，等待 READY 訊號確保完成
                logger.info(f"等待最後一個條帶完成...")
                self.tile_ready_event.clear()
                try:
                    await asyncio.wait_for(self.tile_ready_event.wait(), timeout=30.0)
                    logger.info(f"✓ 最後一個條帶顯示完成")
                except asyncio.TimeoutError:
                    logger.warning(f"⚠️ 等待最後一個條帶超時")
        
        # 統計
        overall_ratio = self.compressor.compress_ratio(total_raw, total_compressed)
        logger.info(f"\n=== 完成 ===")
        logger.info(f"總原始資料: {total_raw} bytes")
        logger.info(f"總傳輸資料: {total_compressed} bytes")
        logger.info(f"整體壓縮率: {overall_ratio:.1f}%")
    
    async def start(self):
        """啟動伺服器"""
        # 啟動 WebSocket Server
        ws_server = await serve(self.handler, self.host, self.port)
        logger.info(f"WebSocket Server 啟動: ws://{self.host}:{self.port}")
        
        # 啟動 HTTP Server
        runner = web.AppRunner(self.http_app)
        await runner.setup()
        site = web.TCPSite(runner, self.host, self.http_port)
        await site.start()
        
        local_ip = get_local_ip()
        logger.info(f"HTTP Server 啟動: http://{self.host}:{self.http_port}")
        logger.info(f"")
        logger.info(f"🌐 網頁控制介面:")
        logger.info(f"   本機存取: http://localhost:{self.http_port}")
        logger.info(f"   網路存取: http://{local_ip}:{self.http_port}")
        logger.info(f"")
        logger.info(f"💡 ESP8266 設定:")
        logger.info(f"   #define SERVER_HOST \"{local_ip}\"")
        logger.info(f"   #define SERVER_PORT {self.port}")
        logger.info(f"")
        
        # 保持運行
        await asyncio.Future()


async def interactive_mode(server: DisplayServer):
    """互動式命令列模式（與 Web UI 並行）"""
    print("\n" + "="*50)
    print("互動式命令列（可選）")
    print("="*50)
    print("可用指令:")
    print("  clients           - 顯示連接的客戶端")
    print("  test              - 發送測試圖案 (800×480)")
    print("  clear             - 清空螢幕")
    print("  tile <圖片路徑>   - 發送分區圖片 (800×480)")
    print("  web               - 顯示網頁界面連結")
    print("  quit              - 結束程式")
    print()
    
    while True:
        try:
            cmd = await asyncio.get_event_loop().run_in_executor(
                None, input, ">>> "
            )
            
            cmd = cmd.strip()
            if not cmd:
                continue
            
            parts = cmd.split(maxsplit=1)
            action = parts[0].lower()
            
            if action == "quit":
                print("結束程式...")
                break
            elif action == "clients":
                print(f"連接的客戶端數: {len(server.clients)}")
                for i, client in enumerate(server.clients, 1):
                    print(f"  {i}. {client.remote_address}")
            elif action == "test":
                await server.send_test_pattern_800()
            elif action == "clear":
                await server.send_command(Command.CLEAR_SCREEN)
            elif action == "tile" and len(parts) > 1:
                await server.send_tiled_image_800(parts[1])
            elif action == "web":
                local_ip = get_local_ip()
                print(f"\n🌐 網頁控制介面:")
                print(f"   http://localhost:{server.http_port}")
                print(f"   http://{local_ip}:{server.http_port}\n")
            else:
                print("未知指令或缺少參數")
        
        except KeyboardInterrupt:
            print("\n結束程式...")
            break
        except Exception as e:
            print(f"錯誤: {e}")


async def main():
    """主程式"""
    server = DisplayServer(host="0.0.0.0", port=8266, http_port=8080)
    
    # 同時運行 WebSocket Server、HTTP Server 和互動模式
    await asyncio.gather(
        server.start(),
        interactive_mode(server)
    )


if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        logger.info("\n程式已停止")
