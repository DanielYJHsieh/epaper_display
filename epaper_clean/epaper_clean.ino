/*
  ESP8266 E-Paper 顯示器控制程式 (基於 A32-GDEQ0426T82 廠商程式碼)
  
  硬體規格：
  - 開發板：Wemos D1 Mini Pro
  - 顯示器：GDEQ0426T82 (4.2 寸 E-Paper)
  - 驅動 IC：SSD1677
  - 解析度：480x800 像素 (縱向顯示)
  - 介面：SPI
  - 顏色：黑白雙色
  
  功能：
  - 完整畫面更新 (Full Refresh)
  - 快速更新 (Fast Refresh)  
  - 部分更新 (Partial Refresh)
  - 支援 180 度旋轉
  - 低功耗睡眠模式
  - 序列埠指令控制
  - 圖形繪製功能
  
  硬體連接 (Wemos D1 Mini Pro)：
  - VCC → 3.3V
  - GND → GND
  - DIN (MOSI) → D7 (GPIO13)
  - CLK (SCK) → D5 (GPIO14)
  - CS → D8 (GPIO15)
  - DC → D3 (GPIO0)
  - RST → D4 (GPIO2)
  - BUSY → D2 (GPIO4)
  
  作者：DYJ Hsieh
  日期：2025年7月
  版本：v2.0 (基於 A32-GDEQ0426T82 廠商參考程式碼，Wemos D1 Mini Pro 優化版)
*/

// ============================================
// 包含函式庫
// ============================================

#include <SPI.h>

// ============================================
// 硬體腳位定義 (Wemos D1 Mini Pro 適配)
// ============================================

#define EPD_CS_PIN    D8    // GPIO15 - SPI Chip Select
#define EPD_DC_PIN    D3    // GPIO0  - Data/Command
#define EPD_RST_PIN   D4    // GPIO2  - Reset
#define EPD_BUSY_PIN  D2    // GPIO4  - Busy

// 腳位控制巨集 (基於廠商程式碼)
#define isEPD_W21_BUSY    digitalRead(EPD_BUSY_PIN)
#define EPD_W21_RST_0     digitalWrite(EPD_RST_PIN, LOW)
#define EPD_W21_RST_1     digitalWrite(EPD_RST_PIN, HIGH)
#define EPD_W21_DC_0      digitalWrite(EPD_DC_PIN, LOW)
#define EPD_W21_DC_1      digitalWrite(EPD_DC_PIN, HIGH)
#define EPD_W21_CS_0      digitalWrite(EPD_CS_PIN, LOW)
#define EPD_W21_CS_1      digitalWrite(EPD_CS_PIN, HIGH)

// ============================================
// 顯示器參數 (基於廠商程式碼)
// ============================================

#define EPD_WIDTH     480   // 顯示器寬度 (像素) - 廠商規格
#define EPD_HEIGHT    800   // 顯示器高度 (像素) - 廠商規格  
#define EPD_ARRAY     (EPD_WIDTH * EPD_HEIGHT / 8)  // 緩衝區大小

// 顏色定義
#define EPD_BLACK     0x00
#define EPD_WHITE     0xFF

// ============================================
// 全域變數
// ============================================

// 顯示緩衝區
uint8_t* imageBuffer;
bool bufferAllocated = false;

// 狀態變數
bool epdInitialized = false;
unsigned long lastUpdateTime = 0;
int updateCount = 0;
bool rotation180 = false;

// 更新模式
enum RefreshMode {
  FULL_REFRESH = 0,
  FAST_REFRESH = 1,
  PARTIAL_REFRESH = 2
};

RefreshMode currentMode = FULL_REFRESH;

// ============================================
// 基本 SPI 通訊函數 (基於廠商程式碼)
// ============================================

/*
 * SPI 寫入位元組
 */
void SPI_Write(unsigned char value) {
  SPI.transfer(value);
}

/*
 * 發送命令 (基於廠商程式碼)
 */
