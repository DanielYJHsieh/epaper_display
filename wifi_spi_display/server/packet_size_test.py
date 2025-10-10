"""
WebSocket 封包大小測試工具

測試 ESP8266 能穩定接收的最大封包大小
依序測試 800×120, 800×130, 800×140, 800×150, 800×160
"""

import asyncio
import websockets.asyncio.server
import logging
import struct
from datetime import datetime

# 設定日誌
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(levelname)s - %(message)s'
)
logger = logging.getLogger(__name__)

# 測試配置
TEST_SIZES = [
    (800, 120, "12000 bytes (12KB) - 已知穩定"),
    (800, 130, "13000 bytes (13KB)"),
    (800, 140, "14000 bytes (14KB)"),
    (800, 150, "15000 bytes (15KB)"),
    (800, 160, "16000 bytes (16KB)"),
    (800, 170, "17000 bytes (17KB)"),
    (800, 180, "18000 bytes (18KB)"),
    (800, 190, "19000 bytes (19KB)"),
    (800, 200, "20000 bytes (20KB)"),
    (800, 210, "21000 bytes (21KB)"),
    (800, 220, "22000 bytes (22KB)"),
    (800, 230, "23000 bytes (23KB)"),
    (800, 240, "24000 bytes (24KB) - 新上限測試"),
]

SERVER_HOST = "0.0.0.0"
SERVER_PORT = 8266

class PacketSizeTest:
    def __init__(self):
        self.clients = set()
        self.test_results = {}
        
    async def handle_client(self, websocket):
        """處理客戶端連接"""
        client_info = f"{websocket.remote_address[0]}:{websocket.remote_address[1]}"
        logger.info(f"✓ 客戶端連接: {client_info}")
        self.clients.add(websocket)
        
        try:
            async for message in websocket:
                # 接收 ESP8266 的回應
                if isinstance(message, str):
                    logger.info(f"收到訊息: {message}")
                    
        except websockets.exceptions.ConnectionClosed:
            logger.warning(f"客戶端斷開: {client_info}")
        finally:
            self.clients.discard(websocket)
            
    async def send_test_packet(self, width: int, height: int, description: str):
        """發送測試封包"""
        if not self.clients:
            logger.error("沒有連接的客戶端")
            return False
            
        # 計算資料大小
        data_size = (width * height) // 8
        
        logger.info(f"\n{'='*60}")
        logger.info(f"測試: {width}×{height} = {data_size} bytes")
        logger.info(f"說明: {description}")
        logger.info(f"{'='*60}")
        
        # 建立測試資料（全部填充 0xAA 的交替模式）
        test_data = bytes([0xAA] * data_size)
        
        # 建立封包：tile_index (1 byte) + data
        tile_index = 0  # 使用 tile 0
        packet = struct.pack('B', tile_index) + test_data
        
        logger.info(f"封包大小: {len(packet)} bytes (含 1 byte header)")
        logger.info(f"發送到 {len(self.clients)} 個客戶端...")
        
        # 發送到所有客戶端
        success_count = 0
        fail_count = 0
        
        for client in list(self.clients):
            try:
                await client.send(packet)
                logger.info(f"✓ 發送成功")
                success_count += 1
                
                # 等待一小段時間觀察連接狀態
                await asyncio.sleep(0.5)
                
                # 檢查連接是否還存活
                if client in self.clients:
                    logger.info(f"✓ 連接穩定 ({width}×{height})")
                    return True
                else:
                    logger.error(f"✗ 連接已斷開 ({width}×{height})")
                    return False
                    
            except Exception as e:
                logger.error(f"✗ 發送失敗: {e}")
                fail_count += 1
                return False
                
        return success_count > 0
        
    async def run_test_sequence(self):
        """執行完整測試序列"""
        logger.info(f"\n{'#'*60}")
        logger.info(f"# WebSocket 封包大小測試")
        logger.info(f"# 等待 ESP8266 連接...")
        logger.info(f"{'#'*60}\n")
        
        # 等待客戶端連接
        while not self.clients:
            await asyncio.sleep(1)
            
        logger.info(f"客戶端已連接，開始測試...\n")
        await asyncio.sleep(2)
        
        # 依序測試每個大小
        for width, height, description in TEST_SIZES:
            data_size = (width * height) // 8
            
            logger.info(f"\n{'='*60}")
            logger.info(f"準備測試: {width}×{height} ({data_size} bytes)")
            logger.info(f"等待 3 秒...")
            logger.info(f"{'='*60}")
            await asyncio.sleep(3)
            
            # 發送測試封包
            success = await self.send_test_packet(width, height, description)
            
            # 記錄結果
            self.test_results[(width, height)] = success
            
            if not success:
                logger.error(f"\n✗✗✗ 測試失敗於 {width}×{height} ({data_size} bytes) ✗✗✗")
                logger.error(f"找到極限：上一個成功的大小是穩定上限")
                break
            else:
                logger.info(f"\n✓✓✓ {width}×{height} ({data_size} bytes) 測試成功 ✓✓✓")
                
            # 等待下一輪測試
            await asyncio.sleep(2)
            
        # 輸出測試結果摘要
        self.print_test_summary()
        
    def print_test_summary(self):
        """輸出測試結果摘要"""
        logger.info(f"\n{'#'*60}")
        logger.info(f"# 測試結果摘要")
        logger.info(f"{'#'*60}\n")
        
        max_stable_size = None
        
        for (width, height), success in self.test_results.items():
            data_size = (width * height) // 8
            status = "✓ 成功" if success else "✗ 失敗"
            logger.info(f"{width}×{height} ({data_size:5d} bytes): {status}")
            
            if success:
                max_stable_size = (width, height, data_size)
                
        if max_stable_size:
            w, h, size = max_stable_size
            logger.info(f"\n{'='*60}")
            logger.info(f"穩定上限: {w}×{h} = {size} bytes ({size/1024:.1f} KB)")
            logger.info(f"{'='*60}")
        else:
            logger.info(f"\n所有測試都失敗")
            
    async def start_server(self):
        """啟動測試伺服器"""
        logger.info(f"啟動測試伺服器於 {SERVER_HOST}:{SERVER_PORT}")
        
        async with websockets.asyncio.server.serve(
            self.handle_client,
            SERVER_HOST,
            SERVER_PORT,
            max_size=30 * 1024  # 允許最大 30KB 封包
        ):
            # 在背景執行測試序列
            test_task = asyncio.create_task(self.run_test_sequence())
            
            # 保持伺服器運行
            await asyncio.Future()  # run forever

async def main():
    """主程式"""
    test = PacketSizeTest()
    await test.start_server()

if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        logger.info("\n測試中斷")
