#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
WiFi SPI Display - 模組測試腳本
"""

print("=" * 50)
print("WiFi SPI Display - Server 模組測試")
print("=" * 50)

# 測試 1: RLE 壓縮
print("\n【測試 1】RLE 壓縮")
print("-" * 50)
from compressor import RLECompressor
compressor = RLECompressor()
test_data = bytes([0xFF] * 100 + [0x00] * 100)
compressed = compressor.compress(test_data)
print(f"原始: {len(test_data)} bytes")
print(f"壓縮: {len(compressed)} bytes")
print(f"壓縮率: {len(compressed)/len(test_data)*100:.1f}%")
decompressed = compressor.decompress(compressed)
assert decompressed == test_data
print("✅ RLE 壓縮測試通過")

# 測試 2: 影像處理
print("\n【測試 2】影像處理")
print("-" * 50)
from image_processor import ImageProcessor
processor = ImageProcessor()
img = processor.create_text_image("Hello World")
print(f"影像尺寸: {img.size}")
img_1bit = processor.convert_to_1bit(img)
print(f"1-bit 影像模式: {img_1bit.mode}")
img_bytes = processor.image_to_bytes(img_1bit)
print(f"Bytes 大小: {len(img_bytes)} (應為 48000)")
assert len(img_bytes) == 48000
print("✅ 影像處理測試通過")

# 測試 3: 協議封包
print("\n【測試 3】協議封包")
print("-" * 50)
from protocol import Protocol
protocol = Protocol()
test_data = bytes([0xFF] * 1000)
packet = protocol.pack_full_frame(1, test_data)
print(f"封包大小: {len(packet)} bytes")
header, pkt_type, seq_id, length = protocol.unpack_header(packet)
print(f"Header Magic: 0x{header:02X}")
print(f"類型: 0x{pkt_type:02X}")
print(f"SeqID: {seq_id}")
print(f"Payload 長度: {length}")
assert header == 0xA5
assert pkt_type == 0x01
assert seq_id == 1
assert length == 1000
print("✅ 協議封包測試通過")

# 測試 4: 完整壓縮流程
print("\n【測試 4】完整壓縮流程")
print("-" * 50)
img = processor.create_text_image("Test\nCompression\nRatio")
img_bytes = processor.image_to_bytes(processor.convert_to_1bit(img))
compressed = compressor.compress(img_bytes)
ratio = len(compressed) / len(img_bytes) * 100
print(f"原始影像: {len(img_bytes)} bytes")
print(f"壓縮後: {compressed} bytes")
print(f"壓縮率: {ratio:.1f}%")
decompressed = compressor.decompress(compressed)
assert decompressed == img_bytes
print("✅ 完整壓縮流程測試通過")

# 測試 5: Delta 壓縮
print("\n【測試 5】Delta 壓縮")
print("-" * 50)
from compressor import DeltaCompressor
delta_comp = DeltaCompressor()
old_data = bytes([0xFF] * 48000)
new_data = bytes([0xFF] * 1000 + [0x00] * 100 + [0xFF] * 46900)

# 第一幀（完整）
diff1, full1 = delta_comp.compress(old_data)
assert diff1 is None
assert full1 == old_data
print("第一幀: 完整資料")

# 第二幀（差分）
diff2, full2 = delta_comp.compress(new_data)
assert diff2 is not None
assert full2 is None
print(f"第二幀: {len(diff2)} 個差異點")
ratio = delta_comp.get_diff_ratio(diff2, len(new_data))
print(f"差異比例: {ratio:.2f}%")
print("✅ Delta 壓縮測試通過")

# 總結
print("\n" + "=" * 50)
print("✅ 所有測試通過！")
print("=" * 50)
