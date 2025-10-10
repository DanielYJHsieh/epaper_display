"""
圖像處理模組
負責將圖片轉換為 1-bit 黑白格式，適合電子紙顯示
"""

from PIL import Image, ImageDraw, ImageFont
import numpy as np
from typing import Tuple


class ImageProcessor:
    """圖像處理器，用於電子紙顯示"""
    
    def __init__(self, width: int = 400, height: int = 240):
        """
        初始化圖像處理器
        
        Args:
            width: 顯示器寬度（像素）- 預設 400x240 以適應 ESP8266 記憶體限制
            height: 顯示器高度（像素）
        """
        self.width = width
        self.height = height
        self.total_pixels = width * height
        self.total_bytes = self.total_pixels // 8  # 1-bit，8 pixels per byte
        
    def convert_to_1bit(self, image: Image.Image, dither: bool = True) -> Image.Image:
        """
        將圖片轉換為 1-bit 黑白格式
        
        Args:
            image: 輸入圖片
            dither: 是否使用抖動演算法（提升顯示品質）
            
        Returns:
            1-bit 黑白圖片
        """
        # 縮放到目標尺寸
        image = image.resize((self.width, self.height), Image.Resampling.LANCZOS)
        
        # 轉換為灰階
        image = image.convert('L')
        
        # 轉換為 1-bit（黑白）
        if dither:
            # 使用 Floyd-Steinberg 抖動演算法
            image = image.convert('1', dither=Image.Dither.FLOYDSTEINBERG)
        else:
            # 直接二值化
            image = image.convert('1', dither=Image.Dither.NONE)
        
        return image
    
    def image_to_bytes(self, image: Image.Image) -> bytes:
        """
        將 1-bit 圖片轉換為 byte array
        
        Args:
            image: 1-bit 黑白圖片
            
        Returns:
            byte array（48000 bytes for 800x480）
        """
        # 確保是 1-bit 模式
        if image.mode != '1':
            image = image.convert('1')
        
        # 取得像素資料
        pixels = np.array(image, dtype=np.uint8)
        
        # 打包為 1-bit（8 pixels per byte）
        # PIL 的 '1' 模式：0=黑，255=白
        # 不反轉，直接使用：白色(255)變成1，黑色(0)變成0
        pixels = (pixels > 0).astype(np.uint8)  # 白色像素 = 1, 黑色像素 = 0
        
        # 打包為 bytes
        packed = np.packbits(pixels, axis=1)
        
        return packed.tobytes()
    
    def create_text_image(self, text: str, font_size: int = 48, 
                         font_path: str = None) -> Image.Image:
        """
        從文字建立圖片
        
        Args:
            text: 要顯示的文字
            font_size: 字體大小
            font_path: 字體檔案路徑（None 則使用預設）
            
        Returns:
            1-bit 黑白圖片
        """
        # 建立白色背景
        img = Image.new('1', (self.width, self.height), 1)
        draw = ImageDraw.Draw(img)
        
        # 載入字體
        try:
            if font_path:
                font = ImageFont.truetype(font_path, font_size)
            else:
                # 嘗試使用系統字體
                try:
                    # Windows
                    font = ImageFont.truetype("c:/windows/fonts/msjh.ttc", font_size)
                except:
                    try:
                        # Linux
                        font = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", font_size)
                    except:
                        # 使用預設字體
                        font = ImageFont.load_default()
        except Exception as e:
            print(f"載入字體失敗，使用預設字體: {e}")
            font = ImageFont.load_default()
        
        # 計算文字邊界
        bbox = draw.textbbox((0, 0), text, font=font)
        text_width = bbox[2] - bbox[0]
        text_height = bbox[3] - bbox[1]
        
        # 置中文字
        x = (self.width - text_width) // 2
        y = (self.height - text_height) // 2
        
        # 繪製文字（黑色）
        draw.text((x, y), text, font=font, fill=0)
        
        return img
    
    def create_test_pattern(self) -> Image.Image:
        """
        建立測試圖案
        
        Returns:
            測試圖案圖片
        """
        img = Image.new('1', (self.width, self.height), 1)
        draw = ImageDraw.Draw(img)
        
        # 繪製邊框
        draw.rectangle([0, 0, self.width-1, self.height-1], outline=0, width=5)
        
        # 繪製對角線
        draw.line([0, 0, self.width, self.height], fill=0, width=3)
        draw.line([self.width, 0, 0, self.height], fill=0, width=3)
        
        # 繪製中心十字
        cx, cy = self.width // 2, self.height // 2
        draw.line([cx, 0, cx, self.height], fill=0, width=2)
        draw.line([0, cy, self.width, cy], fill=0, width=2)
        
        # 繪製文字
        draw.text((cx - 100, cy - 20), "TEST PATTERN", fill=0)
        draw.text((cx - 80, cy + 10), "400 x 240", fill=0)
        
        return img
    
    def get_image_info(self, image: Image.Image) -> dict:
        """
        取得圖片資訊
        
        Args:
            image: 圖片
            
        Returns:
            資訊字典
        """
        return {
            'size': image.size,
            'mode': image.mode,
            'format': image.format,
            'width': image.width,
            'height': image.height,
        }
    
    def split_image_to_tiles(self, image: Image.Image) -> dict:
        """
        將 800x480 圖片分割成 3 個 800x160 水平條帶（垂直分割）
        上、中、下三個區塊，完整覆蓋螢幕
        
        分區排列（從上到下）：
            0: (0,0) - (800,160)   Y: 0-160   (上)
            1: (0,160) - (800,320) Y: 160-320 (中)
            2: (0,320) - (800,480) Y: 320-480 (下)
        
        Args:
            image: 800x480 圖片
            
        Returns:
            {0: band_0, 1: band_1, 2: band_2}
        """
        # 確保圖片是 800x480
        if image.size != (800, 480):
            print(f"圖片尺寸不是 800x480，進行縮放: {image.size} -> (800, 480)")
            image = image.resize((800, 480), Image.Resampling.LANCZOS)
        
        tiles = {}
        
        # 分割 3 個水平條帶（從上到下）
        # 條帶 0: (0, 0, 800, 160) - 上
        tiles[0] = image.crop((0, 0, 800, 160))
        
        # 條帶 1: (0, 160, 800, 320) - 中
        tiles[1] = image.crop((0, 160, 800, 320))
        
        # 條帶 2: (0, 320, 800, 480) - 下
        tiles[2] = image.crop((0, 320, 800, 480))
        
        return tiles
    
    def process_tile(self, tile: Image.Image, dither: bool = True) -> bytes:
        """
        處理單個 800x160 條帶
        
        Args:
            tile: 800x160 條帶圖片
            dither: 是否使用抖動演算法
            
        Returns:
            16000 bytes 的 1-bit 資料 (800x160/8 = 16000)
        """
        # 確保尺寸正確
        if tile.size != (800, 160):
            print(f"條帶尺寸不正確，進行縮放: {tile.size} -> (800, 160)")
            tile = tile.resize((800, 160), Image.Resampling.LANCZOS)
        
        # 轉換為 1-bit 黑白
        if tile.mode != '1':
            tile = self.convert_to_1bit(tile, dither)
        
        # 轉換為 bytes
        return self.image_to_bytes(tile)


if __name__ == "__main__":
    # 測試程式
    processor = ImageProcessor(800, 480)
    
    # 測試 1: 建立測試圖案
    print("建立測試圖案...")
    test_img = processor.create_test_pattern()
    print(f"圖片模式: {test_img.mode}, 大小: {test_img.size}")
    
    # 轉換為 bytes
    data = processor.image_to_bytes(test_img)
    print(f"資料大小: {len(data)} bytes")
    
    # 測試 2: 建立文字圖片
    print("\n建立文字圖片...")
    text_img = processor.create_text_image("Hello, E-Paper!")
    data = processor.image_to_bytes(text_img)
    print(f"資料大小: {len(data)} bytes")
    
    # 儲存測試圖片
    test_img.save("test_pattern.png")
    text_img.save("test_text.png")
    print("\n測試圖片已儲存: test_pattern.png, test_text.png")
