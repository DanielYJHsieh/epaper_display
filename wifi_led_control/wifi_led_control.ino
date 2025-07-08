/*
  ESP8266 WiFi LED 控制程式
  
  功能：
  - 建立 WiFi 熱點或連接現有 WiFi
  - 透過網頁介面控制 LED
  - 手機連接時 LED 恆亮，斷線時閃爍
  
  使用方式：
  1. 上傳程式到 ESP8266
  2. 手機連接 WiFi 熱點 "DYJ_LED_Control"
  3. 瀏覽器開啟 192.168.4.1
  4. 透過網頁控制 LED
  
  硬體需求：
  - ESP8266MOD 開發板 (NodeMCU、Wemos D1 Mini 等)
  - USB 傳輸線
  
  作者：DYJ Hsieh
  日期：2025年7月
  版本：v1.0
*/

// ============================================
// 包含函式庫
// ============================================

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

// ============================================
// WiFi 設定
// ============================================

// 熱點模式設定
const char* ap_ssid = "DYJ_LED_Control";
const char* ap_password = "12345678";

// 或者連接現有 WiFi（取消註解並填入您的 WiFi 資訊）
// const char* wifi_ssid = "您的WiFi名稱";
// const char* wifi_password = "您的WiFi密碼";

// ============================================
// 腳位定義
// ============================================

#define LED_PIN LED_BUILTIN  // 板載LED腳位

// ============================================
// 變數定義
// ============================================

ESP8266WebServer server(80);

// LED 控制變數
bool isClientConnected = false;
bool ledState = false;
bool manualControl = false;  // 手動控制模式
unsigned long previousMillis = 0;
unsigned long lastClientCheck = 0;

// 閃爍時間設定 (毫秒)
const unsigned long LED_ON_TIME = 200;   // LED開啟時間
const unsigned long LED_OFF_TIME = 800;  // LED關閉時間
const unsigned long CLIENT_CHECK_INTERVAL = 1000;  // 客戶端檢查間隔

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
  
  // 設定 WiFi
  setupWiFi();
  
  // 設定網頁伺服器
  setupWebServer();
  
  // 記錄啟動時間
  startTime = millis();
  
  Serial.println("系統初始化完成");
  Serial.println("等待客戶端連接...");
  Serial.println("========================================");
}

// ============================================
// 主要迴圈
// ============================================

void loop() {
  // 處理網頁請求
  server.handleClient();
  
  // 檢查客戶端連接狀態
  checkClientConnection();
  
  // 根據連接狀態控制 LED
  controlLED();
  
  // 處理序列埠指令
  handleSerialCommands();
  
  delay(1);  // 小延遲避免 CPU 過載
}

// ============================================
// WiFi 相關函數
// ============================================

/*
 * 設定 WiFi
 */
void setupWiFi() {
  Serial.println("正在設定 WiFi...");
  
  // 設定為熱點模式
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ap_ssid, ap_password);
  
  IPAddress IP = WiFi.softAPIP();
  Serial.println("WiFi 熱點已建立");
  Serial.println("SSID: " + String(ap_ssid));
  Serial.println("密碼: " + String(ap_password));
  Serial.print("IP 位址: ");
  Serial.println(IP);
  
  // 如果要連接現有 WiFi，使用以下程式碼
  /*
  WiFi.mode(WIFI_STA);
  WiFi.begin(wifi_ssid, wifi_password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  
  Serial.println();
  Serial.println("WiFi 連接成功");
  Serial.print("IP 位址: ");
  Serial.println(WiFi.localIP());
  */
}

/*
 * 設定網頁伺服器
 */
void setupWebServer() {
  // 主頁面
  server.on("/", handleRoot);
  
  // LED 控制 API
  server.on("/led/on", handleLEDOn);
  server.on("/led/off", handleLEDOff);
  server.on("/led/toggle", handleLEDToggle);
  server.on("/status", handleStatus);
  
  // 啟動伺服器
  server.begin();
  Serial.println("網頁伺服器已啟動");
}

/*
 * 檢查客戶端連接狀態
 */
