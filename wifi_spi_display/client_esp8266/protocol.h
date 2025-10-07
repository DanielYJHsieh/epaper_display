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
#define PROTO_TYPE_DELTA 0x02    // 差分更新
#define PROTO_TYPE_CMD 0x03      // 控制指令
#define PROTO_TYPE_ACK 0x10      // 確認
#define PROTO_TYPE_NAK 0x11      // 否認（錯誤）

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
    header.seq_id = (data[2] << 8) | data[3];
    header.length = ((uint32_t)data[4] << 24) | 
                    ((uint32_t)data[5] << 16) | 
                    ((uint32_t)data[6] << 8) | 
                    data[7];
    
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
    buffer[2] = (seq_id >> 8) & 0xFF;
    buffer[3] = seq_id & 0xFF;
    buffer[4] = 0;  // length MSB
    buffer[5] = 0;
    buffer[6] = 0;
    buffer[7] = 1;  // length = 1
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
    buffer[2] = (seq_id >> 8) & 0xFF;
    buffer[3] = seq_id & 0xFF;
    buffer[4] = 0;
    buffer[5] = 0;
    buffer[6] = 0;
    buffer[7] = 1;
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
  
public:
  PacketReceiver() {
    state = WAITING_HEADER;
    payload = nullptr;
    payloadReceived = 0;
  }
  
  ~PacketReceiver() {
    if (payload) {
      free(payload);
    }
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
      
      // 分配 payload 記憶體
      if (header.length > 0) {
        // 檢查記憶體是否足夠
        uint32_t freeHeap = ESP.getFreeHeap();
        Serial.print(F("需要分配: "));
        Serial.print(header.length);
        Serial.print(F(" bytes, 可用: "));
        Serial.print(freeHeap);
        Serial.println(F(" bytes"));
        
        if (freeHeap < header.length + 8000) {  // 保留 8KB 安全餘量
          Serial.println(F("記憶體不足！"));
          return false;
        }
        
        payload = (uint8_t*)malloc(header.length);
        if (!payload) {
          Serial.println(F("記憶體分配失敗！"));
          return false;
        }
        Serial.println(F("Payload 記憶體分配成功"));
      }
      
      // 如果還有資料，複製到 payload
      if (length > PROTO_HEADER_SIZE) {
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
    if (payload) {
      free(payload);
      payload = nullptr;
    }
    payloadReceived = 0;
    state = WAITING_HEADER;
  }
};

#endif // PROTOCOL_H
