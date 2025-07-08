/*
  ESP8266 E-Paper 簡單測試程式
  
  功能：
  - 測試 GDEQ0426T82 (SSD1677) 基本功能
  - 簡化的初始化流程
  - 基本圖形顯示
  
  硬體連接：
  - VCC → 3.3V
  - GND → GND
  - DIN → D7 (GPIO13)
  - CLK → D5 (GPIO14)
  - CS → D8 (GPIO15)
  - DC → D3 (GPIO0)
  - RST → D4 (GPIO2)
  - BUSY → D2 (GPIO4)
  
  使用方式：
  1. 連接硬體
  2. 上傳程式
  3. 觀察序列埠輸出
  4. 查看 E-Paper 顯示結果
  
  作者：DYJ Hsieh
  日期：2025年7月
  版本：v1.0 (簡化測試版)
*/

#include <SPI.h>

// 腳位定義
#define CS_PIN    D8    // GPIO15
#define DC_PIN    D3    // GPIO0
#define RST_PIN   D4    // GPIO2
#define BUSY_PIN  D2    // GPIO4

// 顯示參數
#define WIDTH  800
#define HEIGHT 480

// SSD1677 指令
#define CMD_SW_RESET                    0x12
#define CMD_DRIVER_OUTPUT_CONTROL       0x01
#define CMD_DATA_ENTRY_MODE             0x11
#define CMD_SET_RAM_X_POSITION          0x44
#define CMD_SET_RAM_Y_POSITION          0x45
#define CMD_SET_RAM_X_COUNTER           0x4E
#define CMD_SET_RAM_Y_COUNTER           0x4F
#define CMD_WRITE_RAM                   0x24
#define CMD_DISPLAY_UPDATE_CONTROL_2    0x22
#define CMD_MASTER_ACTIVATION           0x20

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("========================================");
  Serial.println("     ESP8266 E-Paper 簡單測試");
  Serial.println("========================================");
  
  // 初始化腳位
  pinMode(CS_PIN, OUTPUT);
  pinMode(DC_PIN, OUTPUT);
  pinMode(RST_PIN, OUTPUT);
  pinMode(BUSY_PIN, INPUT);
  
  digitalWrite(CS_PIN, HIGH);
  digitalWrite(DC_PIN, LOW);
  
  // 初始化 SPI
  SPI.begin();
  SPI.setFrequency(4000000);
  SPI.setDataMode(SPI_MODE0);
  
  Serial.println("✅ 腳位和 SPI 初始化完成");
  
  // 初始化 E-Paper
  if (initEPaper()) {
    Serial.println("✅ E-Paper 初始化成功");
    
    // 顯示測試圖案
    displayTest();
    Serial.println("✅ 測試圖案顯示完成");
    
    Serial.println("========================================");
    Serial.println("測試完成！請檢查 E-Paper 顯示內容");
  } else {
    Serial.println("❌ E-Paper 初始化失敗");
    Serial.println("請檢查硬體連接");
  }
}

void loop() {
  // 主迴圈保持空白
  delay(1000);
}

// 硬體重置
void resetEPaper() {
  Serial.print("正在重置 E-Paper...");
  digitalWrite(RST_PIN, HIGH);
  delay(200);
  digitalWrite(RST_PIN, LOW);
  delay(2);
  digitalWrite(RST_PIN, HIGH);
  delay(200);
  Serial.println(" 完成");
}

// 等待 BUSY 信號
void waitReady() {
  Serial.print("等待就緒");
  unsigned long startTime = millis();
  
  while (digitalRead(BUSY_PIN) == HIGH) {
    delay(100);
    Serial.print(".");
    
    if (millis() - startTime > 10000) {
      Serial.println(" 超時!");
      return;
    }
  }
  Serial.println(" OK");
}

// 發送命令
void sendCommand(uint8_t cmd) {
  digitalWrite(DC_PIN, LOW);
  digitalWrite(CS_PIN, LOW);
  SPI.transfer(cmd);
  digitalWrite(CS_PIN, HIGH);
}

// 發送資料
void sendData(uint8_t data) {
  digitalWrite(DC_PIN, HIGH);
  digitalWrite(CS_PIN, LOW);
  SPI.transfer(data);
  digitalWrite(CS_PIN, HIGH);
}

// 初始化 E-Paper
bool initEPaper() {
  Serial.println("開始初始化 E-Paper...");
  
  // 重置
  resetEPaper();
  waitReady();
  
  // 軟體重置
  Serial.println("1. 軟體重置");
  sendCommand(CMD_SW_RESET);
  waitReady();
  
  // 驅動輸出控制
  Serial.println("2. 設定驅動輸出");
  sendCommand(CMD_DRIVER_OUTPUT_CONTROL);
  sendData(0xDF);  // HEIGHT-1 低位元組 (479 = 0x01DF)
  sendData(0x01);  // HEIGHT-1 高位元組
  sendData(0x00);  // GD=0, SM=0, TB=0
  
  // 資料輸入模式
  Serial.println("3. 設定資料模式");
  sendCommand(CMD_DATA_ENTRY_MODE);
  sendData(0x03);  // X遞增, Y遞增
  
  // 設定 X 範圍
  Serial.println("4. 設定 X 範圍");
  sendCommand(CMD_SET_RAM_X_POSITION);
  sendData(0x00);        // X 起始位置
  sendData(0x63);        // X 結束位置 (99*8=792, 接近800)
  
  // 設定 Y 範圍
  Serial.println("5. 設定 Y 範圍");
  sendCommand(CMD_SET_RAM_Y_POSITION);
  sendData(0x00);        // Y 起始低位元組
  sendData(0x00);        // Y 起始高位元組
  sendData(0xDF);        // Y 結束低位元組 (479)
  sendData(0x01);        // Y 結束高位元組
  
  Serial.println("✅ E-Paper 初始化完成");
  return true;
}

// 顯示測試圖案
void displayTest() {
  Serial.println("開始顯示測試圖案...");
  
  // 設定記憶體指標
  sendCommand(CMD_SET_RAM_X_COUNTER);
  sendData(0x00);
  
  sendCommand(CMD_SET_RAM_Y_COUNTER);
  sendData(0x00);
  sendData(0x00);
  
  // 寫入影像資料
  Serial.println("寫入測試資料...");
  sendCommand(CMD_WRITE_RAM);
  
  // 簡單的測試圖案：棋盤格 + 邊框
  digitalWrite(DC_PIN, HIGH);  // 資料模式
  digitalWrite(CS_PIN, LOW);
  
  for (int y = 0; y < HEIGHT; y++) {
    for (int x = 0; x < WIDTH/8; x++) {
      uint8_t data = 0xFF;  // 預設白色
      
      // 上下邊框 (黑色)
      if (y < 5 || y >= HEIGHT-5) {
        data = 0x00;
      }
      // 左右邊框 (黑色)
      else if (x < 2 || x >= WIDTH/8-2) {
        data = 0x00;
      }
      // 棋盤格圖案
      else if ((x/4 + y/20) % 2 == 0) {
        data = 0x00;  // 黑色方塊
      }
      
      SPI.transfer(data);
    }
  }
  
  digitalWrite(CS_PIN, HIGH);
  
  // 更新顯示
  Serial.println("更新顯示...");
  sendCommand(CMD_DISPLAY_UPDATE_CONTROL_2);
  sendData(0xC4);
  
  sendCommand(CMD_MASTER_ACTIVATION);
  
  // 等待更新完成
  waitReady();
  
  Serial.println("✅ 測試圖案顯示完成");
}
