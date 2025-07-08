/*
  ESP8266 藍芽模組測試程式
  
  功能：
  - 測試藍芽模組 HC-05/HC-06 的基本功能
  - 透過序列埠設定藍芽模組
  - 顯示藍芽模組資訊
  
  硬體連接：
  - VCC → 3.3V
  - GND → GND
  - RX → D1 (GPIO5)
  - TX → D2 (GPIO4)
  
  使用方式：
  1. 上傳程式到 ESP8266
  2. 開啟序列埠監控視窗 (115200 鮑率)
  3. 輸入 AT 指令測試藍芽模組
  
  常用 AT 指令：
  - AT - 測試連接
  - AT+NAME=新名稱 - 設定設備名稱
  - AT+PSWD=新密碼 - 設定PIN碼
  - AT+VERSION? - 查詢版本
  - AT+STATE? - 查詢狀態
  
  作者：DYJ Hsieh
  日期：2025年7月
  版本：v1.0
*/

// ============================================
// 包含函式庫
// ============================================

#include <SoftwareSerial.h>

// ============================================
// 腳位定義
// ============================================

#define BT_RX_PIN    D1    // GPIO5 - 連接藍芽模組的TX腳位
#define BT_TX_PIN    D2    // GPIO4 - 連接藍芽模組的RX腳位

// ============================================
// 變數定義
// ============================================

// 藍芽序列埠
SoftwareSerial bluetooth(BT_RX_PIN, BT_TX_PIN);

// ============================================
// 初始化設定
// ============================================

void setup() {
  // 初始化序列埠
  Serial.begin(115200);
  delay(100);
  
  // 顯示啟動畫面
  showStartupScreen();
  
  // 初始化藍芽序列埠
  bluetooth.begin(9600);  // HC-05/HC-06 預設鮑率
  delay(1000);
  
  Serial.println("系統初始化完成");
  Serial.println("可以開始輸入 AT 指令測試藍芽模組");
  Serial.println("========================================");
}

// ============================================
// 主要迴圈
// ============================================

void loop() {
  // 從序列埠傳送到藍芽
  if (Serial.available()) {
    String command = Serial.readString();
    command.trim();
    
    Serial.println("發送指令: " + command);
    bluetooth.print(command);
    
    // 等待回應
    delay(1000);
    
    // 顯示藍芽回應
    if (bluetooth.available()) {
      String response = bluetooth.readString();
      Serial.println("藍芽回應: " + response);
    } else {
      Serial.println("無回應");
    }
    Serial.println("--------");
  }
  
  // 從藍芽傳送到序列埠
  if (bluetooth.available()) {
    String data = bluetooth.readString();
    Serial.println("藍芽資料: " + data);
  }
}

// ============================================
// 顯示函數
// ============================================

/*
 * 顯示啟動畫面
 */
void showStartupScreen() {
  Serial.println();
  Serial.println("########################################");
  Serial.println("#                                      #");
  Serial.println("#        ESP8266 藍芽模組測試          #");
  Serial.println("#                                      #");
  Serial.println("########################################");
  Serial.println();
  Serial.println("版本: v1.0");
  Serial.println("功能: 藍芽模組 AT 指令測試");
  Serial.println();
  Serial.println("硬體連接:");
  Serial.println("• 藍芽 VCC → ESP8266 3.3V");
  Serial.println("• 藍芽 GND → ESP8266 GND");
  Serial.println("• 藍芽 RX → ESP8266 D1 (GPIO5)");
  Serial.println("• 藍芽 TX → ESP8266 D2 (GPIO4)");
  Serial.println();
  Serial.println("常用 AT 指令:");
  Serial.println("• AT - 測試連接");
  Serial.println("• AT+NAME=DYJ_BT - 設定名稱");
  Serial.println("• AT+PSWD=1234 - 設定PIN碼");
  Serial.println("• AT+VERSION? - 查詢版本");
  Serial.println("• AT+STATE? - 查詢狀態");
  Serial.println("• AT+ROLE=0 - 設為從設備");
  Serial.println();
}
