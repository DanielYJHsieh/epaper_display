# WiFi SPI Display 專案

**高速無線電子紙顯示系統**

[![Version](https://img.shields.io/badge/version-1.0-blue.svg)](https://github.com/DanielYJHsieh/epaper_display)
[![Status](https://img.shields.io/badge/status-planning-yellow.svg)]()
[![License](https://img.shields.io/badge/license-MIT-green.svg)]()

---

## 🎯 專案簡介

這是一個創新的 **反向架構** WiFi 顯示系統：
- **Server 端** (PC/手機)：負責圖像處理、壓縮、傳輸
- **Client 端** (ESP8266)：專注於接收、解壓、顯示

這樣的設計讓 ESP8266 記憶體使用最小化，運算負擔最輕，達到最佳效能。

---

## ✨ 核心特色

### 🔄 反向架構
```
傳統架構:  手機/PC → 推送 → ESP8266 (Server) ❌ ESP8266 負擔重
新架構:    手機/PC (Server) ← 拉取 ← ESP8266 (Client) ✅ ESP8266 負擔輕
```

### 🚀 高速傳輸
- **WebSocket 二進位模式**: 最快的全雙工通訊
- **RLE 壓縮**: 85-95% 壓縮率（文字）
- **Delta 編碼**: 只傳變化的部分
- **802.11n**: 高速 WiFi 模式

### 💾 低記憶體設計
- **流式處理**: 不儲存完整畫面（節省 48KB）
- **小緩衝**: 僅 512 bytes 接收緩衝
- **即時解壓**: 邊接收邊解壓邊顯示
- **總使用**: < 50KB RAM (安全範圍)

### 🖼️ 大螢幕支援
- **800×480** GDEQ0426T82 電子紙
- **48KB** 黑白位圖
- **2-4 秒** 完整更新
- **< 1 秒** 局部更新

---

## 📁 專案結構

```
wifi_spi_display/
├── PROJECT_PLAN.md          # 📋 完整開發規劃（必讀！）
├── README.md                # 📖 本檔案
│
├── server/                  # 🖥️ Server 端 (Python)
│   ├── README.md           # Server 端說明
│   ├── requirements.txt    # Python 依賴
│   ├── server.py          # 主程式
│   ├── image_processor.py # 圖像處理
│   ├── compressor.py      # 壓縮模組
│   └── protocol.py        # 協議處理
│
└── client_esp8266/         # 📟 Client 端 (ESP8266)
    ├── README.md           # Client 端說明
    ├── wifi_spi_display.ino  # Arduino 主程式
    ├── config.h            # 設定檔
    ├── protocol.h          # 協議定義
    ├── rle_decoder.h       # RLE 解壓縮
    └── display_driver.h    # 顯示驅動
```

---

## 🚀 快速開始

### 1. 閱讀規劃文件 ⭐ **重要**

```bash
請先詳細閱讀 PROJECT_PLAN.md
```

這份文件包含：
- 完整的系統架構設計
- 技術選型與比較分析
- 通訊協議詳細設計
- 記憶體優化策略
- Server/Client 實作細節
- 開發流程與時程
- 測試計畫
- 風險評估

### 2. Server 端設定

```bash
cd server/
pip install -r requirements.txt
python server.py
```

詳見: [server/README.md](server/README.md)

### 3. Client 端設定

1. 開啟 Arduino IDE
2. 載入 `client_esp8266/wifi_spi_display.ino`
3. 修改 `config.h` 設定
4. 編譯並上傳到 ESP8266

詳見: [client_esp8266/README.md](client_esp8266/README.md)

---

## 📊 系統架構圖

```
┌───────────────────────────┐
│   Server 端 (PC/手機)      │
│                           │
│  ┌─────────────────────┐ │
│  │  圖像處理模組        │ │
│  │  - 縮放、轉黑白      │ │
│  │  - 抖動演算法        │ │
│  └─────────────────────┘ │
│           ↓               │
│  ┌─────────────────────┐ │
│  │  壓縮模組           │ │
│  │  - RLE 壓縮         │ │
│  │  - Delta 編碼       │ │
│  └─────────────────────┘ │
│           ↓               │
│  ┌─────────────────────┐ │
│  │  WebSocket Server   │ │
│  │  Port 8266          │ │
│  └─────────────────────┘ │
└───────────┬───────────────┘
            │
        WiFi 網路
        (802.11n)
            │
┌───────────┴───────────────┐
│   Client 端 (ESP8266)      │
│                           │
│  ┌─────────────────────┐ │
│  │  WebSocket Client   │ │
│  │  接收二進位資料      │ │
│  └─────────────────────┘ │
│           ↓               │
│  ┌─────────────────────┐ │
│  │  流式解壓縮          │ │
│  │  512B 小緩衝        │ │
│  └─────────────────────┘ │
│           ↓               │
│  ┌─────────────────────┐ │
│  │  SPI 顯示控制       │ │
│  │  GDEQ0426T82        │ │
│  └─────────────────────┘ │
└───────────────────────────┘
```

---

## 🔧 技術規格

### 通訊協議

**WebSocket 二進位封包格式**:
```
[Header][Type][SeqID][Length][Payload]
  1B     1B     2B      4B     Variable
```

**封包類型**:
- `0x01`: 完整畫面更新（RLE 壓縮）
- `0x02`: 差分更新（Delta + RLE）
- `0x03`: 控制指令

### 壓縮方案

**RLE (Run-Length Encoding)**:
```
原始: [0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00]
壓縮: [0x04, 0xFF, 0x02, 0x00]
壓縮率: 66.7%
```

**適合場景**:
- 文字顯示: 85-95% 壓縮
- 圖片: 40-60% 壓縮

### 記憶體使用

**ESP8266 RAM 分配**:
```
系統基礎:     25KB
WiFi Stack:    3KB
WebSocket:     8KB
接收緩衝:      4KB
顯示控制:      1KB
───────────────────
總計:         41KB
剩餘:         39KB ✅
```

---

## 📈 效能指標

### 目標效能

| 指標 | 目標值 | 備註 |
|------|--------|------|
| 完整更新時間 | < 4 秒 | 包含傳輸+顯示 |
| 差分更新時間 | < 1 秒 | 只更新變化部分 |
| 壓縮率 (文字) | > 85% | RLE 壓縮 |
| RAM 使用 | < 50KB | 安全範圍 |
| WiFi 速度 | 802.11n | 150 Mbps |

### 測試結果

**預期效能** (待實測):
```
圖像處理:    ~50ms  (Server)
RLE 壓縮:    ~5ms   (Server)
傳輸 5KB:    ~500ms (WiFi)
RLE 解壓:    ~50ms  (ESP8266)
電子紙刷新:  ~3000ms (硬體限制)
────────────────────────────
總計:        ~3.6s  ✅
```

---

## 📅 開發時程

```
第 1-2 週: 基礎架構
  ✅ WebSocket Server/Client
  ✅ 基本通訊測試

第 3-4 週: 圖像傳輸
  ⏳ 圖像處理流程
  ⏳ 壓縮/解壓實作
  ⏳ 完整畫面顯示

第 5-6 週: 優化功能
  ⏳ 差分更新
  ⏳ 效能優化
  ⏳ 網頁介面

第 7-8 週: 測試完善
  ⏳ 整合測試
  ⏳ 穩定性測試
  ⏳ 文件完善
```

---

## 🎓 學習資源

### 必讀文件
1. [PROJECT_PLAN.md](PROJECT_PLAN.md) - 完整開發規劃 ⭐
2. [server/README.md](server/README.md) - Server 端說明
3. [client_esp8266/README.md](client_esp8266/README.md) - Client 端說明

### 技術參考
- [WebSocket Protocol RFC 6455](https://tools.ietf.org/html/rfc6455)
- [ESP8266 Arduino Core](https://github.com/esp8266/Arduino)
- [GxEPD2 Library](https://github.com/ZinggJM/GxEPD2)
- [arduinoWebSockets](https://github.com/Links2004/arduinoWebSockets)

---

## ❓ 常見問題

### Q1: 為什麼使用反向架構？
**A**: 傳統的 ESP8266 當 Server 會有記憶體壓力，反向架構讓強大的 PC/手機做運算，ESP8266 只負責顯示，記憶體使用最小化。

### Q2: 為什麼選 WebSocket 而不是 HTTP？
**A**: WebSocket 是全雙工、持久連接，傳輸速度快，支援二進位模式，非常適合即時資料傳輸。HTTP 每次都要建立連線，太慢。

### Q3: RLE 壓縮夠用嗎？
**A**: 對於電子紙顯示（大量相同顏色區域），RLE 壓縮率很高（85-95%），且實作簡單、解壓快、不佔記憶體。

### Q4: 可以支援彩色電子紙嗎？
**A**: 可以！只需調整 `image_processor.py` 的色彩處理邏輯，協議和傳輸部分不需改動。

### Q5: 可以同時連接多個 ESP8266 嗎？
**A**: 可以！Server 支援多客戶端，可以同時控制多個顯示器。

---

## 🤝 貢獻指南

### 開發流程
1. Fork 專案
2. 建立功能分支 (`git checkout -b feature/AmazingFeature`)
3. 提交變更 (`git commit -m 'Add some AmazingFeature'`)
4. 推送到分支 (`git push origin feature/AmazingFeature`)
5. 開啟 Pull Request

### 程式碼規範
- **Python**: PEP 8
- **C++**: Google C++ Style Guide
- **註解**: 中文 + 英文
- **提交訊息**: 使用 Conventional Commits

---

## 📄 授權

本專案採用 MIT License - 詳見 [LICENSE](LICENSE) 檔案

---

## 👤 作者

**Daniel YJ Hsieh**
- GitHub: [@DanielYJHsieh](https://github.com/DanielYJHsieh)

---

## 🙏 致謝

感謝以下開源專案：
- [GxEPD2](https://github.com/ZinggJM/GxEPD2) - 電子紙驅動
- [arduinoWebSockets](https://github.com/Links2004/arduinoWebSockets) - WebSocket 實作
- [Pillow](https://github.com/python-pillow/Pillow) - 圖像處理

---

## 📞 聯絡方式

有任何問題或建議，歡迎：
- 開啟 [Issue](https://github.com/DanielYJHsieh/epaper_display/issues)
- 發送 Pull Request
- 聯絡作者

---

**版本**: 1.0  
**狀態**: 📋 規劃階段 - 等待審核確認  
**建立日期**: 2025-10-04  
**最後更新**: 2025-10-04

---

## ✅ 下一步

1. **審核規劃文件** - 仔細閱讀 `PROJECT_PLAN.md`
2. **提出問題或建議** - 如有疑問請提出
3. **確認後開始開發** - 確認無誤後進入實作階段

**請在確認規劃完整性後，我們再開始開發軟體！** 🚀
