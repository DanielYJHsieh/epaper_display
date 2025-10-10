# ESP8266 電子紙顯示器 - 手機整合方案

## 📱 目標
將 **ESP8266 ← WiFi → PC** 改為 **ESP8266 ← WiFi → 手機**

---

## 🔍 目前架構分析

### 現有系統架構
```
[PC Python Server]
    ↓ WebSocket (port 8266)
    ↓ 傳送：二進位影像資料 (3×16KB 條帶)
    ↓ 接收：ACK/NAK, READY 文字訊息
[ESP8266 Client]
    ↓ SPI
[GDEQ0426T82 電子紙 800×480]
```

### 技術細節
- **通訊協議**：WebSocket (Binary + Text)
- **網路模式**：ESP8266 連接到 WiFi AP (Station Mode)
- **Server IP**：固定 IP `192.168.0.43:8266`
- **資料格式**：
  - Binary: `[Header 9bytes][tile_index 1byte][image_data 16000bytes]`
  - Text: `"READY"` (ESP8266 → Server)
- **影像處理**：Server 端（Python + PIL）

---

## 🎯 手機整合方案（4 種）

### 方案 A：手機當 Server（推薦 ⭐⭐⭐⭐⭐）

**架構**：
```
[手機 WebSocket Server App]
    ↓ WiFi (同網段)
    ↓ WebSocket
[ESP8266 Client] (不需修改)
    ↓ SPI
[電子紙顯示器]
```

**優點**：
- ✅ **ESP8266 完全不需修改**（只需改 `config.h` 的 `SERVER_HOST`）
- ✅ 使用現有協議，穩定可靠
- ✅ 手機有強大的影像處理能力
- ✅ 可以離線使用（不依賴 PC）

**缺點**：
- ⚠️ 需要開發手機 App（Android/iOS）
- ⚠️ 手機需要與 ESP8266 在同一個 WiFi 網段

**實作技術**：
- **Android**: Kotlin + Ktor WebSocket Server + Android Canvas
- **iOS**: Swift + Starscream/NWWebSocket + UIKit/SwiftUI
- **跨平台**: Flutter + dart:io WebSocket + image package

**步驟**：
1. 手機 App 啟動 WebSocket Server（監聽 8266 port）
2. 顯示手機 IP 給使用者
3. 修改 ESP8266 的 `config.h`，設定 `SERVER_HOST` 為手機 IP
4. ESP8266 連接手機，App 傳送影像

---

### 方案 B：ESP8266 當 AP (Access Point)（推薦 ⭐⭐⭐⭐）

**架構**：
```
[手機] → WiFi 連接到 → [ESP8266 AP + WebSocket Server]
                           ↓ SPI
                      [電子紙顯示器]
```

**優點**：
- ✅ 不需要外部 WiFi 路由器
- ✅ 手機直接連接 ESP8266，更簡單
- ✅ 可以完全獨立運作（適合展示、攜帶）

**缺點**：
- ⚠️ **需要大幅修改 ESP8266 程式**（Client → Server）
- ⚠️ ESP8266 記憶體有限，WebSocket Server + 影像處理可能吃緊
- ⚠️ 手機需要實作 WebSocket Client + 影像處理

**技術挑戰**：
- ESP8266 記憶體：目前 30KB RAM 已用 38%，加上 Server 可能不夠
- 需要在 ESP8266 上實作 WebSocket Server（使用 ESPAsyncWebServer）
- 影像處理必須在手機完成（轉換為單色、分區）

**修改範圍**：
- ESP8266: Client → AP + Server（大改）
- 手機: 實作 WebSocket Client + 影像處理

---

### 方案 C：手機透過 HTTP/REST API（推薦 ⭐⭐⭐）

**架構**：
```
[手機瀏覽器/App]
    ↓ HTTP POST (傳送影像 URL 或 Base64)
[PC Python Server] (中間層)
    ↓ WebSocket
[ESP8266 Client]
    ↓ SPI
[電子紙顯示器]
```

