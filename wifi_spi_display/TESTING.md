# WiFi SPI Display - 快速測試指南

本文件提供快速測試步驟，確認 Server 和 Client 端實作是否正常運作。

## 📋 測試前準備

### 1. Python 環境 (Server 端)
```bash
cd server
pip install -r requirements.txt
```

### 2. Arduino 環境 (Client 端)
- 安裝必要函式庫：
  - GxEPD2 (by Jean-Marc Zingg)
  - WebSockets (by Markus Sattler)
  - Adafruit GFX Library
  - ESP8266 開發板支援

### 3. 配置設定
- 編輯 `client_esp8266/config.h`
  - 設定 WiFi SSID 和密碼
  - 設定 Server IP 位址 (預設 192.168.1.100)

## 🧪 測試步驟

### 階段 1: Server 端單元測試

#### 1.1 測試壓縮模組
```python
# 測試 RLE 壓縮
cd server
python

>>> from compressor import RLECompressor
>>> compressor = RLECompressor()
>>> test_data = bytes([0xFF] * 100 + [0x00] * 100)
>>> compressed = compressor.compress(test_data)
>>> print(f"原始: {len(test_data)}, 壓縮: {len(compressed)}")
>>> decompressed = compressor.decompress(compressed)
>>> assert decompressed == test_data
>>> print("✅ RLE 壓縮測試通過")
```

#### 1.2 測試影像處理
```python
>>> from image_processor import ImageProcessor
>>> processor = ImageProcessor()
>>> # 測試文字影像生成
>>> img = processor.create_text_image("Hello World")
>>> print(f"影像尺寸: {img.size}")
>>> # 測試轉換為 1-bit
>>> img_1bit = processor.convert_to_1bit(img)
>>> print(f"1-bit 影像模式: {img_1bit.mode}")
>>> # 測試轉換為 bytes
>>> img_bytes = processor.image_to_bytes(img_1bit)
>>> print(f"Bytes 大小: {len(img_bytes)} (應為 48000)")
>>> assert len(img_bytes) == 48000
>>> print("✅ 影像處理測試通過")
```

#### 1.3 測試協議封包
```python
>>> from protocol import Protocol
>>> protocol = Protocol()
>>> # 測試完整更新封包
>>> test_data = bytes([0xFF] * 48000)
>>> packet = protocol.pack_full_frame(1, test_data)
>>> print(f"封包大小: {len(packet)}")
>>> # 解析封包
>>> parsed = protocol.unpack(packet)
>>> print(f"類型: {parsed['type']}, SeqID: {parsed['seq_id']}")
>>> assert parsed['type'] == 0x01
>>> assert parsed['seq_id'] == 1
>>> print("✅ 協議封包測試通過")
```

#### 1.4 測試完整壓縮流程
```python
>>> from image_processor import ImageProcessor
>>> from compressor import RLECompressor
>>> processor = ImageProcessor()
>>> compressor = RLECompressor()
>>> # 生成測試影像
>>> img = processor.create_text_image("Test\nCompression\nRatio")
>>> img_bytes = processor.image_to_bytes(processor.convert_to_1bit(img))
>>> # 壓縮
>>> compressed = compressor.compress(img_bytes)
>>> ratio = len(compressed) / len(img_bytes) * 100
>>> print(f"壓縮率: {ratio:.1f}% ({len(compressed)}/{len(img_bytes)} bytes)")
>>> # 解壓縮並驗證
>>> decompressed = compressor.decompress(compressed)
>>> assert decompressed == img_bytes
>>> print("✅ 完整壓縮流程測試通過")
```

### 階段 2: Server 端啟動測試

```bash
cd server
python server.py
```

應該看到：
```
=================================
WiFi SPI Display Server v1.0
=================================
啟動 WebSocket Server: 0.0.0.0:8266
等待客戶端連線...

可用指令:
  text <內容>       - 傳送文字
  image <路徑>      - 傳送影像
  test              - 傳送測試圖案
  clear             - 清空螢幕
  clients           - 列出已連線客戶端
  quit              - 離開

>
```

### 階段 3: Client 端編譯測試 (無硬體)

1. 開啟 Arduino IDE
2. 開啟 `client_esp8266/wifi_spi_display.ino`
3. 工具 → 開發板 → LOLIN(WEMOS) D1 R2 & mini
4. 點擊「驗證」按鈕 (✓)
5. 確認編譯通過，無錯誤訊息

應該看到：
```
專案使用了 XXXXX bytes (XX%) 的程式儲存空間
全域變數使用了 XXXXX bytes (XX%) 的動態記憶體
```

### 階段 4: Client 端實體測試 (需要硬體)

#### 4.1 硬體連接
參考 `client_esp8266/README.md` 連接所有腳位。

#### 4.2 上傳程式
1. 連接 WeMos D1 Mini 到電腦
2. 選擇正確的 COM 埠
3. 點擊「上傳」按鈕 (→)
4. 等待上傳完成

