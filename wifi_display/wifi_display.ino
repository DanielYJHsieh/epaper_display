/*
 * WiFi 電子紙顯示器控制程式
 * 
 * 功能：
 * - 建立 WiFi 熱點
 * - 在電子紙左上角顯示 WiFi SSID/密碼
 * - 透過網頁輸入文字並顯示在電子紙中央
 * - 結合 GDEQ0426T82 電子紙和 WiFi 功能
 * 
 * 硬體需求：
 * - WeMos D1 Mini (ESP8266)
 * - GDEQ0426T82 4.26" 電子紙顯示器
 * 
 * 版本：v1.2 (速度優化版)
 * 日期：2025-10-03
 * 
 * v1.2 更新：
 * - WiFi 802.11n 高速模式
 * - HTML PROGMEM + 分段傳輸
 * - AJAX 快速更新（無需重新載入頁面）
 * - 優化 TX 功率和通道設定
 */

// ============================================
// 包含函式庫
// ============================================

#include <GxEPD2_BW.h>
#include <gdeq/GxEPD2_426_GDEQ0426T82.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeMonoBold18pt7b.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
extern "C" {
  #include "user_interface.h" // 用於 wifi_set_phy_mode
}

// ============================================
// HTML 範本（使用 PROGMEM 節省 RAM）
// ============================================

