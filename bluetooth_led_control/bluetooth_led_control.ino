/*
  ESP8266 藍芽 LED 控制程式
  
  功能：
  - 藍芽設備名稱：DYJ_BT
  - 有手機連接時：LED 恆亮
  - 無手機連接時：每秒閃爍（亮200ms，暗800ms）
  - 透過序列埠顯示連接狀態
  
  硬體需求：
  - ESP8266MOD 開發板 (NodeMCU、Wemos D1 Mini 等)
  - USB 傳輸線
  
  注意事項：
  - ESP8266 板載 LED 為反向邏輯 (LOW = 開啟, HIGH = 關閉)
  - 序列埠鮑率設定為 115200
  - 使用軟體序列埠與藍芽模組通訊
  
  藍芽模組連接 (如果使用外接藍芽模組)：
  - VCC → 3.3V
  - GND → GND  
  - RX → D1 (GPIO5)
  - TX → D2 (GPIO4)
  
  作者：DYJ Hsieh
  日期：2025年7月
  版本：v1.0
*/

// ============================================
// 包含函式庫
// ============================================

#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>

// ============================================
// 腳位定義
// ============================================

#define BT_RX_PIN    D1    // GPIO5 - 連接藍芽模組的TX腳位
#define BT_TX_PIN    D2    // GPIO4 - 連接藍芽模組的RX腳位
#define LED_PIN      LED_BUILTIN  // 板載LED腳位

// ============================================
// 變數定義
// ============================================

// 藍芽序列埠
SoftwareSerial bluetooth(BT_RX_PIN, BT_TX_PIN);

// LED 控制變數
bool isBluetoothConnected = false;
bool ledState = false;
unsigned long previousMillis = 0;
unsigned long lastBluetoothCheck = 0;

// 閃爍時間設定 (毫秒)
const unsigned long LED_ON_TIME = 200;   // LED開啟時間
const unsigned long LED_OFF_TIME = 800;  // LED關閉時間
const unsigned long BLUETOOTH_CHECK_INTERVAL = 1000;  // 藍芽狀態檢查間隔

// 統計變數
unsigned long totalFlashes = 0;
unsigned long connectionCount = 0;
unsigned long startTime = 0;

// ============================================
// 初始化設定
// ============================================

void setup() {
  // 初始化序列埠
  Serial.begin(115200);
  delay(100);
  
  // 顯示啟動畫面
  showStartupScreen();
  
  // 設定 LED 腳位
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);  // 初始關閉 (ESP8266反向邏輯)
  
  // 初始化藍芽序列埠
  bluetooth.begin(9600);  // HC-05/HC-06 預設鮑率
  delay(1000);
  
  // 設定藍芽模組
  setupBluetoothModule();
  
  // 記錄啟動時間
  startTime = millis();
  
  Serial.println("系統初始化完成");
  Serial.println("等待藍芽連接...");
  Serial.println("========================================");
}

// ============================================
// 主要迴圈
// ============================================

void loop() {
  // 檢查藍芽連接狀態
  checkBluetoothConnection();
  
  // 根據連接狀態控制 LED
  controlLED();
  
  // 處理序列埠指令
  handleSerialCommands();
  
  // 處理藍芽資料
  handleBluetoothData();
  
  delay(1);  // 小延遲避免 CPU 過載
}

// ============================================
// 藍芽相關函數
// ============================================

/*
 * 設定藍芽模組
 */
void setupBluetoothModule() {
  Serial.println("正在設定藍芽模組...");
  
  // 嘗試進入AT命令模式
  bluetooth.print("AT");
  delay(1000);
  
  // 設定藍芽名稱為 DYJ_BT
  bluetooth.print("AT+NAME=DYJ_BT");
  delay(1000);
  
  // 設定藍芽PIN碼 (可選)
  bluetooth.print("AT+PSWD=1234");
  delay(1000);
  
  // 設定角色為從設備
  bluetooth.print("AT+ROLE=0");
  delay(1000);
  
  Serial.println("藍芽模組設定完成");
  Serial.println("設備名稱: DYJ_BT");
  Serial.println("PIN碼: 1234 (如果需要)");
}

/*
 * 檢查藍芽連接狀態
 */
