"""
壓縮模組
實作 RLE (Run-Length Encoding) 壓縮和 Delta 編碼
"""

from typing import List, Tuple, Optional
import numpy as np


class RLECompressor:
    """RLE 壓縮器"""
    
    @staticmethod
    def compress(data: bytes) -> bytes:
        """
        RLE 壓縮
        
        格式: [count, value, count, value, ...]
        count: 重複次數 (1-255)
        value: byte 值 (0-255)
        
        Args:
            data: 原始資料
            
        Returns:
            壓縮後的資料
        """
        if not data:
            return b''
        
        compressed = []
        i = 0
        
        while i < len(data):
            count = 1
            value = data[i]
            
            # 計算重複次數（最多 255）
            while i + count < len(data) and \
                  data[i + count] == value and \
                  count < 255:
                count += 1
            
            compressed.append(count)
            compressed.append(value)
            i += count
        
        return bytes(compressed)
    
    @staticmethod
    def decompress(data: bytes, expected_size: Optional[int] = None) -> bytes:
        """
        RLE 解壓縮（用於測試）
        
        Args:
            data: 壓縮的資料
            expected_size: 預期解壓後大小（用於驗證）
            
        Returns:
            解壓後的資料
        """
        if not data:
            return b''
        
        decompressed = []
        i = 0
        
        while i < len(data) - 1:
            count = data[i]
            value = data[i + 1]
            
            decompressed.extend([value] * count)
            i += 2
        
        result = bytes(decompressed)
        
        if expected_size and len(result) != expected_size:
            raise ValueError(f"解壓後大小不符: 預期 {expected_size}, 實際 {len(result)}")
        
        return result
    
    @staticmethod
    def compress_ratio(original_size: int, compressed_size: int) -> float:
        """
        計算壓縮率
        
        Args:
            original_size: 原始大小
            compressed_size: 壓縮後大小
            
        Returns:
            壓縮率（百分比）
        """
        if original_size == 0:
            return 0.0
        return (1 - compressed_size / original_size) * 100
    
    @staticmethod
    def analyze_compressibility(data: bytes) -> dict:
        """
        分析資料的可壓縮性
        
        Args:
            data: 要分析的資料
            
        Returns:
            分析結果字典
        """
        if not data:
            return {'status': 'empty'}
        
        # 統計連續相同 byte 的長度
        runs = []
        current_run = 1
        
        for i in range(1, len(data)):
            if data[i] == data[i-1]:
                current_run += 1
            else:
                runs.append(current_run)
                current_run = 1
        runs.append(current_run)
        
        return {
            'original_size': len(data),
            'total_runs': len(runs),
            'avg_run_length': np.mean(runs),
            'max_run_length': max(runs),
            'min_run_length': min(runs),
            'unique_bytes': len(set(data)),
            'estimated_compressed_size': len(runs) * 2,
            'estimated_ratio': (1 - (len(runs) * 2) / len(data)) * 100
        }


class DeltaCompressor:
    """差分壓縮器"""
    
    def __init__(self):
        """初始化"""
        self.last_frame = None
    
    def compress(self, current_frame: bytes) -> Tuple[Optional[List[Tuple[int, int]]], Optional[bytes]]:
        """
        差分壓縮
        
        Args:
            current_frame: 當前幀資料
            
        Returns:
            (差異列表, 完整資料)
            - 如果是第一幀：(None, 完整資料)
            - 如果有差異：(差異列表, None)
        """
        if self.last_frame is None:
            # 第一幀，回傳完整資料
            self.last_frame = bytearray(current_frame)
            return None, current_frame
        
        # 找出差異
        diff = []
        for i in range(min(len(current_frame), len(self.last_frame))):
            if current_frame[i] != self.last_frame[i]:
                diff.append((i, current_frame[i]))
        
        # 更新上一幀
        self.last_frame = bytearray(current_frame)
        
        return diff, None
    
    def get_diff_ratio(self, diff: List[Tuple[int, int]], total_size: int) -> float:
        """
        計算差異比例
        
        Args:
            diff: 差異列表
            total_size: 總大小
            
        Returns:
            差異百分比
        """
        if not diff or total_size == 0:
            return 0.0
        return (len(diff) / total_size) * 100
    
    def reset(self):
        """重置狀態"""
        self.last_frame = None


