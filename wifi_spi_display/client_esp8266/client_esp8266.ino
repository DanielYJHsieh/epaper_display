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

// 分區顯示專用緩衝區（預先配置）
#if ENABLE_TILE_DISPLAY
uint8_t* tileBuffer = nullptr;     // 分區緩衝 (12KB)
bool tileBufferAllocated = false;
#endif

#if ENABLE_CHUNKED_DISPLAY
uint8_t* currentChunk = nullptr;  // 當前塊緩衝 (6KB)
uint8_t* fullFrame = nullptr;     // 完整畫面緩衝 (僅在需要時分配)
uint8_t currentChunkIndex = 0;    // 當前處理的塊索引
bool useChunkedMode = true;       // 分塊模式開關
#else
uint8_t* currentFrame = nullptr;  // 當前畫面緩衝 (48KB)
#endif

bool frameAllocated = false;

uint16_t currentSeqId = 0;
unsigned long lastMemoryCheck = 0;
unsigned long lastReconnectAttempt = 0;

// ============================================
// 函數前向宣告
// ============================================
#if ENABLE_CHUNKED_DISPLAY
void handleFullUpdateChunked(const uint8_t* payload, uint32_t length);
void displayFrameChunked(const uint8_t* frame);
#endif
uint8_t* allocateDisplayBuffer(const char* purpose);
void safeFreeMem(uint8_t** buffer, const char* purpose);

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
        
        // 在處理封包前，設置使用 tileBuffer 作為外部緩衝區（如果已配置）
        #if ENABLE_TILE_DISPLAY
        if (tileBufferAllocated && tileBuffer) {
          packetReceiver.setExternalBuffer(tileBuffer, TILE_BUFFER_SIZE);
        }
        #endif
        
        // 處理封包
        if (packetReceiver.process(payload, length)) {
          handlePacket();
          
          // 注意：handleTileUpdate 內部會提前 reset，這裡檢查是否已 reset
          // 如果還沒 reset，則 reset（適用於非 TILE 類型的封包）
          if (packetReceiver.getPayload() != nullptr) {
            packetReceiver.reset();
          }
          
          // 清除外部緩衝區設置
          #if ENABLE_TILE_DISPLAY
          packetReceiver.clearExternalBuffer();
          #endif
          
          // 立即觸發記憶體整理
          yield();
          
          // 顯示 reset 後的記憶體狀況
          Serial.print(F("📊 Packet處理完成後記憶體: 可用="));
          Serial.print(ESP.getFreeHeap());
          Serial.print(F(" bytes, 最大塊="));
          Serial.print(ESP.getMaxFreeBlockSize());
          Serial.println(F(" bytes"));
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
      
    case PROTO_TYPE_TILE:
      handleTileUpdate((uint8_t*)payload, header.length, header.seq_id);
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
// 處理完整畫面更新（優化版 - 支援動態記憶體）
// ============================================
void handleFullUpdate(const uint8_t* payload, uint32_t length) {
  Serial.println(F("處理完整更新..."));
  
  unsigned long startTime = millis();
  
#if ENABLE_CHUNKED_DISPLAY
  // 動態分配記憶體模式
  uint8_t* targetBuffer = nullptr;
  bool usePureChunkMode = false;  // 純分塊模式標記
  
  if (useChunkedMode && ENABLE_DYNAMIC_ALLOCATION) {
    // 使用輔助函數動態分配記憶體
    targetBuffer = allocateDisplayBuffer("完整更新");
    
    if (!targetBuffer) {
      Serial.println(F("*** 切換到純分塊處理模式 ***"));
      usePureChunkMode = true;
    }
  } else if (fullFrame) {
    targetBuffer = fullFrame;
  } else {
    Serial.println(F("*** 錯誤：沒有可用的顯示緩衝！***"));
    sendNAK(currentSeqId);
    return;
  }
  
  // 如果是純分塊模式，直接處理分塊解壓縮和顯示
  if (usePureChunkMode) {
    handleFullUpdateChunked(payload, length);
    sendACK(currentSeqId);
    Serial.print(F("總耗時: "));
    Serial.print(millis() - startTime);
    Serial.println(F(" ms"));
    return;
  }
#else
  // 傳統模式
  if (!frameAllocated || !currentFrame) {
    Serial.println(F("*** 錯誤：顯示緩衝未分配！***"));
    sendNAK(currentSeqId);
    return;
  }
  uint8_t* targetBuffer = currentFrame;
#endif
  
  // 智能判斷：如果資料長度等於緩衝區大小，表示未壓縮
  bool isCompressed = (length != DISPLAY_BUFFER_SIZE);
  
  if (isCompressed) {
    // 資料已壓縮，需要解壓縮
    Serial.println(F("解壓縮中..."));
    int decompressedSize = RLEDecoder::decode(payload, length, targetBuffer, DISPLAY_BUFFER_SIZE);
    
    if (decompressedSize != DISPLAY_BUFFER_SIZE) {
      Serial.print(F("解壓縮失敗！期望 "));
      Serial.print(DISPLAY_BUFFER_SIZE);
      Serial.print(F(" bytes, 得到 "));
      Serial.println(decompressedSize);
      
#if ENABLE_CHUNKED_DISPLAY
      // 使用安全釋放函數
      if (useChunkedMode && ENABLE_DYNAMIC_ALLOCATION && targetBuffer) {
        safeFreeMem(&targetBuffer, "解壓縮失敗");
      }
#endif
      sendNAK(currentSeqId);
      return;
    }
    
    Serial.print(F("解壓縮完成: "));
  } else {
    // 資料未壓縮，直接使用
    Serial.println(F("資料未壓縮，直接複製..."));
    memcpy(targetBuffer, payload, length);
    Serial.print(F("複製完成: "));
  }
  
  Serial.print(millis() - startTime);
  Serial.println(F(" ms"));
  
  // 顯示
  Serial.println(F("更新顯示中..."));
  displayFrame(targetBuffer);
  
#if ENABLE_CHUNKED_DISPLAY
  // 使用安全釋放函數
  if (useChunkedMode && ENABLE_DYNAMIC_ALLOCATION && targetBuffer) {
    safeFreeMem(&targetBuffer, "顯示完成");
  }
#endif
  
  Serial.print(F("總耗時: "));
  Serial.print(millis() - startTime);
  Serial.println(F(" ms"));
  
  // 發送 ACK
  sendACK(currentSeqId);
}

#if ENABLE_CHUNKED_DISPLAY
// ============================================
// 純分塊處理完整更新（改進版 - 多次重試）
// ============================================
void handleFullUpdateChunked(const uint8_t* payload, uint32_t length) {
  Serial.println(F("*** 使用純分塊處理模式 ***"));
  unsigned long startTime = millis();
  
  // 策略：多次嘗試分配，給系統時間釋放記憶體
  uint8_t* tempBuffer = nullptr;
  const int MAX_RETRY = 3;
  
  for (int retry = 0; retry < MAX_RETRY; retry++) {
    if (retry > 0) {
      Serial.print(F("*** 重試 "));
      Serial.print(retry);
      Serial.println(F(" 次 ***"));
      
      // 強制執行看門狗和記憶體清理
      yield();
      delay(10);  // 給系統一點時間
    }
    
    uint32_t freeHeap = ESP.getFreeHeap();
    uint32_t maxBlock = ESP.getMaxFreeBlockSize();
    
    Serial.print(F("嘗試 "));
    Serial.print(retry + 1);
    Serial.print(F("/"));
    Serial.print(MAX_RETRY);
    Serial.print(F(" - 可用: "));
    Serial.print(freeHeap);
    Serial.print(F(" bytes, 最大塊: "));
    Serial.print(maxBlock);
    Serial.println(F(" bytes"));
    
    if (maxBlock >= DISPLAY_BUFFER_SIZE) {
      tempBuffer = (uint8_t*)malloc(DISPLAY_BUFFER_SIZE);
      
      if (tempBuffer) {
        Serial.println(F("*** 緩衝區分配成功！***"));
        break;
      } else {
        Serial.println(F("*** 分配失敗，但最大塊足夠？記憶體碎片問題 ***"));
      }
    } else {
      Serial.print(F("*** 最大塊不足 (需要 "));
      Serial.print(DISPLAY_BUFFER_SIZE);
      Serial.println(F(" bytes) ***"));
    }
  }
  
  if (!tempBuffer) {
    Serial.println(F("*** 多次嘗試後仍無法分配，採用降級顯示 ***"));
    Serial.print(F("最終狀態 - 可用: "));
    Serial.print(ESP.getFreeHeap());
    Serial.print(F(" bytes, 最大塊: "));
    Serial.print(ESP.getMaxFreeBlockSize());
    Serial.println(F(" bytes"));
    
    // 降級處理：顯示白屏或錯誤圖案
    Serial.println(F("*** 顯示白屏作為降級處理 ***"));
    display.clearScreen();
    display.refresh();
    
    Serial.print(F("降級處理耗時: "));
    Serial.print(millis() - startTime);
    Serial.println(F(" ms"));
    return;
  }
  
  Serial.println(F("*** 開始解壓縮 ***"));
  
  // 解壓縮到臨時緩衝區
  int decompressedSize = RLEDecoder::decode(payload, length, tempBuffer, DISPLAY_BUFFER_SIZE);
  
  if (decompressedSize != DISPLAY_BUFFER_SIZE) {
    Serial.print(F("解壓縮失敗！期望 "));
    Serial.print(DISPLAY_BUFFER_SIZE);
    Serial.print(F(" bytes, 得到 "));
    Serial.println(decompressedSize);
    
    free(tempBuffer);
    return;
  }
  
  Serial.print(F("解壓縮完成: "));
  Serial.print(millis() - startTime);
  Serial.println(F(" ms"));
  
  // 使用分塊顯示
  Serial.println(F("*** 使用分塊顯示模式 ***"));
  displayFrameChunked(tempBuffer);
  
  // 立即釋放緩衝區
  free(tempBuffer);
  
  Serial.print(F("*** 顯示完成，記憶體已釋放，可用: "));
  Serial.print(ESP.getFreeHeap());
  Serial.println(F(" bytes ***"));
  
  Serial.print(F("總耗時: "));
  Serial.print(millis() - startTime);
  Serial.println(F(" ms"));
  Serial.println(F("*** 純分塊處理完成 ***"));
}
#endif

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
      
      // 重置緩衝（根據記憶體模式）
#if ENABLE_CHUNKED_DISPLAY
      if (fullFrame) {
        memset(fullFrame, 0xFF, DISPLAY_BUFFER_SIZE);
      }
      // 動態模式不需要重置，因為會重新分配
#else
      if (frameAllocated && currentFrame) {
        memset(currentFrame, 0xFF, DISPLAY_BUFFER_SIZE);
      }
#endif
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
// 顯示畫面（優化版 - 支援分塊顯示）
// ============================================
void displayFrame(const uint8_t* frame) {
  unsigned long startTime = millis();
  Serial.println(F("更新顯示中..."));

#if ENABLE_CHUNKED_DISPLAY
  if (useChunkedMode) {
    displayFrameChunked(frame);
  } else {
    // 傳統模式：將圖像顯示在螢幕中央（使用快速部分更新）
    display.setPartialWindow(DISPLAY_OFFSET_X, DISPLAY_OFFSET_Y, DISPLAY_WIDTH, DISPLAY_HEIGHT);
    display.writeImage(frame, 0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, false, false, true);  // 最後參數 true = 反相（修正黑底白字問題）
    display.refresh(false);  // 快速部分更新
  }
#else
  // 傳統模式：使用部分窗口快速更新
  display.setPartialWindow(DISPLAY_OFFSET_X, DISPLAY_OFFSET_Y, DISPLAY_WIDTH, DISPLAY_HEIGHT);
  display.writeImage(frame, 0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, false, false, true);  // 最後參數 true = 反相（修正黑底白字問題）
  display.refresh(false);  // 快速部分更新
#endif
  
  Serial.print(F("顯示更新耗時: "));
  Serial.print(millis() - startTime);
  Serial.println(F(" ms"));
}

// ============================================
// 處理分區更新（800×480 全螢幕分 4 區塊）
// ============================================
void handleTileUpdate(uint8_t* payload, uint32_t length, uint16_t seqId) {
  // 解析分區索引
  if (length < 1) {
    Serial.println(F("❌ 分區資料長度不足"));
    sendNAK(seqId);
    return;
  }
  
  uint8_t tileIndex = payload[0];
  payload++;  // 跳過 tile_index
  length--;   // 減去 tile_index 長度
  
  if (tileIndex >= TILE_COUNT) {
    Serial.print(F("❌ 無效的分區索引: "));
    Serial.println(tileIndex);
    sendNAK(seqId);
    return;
  }
  
  // 計算分區座標（垂直分割：從上到下 4 個 800×120 條帶）
  // 分區排列：
  //   0: (0,0)   Y: 0-120
  //   1: (0,120) Y: 120-240
  //   2: (0,240) Y: 240-360
  //   3: (0,360) Y: 360-480
  uint16_t tile_x = 0;                          // 所有條帶 X 都是 0
  uint16_t tile_y = tileIndex * TILE_HEIGHT;    // Y = 0, 120, 240, 360
  
  const char* tileNames[] = {"條帶0", "條帶1", "條帶2", "條帶3"};
  Serial.println(F("========================================"));
  Serial.print(F("📍 分區更新: "));
  Serial.print(tileNames[tileIndex]);
  Serial.print(F(" (索引="));
  Serial.print(tileIndex);
  Serial.print(F(", 座標=("));
  Serial.print(tile_x);
  Serial.print(F(","));
  Serial.print(tile_y);
  Serial.print(F("), 尺寸="));
  Serial.print(TILE_WIDTH);
  Serial.print(F("x"));
  Serial.print(TILE_HEIGHT);
  Serial.println(F(")"));
  
  // 檢查預先配置的緩衝區是否可用
  if (!tileBufferAllocated || !tileBuffer) {
    Serial.println(F("❌ 分區緩衝區未配置"));
    sendNAK(seqId);
    return;
  }
  
  Serial.println(F("✓ 使用預先配置的分區緩衝區"));
  Serial.print(F("🔍 當前記憶體: 可用="));
  Serial.print(ESP.getFreeHeap());
  Serial.print(F(" bytes, 最大塊="));
  Serial.print(ESP.getMaxFreeBlockSize());
  Serial.println(F(" bytes"));
  
  // 檢查 payload 是否就是 tileBuffer（使用外部緩衝區）
  bool isExternalBuffer = (payload == tileBuffer);
  int decompressedSize;
  
  if (isExternalBuffer) {
    Serial.println(F("✓ Payload 使用外部緩衝區（零拷貝）"));
    // payload 已經在 tileBuffer 中，無需任何操作
    decompressedSize = length;
  } else {
    // 舊的邏輯：需要複製或解壓縮
    Serial.println(F("⚠️ Payload 使用內部緩衝區（需要複製）"));
    
    bool isCompressed = (length != TILE_BUFFER_SIZE);
    
    if (isCompressed) {
      Serial.print(F("🗜️  解壓縮分區資料: "));
      Serial.print(length);
      Serial.print(F(" bytes → "));
      unsigned long decompressStart = millis();
      
      decompressedSize = RLEDecoder::decode(payload, length, tileBuffer, TILE_BUFFER_SIZE);
      
      unsigned long decompressTime = millis() - decompressStart;
      Serial.print(decompressedSize);
      Serial.print(F(" bytes ("));
      Serial.print(decompressTime);
      Serial.println(F(" ms)"));
    } else {
      Serial.print(F("📦 複製未壓縮資料: "));
      Serial.print(length);
      Serial.println(F(" bytes"));
      
      unsigned long copyStart = millis();
      memcpy(tileBuffer, payload, length);
      decompressedSize = length;
      unsigned long copyTime = millis() - copyStart;
      
      Serial.print(F("   複製完成 ("));
      Serial.print(copyTime);
      Serial.println(F(" ms)"));
    }
  }
  
  // 立即釋放 PacketReceiver（如果使用了內部緩衝區）
  if (!isExternalBuffer) {
    Serial.println(F("🗑️  釋放 payload 記憶體..."));
    packetReceiver.reset();
    yield();
    
    Serial.print(F("   釋放後記憶體: 可用="));
    Serial.print(ESP.getFreeHeap());
    Serial.print(F(" bytes, 最大塊="));
    Serial.print(ESP.getMaxFreeBlockSize());
    Serial.println(F(" bytes"));
  }
  
  if (decompressedSize != TILE_BUFFER_SIZE) {
    Serial.print(F("❌ 分區資料大小錯誤: 預期 "));
    Serial.print(TILE_BUFFER_SIZE);
    Serial.print(F(" bytes, 實際 "));
    Serial.print(decompressedSize);
    Serial.println(F(" bytes"));
    free(tileBuffer);
    sendNAK(seqId);
    return;
  }
  
  // 顯示分區
  Serial.println(F("🖼️  更新分區顯示..."));
  Serial.print(F("   setPartialWindow("));
  Serial.print(tile_x);
  Serial.print(F(", "));
  Serial.print(tile_y);
  Serial.print(F(", "));
  Serial.print(TILE_WIDTH);
  Serial.print(F(", "));
  Serial.print(TILE_HEIGHT);
  Serial.println(F(")"));
  
  unsigned long displayStart = millis();
  
  // 設置部分窗口到指定的分區座標
  // 注意：setPartialWindow 的座標是絕對螢幕座標
  display.setPartialWindow(tile_x, tile_y, TILE_WIDTH, TILE_HEIGHT);
  
  Serial.println(F("✓ setPartialWindow 完成"));
  yield();
  
  // writeImage 的座標也是絕對螢幕座標（不是相對於 PartialWindow）
  // 所以這裡要使用 tile_x, tile_y 而不是 0, 0
  display.writeImage(tileBuffer, tile_x, tile_y, TILE_WIDTH, TILE_HEIGHT, false, false, true);
  
  // 注意：不釋放 tileBuffer，因為它是預先配置的全局緩衝區
  Serial.println(F("✓ writeImage 完成（緩衝區保留供下次使用）"));
  yield();
  
  // 執行顯示刷新（這個操作需要約 18 秒）
  Serial.println(F("🔄 開始 refresh..."));
  display.refresh(false);  // 快速部分更新
  Serial.println(F("✓ refresh 完成"));
  
  unsigned long displayTime = millis() - displayStart;
  
  // 顯示完成後才發送 ACK
  sendACK(seqId);
  
  Serial.print(F("✅ 分區 "));
  Serial.print(tileNames[tileIndex]);
  Serial.print(F(" 更新完成 ("));
  Serial.print(displayTime);
  Serial.println(F(" ms)"));
  
  // 主動觸發記憶體整理，為下一個分區做準備
  Serial.println(F("🧹 觸發記憶體整理..."));
  yield();
  delay(1000);  // 給系統充足時間整理記憶體堆（增加到 1 秒）
  
  Serial.print(F("   整理後記憶體: 可用="));
  Serial.print(ESP.getFreeHeap());
  Serial.print(F(" bytes, 最大塊="));
  Serial.print(ESP.getMaxFreeBlockSize());
  Serial.println(F(" bytes"));
  Serial.println(F("========================================"));
}

#if ENABLE_CHUNKED_DISPLAY
// ============================================
// 分塊顯示函數（優化版 - 使用部分窗口）
// ============================================
void displayFrameChunked(const uint8_t* frame) {
  Serial.println(F("*** 使用分塊顯示模式 ***"));
  
  for (uint8_t chunk = 0; chunk < MAX_CHUNKS; chunk++) {
    uint16_t y_start = chunk * CHUNK_HEIGHT;
    uint16_t y_end = min((chunk + 1) * CHUNK_HEIGHT, DISPLAY_HEIGHT);
    uint16_t chunk_height = y_end - y_start;
    
    // 計算在原始緩衝區中的偏移位置
    uint32_t chunk_offset = (y_start * DISPLAY_WIDTH) / 8;
    
    Serial.print(F("處理塊 "));
    Serial.print(chunk + 1);
    Serial.print(F("/"));
    Serial.print(MAX_CHUNKS);
    Serial.print(F(" (Y: "));
    Serial.print(y_start);
    Serial.print(F("-"));
    Serial.print(y_end);
    Serial.println(F(")"));
    
    // 設定部分窗口：考慮螢幕偏移量，將圖像顯示在中央
    display.setPartialWindow(DISPLAY_OFFSET_X, DISPLAY_OFFSET_Y + y_start, DISPLAY_WIDTH, chunk_height);
    
    // 使用標準 writeImage 函數寫入該塊
    // 我們需要建立該塊的緩衝區
    uint32_t chunk_buffer_size = (DISPLAY_WIDTH * chunk_height) / 8;
    uint8_t* chunk_buffer = (uint8_t*)malloc(chunk_buffer_size);
    
    if (chunk_buffer) {
      // 複製該塊的資料
      memcpy(chunk_buffer, frame + chunk_offset, chunk_buffer_size);
      
      // 寫入該塊並立即刷新
      display.writeImage(chunk_buffer, 0, 0, DISPLAY_WIDTH, chunk_height, false, false, true);
      display.refresh(false);  // false = 不執行完整刷新，只更新部分窗口
      
      // 釋放塊緩衝區
      free(chunk_buffer);
      
      Serial.print(F("  ✓ 塊 "));
      Serial.print(chunk + 1);
      Serial.println(F(" 寫入並刷新完成"));
    } else {
      Serial.print(F("  ✗ 塊 "));
      Serial.print(chunk + 1);
      Serial.println(F(" 記憶體分配失敗！"));
    }
    
    // 顯示記憶體狀況
    Serial.print(F("  可用記憶體: "));
    Serial.print(ESP.getFreeHeap());
    Serial.println(F(" bytes"));
    
    // 短暫延遲以避免看門狗觸發
    yield();
  }
  
  // 分塊顯示完成（不需要再次 refresh，每塊已經單獨刷新）
  Serial.println(F("*** 分塊顯示完成 ***"));
}
#endif

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
// 記憶體碎片整理（啟動時呼叫）
// ============================================
void defragmentMemory() {
  Serial.println(F("*** 嘗試整理記憶體碎片 ***"));
  
  uint32_t initialFree = ESP.getFreeHeap();
  uint32_t initialMaxBlock = ESP.getMaxFreeBlockSize();
  
  Serial.print(F("整理前 - 可用: "));
  Serial.print(initialFree);
  Serial.print(F(" bytes, 最大塊: "));
  Serial.print(initialMaxBlock);
  Serial.println(F(" bytes"));
  
  // 嘗試分配和釋放一些記憶體塊來整理碎片
  void* ptrs[10];
  int allocated = 0;
  
  // 嘗試分配多個小塊
  for (int i = 0; i < 10; i++) {
    ptrs[i] = malloc(1024);  // 1KB 塊
    if (ptrs[i]) {
      allocated++;
    } else {
      break;
    }
  }
  
  // 釋放所有分配的塊
  for (int i = 0; i < allocated; i++) {
    free(ptrs[i]);
  }
  
  uint32_t finalFree = ESP.getFreeHeap();
  uint32_t finalMaxBlock = ESP.getMaxFreeBlockSize();
  
  Serial.print(F("整理後 - 可用: "));
  Serial.print(finalFree);
  Serial.print(F(" bytes, 最大塊: "));
  Serial.print(finalMaxBlock);
  Serial.println(F(" bytes"));
  
  if (finalMaxBlock > initialMaxBlock) {
    Serial.println(F("*** 記憶體整理成功！***"));
  } else {
    Serial.println(F("*** 記憶體整理效果有限 ***"));
  }
}

// ============================================
// 記憶體監控（增強版）
// ============================================
void checkMemory() {
  unsigned long now = millis();
  
  if (now - lastMemoryCheck >= MEMORY_CHECK_INTERVAL) {
    lastMemoryCheck = now;
    
    uint32_t freeHeap = ESP.getFreeHeap();
    uint32_t maxFreeBlockSize = ESP.getMaxFreeBlockSize();
    uint8_t heapFragmentation = 100 - ((maxFreeBlockSize * 100) / freeHeap);
    
    Serial.print(F("記憶體狀況 - 可用: "));
    Serial.print(freeHeap);
    Serial.print(F(" bytes, 最大塊: "));
    Serial.print(maxFreeBlockSize);
    Serial.print(F(" bytes, 碎片化: "));
    Serial.print(heapFragmentation);
    Serial.println(F("%"));
    
#if ENABLE_CHUNKED_DISPLAY
    // 檢查記憶體狀況並提供建議
    if (ENABLE_DYNAMIC_ALLOCATION && useChunkedMode) {
      if (maxFreeBlockSize < DISPLAY_BUFFER_SIZE) {
        Serial.println(F("*** 提示：使用純分塊模式（記憶體碎片化） ***"));
        if (maxFreeBlockSize < CHUNK_BUFFER_SIZE) {
          Serial.println(F("*** 嚴重警告：連分塊緩衝都無法分配！***"));
          Serial.println(F("*** 建議立即重啟以整理記憶體 ***"));
        }
      }
    }
#endif
    
    if (freeHeap < MEMORY_WARNING_THRESHOLD) {
      Serial.println(F("*** 警告：記憶體不足！***"));
      
      // 嘗試記憶體回收
      if (heapFragmentation > 50) {
        Serial.println(F("*** 記憶體碎片化嚴重，建議重啟 ***"));
      }
    }
  }
}

// ============================================
// 記憶體分配輔助函數（智能版）
// ============================================
uint8_t* allocateDisplayBuffer(const char* purpose) {
  Serial.print(F("*** 嘗試分配顯示緩衝區（"));
  Serial.print(purpose);
  Serial.println(F("）***"));
  
  uint32_t freeHeap = ESP.getFreeHeap();
  uint32_t maxBlock = ESP.getMaxFreeBlockSize();
  
  Serial.print(F("分配前 - 可用: "));
  Serial.print(freeHeap);
  Serial.print(F(" bytes, 最大塊: "));
  Serial.print(maxBlock);
  Serial.println(F(" bytes"));
  
#if ENABLE_CHUNKED_DISPLAY
  // 智能判斷：如果最大塊不足，直接返回 null 並切換到純分塊模式
  if (maxBlock < DISPLAY_BUFFER_SIZE) {
    Serial.println(F("*** 最大塊不足，切換到純分塊模式 ***"));
    Serial.print(F("需要: "));
    Serial.print(DISPLAY_BUFFER_SIZE);
    Serial.print(F(" bytes, 最大塊: "));
    Serial.print(maxBlock);
    Serial.println(F(" bytes"));
    
    // 強制切換到分塊模式
    useChunkedMode = true;
    return nullptr;  // 返回 null，但這是正常行為
  }
#endif
  
  uint8_t* buffer = (uint8_t*)malloc(DISPLAY_BUFFER_SIZE);
  
  if (buffer) {
    Serial.print(F("*** 分配成功，分配後可用: "));
    Serial.print(ESP.getFreeHeap());
    Serial.println(F(" bytes ***"));
  } else {
    Serial.println(F("*** 分配失敗！***"));
    
    Serial.print(F("需要: "));
    Serial.print(DISPLAY_BUFFER_SIZE);
    Serial.print(F(" bytes, 可用: "));
    Serial.print(freeHeap);
    Serial.print(F(" bytes, 最大塊: "));
    Serial.print(maxBlock);
    Serial.println(F(" bytes"));
    
#if ENABLE_CHUNKED_DISPLAY
    // 強制切換到分塊模式
    useChunkedMode = true;
#endif
  }
  
  return buffer;
}

// ============================================
// 安全釋放記憶體
// ============================================
void safeFreeMem(uint8_t** buffer, const char* purpose) {
  if (buffer && *buffer) {
    Serial.print(F("*** 釋放記憶體（"));
    Serial.print(purpose);
    Serial.print(F("），釋放前可用: "));
    Serial.print(ESP.getFreeHeap());
    Serial.print(F(" bytes ***"));
    
    free(*buffer);
    *buffer = nullptr;
    
    Serial.print(F("釋放後可用: "));
    Serial.print(ESP.getFreeHeap());
    Serial.println(F(" bytes"));
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
  Serial.println(F("WiFi SPI Display - Client v1.1"));
  Serial.println(F("=== 記憶體優化版 2024-10-06 ==="));
  Serial.println(F("================================="));
  
  // *** 記憶體碎片整理 ***
  defragmentMemory();
  
  // *** 動態記憶體分配策略（優化版） ***
  Serial.print(F("*** [1/5] 啟動時可用記憶體: "));
  Serial.print(ESP.getFreeHeap());
  Serial.println(F(" bytes ***"));
  
#if ENABLE_CHUNKED_DISPLAY
  if (ENABLE_DYNAMIC_ALLOCATION) {
    Serial.println(F("*** [2/5] 採用分塊顯示模式 - 動態分配 ***"));
    Serial.print(F("每塊大小: "));
    Serial.print(CHUNK_BUFFER_SIZE);
    Serial.println(F(" bytes"));
    
    // 分塊模式下不預先分配大緩衝區
    useChunkedMode = true;
    frameAllocated = true;  // 標記為已分配（實際上是動態分配）
  } else {
    Serial.println(F("*** [2/5] 分塊模式 - 預先分配完整緩衝 ***"));
    fullFrame = (uint8_t*)malloc(DISPLAY_BUFFER_SIZE);
    
    if (!fullFrame) {
      Serial.println(F("*** 完整緩衝分配失敗，切換到動態模式 ***"));
      useChunkedMode = true;
      frameAllocated = true;
    } else {
      useChunkedMode = false;
      frameAllocated = true;
      Serial.println(F("*** ✓ 完整緩衝分配成功！***"));
    }
  }
#else
  Serial.println(F("*** [2/5] 傳統模式 - 預先分配 48KB 顯示緩衝 ***"));
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
#endif

  Serial.print(F("*** 分配後可用記憶體: "));
  Serial.print(ESP.getFreeHeap());
  Serial.println(F(" bytes ***"));
  
#if ENABLE_TILE_DISPLAY
  // 預先配置分區顯示緩衝區
  Serial.println(F("*** [2.5/5] 配置分區顯示緩衝區 (12KB)... ***"));
  tileBuffer = (uint8_t*)malloc(TILE_BUFFER_SIZE);
  
  if (!tileBuffer) {
    Serial.println(F("*** 警告：無法分配分區緩衝區！***"));
    Serial.print(F("需要: "));
    Serial.print(TILE_BUFFER_SIZE);
    Serial.println(F(" bytes"));
    tileBufferAllocated = false;
  } else {
    tileBufferAllocated = true;
    Serial.println(F("*** ✓ 分區緩衝區配置成功！***"));
    Serial.print(F("*** 配置後可用記憶體: "));
    Serial.print(ESP.getFreeHeap());
    Serial.println(F(" bytes ***"));
  }
#endif
  
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
  
  // 增加 WebSocket 接收緩衝區（預設太小）
  // 對於 21KB 壓縮資料，需要更大的緩衝區
  // 注意：這會增加記憶體使用
  
  // *** [5/5] 初始化電子紙顯示器並清除螢幕 ***
  Serial.println(F("*** [5/5] 初始化電子紙顯示器... ***"));
  display.init(SERIAL_BAUD);
  
  Serial.println(F("*** 清除整個螢幕（上電初始化）... ***"));
  display.setFullWindow();
  display.clearScreen();
  display.refresh(true);  // 完整刷新以確保清除乾淨
  
  // 清屏後立即切換到部分窗口模式，確保第一次更新正常
  display.setPartialWindow(DISPLAY_OFFSET_X, DISPLAY_OFFSET_Y, DISPLAY_WIDTH, DISPLAY_HEIGHT);
  Serial.println(F("*** 螢幕清除完成，已切換到部分窗口模式 ***"));
  
  // 顯示測試畫面
  Serial.println(F("*** 顯示測試畫面... ***"));
  displayTestPattern();
  
  Serial.println(F("初始化完成！"));
  Serial.print(F("可用記憶體: "));
  Serial.print(ESP.getFreeHeap());
  Serial.println(F(" bytes"));
}

// ============================================
// 顯示測試畫面
// ============================================
void displayTestPattern() {
  // 分配測試緩衝區
  uint8_t* testBuffer = (uint8_t*)malloc(DISPLAY_BUFFER_SIZE);
  if (!testBuffer) {
    Serial.println(F("*** 無法分配測試緩衝區 ***"));
    return;
  }
  
  // 填充測試圖案：棋盤格
  for (int y = 0; y < DISPLAY_HEIGHT; y++) {
    for (int x = 0; x < DISPLAY_WIDTH; x++) {
      int byteIndex = y * (DISPLAY_WIDTH / 8) + (x / 8);
      int bitIndex = 7 - (x % 8);
      
      // 創建 20x20 的棋盤格
      bool isBlack = ((x / 20) + (y / 20)) % 2 == 0;
      
      if (isBlack) {
        testBuffer[byteIndex] &= ~(1 << bitIndex);  // 黑色
      } else {
        testBuffer[byteIndex] |= (1 << bitIndex);   // 白色
      }
    }
  }
  
  // 顯示測試圖案
  display.setPartialWindow(DISPLAY_OFFSET_X, DISPLAY_OFFSET_Y, DISPLAY_WIDTH, DISPLAY_HEIGHT);
  display.writeImage(testBuffer, 0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, false, false, true);
  display.refresh(false);  // 快速部分更新
  
  // 釋放測試緩衝區
  free(testBuffer);
  Serial.println(F("*** 測試畫面顯示完成 ***"));
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