**優點**：
- ✅ 手機端最簡單（只需要 HTTP request）
- ✅ 可以用網頁界面（PWA）
- ✅ ESP8266 和 Server 完全不需修改

**缺點**：
- ⚠️ 仍然需要 PC Server 運行
- ⚠️ 手機和 PC 都要在同一網段

**實作**：
- 在現有 `server.py` 加入 HTTP API：
  ```python
  from aiohttp import web
  
  async def handle_upload(request):
      data = await request.post()
      image = data['image']
      # 處理影像並傳送到 ESP8266
  ```
- 手機用瀏覽器或 App 上傳圖片

---

### 方案 D：ESP8266 HTTP Server + 手機上傳（推薦 ⭐⭐）

**架構**：
```
[手機]
    ↓ HTTP POST (上傳處理好的影像資料)
[ESP8266 HTTP Server]
    ↓ 直接顯示
[電子紙顯示器]
```

**優點**：
- ✅ 完全去除 PC 依賴
- ✅ 架構簡單

**缺點**：
- ⚠️ **影像處理必須在手機完成**（800×480 → 單色 → 分 3 區）
- ⚠️ 手機端開發複雜度高
- ⚠️ ESP8266 HTTP Server 處理大檔案可能不穩定

---

## 💡 推薦方案比較

| 方案 | ESP8266 修改 | 手機開發 | PC 需求 | 穩定性 | 推薦度 |
|-----|------------|---------|---------|--------|--------|
| **A: 手機當 Server** | ⭐ 極少 | ⭐⭐⭐ 中 | ❌ 不需要 | ⭐⭐⭐⭐⭐ | ⭐⭐⭐⭐⭐ |
| **B: ESP8266 當 AP** | ⭐⭐⭐⭐⭐ 大改 | ⭐⭐⭐⭐ 高 | ❌ 不需要 | ⭐⭐⭐ | ⭐⭐⭐⭐ |
| **C: HTTP API** | ❌ 不需要 | ⭐ 簡單 | ✅ 需要 | ⭐⭐⭐⭐ | ⭐⭐⭐ |
| **D: HTTP Server** | ⭐⭐⭐⭐ 中改 | ⭐⭐⭐⭐⭐ 極高 | ❌ 不需要 | ⭐⭐ | ⭐⭐ |

---

## 🎯 最佳推薦：**方案 A - 手機當 Server**

### 為什麼選擇方案 A？

1. **ESP8266 幾乎不需修改**
   - 只需改 `config.h` 的 IP 位址
   - 現有的 WebSocket Client 完全可用
   - 已經過充分測試，穩定可靠

2. **手機開發相對簡單**
   - 移植現有 Python Server 邏輯
   - 使用成熟的 WebSocket 庫
   - 影像處理庫豐富（Android Canvas, iOS UIKit）

3. **不依賴 PC**
   - 完全獨立運作
   - 適合展示、攜帶

4. **可以漸進式開發**
   - 先做基本功能（顯示預設圖片）
   - 再加相機拍照
   - 最後加圖片編輯

---

## 📋 方案 A 實作計畫

### Phase 1：Android App 開發（推薦先做）

#### 1.1 技術選型
```kotlin
// 使用 Kotlin + Jetpack Compose
dependencies {
    implementation("io.ktor:ktor-server-core:2.3.5")
    implementation("io.ktor:ktor-server-netty:2.3.5")
    implementation("io.ktor:ktor-server-websockets:2.3.5")
    implementation("androidx.compose.ui:ui:1.5.4")
}
```