class HybridCompressor:
    """混合壓縮器（RLE + Delta）"""
    
    def __init__(self):
        """初始化"""
        self.rle = RLECompressor()
        self.delta = DeltaCompressor()
    
    def compress_first_frame(self, data: bytes) -> bytes:
        """
        壓縮首幀（完整資料 + RLE）
        
        Args:
            data: 原始資料
            
        Returns:
            壓縮後的資料
        """
        return self.rle.compress(data)
    
    def compress_update(self, data: bytes) -> Tuple[bool, bytes]:
        """
        壓縮更新幀（嘗試 Delta，否則完整 RLE）
        
        Args:
            data: 當前幀資料
            
        Returns:
            (is_delta, compressed_data)
        """
        diff, full = self.delta.compress(data)
        
        if diff is None:
            # 首幀
            return False, self.rle.compress(full)
        
        # 計算差異大小
        diff_size = len(diff) * 5  # 每個差異: 4 bytes (索引) + 1 byte (值)
        full_compressed = self.rle.compress(data)
        
        # 如果差異太多，直接用完整 RLE
        if diff_size >= len(full_compressed):
            self.delta.reset()  # 重置 delta，下次重新開始
            return False, full_compressed
        
        # 使用 delta（打包差異列表）
        packed_diff = self._pack_diff(diff)
        return True, packed_diff
    
    def _pack_diff(self, diff: List[Tuple[int, int]]) -> bytes:
        """
        打包差異列表
        
        Format: [count(4B)][index(4B), value(1B), ...]
        
        Args:
            diff: 差異列表 [(index, value), ...]
            
        Returns:
            打包後的資料
        """
        result = len(diff).to_bytes(4, 'big')  # 差異數量
        
        for index, value in diff:
            result += index.to_bytes(4, 'big')  # 索引
            result += bytes([value])             # 值
        
        return result
    
    def reset(self):
        """重置狀態"""
        self.delta.reset()


if __name__ == "__main__":
    # 測試程式
    print("=== RLE 壓縮測試 ===")
    
    # 測試 1: 高度重複的資料
    test_data1 = bytes([0xFF] * 100 + [0x00] * 100)
    compressed1 = RLECompressor.compress(test_data1)
    print(f"\n測試 1 (高度重複):")
    print(f"原始大小: {len(test_data1)} bytes")
    print(f"壓縮後: {len(compressed1)} bytes")
    print(f"壓縮率: {RLECompressor.compress_ratio(len(test_data1), len(compressed1)):.1f}%")
    
    # 驗證解壓
    decompressed1 = RLECompressor.decompress(compressed1, len(test_data1))
    print(f"解壓正確: {decompressed1 == test_data1}")
    
    # 測試 2: 隨機資料
    test_data2 = bytes(np.random.randint(0, 256, 200, dtype=np.uint8))
    compressed2 = RLECompressor.compress(test_data2)
    print(f"\n測試 2 (隨機資料):")
    print(f"原始大小: {len(test_data2)} bytes")
    print(f"壓縮後: {len(compressed2)} bytes")
    print(f"壓縮率: {RLECompressor.compress_ratio(len(test_data2), len(compressed2)):.1f}%")
    
    # 測試 3: 分析可壓縮性
    analysis = RLECompressor.analyze_compressibility(test_data1)
    print(f"\n測試 3 (可壓縮性分析):")
    print(f"總 runs: {analysis['total_runs']}")
    print(f"平均 run 長度: {analysis['avg_run_length']:.1f}")
    print(f"預估壓縮率: {analysis['estimated_ratio']:.1f}%")
    
    print("\n=== Delta 壓縮測試 ===")
    
    delta = DeltaCompressor()
    
    # 首幀
    frame1 = bytes([0xFF] * 100)
    diff1, full1 = delta.compress(frame1)
    print(f"\n首幀: 回傳完整資料 {len(full1)} bytes")
    
    # 第二幀（少量變化）
    frame2 = bytearray(frame1)
    frame2[10] = 0x00
    frame2[20] = 0x00
    diff2, full2 = delta.compress(bytes(frame2))
    print(f"第二幀: {len(diff2)} 個差異")
    print(f"差異比例: {delta.get_diff_ratio(diff2, len(frame2)):.2f}%")
    
    print("\n測試完成！")
