"""
測試 800×480 分區顯示功能

創建測試圖片並透過分區傳輸顯示
"""

from PIL import Image, ImageDraw, ImageFont
import os

def create_test_image_800x480():
    """創建 800×480 測試圖片，分區明顯"""
    
    # 創建白色背景
    img = Image.new('RGB', (800, 480), 'white')
    draw = ImageDraw.Draw(img)
    
    # 定義 4 個分區的顏色
    colors = [
        (255, 200, 200),  # 淡紅 - 左上
        (200, 255, 200),  # 淡綠 - 右上
        (200, 200, 255),  # 淡藍 - 左下
        (255, 255, 200),  # 淡黃 - 右下
    ]
    
    tile_names = ["左上", "右上", "左下", "右下"]
    
    # 繪製 4 個分區
    positions = [
        (0, 0, 400, 240),      # 左上
        (400, 0, 800, 240),    # 右上
        (0, 240, 400, 480),    # 左下
        (400, 240, 800, 480),  # 右下
    ]
    
    try:
        # 嘗試使用中文字體
        font_large = ImageFont.truetype("c:/windows/fonts/msjh.ttc", 60)
        font_small = ImageFont.truetype("c:/windows/fonts/msjh.ttc", 30)
    except:
        # 使用預設字體
        font_large = ImageFont.load_default()
        font_small = ImageFont.load_default()
    
    for i, (x1, y1, x2, y2) in enumerate(positions):
        # 填充背景色
        draw.rectangle([x1, y1, x2, y2], fill=colors[i])
        
        # 繪製邊框
        draw.rectangle([x1, y1, x2-1, y2-1], outline='black', width=5)
        
        # 計算文字位置（置中）
        cx = (x1 + x2) // 2
        cy = (y1 + y2) // 2
        
        # 繪製分區標題
        title = f"區域 {i}"
        bbox = draw.textbbox((0, 0), title, font=font_large)
        text_w = bbox[2] - bbox[0]
        text_h = bbox[3] - bbox[1]
        draw.text((cx - text_w//2, cy - 60), title, fill='black', font=font_large)
        
        # 繪製分區名稱
        name = tile_names[i]
        bbox = draw.textbbox((0, 0), name, font=font_large)
        text_w = bbox[2] - bbox[0]
        draw.text((cx - text_w//2, cy - 10), name, fill='black', font=font_large)
        
        # 繪製座標
        coord_text = f"({x1},{y1})"
        bbox = draw.textbbox((0, 0), coord_text, font=font_small)
        text_w = bbox[2] - bbox[0]
        draw.text((cx - text_w//2, cy + 40), coord_text, fill='black', font=font_small)
    
    # 繪製中央分界線
    draw.line([400, 0, 400, 480], fill='red', width=3)
    draw.line([0, 240, 800, 240], fill='red', width=3)
    
    # 繪製標題
    title = "800×480 分區顯示測試"
    bbox = draw.textbbox((0, 0), title, font=font_large)
    text_w = bbox[2] - bbox[0]
    # 繪製在中央交叉點
    draw.rectangle([380, 220, 420 + text_w, 260], fill='white', outline='black', width=2)
    draw.text((390, 225), title, fill='red', font=font_large)
    
    return img


def create_gradient_test():
    """創建漸變測試圖"""
    img = Image.new('L', (800, 480), 255)
    draw = ImageDraw.Draw(img)
    
    # 水平漸變
    for x in range(800):
        gray = int(255 * (1 - x / 800))
        draw.line([(x, 0), (x, 480)], fill=gray)
    
    # 添加文字
    try:
        font = ImageFont.truetype("c:/windows/fonts/arial.ttf", 48)
    except:
        font = ImageFont.load_default()
    
    draw.text((50, 220), "Gradient Test 800x480", fill=0, font=font)
    
    return img


def main():
    """主程式"""
    print("創建測試圖片...")
    
    # 創建輸出目錄
    output_dir = "test_images_800x480"
    os.makedirs(output_dir, exist_ok=True)
    
    # 1. 分區測試圖
    print("1. 創建分區測試圖 (800×480)...")
    tile_test = create_test_image_800x480()
    tile_test_path = os.path.join(output_dir, "tile_test_800x480.png")
    tile_test.save(tile_test_path)
    print(f"   已儲存: {tile_test_path}")
    
    # 2. 漸變測試圖
    print("2. 創建漸變測試圖 (800×480)...")
    gradient = create_gradient_test()
    gradient_path = os.path.join(output_dir, "gradient_800x480.png")
    gradient.save(gradient_path)
    print(f"   已儲存: {gradient_path}")
    
    # 3. 棋盤格測試圖
    print("3. 創建棋盤格測試圖 (800×480)...")
    checkerboard = Image.new('1', (800, 480), 1)
    draw = ImageDraw.Draw(checkerboard)
    tile_size = 40
    for y in range(0, 480, tile_size):
        for x in range(0, 800, tile_size):
            if ((x // tile_size) + (y // tile_size)) % 2 == 0:
                draw.rectangle([x, y, x + tile_size, y + tile_size], fill=0)
    checkerboard_path = os.path.join(output_dir, "checkerboard_800x480.png")
    checkerboard.save(checkerboard_path)
    print(f"   已儲存: {checkerboard_path}")
    
    print("\n=== 測試圖片創建完成 ===")
    print(f"\n測試方式：")
    print(f"1. 啟動 server.py")
    print(f"2. ESP8266 連線後，輸入指令：")
    print(f"   tile {tile_test_path}")
    print(f"   或")
    print(f"   tile {gradient_path}")
    print(f"   或")
    print(f"   tile {checkerboard_path}")
    print()


if __name__ == "__main__":
    main()
