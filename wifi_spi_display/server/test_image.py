# 在伺服器目錄執行
from PIL import Image, ImageDraw, ImageFont

# 建立簡單白色背景
img = Image.new('RGB', (400, 240), 'white')
draw = ImageDraw.Draw(img)

# 繪製黑色邊框
draw.rectangle([10, 10, 390, 230], outline='black', width=5)

# 繪製一些文字
draw.text((100, 100), "ESP8266 Display Test", fill='black')
draw.text((120, 130), "400 x 240 pixels", fill='black')

# 儲存
img.save('simple_test.png')
print("已建立 simple_test.png")