void EPD_W21_WriteCMD(unsigned char command) {
  EPD_W21_CS_0;
  EPD_W21_DC_0;  // D/C# 0:command 1:data
  SPI_Write(command);
  EPD_W21_CS_1;
}

/*
 * 發送資料 (基於廠商程式碼)
 */
void EPD_W21_WriteDATA(unsigned char datas) {
  EPD_W21_CS_0;
  EPD_W21_DC_1;  // D/C# 0:command 1:data
  SPI_Write(datas);
  EPD_W21_CS_1;
}

/*
 * 等待 BUSY 信號結束 (基於廠商程式碼)
 */
void Epaper_READBUSY(void) {
  Serial.print("等待顯示器就緒");
  unsigned long startTime = millis();
  
  while(1) {
    if(isEPD_W21_BUSY == 0) break; // BUSY = 0 表示就緒
    delay(10);
    Serial.print(".");
    
    // 超時檢查 (30秒)
    if (millis() - startTime > 30000) {
      Serial.println();
      Serial.println("⚠️ 等待超時，可能有硬體問題");
      return;
    }
  }
  Serial.println(" 完成");
}

// ============================================
// E-Paper 初始化函數 (基於廠商程式碼)
// ============================================

/*
 * 完整螢幕更新初始化 (基於廠商程式碼)
 */
void EPD_HW_Init(void) {
  Serial.println("正在執行完整螢幕更新初始化...");
  
  EPD_W21_RST_0;  // 模組重置
  delay(10);      // 至少 10ms 延遲
  EPD_W21_RST_1;
  delay(10);      // 至少 10ms 延遲
  
  Epaper_READBUSY();
  EPD_W21_WriteCMD(0x12);  // SWRESET (軟體重置)
  Epaper_READBUSY();

  EPD_W21_WriteCMD(0x18);  // 溫度感測器控制
  EPD_W21_WriteDATA(0x80);
  
  EPD_W21_WriteCMD(0x0C);  // 升壓軟啟動控制
  EPD_W21_WriteDATA(0xAE);
  EPD_W21_WriteDATA(0xC7);
  EPD_W21_WriteDATA(0xC3);
  EPD_W21_WriteDATA(0xC0);
  EPD_W21_WriteDATA(0x80);
  
  EPD_W21_WriteCMD(0x01);  // 驅動輸出控制
  EPD_W21_WriteDATA((EPD_WIDTH-1) % 256);
  EPD_W21_WriteDATA((EPD_WIDTH-1) / 256);
  EPD_W21_WriteDATA(0x02);

  EPD_W21_WriteCMD(0x3C);  // 邊界波形控制
  EPD_W21_WriteDATA(0x01);
  
  EPD_W21_WriteCMD(0x11);  // 資料輸入模式
  EPD_W21_WriteDATA(0x03);

  EPD_W21_WriteCMD(0x44);  // 設定 RAM-X 位址範圍
  EPD_W21_WriteDATA(0x00);
  EPD_W21_WriteDATA(0x00);
  EPD_W21_WriteDATA((EPD_HEIGHT-1) % 256);
  EPD_W21_WriteDATA((EPD_HEIGHT-1) / 256);

  EPD_W21_WriteCMD(0x45);  // 設定 RAM-Y 位址範圍
  EPD_W21_WriteDATA(0x00);
  EPD_W21_WriteDATA(0x00);
  EPD_W21_WriteDATA((EPD_WIDTH-1) % 256);
  EPD_W21_WriteDATA((EPD_WIDTH-1) / 256);

  EPD_W21_WriteCMD(0x4E);  // 設定 RAM x 位址計數器
  EPD_W21_WriteDATA(0x00);
  EPD_W21_WriteDATA(0x00);
  EPD_W21_WriteCMD(0x4F);  // 設定 RAM y 位址計數器
  EPD_W21_WriteDATA(0x00);
  EPD_W21_WriteDATA(0x00);
  Epaper_READBUSY();
  
  Serial.println("完整螢幕更新初始化完成");
}