#### 4.3 查看序列埠
工具 → 序列埠監控視窗，設定鮑率 115200

應該看到：
```
=================================
WiFi SPI Display - Client v1.0
=================================
初始化電子紙...
連線到 WiFi: your_ssid
.....
WiFi 已連線
IP 位址: 192.168.1.xxx
訊號強度: -XX dBm
連線到伺服器: 192.168.1.100:8266
WebSocket 已連線: ws://192.168.1.100:8266/
初始化完成！
可用記憶體: XXXXX bytes
```

### 階段 5: 整合測試

#### 5.1 文字顯示測試
在 Server 端輸入：
```
> text Hello ESP8266!
```

Client 序列埠應輸出：
```
收到二進位資料: XXX bytes
處理封包: Type=FULL_UPDATE, SeqID=1, Length=XXX
解壓縮中...
解壓縮完成: XX ms
更新顯示中...
顯示更新耗時: XXXX ms
發送 ACK: SeqID=1
```

電子紙應顯示 "Hello ESP8266!"。

#### 5.2 測試圖案測試
在 Server 端輸入：
```
> test
```

電子紙應顯示測試圖案（條紋、網格等）。

#### 5.3 清空螢幕測試
在 Server 端輸入：
```
> clear
```

電子紙應變成全白。

#### 5.4 影像顯示測試
準備一張圖片 (任意尺寸)，在 Server 端輸入：
```
> image path/to/your/image.jpg
```

電子紙應顯示該影像（自動調整大小和轉換為黑白）。

## ✅ 驗收標準

### Server 端
- ✅ 所有模組單元測試通過
- ✅ Server 啟動無錯誤
- ✅ 可以接受 WebSocket 連線
- ✅ 可以處理指令並傳送封包

### Client 端
- ✅ 編譯無錯誤警告
- ✅ WiFi 連線成功
- ✅ WebSocket 連線成功
- ✅ 可以接收並解析封包
- ✅ 文字顯示正確
- ✅ 影像顯示正確
- ✅ 記憶體使用正常 (< 50KB)

### 整合
- ✅ Server 與 Client 可以通訊
- ✅ 封包傳輸無遺失
- ✅ ACK/NAK 機制正常
- ✅ 壓縮率達到預期 (文字 > 85%)
- ✅ 顯示更新速度合理 (< 5 秒)

## 🐛 常見問題

### Server 端
**Q: `ModuleNotFoundError: No module named 'websockets'`**
```bash
pip install -r requirements.txt
```

**Q: Port 8266 already in use**
```bash
# 修改 server.py 中的 PORT 常數
# 或關閉占用該埠的程式
```

### Client 端
**Q: 編譯錯誤 `GxEPD2.h: No such file or directory`**
- 安裝 GxEPD2 函式庫

**Q: WiFi 連線失敗**
- 檢查 SSID 和密碼
- 確認路由器支援 2.4GHz
- 檢查訊號強度

**Q: WebSocket 連線失敗**
- 確認 Server 已啟動
- 檢查 IP 位址是否正確
- 檢查防火牆設定

**Q: 記憶體不足**
- 修改 `config.h` 減少緩衝大小
- 只保留一個畫面緩衝

## 📊 效能測試

### 壓縮率測試
記錄不同類型影像的壓縮率：
- 純文字：應 > 90%
- 圖表/線條：應 > 80%
- 照片：約 60-70%

### 速度測試
記錄端到端時間：
- Server 壓縮時間：< 100ms
- 網路傳輸時間：< 500ms
- Client 解壓時間：< 200ms
- 顯示更新時間：2500-3500ms

### 記憶體測試
監控 Client 記憶體使用：
- 初始化後：應 > 40KB 可用
- 接收封包時：應 > 20KB 可用
- 顯示更新時：應 > 10KB 可用

## 📝 測試記錄範例

```
測試日期: 2024-XX-XX
測試人員: XXX

【Server 端】
✅ RLE 壓縮測試 - 通過
✅ 影像處理測試 - 通過
✅ 協議封包測試 - 通過
✅ Server 啟動 - 通過

【Client 端】
✅ 編譯測試 - 通過 (程式 32KB, 記憶體 35KB)
✅ WiFi 連線 - 通過 (IP: 192.168.1.123, RSSI: -45dBm)
✅ WebSocket 連線 - 通過

【整合測試】
✅ 文字顯示 - 通過 (總耗時 3.2s)
✅ 影像顯示 - 通過 (總耗時 4.1s)
✅ 測試圖案 - 通過
✅ 清空螢幕 - 通過

【效能測試】
- 文字壓縮率: 92.3%
- 影像壓縮率: 65.8%
- 平均更新時間: 3.5s
- 可用記憶體: 42KB

結論: ✅ 所有測試通過
```

## 🔄 下一步

測試通過後，可以：
1. 優化效能（串流處理、局部更新）
2. 新增功能（多頁顯示、動畫、互動）
3. 改進 UI（Server 端 Web 介面）
4. 部署到生產環境

---

**Happy Testing! 🚀**