void checkClientConnection() {
  unsigned long currentMillis = millis();
  
  if (currentMillis - lastClientCheck >= CLIENT_CHECK_INTERVAL) {
    lastClientCheck = currentMillis;
    
    bool previousState = isClientConnected;
    
    // 檢查是否有客戶端連接到熱點
    isClientConnected = (WiFi.softAPgetStationNum() > 0);
    
    // 如果狀態改變，顯示訊息
    if (previousState != isClientConnected) {
      if (isClientConnected) {
        Serial.println("📱 客戶端已連接 - LED 恆亮模式");
        connectionCount++;
      } else {
        Serial.println("📱 客戶端已斷線 - LED 閃爍模式");
        manualControl = false;  // 重置手動控制
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
  if (isClientConnected && !manualControl) {
    // 連接時 LED 恆亮
    digitalWrite(LED_PIN, LOW);  // ESP8266 反向邏輯：LOW = 開啟
    ledState = true;
  } else if (!isClientConnected) {
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
 * 根據連接狀態控制 LED
 */
void controlLED() {
  if (manualControl) {
    // 手動控制模式，不自動改變
    return;
  }
  
  if (isClientConnected) {
    // 客戶端已連接：LED 恆亮
    if (!ledState) {
      digitalWrite(LED_PIN, LOW);  // ESP8266 反向邏輯：LOW = 開啟
      ledState = true;
    }
  } else {
    // 客戶端未連接：閃爍模式 (亮200ms，暗800ms)
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
// 網頁處理函數
// ============================================

/*
 * 主頁面
 */
void handleRoot() {
  String html = "<!DOCTYPE html><html><head>";
  html += "<meta charset='UTF-8'>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<title>DYJ LED 控制器</title>";
  html += "<style>";
  html += "body { font-family: Arial; text-align: center; margin: 20px; background: #f0f0f0; }";
  html += ".container { max-width: 400px; margin: 0 auto; padding: 20px; background: white; border-radius: 10px; box-shadow: 0 4px 6px rgba(0,0,0,0.1); }";
  html += ".button { display: inline-block; padding: 15px 25px; margin: 10px; font-size: 16px; text-decoration: none; border-radius: 5px; color: white; }";
  html += ".btn-on { background-color: #4CAF50; }";
  html += ".btn-off { background-color: #f44336; }";
  html += ".btn-toggle { background-color: #2196F3; }";
  html += ".status { margin: 20px 0; padding: 15px; background: #e7f3ff; border-radius: 5px; }";
  html += "h1 { color: #333; }";
  html += "</style>";
  html += "</head><body>";
  
  html += "<div class='container'>";
  html += "<h1>🔵 DYJ LED 控制器</h1>";
  
  html += "<div class='status'>";
  html += "<h3>目前狀態</h3>";
  html += "<p>LED: " + String(ledState ? "🔵 開啟" : "⚫ 關閉") + "</p>";
  html += "<p>連接狀態: " + String(isClientConnected ? "✅ 已連接" : "❌ 未連接") + "</p>";
  html += "<p>模式: " + String(manualControl ? "手動控制" : (isClientConnected ? "恆亮模式" : "閃爍模式")) + "</p>";
  html += "</div>";
  
  html += "<a href='/led/on' class='button btn-on'>開啟 LED</a><br>";
  html += "<a href='/led/off' class='button btn-off'>關閉 LED</a><br>";
  html += "<a href='/led/toggle' class='button btn-toggle'>切換 LED</a><br>";
  
  html += "<p><small>總閃爍次數: " + String(totalFlashes) + "</small></p>";
  html += "<p><small>連接次數: " + String(connectionCount) + "</small></p>";
  html += "<p><small>運行時間: " + String((millis() - startTime) / 1000) + " 秒</small></p>";
  
  html += "</div>";
  
  html += "<script>";
  html += "setTimeout(function() { location.reload(); }, 5000);";  // 每5秒重新整理
  html += "</script>";
  
  html += "</body></html>";
  
  server.send(200, "text/html", html);
}

/*
 * 開啟 LED
 */
void handleLEDOn() {
  digitalWrite(LED_PIN, LOW);  // ESP8266 反向邏輯
  ledState = true;
  manualControl = true;
  Serial.println("網頁控制: LED 開啟");
  server.sendHeader("Location", "/");
  server.send(302, "text/plain", "");
}

/*
 * 關閉 LED
 */
void handleLEDOff() {
  digitalWrite(LED_PIN, HIGH);  // ESP8266 反向邏輯
  ledState = false;
  manualControl = true;
  Serial.println("網頁控制: LED 關閉");
  server.sendHeader("Location", "/");
  server.send(302, "text/plain", "");
}

/*
 * 切換 LED
 */
void handleLEDToggle() {
  ledState = !ledState;
  digitalWrite(LED_PIN, ledState ? LOW : HIGH);  // ESP8266 反向邏輯
  manualControl = true;
  Serial.println("網頁控制: LED " + String(ledState ? "開啟" : "關閉"));
  server.sendHeader("Location", "/");
  server.send(302, "text/plain", "");
}

/*
 * 狀態 API
 */
void handleStatus() {
  String json = "{";
  json += "\"led\":" + String(ledState ? "true" : "false") + ",";
  json += "\"connected\":" + String(isClientConnected ? "true" : "false") + ",";
  json += "\"manual\":" + String(manualControl ? "true" : "false") + ",";
  json += "\"flashes\":" + String(totalFlashes) + ",";
  json += "\"connections\":" + String(connectionCount) + ",";
  json += "\"uptime\":" + String((millis() - startTime) / 1000);
  json += "}";
  
  server.send(200, "application/json", json);
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
    } else if (command == "on") {
      digitalWrite(LED_PIN, LOW);
      ledState = true;
      manualControl = true;
      Serial.println("LED 開啟");
    } else if (command == "off") {
      digitalWrite(LED_PIN, HIGH);
      ledState = false;
      manualControl = true;
      Serial.println("LED 關閉");
    } else if (command == "auto") {
      manualControl = false;
      Serial.println("切換到自動模式");
    } else if (command == "clear") {
      clearStatistics();
    } else if (command.length() > 0) {
      Serial.println("未知指令: " + command);
      Serial.println("輸入 'help' 查看可用指令");
    }
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
  Serial.println("#      ESP8266 WiFi LED 控制程式       #");
  Serial.println("#                                      #");
  Serial.println("########################################");
  Serial.println();
  Serial.println("版本: v1.0");
  Serial.println("WiFi 熱點: DYJ_LED_Control");
  Serial.println("功能: 根據 WiFi 連接狀態控制 LED");
  Serial.println();
  Serial.println("連接狀態:");
  Serial.println("• 有客戶端連接 → LED 恆亮");
  Serial.println("• 無客戶端連接 → LED 閃爍 (200ms亮/800ms暗)");
  Serial.println();
}

/*
 * 顯示目前狀態
 */
void showStatus() {
  Serial.println("========================================");
  Serial.println("              目前狀態");
  Serial.println("========================================");
  Serial.print("WiFi 狀態: ");
  Serial.println("熱點模式");
  Serial.print("客戶端連接: ");
  Serial.println(isClientConnected ? "✅ 已連接" : "❌ 未連接");
  Serial.print("客戶端數量: ");
  Serial.println(WiFi.softAPgetStationNum());
  Serial.print("LED 狀態: ");
  Serial.println(ledState ? "🔵 開啟" : "⚫ 關閉");
  Serial.print("控制模式: ");
  Serial.println(manualControl ? "手動控制" : "自動控制");
  Serial.print("運行模式: ");
  Serial.println(isClientConnected ? "恆亮模式" : "閃爍模式");
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
  Serial.println("WiFi 設定:");
  Serial.println("• 熱點名稱: DYJ_LED_Control");
  Serial.println("• 熱點密碼: 12345678");
  Serial.print("• IP 位址: ");
  Serial.println(WiFi.softAPIP());
  Serial.println("• 網頁控制: http://192.168.4.1");
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
  Serial.println("on          - 開啟 LED");
  Serial.println("off         - 關閉 LED");
  Serial.println("auto        - 切換到自動模式");
  Serial.println("clear       - 清除統計資料");
  Serial.println("reset (r)   - 重新啟動系統");
  Serial.println();
  Serial.println("WiFi 功能:");
  Serial.println("• 手機連接 WiFi 熱點 'DYJ_LED_Control'");
  Serial.println("• 密碼: 12345678");
  Serial.println("• 瀏覽器開啟: http://192.168.4.1");
  Serial.println("• 透過網頁控制 LED 開關");
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