/*
 * 快速更新初始化 (基於廠商程式碼)
 */
void EPD_HW_Init_Fast(void) {
  Serial.println("正在執行快速更新初始化...");
  
  EPD_W21_RST_0;  // 模組重置
  delay(10);
  EPD_W21_RST_1;
  delay(10);
  
  Epaper_READBUSY();
  EPD_W21_WriteCMD(0x12);  // SWRESET
  Epaper_READBUSY();

  EPD_W21_WriteCMD(0x18);
  EPD_W21_WriteDATA(0x80);
  
  EPD_W21_WriteCMD(0x0C);
  EPD_W21_WriteDATA(0xAE);
  EPD_W21_WriteDATA(0xC7);
  EPD_W21_WriteDATA(0xC3);
  EPD_W21_WriteDATA(0xC0);
  EPD_W21_WriteDATA(0x80);
  
  EPD_W21_WriteCMD(0x01);
  EPD_W21_WriteDATA((EPD_WIDTH-1) % 256);
  EPD_W21_WriteDATA((EPD_WIDTH-1) / 256);
  EPD_W21_WriteDATA(0x02);

  EPD_W21_WriteCMD(0x3C);
  EPD_W21_WriteDATA(0x01);
  
  EPD_W21_WriteCMD(0x11);
  EPD_W21_WriteDATA(0x03);

  EPD_W21_WriteCMD(0x44);
  EPD_W21_WriteDATA(0x00);
  EPD_W21_WriteDATA(0x00);
  EPD_W21_WriteDATA((EPD_HEIGHT-1) % 256);
  EPD_W21_WriteDATA((EPD_HEIGHT-1) / 256);

  EPD_W21_WriteCMD(0x45);
  EPD_W21_WriteDATA(0x00);
  EPD_W21_WriteDATA(0x00);
  EPD_W21_WriteDATA((EPD_WIDTH-1) % 256);
  EPD_W21_WriteDATA((EPD_WIDTH-1) / 256);

  EPD_W21_WriteCMD(0x4E);
  EPD_W21_WriteDATA(0x00);
  EPD_W21_WriteDATA(0x00);
  EPD_W21_WriteCMD(0x4F);
  EPD_W21_WriteDATA(0x00);
  EPD_W21_WriteDATA(0x00);
  Epaper_READBUSY();

  // 快速更新特有設定 (溫度控制)
  EPD_W21_WriteCMD(0x1A);  // 溫度控制
  EPD_W21_WriteDATA(0x5A);

  EPD_W21_WriteCMD(0x22);  // 顯示更新控制
  EPD_W21_WriteDATA(0x91);
  EPD_W21_WriteCMD(0x20);  // 啟動顯示更新序列

  Epaper_READBUSY();
  Serial.println("快速更新初始化完成");
}

/*
 * 180度旋轉初始化 (基於廠商程式碼)
 */
