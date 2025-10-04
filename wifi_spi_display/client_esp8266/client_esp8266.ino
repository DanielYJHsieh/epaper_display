/*
 * WiFi SPI Display - ESP8266 客戶端
 * 
 * 透過 WiFi 接收影像並顯示在電子紙上
 * 支援 RLE 壓縮與串流處理
 */

#include <ESP8266WiFi.h>
#include <WebSocketsClient.h>
#include <GxEPD2_BW.h>
// FreeSansBold12pt7b 已移除以節省記憶體

#include "config.h"
#include "protocol.h"
#include "rle_decoder.h"
#include "GxEPD2_display_selection_GDEQ0426T82.h"

// 確保關鍵常數已定義（後備定義）
#ifndef DISPLAY_BUFFER_SIZE
#define DISPLAY_BUFFER_SIZE 48000
#endif

#ifndef MEMORY_WARNING_THRESHOLD
#define MEMORY_WARNING_THRESHOLD 10000
#endif

// ============================================
// 全域變數
// ============================================
WebSocketsClient webSocket;
PacketReceiver packetReceiver;

uint8_t* currentFrame = nullptr;  // 當前畫面緩衝
// previousFrame 已移除以節省記憶體 - 不支援 Delta 更新
bool frameAllocated = false;

uint16_t currentSeqId = 0;
unsigned long lastMemoryCheck = 0;
unsigned long lastReconnectAttempt = 0;

// ============================================
// 顯示回調函數（串流處理用）
// ============================================
void displayCallback(const uint8_t* data, size_t size) {
  // 這裡可以直接寫入到顯示緩衝
  // 目前版本先不用，保留給未來優化
}

// ============================================
// WebSocket 事件處理
// ============================================
void webSocketEvent(WStype_t type, uint8_t* payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED:
      Serial.println(F("WebSocket 已斷線"));
      break;
      
    case WStype_CONNECTED:
      {
        Serial.print(F("WebSocket 已連線: "));
        Serial.println((char*)payload);
        
        // 發送歡迎訊息
        webSocket.sendTXT("ESP8266 Client Ready");
      }
      break;
      
    case WStype_BIN:
      {
        Serial.print(F("收到二進位資料: "));
        Serial.print(length);
        Serial.println(F(" bytes"));
        
        // 處理封包
        if (packetReceiver.process(payload, length)) {
          handlePacket();
          packetReceiver.reset();
        }
      }
      break;
      
    case WStype_TEXT:
      {
        Serial.print(F("收到文字訊息: "));
        Serial.println((char*)payload);
      }
      break;
      
    case WStype_ERROR:
      Serial.println(F("WebSocket 錯誤"));
      break;
      
    default:
      break;
  }
}

// ============================================
// 處理完整封包
// ============================================
void handlePacket() {
  const PacketHeader& header = packetReceiver.getHeader();
  const uint8_t* payload = packetReceiver.getPayload();
  
  Serial.print(F("處理封包: Type="));
  Serial.print(ProtocolParser::getTypeName(header.type));
  Serial.print(F(", SeqID="));
  Serial.print(header.seq_id);
  Serial.print(F(", Length="));
  Serial.println(header.length);
  
  currentSeqId = header.seq_id;
  
  switch (header.type) {
    case PROTO_TYPE_FULL:
      handleFullUpdate(payload, header.length);
      break;
      
    case PROTO_TYPE_DELTA:
      handleDeltaUpdate(payload, header.length);
      break;
      
    case PROTO_TYPE_CMD:
      handleCommand(payload, header.length);
      break;
      
    default:
      Serial.println(F("未知的封包類型！"));
      sendNAK(header.seq_id);
      break;
  }
}

