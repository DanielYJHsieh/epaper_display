"""
WiFi SPI Display Server
WebSocket 伺服器主程式
"""

import asyncio
import websockets
from websockets.server import serve
from pathlib import Path
import logging
from typing import Set, Optional
from PIL import Image

from image_processor import ImageProcessor
from compressor import RLECompressor, HybridCompressor
from protocol import Protocol, PacketType, Command

# 設定日誌
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(levelname)s - %(message)s'
)
logger = logging.getLogger(__name__)


class DisplayServer:
    """電子紙顯示伺服器"""
    
    def __init__(self, host: str = "0.0.0.0", port: int = 8266):
        """
        初始化伺服器
        
        Args:
            host: 監聽位址
            port: 監聽埠號
        """
        self.host = host
        self.port = port
        self.clients: Set[websockets.WebSocketServerProtocol] = set()
        self.seq_id = 0
        
        # 初始化模組 - 使用 400x240 解析度以適應 ESP8266 記憶體限制
        self.processor = ImageProcessor(400, 240)
        self.compressor = RLECompressor()
        self.hybrid = HybridCompressor()
        
        logger.info(f"伺服器初始化完成")
        logger.info(f"顯示器: 400x240 (實體螢幕 800x480 中央區域), 12000 bytes")
    
    async def register(self, websocket):
        """註冊客戶端"""
        self.clients.add(websocket)
        logger.info(f"客戶端連接: {websocket.remote_address}")
        logger.info(f"目前連接數: {len(self.clients)}")
    
    async def unregister(self, websocket):
        """註銷客戶端"""
        self.clients.discard(websocket)
        logger.info(f"客戶端斷開: {websocket.remote_address}")
        logger.info(f"目前連接數: {len(self.clients)}")
    
    async def handler(self, websocket, path):
        """處理客戶端連接"""
        await self.register(websocket)
        
        try:
            async for message in websocket:
                # 接收來自 ESP8266 的訊息（ACK/NAK）
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
            logger.info(f"收到文字訊息: {message}")
    
    async def send_image(self, image_path: str):
        """
        發送圖片到所有客戶端
        
        Args:
            image_path: 圖片檔案路徑
        """
        if not self.clients:
            logger.warning("沒有連接的客戶端")
            return
        
        logger.info(f"處理圖片: {image_path}")
        
        try:
            # 載入並處理圖像
            img = Image.open(image_path)
            logger.info(f"原始圖片: {img.size}, 模式: {img.mode}")
            
            processed = self.processor.convert_to_1bit(img, dither=True)
            logger.info(f"轉換為 1-bit: {processed.size}")
            
            raw_data = self.processor.image_to_bytes(processed)
            logger.info(f"原始資料: {len(raw_data)} bytes")
            
            # 智能壓縮：比較 RLE 和無壓縮，選擇較小的
            compressed, is_compressed = self.compressor.compress_smart(raw_data)
            ratio = self.compressor.compress_ratio(len(raw_data), len(compressed))
            
            if is_compressed:
                logger.info(f"壓縮後: {len(compressed)} bytes (壓縮率: {ratio:.1f}%, 使用 RLE)")
            else:
                logger.info(f"未壓縮: {len(compressed)} bytes (RLE 無效，使用原始資料)")
            
            # 打包協議
            self.seq_id += 1
            packet = Protocol.pack_full_frame(self.seq_id, compressed)
            logger.info(f"封包大小: {len(packet)} bytes (含標頭)")
            
            # 發送到所有客戶端
            logger.info(f"發送到 {len(self.clients)} 個客戶端...")
            await asyncio.gather(
                *[client.send(packet) for client in self.clients],
                return_exceptions=True
            )
            logger.info("✓ 發送完成")
            
        except Exception as e:
            logger.error(f"發送圖片失敗: {e}")
    
    async def send_text(self, text: str, font_size: int = 48):
        """
        發送文字到所有客戶端
        
        Args:
            text: 要顯示的文字
            font_size: 字體大小
        """
        if not self.clients:
            logger.warning("沒有連接的客戶端")
            return
        
        logger.info(f"處理文字: {text}")
        
        try:
            # 建立文字圖像
            img = self.processor.create_text_image(text, font_size)
            raw_data = self.processor.image_to_bytes(img)
            logger.info(f"原始資料: {len(raw_data)} bytes")
            
            # 智能壓縮
            compressed, is_compressed = self.compressor.compress_smart(raw_data)
            ratio = self.compressor.compress_ratio(len(raw_data), len(compressed))
            
            if is_compressed:
                logger.info(f"壓縮後: {len(compressed)} bytes (壓縮率: {ratio:.1f}%, 使用 RLE)")
            else:
                logger.info(f"未壓縮: {len(compressed)} bytes (RLE 無效，使用原始資料)")
            
            # 打包協議
            self.seq_id += 1
            packet = Protocol.pack_full_frame(self.seq_id, compressed)
            
            # 發送
            logger.info(f"發送到 {len(self.clients)} 個客戶端...")
            await asyncio.gather(
                *[client.send(packet) for client in self.clients],
                return_exceptions=True
            )
            logger.info("✓ 發送完成")
            
        except Exception as e:
            logger.error(f"發送文字失敗: {e}")
    
    async def send_test_pattern(self):
        """發送測試圖案"""
        if not self.clients:
            logger.warning("沒有連接的客戶端")
            return
        
        logger.info("發送測試圖案")
        
        try:
            # 建立測試圖案
            img = self.processor.create_test_pattern()
            raw_data = self.processor.image_to_bytes(img)
            
            # 智能壓縮
            compressed, is_compressed = self.compressor.compress_smart(raw_data)
            ratio = self.compressor.compress_ratio(len(raw_data), len(compressed))
            
            if is_compressed:
                logger.info(f"壓縮率: {ratio:.1f}% (使用 RLE)")
            else:
                logger.info(f"未壓縮 (RLE 無效，使用原始資料)")
            
            # 打包並發送
            self.seq_id += 1
            packet = Protocol.pack_full_frame(self.seq_id, compressed)
            
            await asyncio.gather(
                *[client.send(packet) for client in self.clients],
                return_exceptions=True
            )
            logger.info("✓ 測試圖案已發送")
            
        except Exception as e:
            logger.error(f"發送測試圖案失敗: {e}")
    
    async def send_command(self, command: Command, param: int = 0):
        """
        發送控制指令
        
        Args:
            command: 指令
            param: 參數
        """
        if not self.clients:
            logger.warning("沒有連接的客戶端")
            return
        
        logger.info(f"發送指令: {command.name}")
        
        try:
            self.seq_id += 1
            packet = Protocol.pack_command(self.seq_id, command, param)
            
            await asyncio.gather(
                *[client.send(packet) for client in self.clients],
                return_exceptions=True
            )
            logger.info("✓ 指令已發送")
            
        except Exception as e:
            logger.error(f"發送指令失敗: {e}")
    
    async def start(self):
        """啟動伺服器"""
        logger.info(f"啟動 WebSocket 伺服器...")
        logger.info(f"監聽: {self.host}:{self.port}")
        
        async with serve(self.handler, self.host, self.port):
            logger.info("✓ 伺服器已啟動")
            logger.info("等待客戶端連接...")
            await asyncio.Future()  # 永久運行


async def interactive_mode(server: DisplayServer):
    """互動模式"""
    print("\n" + "="*50)
    print("WiFi SPI Display Server - 互動模式")
    print("="*50)
    print("\n指令:")
    print("  text <文字>       - 發送文字")
    print("  image <檔案>      - 發送圖片")
    print("  test              - 發送測試圖案")
    print("  clear             - 清空螢幕")
    print("  clients           - 顯示連接的客戶端")
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
                await server.send_test_pattern()
            elif action == "clear":
                await server.send_command(Command.CLEAR_SCREEN)
            elif action == "text" and len(parts) > 1:
                await server.send_text(parts[1])
            elif action == "image" and len(parts) > 1:
                await server.send_image(parts[1])
            else:
                print("未知指令或缺少參數")
        
        except KeyboardInterrupt:
            print("\n結束程式...")
            break
        except Exception as e:
            print(f"錯誤: {e}")


async def main():
    """主程式"""
    server = DisplayServer(host="0.0.0.0", port=8266)
    
    # 同時運行伺服器和互動模式
    await asyncio.gather(
        server.start(),
        interactive_mode(server)
    )


if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        logger.info("\n程式已停止")