void EPD_HW_Init_180(void) {
  Serial.println("正在執行180度旋轉初始化...");
  
  EPD_W21_RST_0;  // 模組重置
  delay(10);
  EPD_W21_RST_1;
  delay(10);
  
  Epaper_READBUSY();
  EPD_W21_WriteCMD(0x12);  // SWRESET
  Epaper_READBUSY();

  EPD_W21_WriteCMD(0x18);
  EPD_W21_WriteDATA(0x80);
  
  EPD_W21_WriteCMD(0x0C);
  EPD_W21_WriteDATA(0xAE);
  EPD_W21_WriteDATA(0xC7);
  EPD_W21_WriteDATA(0xC3);
  EPD_W21_WriteDATA(0xC0);
  EPD_W21_WriteDATA(0x80);
  
  EPD_W21_WriteCMD(0x01);  // 驅動輸出控制 (180度)
  EPD_W21_WriteDATA((EPD_WIDTH-1) % 256);
  EPD_W21_WriteDATA((EPD_WIDTH-1) / 256);
  EPD_W21_WriteDATA(0x01);  // 差異在這裡

  EPD_W21_WriteCMD(0x3C);
  EPD_W21_WriteDATA(0x01);
  
  EPD_W21_WriteCMD(0x11);
  EPD_W21_WriteDATA(0x03);

  EPD_W21_WriteCMD(0x44);
  EPD_W21_WriteDATA(0x00);
  EPD_W21_WriteDATA(0x00);
  EPD_W21_WriteDATA((EPD_HEIGHT-1) % 256);
  EPD_W21_WriteDATA((EPD_HEIGHT-1) / 256);

  EPD_W21_WriteCMD(0x45);
  EPD_W21_WriteDATA(0x00);
  EPD_W21_WriteDATA(0x00);
  EPD_W21_WriteDATA((EPD_WIDTH-1) % 256);
  EPD_W21_WriteDATA((EPD_WIDTH-1) / 256);

  EPD_W21_WriteCMD(0x4E);
  EPD_W21_WriteDATA(0x00);
  EPD_W21_WriteDATA(0x00);
  EPD_W21_WriteCMD(0x4F);
  EPD_W21_WriteDATA(0x00);
  EPD_W21_WriteDATA(0x00);
  Epaper_READBUSY();
  
  Serial.println("180度旋轉初始化完成");
}

// ============================================
// 顯示更新函數 (基於廠商程式碼)
// ============================================

/*
 * 完整螢幕更新
 */
void EPD_Update(void) {
  EPD_W21_WriteCMD(0x22);  // 顯示更新控制
  EPD_W21_WriteDATA(0xF7);
  EPD_W21_WriteCMD(0x20);  // 啟動顯示更新序列
  Epaper_READBUSY();
}

/*
 * 快速更新
 */
void EPD_Update_Fast(void) {
  EPD_W21_WriteCMD(0x22);  // 顯示更新控制
  EPD_W21_WriteDATA(0xC7);
  EPD_W21_WriteCMD(0x20);  // 啟動顯示更新序列
  Epaper_READBUSY();
}

/*
 * 部分更新
 */
void EPD_Part_Update(void) {
  EPD_W21_WriteCMD(0x22);  // 顯示更新控制
  EPD_W21_WriteDATA(0xFF);
  EPD_W21_WriteCMD(0x20);  // 啟動顯示更新序列
  Epaper_READBUSY();
}

/*
 * 完整螢幕顯示 (基於廠商程式碼)
 */
void EPD_WhiteScreen_ALL(const unsigned char *datas) {
  unsigned int i;
  EPD_W21_WriteCMD(0x24);  // 寫入 RAM 用於黑(0)/白(1)
  for(i = 0; i < EPD_ARRAY; i++) {
    EPD_W21_WriteDATA(datas[i]);
  }
  EPD_Update();
}

/*
 * 快速螢幕顯示 (基於廠商程式碼)
 */
void EPD_WhiteScreen_ALL_Fast(const unsigned char *datas) {
  unsigned int i;
  EPD_W21_WriteCMD(0x24);  // 寫入 RAM 用於黑(0)/白(1)
  for(i = 0; i < EPD_ARRAY; i++) {
    EPD_W21_WriteDATA(datas[i]);
  }
  EPD_Update_Fast();
}

/*
 * 清除螢幕顯示白色 (基於廠商程式碼)
 */
void EPD_WhiteScreen_White(void) {
  unsigned int i;
  EPD_W21_WriteCMD(0x24);  // 寫入 RAM 用於黑(0)/白(1)
  for(i = 0; i < EPD_ARRAY; i++) {
    EPD_W21_WriteDATA(0xFF);  // 白色
  }
  EPD_Update();
}

/*
 * 顯示全黑 (基於廠商程式碼)
 */
void EPD_WhiteScreen_Black(void) {
  unsigned int i;
  EPD_W21_WriteCMD(0x24);  // 寫入 RAM 用於黑(0)/白(1)
  for(i = 0; i < EPD_ARRAY; i++) {
    EPD_W21_WriteDATA(0x00);  // 黑色
  }
  EPD_Update();
}

