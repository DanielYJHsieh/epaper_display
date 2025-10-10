/*
 * WiFi SPI Display - 通訊協議定義
 * 
 * 與 Server 端的 protocol.py 對應
 */

#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <Arduino.h>

// ============================================
// 協議常數
// ============================================
#define PROTO_HEADER 0xA5
#define PROTO_HEADER_SIZE 8  // 1 + 1 + 2 + 4 bytes

// ============================================
// 封包類型
// ============================================
#define PROTO_TYPE_FULL 0x01     // 完整畫面更新
#define PROTO_TYPE_TILE 0x02     // 分區更新（新增）
#define PROTO_TYPE_DELTA 0x03    // 差分更新
#define PROTO_TYPE_CMD 0x04      // 控制指令
#define PROTO_TYPE_ACK 0x10      // 確認
#define PROTO_TYPE_NAK 0x11      // 否認（錯誤）

// ============================================
// 分區索引定義（垂直分割：從上到下 3 個 800×160 條帶）
// 上、中、下三個區塊，完整覆蓋 800×480 螢幕
// ============================================
#define TILE_INDEX_BAND_0       0  // 第 0 條帶（上，Y: 0-160）
#define TILE_INDEX_BAND_1       1  // 第 1 條帶（中，Y: 160-320）
#define TILE_INDEX_BAND_2       2  // 第 2 條帶（下，Y: 320-480）
#define TILE_COUNT              3  // 總區域數

// ============================================
// 控制指令
// ============================================
#define CMD_CLEAR_SCREEN 0x10    // 清空螢幕
#define CMD_SLEEP 0x11           // 休眠模式
#define CMD_WAKE 0x12            // 喚醒
#define CMD_PARTIAL_MODE 0x13    // 局部更新模式
#define CMD_FULL_MODE 0x14       // 全螢幕更新模式

// ============================================
// 封包頭結構
// ============================================
struct PacketHeader {
  uint8_t header;      // 0xA5
  uint8_t type;        // 封包類型
  uint16_t seq_id;     // 序號
  uint32_t length;     // Payload 長度
} __attribute__((packed));

// ============================================
// 分區資訊結構
// ============================================
struct TileInfo {
  uint8_t tile_index;  // 區域索引 (0~3)
} __attribute__((packed));

// ============================================
// 協議解析器
// ============================================
class ProtocolParser {
public:
  /**
   * 解析封包頭
   * 
   * @param data 資料緩衝
   * @param header 輸出：封包頭結構
   * @return 是否成功解析
   */
  static bool parseHeader(const uint8_t* data, PacketHeader& header) {
    if (data[0] != PROTO_HEADER) {
      return false;
    }
    
    header.header = data[0];
    header.type = data[1];
    // 小端序 (Little-Endian) - 與 Python struct.pack('<BBHI') 一致
    header.seq_id = data[2] | (data[3] << 8);
    header.length = data[4] | 
                    (data[5] << 8) | 
                    (data[6] << 16) | 
                    ((uint32_t)data[7] << 24);
    
    return true;
  }
  
  /**
   * 驗證封包頭
   * 
   * @param data 資料緩衝
   * @param size 資料大小
   * @return 是否有效
   */
  static bool validateHeader(const uint8_t* data, size_t size) {
    if (size < PROTO_HEADER_SIZE) {
      return false;
    }
    
    return data[0] == PROTO_HEADER;
  }
  
  /**
   * 打包 ACK
   * 
   * @param seq_id 序號
   * @param buffer 輸出緩衝
   * @param status 狀態（1=成功，0=失敗）
   * @return 封包大小
   */
  static size_t packACK(uint16_t seq_id, uint8_t* buffer, uint8_t status = 1) {
    buffer[0] = PROTO_HEADER;
    buffer[1] = PROTO_TYPE_ACK;
    // 小端序 (Little-Endian)
    buffer[2] = seq_id & 0xFF;
    buffer[3] = (seq_id >> 8) & 0xFF;
    buffer[4] = 1;  // length = 1 (小端序)
    buffer[5] = 0;
    buffer[6] = 0;
    buffer[7] = 0;
    buffer[8] = status;
    
    return 9;  // 8 + 1 bytes
  }
  
  /**
   * 打包 NAK
   * 
   * @param seq_id 序號
   * @param buffer 輸出緩衝
   * @return 封包大小
   */
  static size_t packNAK(uint16_t seq_id, uint8_t* buffer) {
    buffer[0] = PROTO_HEADER;
    buffer[1] = PROTO_TYPE_NAK;
    // 小端序 (Little-Endian)
    buffer[2] = seq_id & 0xFF;
    buffer[3] = (seq_id >> 8) & 0xFF;
    buffer[4] = 1;  // length = 1 (小端序)
    buffer[5] = 0;
    buffer[6] = 0;
    buffer[7] = 0;
    buffer[8] = 0;
    
    return 9;
  }
  
  /**
   * 取得封包類型名稱（用於除錯）
   * 
   * @param type 封包類型
   * @return 類型名稱
   */
  static const char* getTypeName(uint8_t type) {
    switch (type) {
      case PROTO_TYPE_FULL: return "FULL_UPDATE";
      case PROTO_TYPE_TILE: return "TILE_UPDATE";
      case PROTO_TYPE_DELTA: return "DELTA_UPDATE";
      case PROTO_TYPE_CMD: return "COMMAND";
      case PROTO_TYPE_ACK: return "ACK";
      case PROTO_TYPE_NAK: return "NAK";
      default: return "UNKNOWN";
    }
  }
};

