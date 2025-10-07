/*
 * WiFi SPI Display - RLE 解壓縮器
 * 
 * 串流式解壓縮，無需額外記憶體
 * 與 Server 端的 compressor.py 對應
 */

#ifndef RLE_DECODER_H
#define RLE_DECODER_H

#include <Arduino.h>

// ============================================
// RLE 解碼器
// ============================================
class RLEDecoder {
public:
  /**
   * RLE 解壓縮（一次性）
   * 
   * @param compressed 壓縮資料
   * @param compressedSize 壓縮資料大小
   * @param output 輸出緩衝
   * @param maxOutputSize 最大輸出大小
   * @return 解壓縮後的大小，-1 表示錯誤
   */
  static int decode(const uint8_t* compressed, size_t compressedSize, 
                    uint8_t* output, size_t maxOutputSize) {
    size_t inPos = 0;
    size_t outPos = 0;
    
    while (inPos < compressedSize) {
      if (outPos >= maxOutputSize) {
        Serial.println(F("輸出緩衝溢出！"));
        return -1;
      }
      
      // 讀取 count 和 value
      if (inPos + 1 >= compressedSize) {
        Serial.println(F("壓縮資料不完整！"));
        return -1;
      }
      
      uint8_t count = compressed[inPos++];
      uint8_t value = compressed[inPos++];
      
      // 展開
      for (uint8_t i = 0; i < count; i++) {
        if (outPos >= maxOutputSize) {
          Serial.println(F("輸出緩衝溢出！"));
          return -1;
        }
        output[outPos++] = value;
      }
      
      // 每處理一些資料就餵食看門狗
      if ((inPos % 512) == 0) {
        yield();
      }
    }
    
    return outPos;
  }
  
  /**
   * 串流式 RLE 解壓縮
   * 
   * 使用回調函數處理解壓縮的資料，無需大緩衝
   * 
   * @param compressed 壓縮資料
   * @param compressedSize 壓縮資料大小
   * @param callback 回調函數 (data, size)
   * @param bufferSize 內部緩衝大小
   * @return 解壓縮的總大小，-1 表示錯誤
   */
  static int decodeStream(const uint8_t* compressed, size_t compressedSize,
                          void (*callback)(const uint8_t*, size_t),
                          size_t bufferSize = 256) {
    uint8_t* buffer = (uint8_t*)malloc(bufferSize);
    if (!buffer) {
      Serial.println(F("記憶體分配失敗！"));
      return -1;
    }
    
    size_t inPos = 0;
    size_t outPos = 0;
    size_t totalOut = 0;
    
    while (inPos < compressedSize) {
      // 讀取 count 和 value
      if (inPos + 1 >= compressedSize) {
        free(buffer);
        Serial.println(F("壓縮資料不完整！"));
        return -1;
      }
      
      uint8_t count = compressed[inPos++];
      uint8_t value = compressed[inPos++];
      
      // 展開到緩衝
      for (uint8_t i = 0; i < count; i++) {
        buffer[outPos++] = value;
        totalOut++;
        
        // 緩衝滿了，呼叫回調
        if (outPos >= bufferSize) {
          callback(buffer, outPos);
          outPos = 0;
        }
      }
    }
    
    // 處理剩餘資料
    if (outPos > 0) {
      callback(buffer, outPos);
    }
    
    free(buffer);
    return totalOut;
  }
  
  /**
   * 計算解壓縮後的大小（不實際解壓縮）
   * 
   * @param compressed 壓縮資料
   * @param compressedSize 壓縮資料大小
   * @return 解壓縮後的大小，-1 表示錯誤
   */
  static int getDecompressedSize(const uint8_t* compressed, size_t compressedSize) {
    size_t inPos = 0;
    size_t totalSize = 0;
    
    while (inPos < compressedSize) {
      if (inPos + 1 >= compressedSize) {
        return -1;
      }
      
      uint8_t count = compressed[inPos++];
      inPos++;  // Skip value
      totalSize += count;
    }
    
    return totalSize;
  }
};