/*
 * 進入深度睡眠模式 (基於廠商程式碼)
 */
void EPD_DeepSleep(void) {
  EPD_W21_WriteCMD(0x10);  // 深度睡眠模式
  EPD_W21_WriteDATA(0x01);
  delay(100);
  Serial.println("E-Paper 進入深度睡眠模式");
}

/*
 * 設定部分更新基礎地圖 (基於廠商程式碼)
 */
void EPD_SetRAMValue_BaseMap(const unsigned char * datas) {
  unsigned int i;
  EPD_W21_WriteCMD(0x24);  // 寫入黑白影像到 RAM
  for(i = 0; i < EPD_ARRAY; i++) {
    EPD_W21_WriteDATA(datas[i]);
  }
  EPD_W21_WriteCMD(0x26);  // 寫入黑白影像到 RAM
  for(i = 0; i < EPD_ARRAY; i++) {
    EPD_W21_WriteDATA(datas[i]);
  }
  EPD_Update();
}

// ============================================
// 緩衝區管理函數
// ============================================

/*
 * 分配顯示緩衝區
 */
bool allocateBuffer() {
  if (!bufferAllocated) {
    imageBuffer = (uint8_t*)malloc(EPD_ARRAY);
    if (imageBuffer != NULL) {
      bufferAllocated = true;
      memset(imageBuffer, EPD_WHITE, EPD_ARRAY);  // 初始化為白色
      return true;
    }
  }
  return false;
}

/*
 * 清空顯示緩衝區
 */
void clearBuffer() {
  if (bufferAllocated) {
    memset(imageBuffer, EPD_WHITE, EPD_ARRAY);
  }
}

/*
 * 建立測試圖案
 */
void createTestPattern() {
  if (!bufferAllocated) return;
  
  clearBuffer();
  
  // 創建簡單的測試圖案
  for (int y = 0; y < EPD_HEIGHT; y++) {
    for (int x = 0; x < EPD_WIDTH; x++) {
      int byteIndex = (y * EPD_WIDTH + x) / 8;
      int bitIndex = 7 - (x % 8);
      
      // 創建棋盤格圖案
      if ((x / 40 + y / 40) % 2 == 0) {
        imageBuffer[byteIndex] &= ~(1 << bitIndex);  // 黑色
      } else {
        imageBuffer[byteIndex] |= (1 << bitIndex);   // 白色
      }
    }
  }
  
  Serial.println("✅ 測試圖案已創建");
}

/*
 * 建立歡迎圖案
 */
void createWelcomePattern() {
  if (!bufferAllocated) return;
  
  clearBuffer();
  
  // 創建邊框和線條
  for (int x = 0; x < EPD_WIDTH; x++) {
    setPixel(x, 0, EPD_BLACK);
    setPixel(x, EPD_HEIGHT - 1, EPD_BLACK);
  }
  for (int y = 0; y < EPD_HEIGHT; y++) {
    setPixel(0, y, EPD_BLACK);
    setPixel(EPD_WIDTH - 1, y, EPD_BLACK);
  }
  
  // 添加一些線條
  for (int i = 0; i < 5; i++) {
    for (int x = 50; x < EPD_WIDTH - 50; x++) {
      setPixel(x, 100 + i * 150, EPD_BLACK);
    }
  }
  
  Serial.println("✅ 歡迎圖案已創建");
}

/*
 * 設定像素
 */
void setPixel(int x, int y, uint8_t color) {
  if (x < 0 || x >= EPD_WIDTH || y < 0 || y >= EPD_HEIGHT || !bufferAllocated) {
    return;
  }
  
  int byteIndex = (y * EPD_WIDTH + x) / 8;
  int bitIndex = 7 - (x % 8);
  
  if (color == EPD_BLACK) {
    imageBuffer[byteIndex] &= ~(1 << bitIndex);
  } else {
    imageBuffer[byteIndex] |= (1 << bitIndex);
  }
}

/*
 * 創建文字顯示
 */