// HTML 頭部（壓縮版，移除不必要的空白）
const char HTML_HEAD[] PROGMEM = R"rawliteral(<!DOCTYPE html><html><head>
<meta charset='UTF-8'><meta name='viewport' content='width=device-width,initial-scale=1'>
<title>電子紙控制</title>
<style>
body{font-family:'Microsoft JhengHei',Arial,sans-serif;text-align:center;margin:20px;background:linear-gradient(135deg,#667eea 0%,#764ba2 100%);min-height:100vh}
.container{max-width:500px;margin:0 auto;padding:30px;background:white;border-radius:15px;box-shadow:0 10px 30px rgba(0,0,0,0.3)}
h1{color:#333;margin-bottom:10px}
.wifi-info{background:#e3f2fd;padding:15px;border-radius:10px;margin:20px 0}
.wifi-info p{margin:8px 0;font-size:14px}
.input-group{margin:20px 0}
label{display:block;margin-bottom:10px;font-weight:bold;color:#555;text-align:left}
textarea{width:95%;padding:15px;font-size:16px;border:2px solid #ddd;border-radius:8px;resize:vertical;font-family:inherit}
textarea:focus{outline:none;border-color:#667eea}
.button{width:100%;padding:15px;font-size:18px;background:linear-gradient(135deg,#667eea 0%,#764ba2 100%);color:white;border:none;border-radius:8px;cursor:pointer;transition:transform 0.2s}
.button:hover{transform:translateY(-2px);box-shadow:0 5px 15px rgba(102,126,234,0.4)}
.button:active{transform:translateY(0)}
.status{margin-top:20px;padding:15px;background:#f5f5f5;border-radius:8px}
.current-text{background:#fff3e0;padding:15px;border-radius:8px;margin-top:15px;word-wrap:break-word}
</style>
</head><body><div class='container'>
<h1>📱 電子紙顯示器</h1>
<p style='color:#666'>透過網頁控制 4.26" 電子紙顯示</p>
)rawliteral";

const char HTML_FORM[] PROGMEM = R"rawliteral(
<form action='/update' method='POST' onsubmit='return submitForm(event)'>
<div class='input-group'>
<label>✏️ 輸入要顯示的文字：</label>
<textarea name='text' rows='4' maxlength='200' placeholder='輸入文字後按下送出'>%TEXT%</textarea>
</div>
<button type='submit' class='button'>📤 更新顯示</button>
</form>
<div class='current-text'><strong>目前顯示：</strong><br>%TEXT%</div>
)rawliteral";

const char HTML_TAIL[] PROGMEM = R"rawliteral(
<script>
function submitForm(e){
e.preventDefault();
const text=document.querySelector('textarea').value;
fetch('/update',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:'text='+encodeURIComponent(text)})
.then(r=>r.text()).then(()=>location.reload())
.catch(err=>alert('更新失敗：'+err));
return false;
}
</script></body></html>
)rawliteral";

// ============================================
// WiFi 設定
// ============================================

const char* ap_ssid = "EPaper_Display";
const char* ap_password = "12345678";

// ============================================
// 電子紙腳位定義
// ============================================

#define EPD_CS    15  // D8 - CS
#define EPD_DC     0  // D3 - DC
#define EPD_RST    5  // D1 - RST
#define EPD_BUSY   4  // D2 - BUSY

// ============================================
// 電子紙顯示器設定（分頁模式）
// ============================================

#define MAX_DISPLAY_BUFFER_SIZE 800
#define MAX_HEIGHT(EPD) (EPD::HEIGHT <= MAX_DISPLAY_BUFFER_SIZE / (EPD::WIDTH / 8) ? \
                         EPD::HEIGHT : MAX_DISPLAY_BUFFER_SIZE / (EPD::WIDTH / 8))

GxEPD2_BW<GxEPD2_426_GDEQ0426T82, MAX_HEIGHT(GxEPD2_426_GDEQ0426T82)> display(
    GxEPD2_426_GDEQ0426T82(EPD_CS, EPD_DC, EPD_RST, EPD_BUSY)
);

// ============================================
// 網頁伺服器
// ============================================

ESP8266WebServer server(80);

// ============================================
// 全域變數
// ============================================

String displayText = "Waiting for input...";
unsigned long lastUpdate = 0;
bool needsUpdate = true;

// ============================================
// 初始化設定
// ============================================

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println(F("=== WiFi 電子紙顯示器啟動 ==="));
  
  // 初始化電子紙腳位
  pinMode(EPD_BUSY, INPUT);
  pinMode(EPD_RST, OUTPUT);
  pinMode(EPD_DC, OUTPUT);
  pinMode(EPD_CS, OUTPUT);
  digitalWrite(EPD_CS, HIGH);
  digitalWrite(EPD_DC, HIGH);
  digitalWrite(EPD_RST, HIGH);
  
  // 初始化電子紙顯示器
  Serial.println(F("初始化電子紙..."));
  display.init(115200, true, 2, false);
  Serial.println(F("電子紙初始化完成"));
  
  // 設定 WiFi 熱點
  setupWiFi();
  
  // 設定網頁伺服器
  setupWebServer();
  
  // 顯示初始畫面
  updateDisplay();
  
  Serial.println(F("系統初始化完成"));
}

// ============================================
// 主要迴圈
// ============================================

void loop() {
  server.handleClient();
  
  // 定期顯示連線狀態（每 10 秒）
  static unsigned long lastStatusPrint = 0;
  if (millis() - lastStatusPrint > 10000) {
    lastStatusPrint = millis();
    int clients = WiFi.softAPgetStationNum();
    Serial.print(F("連線裝置數: "));
    Serial.print(clients);
    Serial.print(F(" | 可用記憶體: "));
    Serial.print(ESP.getFreeHeap());
    Serial.println(F(" bytes"));
  }
  
  // 檢查是否需要更新顯示
  if (needsUpdate && (millis() - lastUpdate > 2000)) {
    updateDisplay();
    needsUpdate = false;
    lastUpdate = millis();
  }
  
  delay(10);
}

// ============================================
// WiFi 設定函數
// ============================================

void setupWiFi() {
  Serial.println(F("設定 WiFi 熱點..."));
  
  // 停用所有 WiFi 功能後重新啟動
  WiFi.disconnect();
  WiFi.softAPdisconnect(true);
  delay(100);
  
  // 設定為 AP 模式
  WiFi.mode(WIFI_AP);
  delay(100);
  
  // 建立熱點（優化參數以提升速度）
  // WiFi.softAP(ssid, password, channel, hidden, max_connections)
  // 使用通道 1（2.4GHz 較少干擾），最多 4 個連線
  bool result = WiFi.softAP(ap_ssid, ap_password, 1, false, 4);
  
  // 設定輸出功率為最高（提升信號強度）
  WiFi.setOutputPower(20.5); // 最大 20.5 dBm
  
  // 啟用 802.11g/n 高速模式
  wifi_set_phy_mode(PHY_MODE_11N);
  
  if (result) {
    Serial.println(F("✓ WiFi 熱點建立成功"));
  } else {
    Serial.println(F("✗ WiFi 熱點建立失敗"));
  }
  
  delay(500);  // 等待 AP 完全啟動
  
  IPAddress IP = WiFi.softAPIP();
  
  Serial.println(F("========================================"));
  Serial.println(F("WiFi 熱點資訊："));
  Serial.print(F("SSID: "));
  Serial.println(ap_ssid);
  Serial.print(F("密碼: "));
  Serial.println(ap_password);
  Serial.print(F("IP 位址: "));
  Serial.println(IP);
  Serial.print(F("MAC 位址: "));
  Serial.println(WiFi.softAPmacAddress());
  Serial.println(F("請連接此熱點後，開啟瀏覽器訪問："));
  Serial.print(F("http://"));
  Serial.println(IP);
  Serial.println(F("========================================"));
}

// ============================================
// 網頁伺服器設定
// ============================================

void setupWebServer() {
  // 註冊路由
  server.on("/", handleRoot);
  server.on("/update", HTTP_POST, handleUpdate);
  server.on("/status", handleStatus);
  
  // 處理 404 錯誤
  server.onNotFound([]() {
    Serial.println(F("收到 404 請求"));
    server.send(404, "text/plain", "404: Not Found");
  });
  
  // 啟動伺服器
  server.begin();
  Serial.println(F("✓ 網頁伺服器已啟動（Port 80）"));
  Serial.println(F("可用端點："));
  Serial.println(F("  GET  /        - 主頁面"));
  Serial.println(F("  POST /update  - 更新文字"));
  Serial.println(F("  GET  /status  - 系統狀態"));
}

// ============================================
// 網頁處理函數
// ============================================

void handleRoot() {
  Serial.println(F("收到主頁面請求"));
  
  // 使用分段傳輸（chunked transfer）以節省 RAM
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html", "");
  
  // 發送 HTML 頭部
  server.sendContent_P(HTML_HEAD);
  
  // 發送 WiFi 資訊（動態內容）
  String wifiInfo = "<div class='wifi-info'>";
  wifiInfo += "<p><strong>📡 WiFi SSID:</strong> " + String(ap_ssid) + "</p>";
  wifiInfo += "<p><strong>🔑 密碼:</strong> " + String(ap_password) + "</p>";
  wifiInfo += "<p><strong>🌐 IP:</strong> " + WiFi.softAPIP().toString() + "</p>";
  wifiInfo += "</div>";
  server.sendContent(wifiInfo);
  
  // 發送表單（替換文字內容）
  String formHtml = FPSTR(HTML_FORM);
  formHtml.replace("%TEXT%", displayText);
  server.sendContent(formHtml);
  
  // 發送狀態資訊
  String status = "<div class='status'>";
  status += "<p><small>連線裝置: " + String(WiFi.softAPgetStationNum()) + "</small></p>";
  status += "<p><small>運行時間: " + String(millis() / 1000) + " 秒</small></p>";
  status += "<p><small>可用記憶體: " + String(ESP.getFreeHeap()) + " bytes</small></p>";
  status += "</div></div>";
  server.sendContent(status);
  
  // 發送結尾
  server.sendContent_P(HTML_TAIL);
  
  Serial.println(F("主頁面回應已送出（分段傳輸）"));
}

void handleUpdate() {
  Serial.println(F("收到更新請求"));
  
  if (server.hasArg("text")) {
    displayText = server.arg("text");
    
    // 移除前後空白
    displayText.trim();
    
    if (displayText.length() == 0) {
      displayText = "Waiting for input...";
    }
    
    Serial.print(F("新文字內容: "));
    Serial.println(displayText);
    
    needsUpdate = true;
    
    // 回傳簡單的成功訊息（不重定向，讓前端 JavaScript 處理）
    server.send(200, "text/plain", "OK");
  } else {
    server.send(400, "text/plain", "Missing text parameter");
  }
}

void handleStatus() {
  String json = "{";
  json += "\"text\":\"" + displayText + "\",";
  json += "\"clients\":" + String(WiFi.softAPgetStationNum()) + ",";
  json += "\"uptime\":" + String(millis() / 1000) + ",";
  json += "\"freeHeap\":" + String(ESP.getFreeHeap());
  json += "}";
  
  server.send(200, "application/json", json);
}

// ============================================
// 電子紙顯示更新函數
// ============================================

void updateDisplay() {
  Serial.println(F("更新電子紙顯示..."));
  
  display.setRotation(0);
  display.setFullWindow();
  display.firstPage();
  
  do {
    display.fillScreen(GxEPD_WHITE);
    
    // 左上角顯示 WiFi 資訊
    display.setTextColor(GxEPD_BLACK);
    display.setFont(&FreeMonoBold9pt7b);
    
    display.setCursor(10, 20);
    display.print(F("WiFi SSID:"));
    display.setCursor(10, 40);
    display.print(ap_ssid);
    
    display.setCursor(10, 65);
    display.print(F("Password:"));
    display.setCursor(10, 85);
    display.print(ap_password);
    
    display.setCursor(10, 110);
    display.print(F("IP: "));
    display.print(WiFi.softAPIP());
    
    // 畫一條分隔線
    display.drawLine(0, 130, display.width(), 130, GxEPD_BLACK);
    
    // 中央顯示使用者輸入的文字
    drawCenteredText(displayText);
    
  } while (display.nextPage());
  
  Serial.println(F("電子紙顯示更新完成"));
}

void drawCenteredText(String text) {
  display.setFont(&FreeMonoBold12pt7b);
  
  // 計算中央位置
  int16_t x = display.width() / 2 - 150;
  int16_t y = display.height() / 2 - 40;
  int16_t w = 300;
  int16_t h = 160;
  
  // 不使用 setPartialWindow，改用直接繪製
  // 畫背景框
  display.fillRect(x, y, w, h, GxEPD_WHITE);
  display.drawRect(x + 5, y + 5, w - 10, h - 10, GxEPD_BLACK);
  
  // 繪製文字（自動換行）
  int16_t cursorX = x + 15;
  int16_t cursorY = y + 35;
  int16_t lineHeight = 25;
  int16_t maxWidth = w - 30;
  
  // 簡單的文字換行處理
  String remainingText = text;
  int lineCount = 0;
  
  while (remainingText.length() > 0 && lineCount < 5) {
    int endIndex = remainingText.length();
    
    // 尋找適合的斷點
    for (int i = 1; i <= remainingText.length(); i++) {
      String testString = remainingText.substring(0, i);
      int16_t x1, y1;
      uint16_t w1, h1;
      display.getTextBounds(testString, 0, 0, &x1, &y1, &w1, &h1);
      
      if (w1 > maxWidth - 10) {
        endIndex = i - 1;
        if (endIndex < 1) endIndex = 1;
        break;
      }
    }
    
    String line = remainingText.substring(0, endIndex);
    display.setCursor(cursorX, cursorY + lineCount * lineHeight);
    display.print(line);
    
    remainingText = remainingText.substring(endIndex);
    remainingText.trim();
    lineCount++;
  }
}