// ============================================
// 封包狀態
// ============================================
enum PacketState {
  WAITING_HEADER,   // 等待封包頭
  RECEIVING_PAYLOAD // 接收 Payload
};

// ============================================
// 封包接收器
// ============================================
class PacketReceiver {
private:
  PacketState state;
  PacketHeader header;
  uint8_t* payload;
  size_t payloadReceived;
  bool useExternalBuffer;  // 是否使用外部緩衝區
  
public:
  PacketReceiver() {
    state = WAITING_HEADER;
    payload = nullptr;
    payloadReceived = 0;
    useExternalBuffer = false;
  }
  
  ~PacketReceiver() {
    // 只釋放自己分配的記憶體
    if (payload && !useExternalBuffer) {
      free(payload);
    }
  }
  
  /**
   * 設置外部緩衝區（用於避免額外記憶體分配）
   * 
   * @param buffer 外部緩衝區指標
   * @param size 緩衝區大小
   */
  void setExternalBuffer(uint8_t* buffer, size_t size) {
    // 釋放之前的內部緩衝區
    if (payload && !useExternalBuffer) {
      free(payload);
    }
    payload = buffer;
    useExternalBuffer = true;
  }
  
  /**
   * 清除外部緩衝區設置
   */
  void clearExternalBuffer() {
    payload = nullptr;
    useExternalBuffer = false;
  }
  
  /**
   * 處理接收到的資料
   * 
   * @param data 資料緩衝
   * @param length 資料長度
   * @return 是否接收完整封包
   */
  bool process(const uint8_t* data, size_t length) {
    if (state == WAITING_HEADER) {
      // 解析封包頭
      if (length < PROTO_HEADER_SIZE) {
        return false;
      }
      
      if (!ProtocolParser::parseHeader(data, header)) {
        return false;
      }
      
      // 分配 payload 記憶體（或使用外部緩衝區）
      if (header.length > 0) {
        if (useExternalBuffer && payload) {
          // 使用外部緩衝區，不需要分配記憶體
          Serial.print(F("✓ 使用外部緩衝區: "));
          Serial.print(header.length);
          Serial.println(F(" bytes"));
        } else {
          // 需要動態分配記憶體
          uint32_t freeHeap = ESP.getFreeHeap();
          uint32_t maxBlock = ESP.getMaxFreeBlockSize();
          Serial.print(F("需要分配: "));
          Serial.print(header.length);
          Serial.print(F(" bytes, 可用: "));
          Serial.print(freeHeap);
          Serial.print(F(" bytes, 最大塊: "));
          Serial.print(maxBlock);
          Serial.println(F(" bytes"));
          
          // 檢查最大連續塊是否足夠
          if (maxBlock < header.length) {
            Serial.print(F("記憶體碎片化！最大連續塊 "));
            Serial.print(maxBlock);
            Serial.print(F(" bytes 小於需求 "));
            Serial.print(header.length);
            Serial.println(F(" bytes"));
            return false;
          }
          
          // 最小安全餘量
          if (freeHeap < header.length + 100) {
            Serial.print(F("記憶體不足！需要 "));
            Serial.print(header.length + 100);
            Serial.print(F(" bytes (payload + 100B 安全餘量), 可用 "));
            Serial.print(freeHeap);
            Serial.println(F(" bytes"));
            return false;
          }
          
          payload = (uint8_t*)malloc(header.length);
          if (!payload) {
            Serial.println(F("記憶體分配失敗！"));
            return false;
          }
          Serial.println(F("Payload 記憶體分配成功"));
        }
      } else if (!useExternalBuffer) {
        payload = nullptr;
      }
      
      // 如果還有資料，複製到 payload
      if (header.length > 0 && length > PROTO_HEADER_SIZE) {
        size_t payloadSize = length - PROTO_HEADER_SIZE;
        if (payloadSize > header.length) {
          payloadSize = header.length;
        }
        memcpy(payload, data + PROTO_HEADER_SIZE, payloadSize);
        payloadReceived = payloadSize;
        
        // 餵食看門狗
        yield();
      }
      
      state = RECEIVING_PAYLOAD;
      
      // 檢查是否已完整
      return payloadReceived >= header.length;
    }
    else {
      // 繼續接收 payload
      size_t remaining = header.length - payloadReceived;
      size_t toCopy = (length > remaining) ? remaining : length;
      
      memcpy(payload + payloadReceived, data, toCopy);
      payloadReceived += toCopy;
      
      // 餵食看門狗
      yield();
      
      return payloadReceived >= header.length;
    }
  }
  
  /**
   * 取得封包頭
   */
  const PacketHeader& getHeader() const {
    return header;
  }
  
  /**
   * 取得 Payload
   */
  const uint8_t* getPayload() const {
    return payload;
  }
  
  /**
   * 重置接收器
   */
  void reset() {
    // 只釋放自己分配的記憶體
    if (payload && !useExternalBuffer) {
      free(payload);
    }
    // 如果使用外部緩衝區，不要清空 payload 指標（讓它繼續指向外部緩衝區）
    if (!useExternalBuffer) {
      payload = nullptr;
    }
    payloadReceived = 0;
    state = WAITING_HEADER;
  }
};

#endif // PROTOCOL_H