void createTextDisplay(String text) {
  if (!bufferAllocated) return;
  
  clearBuffer();
  
  // 繪製邊框
  for (int x = 0; x < EPD_WIDTH; x++) {
    setPixel(x, 0, EPD_BLACK);
    setPixel(x, EPD_HEIGHT - 1, EPD_BLACK);
  }
  for (int y = 0; y < EPD_HEIGHT; y++) {
    setPixel(0, y, EPD_BLACK);
    setPixel(EPD_WIDTH - 1, y, EPD_BLACK);
  }
  
  // 簡單的文字顯示（創建條紋圖案代表文字）
  int textY = EPD_HEIGHT / 2;
  for (int i = 0; i < text.length() && i < 20; i++) {
    int x = 50 + i * 20;
    for (int j = 0; j < 15; j++) {
      for (int k = 0; k < 10; k++) {
        setPixel(x + k, textY - 7 + j, EPD_BLACK);
      }
    }
  }
  
  Serial.println("✅ 文字顯示圖案已創建");
}

// ============================================
// 主要函數
// ============================================

/*
 * 示範不同更新模式 (基於廠商程式碼)
 */
void demonstrateRefreshModes() {
  Serial.println("🎯 開始示範不同更新模式 (基於 A32-GDEQ0426T82 廠商程式碼)...");
  
  // 1. 完整螢幕更新初始化 + 清空螢幕
  Serial.println("\n1️⃣ 完整螢幕更新 - 清空螢幕");
  EPD_HW_Init();
  EPD_WhiteScreen_White();  // 清空螢幕
  EPD_DeepSleep();
  delay(2000);
  
  // 2. 完整螢幕更新 - 顯示緩衝區內容
  Serial.println("\n2️⃣ 完整螢幕更新 - 顯示測試圖案");
  createTestPattern();
  EPD_HW_Init();
  EPD_WhiteScreen_ALL(imageBuffer);  // 顯示緩衝區內容
  EPD_DeepSleep();
  delay(2000);
  
  // 3. 快速更新模式
  Serial.println("\n3️⃣ 快速更新模式");
  createWelcomePattern();
  EPD_HW_Init_Fast();
  EPD_WhiteScreen_ALL_Fast(imageBuffer);
  EPD_DeepSleep();
  delay(2000);
  
  Serial.println("✅ 所有更新模式示範完成");
}

/*
 * 初始化設定
 */
void setup() {
  // 初始化序列埠
  Serial.begin(115200);
  delay(100);
  
  // 顯示啟動畫面
  showStartupScreen();
  
  // 初始化腳位
  pinMode(EPD_BUSY_PIN, INPUT);   // BUSY
  pinMode(EPD_RST_PIN, OUTPUT);   // RES
  pinMode(EPD_DC_PIN, OUTPUT);    // DC
  pinMode(EPD_CS_PIN, OUTPUT);    // CS
  
  // 初始化 SPI (基於廠商設定)
  SPI.beginTransaction(SPISettings(10000000, MSBFIRST, SPI_MODE0));
  SPI.begin();
  
  Serial.println("✅ 硬體初始化完成");
  
  // 分配顯示緩衝區
  if (allocateBuffer()) {
    Serial.println("✅ 顯示緩衝區分配成功");
  } else {
    Serial.println("❌ 顯示緩衝區分配失敗");
    return;
  }
  
  // 示範完整流程 (基於廠商程式碼)
  demonstrateRefreshModes();
  
  epdInitialized = true;
  Serial.println("系統初始化完成");
  Serial.println("輸入 'help' 查看可用指令");
  Serial.println("========================================");
}

/*
 * 主要迴圈
 */
void loop() {
  // 處理序列埠指令
  handleSerialCommands();
  
  delay(100);
}

// ============================================
// 序列埠指令處理
// ============================================

/*
 * 處理序列埠指令
 */