// ============================================
// Delta 解碼器
// ============================================
class DeltaDecoder {
public:
  /**
   * Delta 解壓縮
   * 
   * @param delta Delta 資料
   * @param deltaSize Delta 資料大小
   * @param oldFrame 舊畫面
   * @param output 輸出緩衝（新畫面）
   * @param frameSize 畫面大小
   * @return 是否成功
   */
  static bool decode(const uint8_t* delta, size_t deltaSize,
                     const uint8_t* oldFrame, uint8_t* output,
                     size_t frameSize) {
    // 複製舊畫面
    memcpy(output, oldFrame, frameSize);
    
    // 應用 Delta
    size_t pos = 0;
    while (pos < deltaSize) {
      if (pos + 4 >= deltaSize) {
        Serial.println(F("Delta 資料不完整！"));
        return false;
      }
      
      // 讀取位置（4 bytes）
      uint32_t offset = ((uint32_t)delta[pos] << 24) |
                        ((uint32_t)delta[pos + 1] << 16) |
                        ((uint32_t)delta[pos + 2] << 8) |
                        delta[pos + 3];
      pos += 4;
      
      // 讀取長度（1 byte）
      if (pos >= deltaSize) {
        Serial.println(F("Delta 資料不完整！"));
        return false;
      }
      uint8_t length = delta[pos++];
      
      // 檢查越界
      if (offset + length > frameSize || pos + length > deltaSize) {
        Serial.println(F("Delta 資料越界！"));
        return false;
      }
      
      // 複製資料
      memcpy(output + offset, delta + pos, length);
      pos += length;
    }
    
    return true;
  }
  
  /**
   * 串流式 Delta 解壓縮
   * 
   * @param delta Delta 資料
   * @param deltaSize Delta 資料大小
   * @param oldFrame 舊畫面
   * @param callback 回調函數 (offset, data, size)
   * @return 是否成功
   */
  static bool decodeStream(const uint8_t* delta, size_t deltaSize,
                           const uint8_t* oldFrame,
                           void (*callback)(uint32_t, const uint8_t*, size_t)) {
    size_t pos = 0;
    
    while (pos < deltaSize) {
      if (pos + 4 >= deltaSize) {
        Serial.println(F("Delta 資料不完整！"));
        return false;
      }
      
      // 讀取位置（4 bytes）
      uint32_t offset = ((uint32_t)delta[pos] << 24) |
                        ((uint32_t)delta[pos + 1] << 16) |
                        ((uint32_t)delta[pos + 2] << 8) |
                        delta[pos + 3];
      pos += 4;
      
      // 讀取長度（1 byte）
      if (pos >= deltaSize) {
        Serial.println(F("Delta 資料不完整！"));
        return false;
      }
      uint8_t length = delta[pos++];
      
      // 檢查越界
      if (pos + length > deltaSize) {
        Serial.println(F("Delta 資料越界！"));
        return false;
      }
      
      // 呼叫回調處理這個區塊
      callback(offset, delta + pos, length);
      pos += length;
    }
    
    return true;
  }
};

// ============================================
// 混合解碼器（RLE + Delta）
// ============================================
class HybridDecoder {
public:
  /**
   * 混合解壓縮
   * 
   * @param compressed 壓縮資料
   * @param compressedSize 壓縮資料大小
   * @param oldFrame 舊畫面（用於 Delta）
   * @param output 輸出緩衝
   * @param frameSize 畫面大小
   * @param isDelta 是否為 Delta 模式
   * @return 解壓縮後的大小，-1 表示錯誤
   */
  static int decode(const uint8_t* compressed, size_t compressedSize,
                    const uint8_t* oldFrame, uint8_t* output,
                    size_t frameSize, bool isDelta) {
    if (isDelta) {
      // Delta + RLE 模式
      // 先用 RLE 解壓縮
      uint8_t* rleOutput = (uint8_t*)malloc(frameSize);
      if (!rleOutput) {
        Serial.println(F("記憶體分配失敗！"));
        return -1;
      }
      
      int rleSize = RLEDecoder::decode(compressed, compressedSize, rleOutput, frameSize);
      if (rleSize < 0) {
        free(rleOutput);
        return -1;
      }
      
      // 再用 Delta 解壓縮
      bool success = DeltaDecoder::decode(rleOutput, rleSize, oldFrame, output, frameSize);
      free(rleOutput);
      
      return success ? frameSize : -1;
    }
    else {
      // 只有 RLE
      return RLEDecoder::decode(compressed, compressedSize, output, frameSize);
    }
  }
};

#endif // RLE_DECODER_H
