#include <SPI.h>
#include <ESP8266WiFi.h>
//EPD
#include "Display_EPD_W21_spi.h"
#include "Display_EPD_W21.h"
#include "Ap_29demo_ESP8266.h"  

// ESP8266 Wemos D1 Mini Pro pin definitions
#define EPD_BUSY_PIN  D3  // GPIO0  - BUSY
#define EPD_RST_PIN   D2  // GPIO4  - RESET 
#define EPD_DC_PIN    D4  // GPIO2  - DC 
#define EPD_CS_PIN    D0  // GPIO16 - CS

// 看門狗餵食函式
void feedWatchdog() {
  ESP.wdtFeed();
  yield();
}

// 強制執行白屏測試（跳過 BUSY 檢查）
void ForceWhiteScreenTest() {
  Serial.println("\n🚀 === FORCE WHITE SCREEN TEST (No BUSY wait) ===");
  
  // 1. 強制重置
  Serial.println("1. Force Reset...");
  digitalWrite(EPD_RST_PIN, LOW);
  delay(50);
  digitalWrite(EPD_RST_PIN, HIGH);
  delay(50);
  feedWatchdog();
  
  // 2. 軟體重置（無 BUSY 等待）
  Serial.println("2. Software Reset (no BUSY wait)...");
  digitalWrite(EPD_CS_PIN, LOW);
  delayMicroseconds(50);
  digitalWrite(EPD_DC_PIN, LOW);
  delayMicroseconds(50);
  SPI.transfer(0x12);  // SWRESET
  delayMicroseconds(50);
  digitalWrite(EPD_CS_PIN, HIGH);
  delay(200);  // 固定延遲而非等待 BUSY
  feedWatchdog();
  
  // 3. 基本設定（無 BUSY 等待）
  Serial.println("3. Basic Config (no BUSY wait)...");
  
  // 設定 1: 0x18 指令
  digitalWrite(EPD_CS_PIN, LOW);
  delayMicroseconds(50);
  digitalWrite(EPD_DC_PIN, LOW);
  delayMicroseconds(50);
  SPI.transfer(0x18);
  delayMicroseconds(50);
  digitalWrite(EPD_CS_PIN, HIGH);
  delayMicroseconds(100);
  
  digitalWrite(EPD_CS_PIN, LOW);
  delayMicroseconds(50);
  digitalWrite(EPD_DC_PIN, HIGH);
  delayMicroseconds(50);
  SPI.transfer(0x80);
  delayMicroseconds(50);
  digitalWrite(EPD_CS_PIN, HIGH);
  delay(100);
  feedWatchdog();
  
  // 4. 白屏資料填充
  Serial.println("4. Writing white screen data...");
  
  // 設定顯示區域 0x24 (黑/白資料)
  digitalWrite(EPD_CS_PIN, LOW);
  delayMicroseconds(50);
  digitalWrite(EPD_DC_PIN, LOW);
  delayMicroseconds(50);
  SPI.transfer(0x24);  // Write RAM (Black/White)
  delayMicroseconds(50);
  digitalWrite(EPD_CS_PIN, HIGH);
  delayMicroseconds(100);
  
  // 寫入白色資料 (0xFF = 白色)
  digitalWrite(EPD_CS_PIN, LOW);
  delayMicroseconds(50);
  digitalWrite(EPD_DC_PIN, HIGH);  // 資料模式
  delayMicroseconds(50);
  
  // 寫入部分白色資料來測試
  for(int i = 0; i < 100; i++) {
    SPI.transfer(0xFF);  // 白色
    delayMicroseconds(20);
    if(i % 10 == 0) {
      feedWatchdog();  // 定期餵食看門狗
    }
  }
  
  digitalWrite(EPD_CS_PIN, HIGH);
  delay(100);
  feedWatchdog();
  
  // 5. 觸發更新
  Serial.println("5. Trigger display update...");
  
  digitalWrite(EPD_CS_PIN, LOW);
  delayMicroseconds(50);
  digitalWrite(EPD_DC_PIN, LOW);
  delayMicroseconds(50);
  SPI.transfer(0x20);  // Display Update
  delayMicroseconds(50);
  digitalWrite(EPD_CS_PIN, HIGH);
  delay(500);  // 等待更新
  feedWatchdog();
  
  Serial.println("6. Force test completed!");
  Serial.println(">>> Check EPD for ANY changes (white areas, flicker, etc.) <<<");
}

void setup() {
   Serial.begin(115200);
   delay(1000);
   Serial.println("\n=== EPD FORCE TEST (Skip BUSY) ===");
   
   // 引腳設定
   pinMode(EPD_BUSY_PIN, INPUT);
   pinMode(EPD_RST_PIN, OUTPUT);
   pinMode(EPD_DC_PIN, OUTPUT);
   pinMode(EPD_CS_PIN, OUTPUT);
   
   digitalWrite(EPD_RST_PIN, HIGH);
   digitalWrite(EPD_DC_PIN, HIGH);
   digitalWrite(EPD_CS_PIN, HIGH);
   
   Serial.println("✓ Pins configured");
   
   // SPI 設定 - 更低頻率
   SPI.begin();
   SPI.setFrequency(25000);   // 25kHz - 極慢速度
   SPI.setDataMode(SPI_MODE0);
   Serial.println("✓ SPI: 25kHz, Mode 0");
   
   Serial.println("✓ Free heap: " + String(ESP.getFreeHeap()) + " bytes");
}

void loop() {
   Serial.println("\n--- Force Test Cycle ---");
   Serial.println("BUSY pin state: " + String(digitalRead(EPD_BUSY_PIN)));
   
   // 執行強制測試
   ForceWhiteScreenTest();
   
   Serial.println("\nTest completed. Check EPD screen!");
   Serial.println("Waiting 20 seconds before next cycle...");
   
   // 分段延遲並餵食看門狗
   for(int i = 0; i < 20; i++) {
     delay(1000);
     feedWatchdog();
   }
}