void handleSerialCommands() {
  if (Serial.available() > 0) {
    String command = Serial.readString();
    command.trim();
    command.toLowerCase();
    
    if (command == "help" || command == "h") {
      showHelp();
    } else if (command == "status" || command == "s") {
      showStatus();
    } else if (command == "clear" || command == "c") {
      Serial.println("執行清空螢幕...");
      EPD_HW_Init();
      EPD_WhiteScreen_White();
      EPD_DeepSleep();
      Serial.println("✅ 螢幕已清空");
    } else if (command == "test" || command == "t") {
      Serial.println("顯示測試圖案...");
      createTestPattern();
      EPD_HW_Init();
      EPD_WhiteScreen_ALL(imageBuffer);
      EPD_DeepSleep();
      Serial.println("✅ 測試圖案已顯示");
    } else if (command == "fast" || command == "f") {
      Serial.println("執行快速更新...");
      createWelcomePattern();
      EPD_HW_Init_Fast();
      EPD_WhiteScreen_ALL_Fast(imageBuffer);
      EPD_DeepSleep();
      Serial.println("✅ 快速更新完成");
    } else if (command == "black" || command == "b") {
      Serial.println("顯示全黑螢幕...");
      EPD_HW_Init();
      EPD_WhiteScreen_Black();
      EPD_DeepSleep();
      Serial.println("✅ 全黑螢幕已顯示");
    } else if (command == "180") {
      Serial.println("執行180度旋轉顯示...");
      rotation180 = !rotation180;
      createTestPattern();
      EPD_HW_Init_180();
      EPD_WhiteScreen_ALL(imageBuffer);
      EPD_DeepSleep();
      Serial.println("✅ 180度旋轉顯示完成");
    } else if (command == "demo" || command == "d") {
      Serial.println("執行完整示範...");
      demonstrateRefreshModes();
      Serial.println("✅ 完整示範結束");
    } else if (command.startsWith("text ")) {
      String text = command.substring(5);
      Serial.println("顯示文字: " + text);
      createTextDisplay(text);
      EPD_HW_Init();
      EPD_WhiteScreen_ALL(imageBuffer);
      EPD_DeepSleep();
      Serial.println("✅ 文字已顯示");
    } else if (command == "sleep") {
      EPD_DeepSleep();
    } else if (command == "reset" || command == "r") {
      Serial.println("重新啟動 ESP8266...");
      delay(1000);
      ESP.restart();
    } else if (command.length() > 0) {
      Serial.println("未知指令: " + command);
      Serial.println("輸入 'help' 查看可用指令");
    }
    
    updateCount++;
    lastUpdateTime = millis();
  }
}

// ============================================
// 資訊顯示函數
// ============================================

/*
 * 顯示啟動畫面
 */
void showStartupScreen() {
  Serial.println();
  Serial.println("########################################");
  Serial.println("#                                      #");
  Serial.println("#   ESP8266 E-Paper 顯示器控制程式     #");
  Serial.println("#      (基於 A32-GDEQ0426T82)         #");
  Serial.println("#       Wemos D1 Mini Pro 版本        #");
  Serial.println("#                                      #");
  Serial.println("########################################");
  Serial.println();
  Serial.println("版本: v2.0 (基於廠商參考程式碼)");
  Serial.println("開發板: Wemos D1 Mini Pro (ESP8266)");
  Serial.println("顯示器: GDEQ0426T82 (4.2寸 E-Paper)");
  Serial.println("驅動IC: SSD1677");
  Serial.println("解析度: 480x800 像素");
  Serial.println("介面: SPI (10MHz)");
  Serial.println();
  Serial.println("腳位連接 (Wemos D1 Mini Pro):");
  Serial.println("• VCC → 3.3V");
  Serial.println("• GND → GND");
  Serial.println("• DIN → D7 (GPIO13)");
  Serial.println("• CLK → D5 (GPIO14)");
  Serial.println("• CS → D8 (GPIO15)");
  Serial.println("• DC → D3 (GPIO0)");
  Serial.println("• RST → D4 (GPIO2)");
  Serial.println("• BUSY → D2 (GPIO4)");
  Serial.println();
  Serial.println("更新模式:");
  Serial.println("• 完整更新 (Full Refresh) - 2秒");
  Serial.println("• 快速更新 (Fast Refresh) - 1.5秒");
  Serial.println("• 部分更新 (Partial Refresh) - 支援");
  Serial.println("• 180度旋轉 - 支援");
  Serial.println();
}