void checkBluetoothConnection() {
  unsigned long currentMillis = millis();
  
  // 每秒檢查一次連接狀態
  if (currentMillis - lastBluetoothCheck >= BLUETOOTH_CHECK_INTERVAL) {
    lastBluetoothCheck = currentMillis;
    
    bool previousState = isBluetoothConnected;
    
    // 檢查是否有資料可讀取或連接狀態
    // 這裡使用簡單的方法：如果有藍芽資料就認為已連接
    if (bluetooth.available() > 0) {
      isBluetoothConnected = true;
    } else {
      // 發送測試指令檢查連接
      bluetooth.print("AT+STATE");
      delay(100);
      
      String response = "";
      while (bluetooth.available()) {
        response += (char)bluetooth.read();
      }
      
      // 根據回應判斷連接狀態
      if (response.indexOf("CONNECTED") >= 0 || response.indexOf("OK+CONN") >= 0) {
        isBluetoothConnected = true;
      } else {
        isBluetoothConnected = false;
      }
    }
    
    // 如果狀態改變，顯示訊息
    if (previousState != isBluetoothConnected) {
      if (isBluetoothConnected) {
        Serial.println("🔵 藍芽已連接 - LED 恆亮模式");
        connectionCount++;
      } else {
        Serial.println("🔴 藍芽已斷線 - LED 閃爍模式");
      }
      
      // 立即更新 LED 狀態
      updateLEDForNewState();
    }
  }
}

/*
 * 當連接狀態改變時立即更新 LED
 */
void updateLEDForNewState() {
  if (isBluetoothConnected) {
    // 連接時 LED 恆亮
    digitalWrite(LED_PIN, LOW);  // ESP8266 反向邏輯：LOW = 開啟
    ledState = true;
  } else {
    // 斷線時重置閃爍計時
    previousMillis = millis();
    digitalWrite(LED_PIN, HIGH);  // 先關閉
    ledState = false;
  }
}

// ============================================
// LED 控制函數
// ============================================

/*
 * 根據藍芽連接狀態控制 LED
 */
void controlLED() {
  if (isBluetoothConnected) {
    // 藍芽已連接：LED 恆亮
    if (!ledState) {
      digitalWrite(LED_PIN, LOW);  // ESP8266 反向邏輯：LOW = 開啟
      ledState = true;
    }
  } else {
    // 藍芽未連接：閃爍模式 (亮200ms，暗800ms)
    flashLED();
  }
}

/*
 * LED 閃爍控制
 */
void flashLED() {
  unsigned long currentMillis = millis();
  
  if (ledState) {
    // LED 目前是亮的，檢查是否該關閉
    if (currentMillis - previousMillis >= LED_ON_TIME) {
      digitalWrite(LED_PIN, HIGH);  // 關閉 LED
      ledState = false;
      previousMillis = currentMillis;
      totalFlashes++;
    }
  } else {
    // LED 目前是暗的，檢查是否該開啟
    if (currentMillis - previousMillis >= LED_OFF_TIME) {
      digitalWrite(LED_PIN, LOW);   // 開啟 LED
      ledState = true;
      previousMillis = currentMillis;
    }
  }
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
    
    if (command == "status" || command == "s") {
      showStatus();
    } else if (command == "info" || command == "i") {
      showSystemInfo();
    } else if (command == "help" || command == "h") {
      showHelp();
    } else if (command == "reset" || command == "r") {
      Serial.println("重新啟動 ESP8266...");
      delay(1000);
      ESP.restart();
    } else if (command == "test") {
      testBluetoothModule();
    } else if (command == "clear") {
      clearStatistics();
    } else if (command.length() > 0) {
      Serial.println("未知指令: " + command);
      Serial.println("輸入 'help' 查看可用指令");
    }
  }
}

/*
 * 處理藍芽資料
 */
void handleBluetoothData() {
  if (bluetooth.available()) {
    String data = bluetooth.readString();
    data.trim();
    
    Serial.println("收到藍芽資料: " + data);
    
    // 回傳確認訊息
    bluetooth.println("ESP8266 收到: " + data);
    
    // 轉換為小寫以便比較
    data.toLowerCase();
    
    // 處理特殊指令
    if (data == "status") {
      bluetooth.println("LED狀態: " + String(ledState ? "開啟" : "關閉"));
      bluetooth.println("總閃爍次數: " + String(totalFlashes));
    }
  }
}

// ============================================
// 測試和資訊顯示函數
// ============================================