// ============================================
// 處理完整畫面更新
// ============================================
void handleFullUpdate(const uint8_t* payload, uint32_t length) {
  Serial.println(F("處理完整更新..."));
  
  unsigned long startTime = millis();
  
  // 記憶體應該已經在 setup() 中分配了
  if (!frameAllocated || !currentFrame) {
    Serial.println(F("*** 錯誤：顯示緩衝未分配！***"));
    sendNAK(currentSeqId);
    return;
  }
  
  // 解壓縮
  Serial.println(F("解壓縮中..."));
  int decompressedSize = RLEDecoder::decode(payload, length, currentFrame, DISPLAY_BUFFER_SIZE);
  
  if (decompressedSize != DISPLAY_BUFFER_SIZE) {
    Serial.print(F("解壓縮失敗！期望 "));
    Serial.print(DISPLAY_BUFFER_SIZE);
    Serial.print(F(" bytes, 得到 "));
    Serial.println(decompressedSize);
    sendNAK(currentSeqId);
    return;
  }
  
  Serial.print(F("解壓縮完成: "));
  Serial.print(millis() - startTime);
  Serial.println(F(" ms"));
  
  // 顯示
  Serial.println(F("更新顯示中..."));
  displayFrame(currentFrame);
  
  // 不再儲存 previousFrame 以節省記憶體
  
  Serial.print(F("總耗時: "));
  Serial.print(millis() - startTime);
  Serial.println(F(" ms"));
  
  // 發送 ACK
  sendACK(currentSeqId);
}

// ============================================
// 處理差分更新
// ============================================
void handleDeltaUpdate(const uint8_t* payload, uint32_t length) {
  Serial.println(F("❌ Delta 更新已禁用（記憶體不足）"));
  sendNAK(currentSeqId);
}

// ============================================
// 處理控制指令
// ============================================
void handleCommand(const uint8_t* payload, uint32_t length) {
  if (length < 1) {
    sendNAK(currentSeqId);
    return;
  }
  
  uint8_t cmd = payload[0];
  Serial.print(F("處理指令: 0x"));
  Serial.println(cmd, HEX);
  
  switch (cmd) {
    case CMD_CLEAR_SCREEN:
      Serial.println(F("清空螢幕"));
      display.clearScreen();
      
      // 重置緩衝
      if (frameAllocated) {
        memset(currentFrame, 0xFF, DISPLAY_BUFFER_SIZE);
      }
      break;
      
    case CMD_SLEEP:
      Serial.println(F("進入休眠"));
      display.hibernate();
      break;
      
    case CMD_WAKE:
      Serial.println(F("喚醒顯示"));
      display.init(SERIAL_BAUD);
      break;
      
    default:
      Serial.println(F("未知指令"));
      sendNAK(currentSeqId);
      return;
  }
  
  sendACK(currentSeqId);
}

// ============================================
// 顯示畫面
// ============================================
void displayFrame(const uint8_t* frame) {
  unsigned long startTime = millis();
  
  // 延遲初始化：第一次呼叫時才初始化顯示器
  static bool displayInitialized = false;
  if (!displayInitialized) {
    Serial.println(F("*** 首次使用，初始化電子紙... ***"));
    display.init(SERIAL_BAUD);
    displayInitialized = true;
    Serial.print(F("*** 初始化後可用記憶體: "));
    Serial.print(ESP.getFreeHeap());
    Serial.println(F(" bytes ***"));
  }
  
  // 使用 writeImage 直接寫入，不需要 firstPage/nextPage
  // 這樣可以避免 GxEPD2 的內部緩衝分配
  display.setFullWindow();
  display.writeImage(frame, 0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, false, false, true);
  display.refresh();
  
  Serial.print(F("顯示更新耗時: "));
  Serial.print(millis() - startTime);
  Serial.println(F(" ms"));
}

// ============================================
// 發送 ACK
// ============================================
void sendACK(uint16_t seq_id) {
  uint8_t buffer[16];
  size_t size = ProtocolParser::packACK(seq_id, buffer, 1);
  webSocket.sendBIN(buffer, size);
  
  Serial.print(F("發送 ACK: SeqID="));
  Serial.println(seq_id);
}

// ============================================
// 發送 NAK
// ============================================
void sendNAK(uint16_t seq_id) {
  uint8_t buffer[16];
  size_t size = ProtocolParser::packNAK(seq_id, buffer);
  webSocket.sendBIN(buffer, size);
  
  Serial.print(F("發送 NAK: SeqID="));
  Serial.println(seq_id);
}

// ============================================
// 連線到 WiFi
// ============================================
void connectWiFi() {
  Serial.println();
  Serial.print(F("連線到 WiFi: "));
  Serial.println(WIFI_SSID);
  
  WiFi.mode(WIFI_STA);
  
#ifdef USE_802_11N
  WiFi.setPhyMode(WIFI_PHY_MODE_11N);
#endif
  
#ifdef WIFI_TX_POWER
  WiFi.setOutputPower(WIFI_TX_POWER);
#endif
  
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println();
    Serial.println(F("WiFi 已連線"));
    Serial.print(F("IP 位址: "));
    Serial.println(WiFi.localIP());
    Serial.print(F("訊號強度: "));
    Serial.print(WiFi.RSSI());
    Serial.println(F(" dBm"));
  }
  else {
    Serial.println();
    Serial.println(F("WiFi 連線失敗！"));
  }
}