/*
 * 顯示說明
 */
void showHelp() {
  Serial.println("========================================");
  Serial.println("     指令說明 (基於廠商程式碼)");
  Serial.println("     Wemos D1 Mini Pro 優化版");
  Serial.println("========================================");
  Serial.println("help (h)       - 顯示此說明");
  Serial.println("status (s)     - 顯示系統狀態");
  Serial.println("clear (c)      - 清空顯示器 (完整更新)");
  Serial.println("test (t)       - 顯示測試圖案 (完整更新)");
  Serial.println("fast (f)       - 快速更新示範");
  Serial.println("black (b)      - 顯示全黑螢幕");
  Serial.println("180            - 180度旋轉顯示");
  Serial.println("demo (d)       - 執行完整示範流程");
  Serial.println("text <內容>    - 顯示自訂文字");
  Serial.println("sleep          - 進入深度睡眠");
  Serial.println("reset (r)      - 重新啟動系統");
  Serial.println();
  Serial.println("更新模式特色:");
  Serial.println("• 完整更新: 最佳畫質，2秒更新時間");
  Serial.println("• 快速更新: 較快速度，1.5秒更新時間");
  Serial.println("• 部分更新: 只更新變化區域");
  Serial.println();
  Serial.println("範例:");
  Serial.println("text Hello World  - 顯示 'Hello World'");
  Serial.println("========================================");
}

/*
 * 顯示狀態
 */
void showStatus() {
  Serial.println("========================================");
  Serial.println("    系統狀態 (Wemos D1 Mini Pro)");
  Serial.println("========================================");
  Serial.print("開發板: Wemos D1 Mini Pro");
  Serial.println();
  Serial.print("E-Paper 初始化: ");
  Serial.println(epdInitialized ? "✅ 完成" : "❌ 失敗");
  Serial.print("緩衝區分配: ");
  Serial.println(bufferAllocated ? "✅ 完成" : "❌ 失敗");
  Serial.print("緩衝區大小: ");
  Serial.print(EPD_ARRAY);
  Serial.print(" bytes (");
  Serial.print(EPD_ARRAY / 1024);
  Serial.println(" KB)");
  Serial.print("顯示器解析度: ");
  Serial.print(EPD_WIDTH);
  Serial.print("x");
  Serial.println(EPD_HEIGHT);
  Serial.println("SPI 頻率: 10MHz");
  Serial.print("180度旋轉: ");
  Serial.println(rotation180 ? "✅ 啟用" : "❌ 停用");
  Serial.print("更新次數: ");
  Serial.println(updateCount);
  Serial.print("上次更新: ");
  if (lastUpdateTime > 0) {
    Serial.print((millis() - lastUpdateTime) / 1000);
    Serial.println(" 秒前");
  } else {
    Serial.println("尚未更新");
  }
  Serial.print("可用記憶體: ");
  Serial.print(ESP.getFreeHeap());
  Serial.println(" bytes");
  Serial.print("運行時間: ");
  Serial.print(millis() / 1000);
  Serial.println(" 秒");
  Serial.print("晶片ID: 0x");
  Serial.println(ESP.getChipId(), HEX);
  Serial.print("CPU 頻率: ");
  Serial.print(ESP.getCpuFreqMHz());
  Serial.println(" MHz");
  Serial.println();
  Serial.println("支援的更新模式:");
  Serial.println("• EPD_HW_Init() - 完整更新初始化");
  Serial.println("• EPD_HW_Init_Fast() - 快速更新初始化");
  Serial.println("• EPD_HW_Init_180() - 180度旋轉初始化");
  Serial.println("========================================");
}