#### 1.2 核心功能模組
```
App Architecture:
├── WebSocketServer (port 8266)
│   ├── handleConnection() - 管理 ESP8266 連接
│   ├── sendImage() - 發送影像資料
│   └── receiveMessage() - 接收 ACK/READY
├── ImageProcessor
│   ├── resize() - 調整為 800×480
│   ├── dither() - Floyd-Steinberg 抖動
│   └── splitTiles() - 分割為 3×16KB 條帶
└── UI
    ├── 顯示手機 IP
    ├── 選擇圖片
    └── 發送狀態
```

#### 1.3 開發步驟

**Step 1: 建立 WebSocket Server**
```kotlin
fun startWebSocketServer() {
    embeddedServer(Netty, port = 8266) {
        install(WebSockets)
        routing {
            webSocket("/") {
                // 處理 ESP8266 連接
                handleESP8266Connection(this)
            }
        }
    }.start(wait = false)
}
```

**Step 2: 實作影像處理**
```kotlin
fun processImage(bitmap: Bitmap): List<ByteArray> {
    // 1. Resize to 800×480
    val resized = Bitmap.createScaledBitmap(bitmap, 800, 480, true)
    
    // 2. Convert to monochrome with dithering
    val monochrome = applyFloydSteinberg(resized)
    
    // 3. Split into 3 tiles (800×160 each)
    val tiles = listOf(
        extractTile(monochrome, 0, 0, 800, 160),    // 上
        extractTile(monochrome, 0, 160, 800, 160),  // 中
        extractTile(monochrome, 0, 320, 800, 160)   // 下
    )
    
    return tiles
}
```

**Step 3: 實作協議封包**
```kotlin
fun packTilePacket(seqId: Int, tileIndex: Int, data: ByteArray): ByteArray {
    val header = ByteBuffer.allocate(9).apply {
        put(0xAA.toByte())        // Magic
        put(0x55.toByte())        // Magic
        put(0x02.toByte())        // Type: TILE_UPDATE
        putShort(seqId.toShort()) // SeqID
        putInt(data.size + 1)     // Length (含 tile_index)
    }
    
    return header.array() + tileIndex.toByte() + data
}
```

**Step 4: UI 設計**
```kotlin
@Composable
fun MainScreen() {
    Column {
        Text("ESP8266 Display Server")
        Text("IP: ${getLocalIPAddress()}")
        Text("Port: 8266")
        
        Button(onClick = { selectImage() }) {
            Text("選擇圖片")
        }
        
        if (connected) {
            Text("ESP8266 已連接 ✓")
            Button(onClick = { sendCurrentImage() }) {
                Text("傳送到顯示器")
            }
        }
    }
}
```

#### 1.4 ESP8266 配置修改

**只需修改 `config.h`**：
```cpp
#define SERVER_HOST "192.168.0.100"  // 改為手機 IP
#define SERVER_PORT 8266             // 保持不變
```

**如何取得手機 IP**：
- App 自動偵測並顯示在畫面上
- 使用者在 ESP8266 端設定

---

### Phase 2：進階功能

#### 2.1 相機整合
```kotlin
// 拍照後直接處理並發送
fun captureAndSend() {
    cameraController.takePicture { bitmap ->
        val tiles = processImage(bitmap)
        sendToESP8266(tiles)
    }
}
```

#### 2.2 圖片編輯
- 裁切、旋轉
- 濾鏡（黑白、對比度）
- 文字疊加

#### 2.3 圖庫管理
- 儲存常用圖片
- 一鍵發送
- 歷史記錄

#### 2.4 WiFi 配置簡化
- 內建 mDNS/Bonjour 自動發現 ESP8266
- 或使用 QR Code 配置

---

## 🔧 技術細節

### WebSocket 協議（保持不變）

**Binary Frame (Server → ESP8266)**：
```
[0xAA 0x55] [Type:1] [SeqID:2] [Length:4] [tile_index:1] [data:16000]
```

**Text Frame (ESP8266 → Server)**：
```
"READY"
```

### 影像處理流程

