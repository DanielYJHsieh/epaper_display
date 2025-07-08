/*
  ESP8266 基本 LED 控制程式
  
  功能：
  - 控制板載 LED 每秒閃爍一次
  - 透過序列埠輸出狀態訊息
  - 適合 ESP8266 初學者學習
  
  硬體需求：
  - ESP8266MOD 開發板 (NodeMCU、Wemos D1 Mini 等)
  - USB 傳輸線
  
  注意事項：
  - ESP8266 板載 LED 為低電位觸發 (LOW = 開啟, HIGH = 關閉)
  - 序列埠鮑率設定為 115200
  
  作者：DYJ Hsieh
  日期：2025年7月
  版本：v1.0
*/

// ============================================
// 變數定義
// ============================================

// LED 狀態變數
bool ledState = false;

// 計時變數
unsigned long previousMillis = 0;
const long interval = 1000;  // 閃爍間隔 (毫秒)

// ============================================
// 初始化設定
// ============================================

void setup() {
  // 初始化序列埠通訊
  Serial.begin(115200);
  
  // 等待序列埠就緒
  delay(100);
  
  // 清空序列埠緩衝區
  Serial.println();
  Serial.println("====================================");
  Serial.println("    ESP8266 基本 LED 控制程式");
  Serial.println("====================================");
  Serial.println("版本：v1.0");
  Serial.println("功能：板載 LED 閃爍控制");
  Serial.println("====================================");
  
  // 設定板載 LED 為輸出模式
  pinMode(LED_BUILTIN, OUTPUT);
  
  // 初始化 LED 為關閉狀態
  digitalWrite(LED_BUILTIN, HIGH);  // ESP8266 高電位為關閉
  
  // 顯示 LED 腳位資訊
  Serial.print("板載 LED 腳位：GPIO");
  Serial.println(LED_BUILTIN);
  
  // 顯示開發板資訊
  showBoardInfo();
  
  Serial.println("程式初始化完成");
  Serial.println("LED 開始閃爍...");
  Serial.println("====================================");
}

// ============================================
// 主要迴圈
// ============================================

void loop() {
  // 取得當前時間
  unsigned long currentMillis = millis();
  
  // 檢查是否到了切換 LED 的時間
  if (currentMillis - previousMillis >= interval) {
    // 儲存最後切換時間
    previousMillis = currentMillis;
    
    // 切換 LED 狀態
    ledState = !ledState;
    
    // 控制 LED (注意：ESP8266 為反向邏輯)
    if (ledState) {
      digitalWrite(LED_BUILTIN, LOW);   // 開啟 LED
      Serial.println("🔵 LED 開啟");
    } else {
      digitalWrite(LED_BUILTIN, HIGH);  // 關閉 LED
      Serial.println("⚫ LED 關閉");
    }
    
    // 顯示運行時間
    Serial.print("運行時間：");
    Serial.print(currentMillis / 1000);
    Serial.println(" 秒");
    Serial.println("----------------");
  }
  
  // 檢查序列埠輸入
  handleSerialInput();
}

// ============================================
// 輔助函數
// ============================================

/*
 * 顯示開發板資訊
 */
void showBoardInfo() {
  Serial.println("開發板資訊：");
  Serial.print("- 晶片 ID：0x");
  Serial.println(ESP.getChipId(), HEX);
  Serial.print("- Flash 大小：");
  Serial.print(ESP.getFlashChipSize() / 1024 / 1024);
  Serial.println(" MB");
  Serial.print("- CPU 頻率：");
  Serial.print(ESP.getCpuFreqMHz());
  Serial.println(" MHz");
  Serial.print("- 可用記憶體：");
  Serial.print(ESP.getFreeHeap());
  Serial.println(" bytes");
}

/*
 * 處理序列埠輸入
 */
void handleSerialInput() {
  if (Serial.available() > 0) {
    String input = Serial.readString();
    input.trim();  // 移除前後空白字元
    
    if (input.equals("help") || input.equals("?")) {
      showHelp();
    } else if (input.equals("info")) {
      showBoardInfo();
    } else if (input.equals("reset")) {
      Serial.println("重新啟動 ESP8266...");
      delay(1000);
      ESP.restart();
    } else if (input.equals("status")) {
      showStatus();
    } else if (input.length() > 0) {
      Serial.println("未知指令：" + input);
      Serial.println("輸入 'help' 查看可用指令");
    }
  }
}

/*
 * 顯示說明資訊
 */
void showHelp() {
  Serial.println("====================================");
  Serial.println("           可用指令列表");
  Serial.println("====================================");
  Serial.println("help   - 顯示此說明");
  Serial.println("info   - 顯示開發板資訊");
  Serial.println("status - 顯示目前狀態");
  Serial.println("reset  - 重新啟動 ESP8266");
  Serial.println("====================================");
}

/*
 * 顯示目前狀態
 */
void showStatus() {
  Serial.println("====================================");
  Serial.println("           目前系統狀態");
  Serial.println("====================================");
  Serial.print("LED 狀態：");
  Serial.println(ledState ? "開啟" : "關閉");
  Serial.print("閃爍間隔：");
  Serial.print(interval);
  Serial.println(" 毫秒");
  Serial.print("系統運行時間：");
  Serial.print(millis() / 1000);
  Serial.println(" 秒");
  Serial.print("記憶體使用：");
  Serial.print(ESP.getFreeHeap());
  Serial.println(" bytes 可用");
  Serial.println("====================================");
}