// ============================================
// 記憶體監控
// ============================================
void checkMemory() {
  unsigned long now = millis();
  
  if (now - lastMemoryCheck >= MEMORY_CHECK_INTERVAL) {
    lastMemoryCheck = now;
    
    uint32_t freeHeap = ESP.getFreeHeap();
    
    Serial.print(F("可用記憶體: "));
    Serial.print(freeHeap);
    Serial.println(F(" bytes"));
    
    if (freeHeap < MEMORY_WARNING_THRESHOLD) {
      Serial.println(F("*** 警告：記憶體不足！***"));
    }
  }
}

// ============================================
// Setup
// ============================================
void setup() {
  Serial.begin(SERIAL_BAUD);
  delay(1000);  // 等待序列埠穩定
  Serial.println();
  Serial.println(F("================================="));
  Serial.println(F("WiFi SPI Display - Client v1.0"));
  Serial.println(F("=== 修改版 2024-10-04 ==="));  // 版本標記
  Serial.println(F("================================="));
  
  // *** 優先分配記憶體（在記憶體最充足時） ***
  Serial.print(F("*** [1/5] 啟動時可用記憶體: "));
  Serial.print(ESP.getFreeHeap());
  Serial.println(F(" bytes ***"));
  
  Serial.println(F("*** [2/5] 預先分配 48KB 顯示緩衝... ***"));
  currentFrame = (uint8_t*)malloc(DISPLAY_BUFFER_SIZE);
  
  if (!currentFrame) {
    Serial.println(F("*** 嚴重錯誤：無法分配記憶體！***"));
    Serial.print(F("需要: "));
    Serial.print(DISPLAY_BUFFER_SIZE);
    Serial.println(F(" bytes"));
    while (1) delay(1000);  // 停止執行
  }
  
  frameAllocated = true;
  Serial.println(F("*** ✓ 記憶體分配成功！***"));
  Serial.print(F("*** 分配後可用記憶體: "));
  Serial.print(ESP.getFreeHeap());
  Serial.println(F(" bytes ***"));
  
  // *** 跳過 display.init() - 延遲到第一次顯示時再初始化 ***
  // display.init() 會消耗大量記憶體，我們在 displayFrame() 第一次呼叫時才初始化
  
  // 連線 WiFi
  Serial.println(F("*** [3/5] 連線 WiFi... ***"));
  connectWiFi();
  
  if (WiFi.status() != WL_CONNECTED) {
    // 跳過顯示錯誤訊息以節省記憶體
    Serial.println(F("無法連線 WiFi，停止執行"));
    while (1) delay(1000);
  }
  
  // 設定 WebSocket
  Serial.println(F("*** [4/5] 設定 WebSocket... ***"));
  Serial.print(F("連線到伺服器: "));
  Serial.print(SERVER_HOST);
  Serial.print(":");
  Serial.println(SERVER_PORT);
  
  webSocket.begin(SERVER_HOST, SERVER_PORT, "/");
  webSocket.onEvent(webSocketEvent);
  webSocket.setReconnectInterval(5000);
  
  // 跳過顯示準備畫面以節省記憶體
  // display.firstPage() 會分配 48KB 緩衝，導致記憶體不足
  
  Serial.println(F("初始化完成！"));
  Serial.print(F("可用記憶體: "));
  Serial.print(ESP.getFreeHeap());
  Serial.println(F(" bytes"));
}

// ============================================
// Loop
// ============================================
void loop() {
  // 處理 WebSocket
  webSocket.loop();
  
  // 記憶體監控
  checkMemory();
  
  // 檢查 WiFi 連線
  if (WiFi.status() != WL_CONNECTED) {
    unsigned long now = millis();
    if (now - lastReconnectAttempt >= 10000) {
      lastReconnectAttempt = now;
      Serial.println(F("嘗試重新連線 WiFi..."));
      connectWiFi();
    }
  }
  
  yield();
}