/*
 * 測試藍芽模組
 */
void testBluetoothModule() {
  Serial.println("========================================");
  Serial.println("          藍芽模組測試");
  Serial.println("========================================");
  
  bluetooth.print("AT");
  delay(1000);
  
  if (bluetooth.available()) {
    String response = bluetooth.readString();
    Serial.println("藍芽模組回應: " + response);
    Serial.println("✅ 藍芽模組正常");
  } else {
    Serial.println("❌ 藍芽模組無回應");
    Serial.println("請檢查連接線路");
  }
  
  Serial.println("========================================");
}

/*
 * 顯示啟動畫面
 */
void showStartupScreen() {
  Serial.println();
  Serial.println("########################################");
  Serial.println("#                                      #");
  Serial.println("#      ESP8266 藍芽 LED 控制程式       #");
  Serial.println("#                                      #");
  Serial.println("########################################");
  Serial.println();
  Serial.println("版本: v1.0");
  Serial.println("藍芽設備名稱: DYJ_BT");
  Serial.println("功能: 根據藍芽連接狀態控制 LED");
  Serial.println();
  Serial.println("連接狀態:");
  Serial.println("• 已連接 → LED 恆亮");
  Serial.println("• 未連接 → LED 閃爍 (200ms亮/800ms暗)");
  Serial.println();
}

/*
 * 顯示目前狀態
 */
void showStatus() {
  Serial.println("========================================");
  Serial.println("              目前狀態");
  Serial.println("========================================");
  Serial.print("藍芽連接: ");
  Serial.println(isBluetoothConnected ? "✅ 已連接" : "❌ 未連接");
  Serial.print("LED 狀態: ");
  Serial.println(ledState ? "🔵 開啟" : "⚫ 關閉");
  Serial.print("運行模式: ");
  Serial.println(isBluetoothConnected ? "恆亮模式" : "閃爍模式");
  Serial.print("總閃爍次數: ");
  Serial.println(totalFlashes);
  Serial.print("連接次數: ");
  Serial.println(connectionCount);
  Serial.print("運行時間: ");
  Serial.print((millis() - startTime) / 1000);
  Serial.println(" 秒");
  Serial.println("========================================");
}

/*
 * 顯示系統資訊
 */
void showSystemInfo() {
  Serial.println("========================================");
  Serial.println("              系統資訊");
  Serial.println("========================================");
  Serial.print("晶片 ID: 0x");
  Serial.println(ESP.getChipId(), HEX);
  Serial.print("Flash 大小: ");
  Serial.print(ESP.getFlashChipSize() / 1024 / 1024);
  Serial.println(" MB");
  Serial.print("CPU 頻率: ");
  Serial.print(ESP.getCpuFreqMHz());
  Serial.println(" MHz");
  Serial.print("可用記憶體: ");
  Serial.print(ESP.getFreeHeap());
  Serial.println(" bytes");
  Serial.println();
  Serial.println("藍芽設定:");
  Serial.println("• 設備名稱: DYJ_BT");
  Serial.println("• 通訊鮑率: 9600");
  Serial.println("• RX 腳位: D1 (GPIO5)");
  Serial.println("• TX 腳位: D2 (GPIO4)");
  Serial.println("========================================");
}

/*
 * 顯示說明
 */
void showHelp() {
  Serial.println("========================================");
  Serial.println("              指令說明");
  Serial.println("========================================");
  Serial.println("status (s)  - 顯示目前狀態");
  Serial.println("info (i)    - 顯示系統資訊");
  Serial.println("help (h)    - 顯示此說明");
  Serial.println("test        - 測試藍芽模組");
  Serial.println("clear       - 清除統計資料");
  Serial.println("reset (r)   - 重新啟動系統");
  Serial.println();
  Serial.println("藍芽功能:");
  Serial.println("• 用手機搜尋 'DYJ_BT' 並配對");
  Serial.println("• 連接後 LED 會恆亮");
  Serial.println("• 斷線後 LED 會閃爍");
  Serial.println("• 可透過藍芽發送指令與 ESP8266 通訊");
  Serial.println("========================================");
}

/*
 * 清除統計資料
 */
void clearStatistics() {
  totalFlashes = 0;
  connectionCount = 0;
  startTime = millis();
  Serial.println("📊 統計資料已清除");
}
