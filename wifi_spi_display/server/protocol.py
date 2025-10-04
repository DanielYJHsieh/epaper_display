"""
通訊協議模組
定義 WebSocket 二進位協議格式
"""

import struct
from typing import Tuple, List
from enum import IntEnum


class PacketType(IntEnum):
    """封包類型"""
    FULL_UPDATE = 0x01    # 完整畫面更新
    DELTA_UPDATE = 0x02   # 差分更新
    COMMAND = 0x03        # 控制指令
    ACK = 0x10           # 確認
    NAK = 0x11           # 否認（錯誤）


class Command(IntEnum):
    """控制指令"""
    CLEAR_SCREEN = 0x10   # 清空螢幕
    SLEEP = 0x11          # 休眠模式
    WAKE = 0x12           # 喚醒
    PARTIAL_MODE = 0x13   # 局部更新模式
    FULL_MODE = 0x14      # 全螢幕更新模式
    

class Protocol:
    """通訊協議處理"""
    
    HEADER_MAGIC = 0xA5
    HEADER_SIZE = 8  # 1 + 1 + 2 + 4 bytes
    
    @staticmethod
    def pack_header(packet_type: int, seq_id: int, length: int) -> bytes:
        """
        打包封包頭
        
        Format: [Header(1B)][Type(1B)][SeqID(2B)][Length(4B)]
        
        Args:
            packet_type: 封包類型
            seq_id: 序號
            length: Payload 長度
            
        Returns:
            封包頭 bytes
        """
        return struct.pack('!BBHi',
                          Protocol.HEADER_MAGIC,
                          packet_type,
                          seq_id,
                          length)
    
    @staticmethod
    def unpack_header(data: bytes) -> Tuple[int, int, int, int]:
        """
        解析封包頭
        
        Args:
            data: 至少 8 bytes 的資料
            
        Returns:
            (header, type, seq_id, length)
        """
        if len(data) < Protocol.HEADER_SIZE:
            raise ValueError(f"資料太短: {len(data)} < {Protocol.HEADER_SIZE}")
        
        return struct.unpack('!BBHi', data[:Protocol.HEADER_SIZE])
    
    @staticmethod
    def pack_full_frame(seq_id: int, data: bytes) -> bytes:
        """
        打包完整畫面更新
        
        Args:
            seq_id: 序號
            data: 壓縮後的圖像資料
            
        Returns:
            完整封包
        """
        header = Protocol.pack_header(PacketType.FULL_UPDATE, seq_id, len(data))
        return header + data
    
    @staticmethod
    def pack_delta(seq_id: int, regions: List[Tuple[int, int, int, int, bytes]]) -> bytes:
        """
        打包差分更新
        
        Args:
            seq_id: 序號
            regions: 改變區域列表 [(x, y, w, h, data), ...]
            
        Returns:
            完整封包
        """
        payload = b''
        
        for x, y, w, h, data in regions:
            # 每個區域: [X(2B)][Y(2B)][W(2B)][H(2B)][Length(4B)][Data]
            region_header = struct.pack('!HHHHi', x, y, w, h, len(data))
            payload += region_header + data
        
        header = Protocol.pack_header(PacketType.DELTA_UPDATE, seq_id, len(payload))
        return header + payload
    
    @staticmethod
    def pack_command(seq_id: int, command: int, param: int = 0) -> bytes:
        """
        打包控制指令
        
        Args:
            seq_id: 序號
            command: 指令碼
            param: 參數（可選）
            
        Returns:
            完整封包
        """
        payload = struct.pack('!BB', command, param)
        header = Protocol.pack_header(PacketType.COMMAND, seq_id, len(payload))
        return header + payload
    
    @staticmethod
    def pack_ack(seq_id: int, status: int = 1) -> bytes:
        """
        打包 ACK
        
        Args:
            seq_id: 序號
            status: 狀態（1=成功，0=失敗）
            
        Returns:
            完整封包
        """
        payload = bytes([status])
        header = Protocol.pack_header(PacketType.ACK, seq_id, len(payload))
        return header + payload
    
    @staticmethod
    def validate_header(data: bytes) -> bool:
        """
        驗證封包頭
        
        Args:
            data: 封包資料
            
        Returns:
            是否有效
        """
        if len(data) < Protocol.HEADER_SIZE:
            return False
        
        try:
            header, _, _, length = Protocol.unpack_header(data)
            return header == Protocol.HEADER_MAGIC and length >= 0
        except:
            return False
    
    @staticmethod
    def get_packet_info(data: bytes) -> dict:
        """
        取得封包資訊（用於除錯）
        
        Args:
            data: 封包資料
            
        Returns:
            資訊字典
        """
        if not Protocol.validate_header(data):
            return {'valid': False}
        
        header, ptype, seq_id, length = Protocol.unpack_header(data)
        
        type_names = {
            PacketType.FULL_UPDATE: 'FULL_UPDATE',
            PacketType.DELTA_UPDATE: 'DELTA_UPDATE',
            PacketType.COMMAND: 'COMMAND',
            PacketType.ACK: 'ACK',
            PacketType.NAK: 'NAK',
        }
        
        return {
            'valid': True,
            'header': f'0x{header:02X}',
            'type': type_names.get(ptype, f'UNKNOWN(0x{ptype:02X})'),
            'type_code': ptype,
            'seq_id': seq_id,
            'payload_length': length,
            'total_length': Protocol.HEADER_SIZE + length
        }


if __name__ == "__main__":
    # 測試程式
    print("=== 協議測試 ===")
    
    # 測試 1: 完整畫面更新
    print("\n測試 1: 完整畫面更新")
    test_data = b'\xFF' * 100
    packet1 = Protocol.pack_full_frame(seq_id=1, data=test_data)
    info1 = Protocol.get_packet_info(packet1)
    print(f"封包類型: {info1['type']}")
    print(f"序號: {info1['seq_id']}")
    print(f"Payload 長度: {info1['payload_length']}")
    print(f"總長度: {info1['total_length']}")
    
    # 測試 2: 差分更新
    print("\n測試 2: 差分更新")
    regions = [
        (10, 20, 100, 50, b'\x00' * 50),  # (x, y, w, h, data)
        (200, 100, 80, 60, b'\xFF' * 40),
    ]
    packet2 = Protocol.pack_delta(seq_id=2, regions=regions)
    info2 = Protocol.get_packet_info(packet2)
    print(f"封包類型: {info2['type']}")
    print(f"序號: {info2['seq_id']}")
    print(f"Payload 長度: {info2['payload_length']}")
    
    # 測試 3: 控制指令
    print("\n測試 3: 控制指令")
    packet3 = Protocol.pack_command(seq_id=3, command=Command.CLEAR_SCREEN)
    info3 = Protocol.get_packet_info(packet3)
    print(f"封包類型: {info3['type']}")
    print(f"序號: {info3['seq_id']}")
    
    # 測試 4: ACK
    print("\n測試 4: ACK")
    packet4 = Protocol.pack_ack(seq_id=1, status=1)
    info4 = Protocol.get_packet_info(packet4)
    print(f"封包類型: {info4['type']}")
    print(f"序號: {info4['seq_id']}")
    
    # 測試 5: 驗證
    print("\n測試 5: 封包驗證")
    print(f"有效封包: {Protocol.validate_header(packet1)}")
    print(f"無效封包: {Protocol.validate_header(b'\\x00\\x00\\x00')}")
    
    print("\n測試完成！")