```
[原始圖片] 
  ↓ resize to 800×480
[調整後圖片]
  ↓ Floyd-Steinberg dithering
[單色圖片]
  ↓ split into 3 tiles
[Tile 0: 800×160 (0-159)]
[Tile 1: 800×160 (160-319)]
[Tile 2: 800×160 (320-479)]
  ↓ convert to packed bytes
[3 × 16KB binary data]
  ↓ WebSocket send
[ESP8266]
```

---

## 📱 使用情境

### 情境 1：離線展示
```
手機 (開啟熱點) ← ESP8266 連接
手機 App 傳送圖片
電子紙顯示
```

### 情境 2：家庭網路
```
家用 WiFi AP
  ├── 手機 (192.168.0.100)
  └── ESP8266 (連接到手機 Server)
```

### 情境 3：拍照即顯示
```
手機相機 → 拍照 → App 處理 → WebSocket → ESP8266 → 顯示
(即時預覽，約 1 分鐘內顯示)
```

---

## 🚀 快速開始（方案 A）

### 開發優先順序

1. **Week 1-2**: Android App 基礎
   - WebSocket Server
   - 基本影像處理
   - UI 框架

2. **Week 3**: 協議整合
   - 實作封包格式
   - 測試與 ESP8266 通訊

3. **Week 4**: 影像處理優化
   - Floyd-Steinberg 演算法
   - 分區切割

4. **Week 5+**: 進階功能
   - 相機、圖庫
   - UI/UX 優化

### 測試計畫

**Phase 1: 基本通訊**
- [ ] 手機 WebSocket Server 啟動成功
- [ ] ESP8266 成功連接
- [ ] 收到 ESP8266 的文字訊息

**Phase 2: 影像傳輸**
- [ ] 傳送靜態測試圖片（單色棋盤格）
- [ ] ESP8266 正確顯示

**Phase 3: 完整流程**
- [ ] 從手機相簿選擇圖片
- [ ] 影像處理（resize + dither）
- [ ] 分 3 個條帶傳送
- [ ] 接收 READY 訊號
- [ ] 全部顯示正確

---

## 📚 相關資源

### Android 開發
- [Ktor WebSocket Server](https://ktor.io/docs/websocket.html)
- [Android Canvas Drawing](https://developer.android.com/develop/ui/views/graphics/drawables)
- [Floyd-Steinberg Dithering Algorithm](https://en.wikipedia.org/wiki/Floyd%E2%80%93Steinberg_dithering)

### ESP8266
- 現有 `client_esp8266.ino` - 無需修改
- 只需修改 `config.h` 設定檔

### 協議文檔
- `protocol.h` - ESP8266 端協議定義
- `protocol.py` - Python 端協議實作（參考用）

---

## ❓ FAQ

**Q1: 手機 App 可以同時當 Server 並使用其他 App 嗎？**
A: 可以！WebSocket Server 在背景運行，可以切換到其他 App。

**Q2: 需要 Root 或特殊權限嗎？**
A: 不需要。標準 Android/iOS 權限即可（網路、相機、儲存）。

**Q3: 耗電嗎？**
A: WebSocket Server 待機耗電很低，傳輸時才會用到網路。

**Q4: 可以多台 ESP8266 嗎？**
A: 可以！Server 支援多個 WebSocket 連接。

**Q5: iOS 也能做嗎？**
A: 可以！使用 Swift + NWWebSocket 或 Starscream。

---

## 🎯 結論

**最佳方案：方案 A - 手機當 WebSocket Server**

- ✅ ESP8266 修改最少（只改 IP）
- ✅ 穩定性最高（複用現有協議）
- ✅ 開發成本合理
- ✅ 完全獨立運作
- ✅ 可擴展性強

**建議從 Android 開始開發**（市場佔有率高、開發工具完整）

需要我協助：
1. 提供 Android App 完整程式碼框架？
2. 詳細的協議實作範例？
3. 影像處理演算法實作？
4. ESP8266 配置修改指南？

請告訴我你想先從哪裡開始！🚀
