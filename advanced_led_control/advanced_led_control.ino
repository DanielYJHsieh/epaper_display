/*
  ESP8266 進階 LED 控制程式
  
  功能：
  - 可調整閃爍頻率 (1-9 速度等級)
  - 序列埠指令控制
  - 非阻塞式程式設計
  - 即時狀態回饋
  - 多種閃爍模式
  
  指令說明：
  - 1-9: 設定閃爍速度 (1=最快, 9=最慢)
  - s: 停止閃爍
  - r: 恢復閃爍
  - p: 切換閃爍模式
  - i: 顯示系統資訊
  - h: 顯示說明
  
  硬體需求：
  - ESP8266MOD 開發板 (NodeMCU、Wemos D1 Mini 等)
  - USB 傳輸線
  
  作者：DYJ Hsieh
  日期：2025年7月
  版本：v2.0
*/

// ============================================
// 包含函式庫
// ============================================

#include <ESP8266WiFi.h>  // WiFi 功能 (選用)

// ============================================
// 常數定義
// ============================================

// 閃爍速度對應表 (毫秒)
const int BLINK_SPEEDS[] = {100, 200, 300, 400, 500, 750, 1000, 1500, 2000};
const int SPEED_COUNT = sizeof(BLINK_SPEEDS) / sizeof(BLINK_SPEEDS[0]);

// 閃爍模式
enum BlinkMode {
  MODE_NORMAL,      // 正常閃爍
  MODE_DOUBLE,      // 雙重閃爍
  MODE_TRIPLE,      // 三重閃爍
  MODE_BREATHING,   // 呼吸燈
  MODE_RANDOM       // 隨機模式
};

// ============================================
// 變數定義
// ============================================

// LED 控制變數
bool ledState = false;
bool blinkEnabled = true;
int currentSpeed = 4;  // 預設速度 (500ms)
BlinkMode currentMode = MODE_NORMAL;

// 計時變數
unsigned long previousMillis = 0;
unsigned long modeTimer = 0;
int blinkCount = 0;

// 呼吸燈變數
int breathingBrightness = 0;
int breathingDirection = 1;

// 統計變數
unsigned long totalBlinks = 0;
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
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);  // 初始關閉
  
  // 記錄啟動時間
  startTime = millis();
  
  // 顯示初始狀態
  showCurrentSettings();
  
  Serial.println("系統就緒！請輸入指令或按 'h' 查看說明");
  Serial.println("========================================");
}

// ============================================
// 主要迴圈
// ============================================

void loop() {
  // 處理序列埠指令
  handleSerialCommand();
  
  // 執行 LED 控制
  if (blinkEnabled) {
    switch (currentMode) {
      case MODE_NORMAL:
        handleNormalBlink();
        break;
      case MODE_DOUBLE:
        handleDoubleBlink();
        break;
      case MODE_TRIPLE:
        handleTripleBlink();
        break;
      case MODE_BREATHING:
        handleBreathingEffect();
        break;
      case MODE_RANDOM:
        handleRandomBlink();
        break;
    }
  }
  
  // 小延遲以避免 CPU 過載
  delay(1);
}

// ============================================
// LED 控制函數
// ============================================

/*
 * 正常閃爍模式
 */
void handleNormalBlink() {
  unsigned long currentMillis = millis();
  
  if (currentMillis - previousMillis >= BLINK_SPEEDS[currentSpeed]) {
    previousMillis = currentMillis;
    toggleLED();
  }
}

/*
 * 雙重閃爍模式
 */
void handleDoubleBlink() {
  unsigned long currentMillis = millis();
  int interval = BLINK_SPEEDS[currentSpeed];
  
  if (currentMillis - previousMillis >= interval / 4) {
    previousMillis = currentMillis;
    
    if (blinkCount < 4) {  // 兩次快速閃爍
      toggleLED();
      blinkCount++;
    } else {
      // 長時間暫停
      if (currentMillis - modeTimer >= interval * 2) {
        modeTimer = currentMillis;
        blinkCount = 0;
      }
    }
  }
}

/*
 * 三重閃爍模式
 */
void handleTripleBlink() {
  unsigned long currentMillis = millis();
  int interval = BLINK_SPEEDS[currentSpeed];
  
  if (currentMillis - previousMillis >= interval / 6) {
    previousMillis = currentMillis;
    
    if (blinkCount < 6) {  // 三次快速閃爍
      toggleLED();
      blinkCount++;
    } else {
      // 長時間暫停
      if (currentMillis - modeTimer >= interval * 3) {
        modeTimer = currentMillis;
        blinkCount = 0;
      }
    }
  }
}

/*
 * 呼吸燈效果
 */
void handleBreathingEffect() {
  unsigned long currentMillis = millis();
  
  if (currentMillis - previousMillis >= 20) {  // 更新頻率 50Hz
    previousMillis = currentMillis;
    
    breathingBrightness += breathingDirection * 5;
    
    if (breathingBrightness >= 255) {
      breathingBrightness = 255;
      breathingDirection = -1;
    } else if (breathingBrightness <= 0) {
      breathingBrightness = 0;
      breathingDirection = 1;
    }
    
    // ESP8266 的 analogWrite 範圍是 0-1023
    analogWrite(LED_BUILTIN, 1023 - (breathingBrightness * 4));
  }
}

/*
 * 隨機閃爍模式
 */
void handleRandomBlink() {
  unsigned long currentMillis = millis();
  
  if (currentMillis - previousMillis >= random(50, BLINK_SPEEDS[currentSpeed] * 2)) {
    previousMillis = currentMillis;
    toggleLED();
  }
}

/*
 * 切換 LED 狀態
 */
void toggleLED() {
  ledState = !ledState;
  digitalWrite(LED_BUILTIN, ledState ? LOW : HIGH);
  
  if (ledState) {
    totalBlinks++;
  }
  
  // 顯示狀態 (僅在正常模式下)
  if (currentMode == MODE_NORMAL) {
    Serial.print(ledState ? "🔵 LED 開啟" : "⚫ LED 關閉");
    Serial.print(" | 總閃爍次數: ");
    Serial.println(totalBlinks);
  }
}

// ============================================
// 序列埠指令處理
// ============================================

void handleSerialCommand() {
  if (Serial.available() > 0) {
    String command = Serial.readString();
    command.trim();
    command.toLowerCase();
    
    if (command.length() == 1) {
      char cmd = command.charAt(0);
      
      switch (cmd) {
        case '1': case '2': case '3': case '4': case '5':
        case '6': case '7': case '8': case '9':
          setBlinkSpeed(cmd - '1');
          break;
          
        case 's':
          stopBlinking();
          break;
          
        case 'r':
          resumeBlinking();
          break;
          
        case 'p':
          switchMode();
          break;
          
        case 'i':
          showSystemInfo();
          break;
          
        case 'h':
          showHelp();
          break;
          
        case 'c':
          clearStatistics();
          break;
          
        default:
          Serial.println("未知指令: " + String(cmd));
          Serial.println("輸入 'h' 查看可用指令");
          break;
      }
    } else {
      handleMultiCharCommand(command);
    }
  }
}

/*
 * 處理多字元指令
 */
void handleMultiCharCommand(String command) {
  if (command == "help") {
    showHelp();
  } else if (command == "info") {
    showSystemInfo();
  } else if (command == "status") {
    showCurrentSettings();
  } else if (command == "reset") {
    Serial.println("重新啟動 ESP8266...");
    delay(1000);
    ESP.restart();
  } else if (command == "clear") {
    clearStatistics();
  } else {
    Serial.println("未知指令: " + command);
    Serial.println("輸入 'help' 查看可用指令");
  }
}

// ============================================
// 控制函數
// ============================================

/*
 * 設定閃爍速度
 */
void setBlinkSpeed(int speed) {
  if (speed >= 0 && speed < SPEED_COUNT) {
    currentSpeed = speed;
    Serial.print("閃爍速度設定為等級 ");
    Serial.print(speed + 1);
    Serial.print(" (");
    Serial.print(BLINK_SPEEDS[speed]);
    Serial.println(" ms)");
  }
}

/*
 * 停止閃爍
 */
void stopBlinking() {
  blinkEnabled = false;
  digitalWrite(LED_BUILTIN, HIGH);  // 確保 LED 關閉
  Serial.println("⏹️  閃爍已停止");
}

/*
 * 恢復閃爍
 */
void resumeBlinking() {
  blinkEnabled = true;
  Serial.println("▶️  閃爍已恢復");
}

/*
 * 切換閃爍模式
 */
void switchMode() {
  currentMode = (BlinkMode)((currentMode + 1) % 5);
  
  // 重置模式相關變數
  blinkCount = 0;
  modeTimer = millis();
  breathingBrightness = 0;
  breathingDirection = 1;
  
  String modeName;
  switch (currentMode) {
    case MODE_NORMAL: modeName = "正常閃爍"; break;
    case MODE_DOUBLE: modeName = "雙重閃爍"; break;
    case MODE_TRIPLE: modeName = "三重閃爍"; break;
    case MODE_BREATHING: modeName = "呼吸燈"; break;
    case MODE_RANDOM: modeName = "隨機模式"; break;
  }
  
  Serial.println("🔄 模式切換為: " + modeName);
}

/*
 * 清除統計資料
 */
void clearStatistics() {
  totalBlinks = 0;
  startTime = millis();
  Serial.println("📊 統計資料已清除");
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
  Serial.println("#     ESP8266 進階 LED 控制程式        #");
  Serial.println("#                                      #");
  Serial.println("########################################");
  Serial.println();
  Serial.println("版本: v2.0");
  Serial.println("作者: DYJ Hsieh");
  Serial.println("日期: 2025年7月");
  Serial.println();
  Serial.println("功能特色:");
  Serial.println("• 可調整閃爍速度 (9個等級)");
  Serial.println("• 多種閃爍模式");
  Serial.println("• 即時狀態監控");
  Serial.println("• 非阻塞式設計");
  Serial.println();
}

/*
 * 顯示說明資訊
 */
void showHelp() {
  Serial.println("========================================");
  Serial.println("              指令說明");
  Serial.println("========================================");
  Serial.println("數字指令:");
  Serial.println("  1-9    設定閃爍速度 (1=最快, 9=最慢)");
  Serial.println();
  Serial.println("控制指令:");
  Serial.println("  s      停止閃爍");
  Serial.println("  r      恢復閃爍");
  Serial.println("  p      切換閃爍模式");
  Serial.println("  c      清除統計資料");
  Serial.println();
  Serial.println("資訊指令:");
  Serial.println("  h      顯示此說明");
  Serial.println("  i      顯示系統資訊");
  Serial.println("  status 顯示目前設定");
  Serial.println("  reset  重新啟動");
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
  Serial.print("運行時間: ");
  Serial.print((millis() - startTime) / 1000);
  Serial.println(" 秒");
  Serial.print("總閃爍次數: ");
  Serial.println(totalBlinks);
  Serial.println("========================================");
}

/*
 * 顯示目前設定
 */
void showCurrentSettings() {
  Serial.println("========================================");
  Serial.println("              目前設定");
  Serial.println("========================================");
  
  String modeName;
  switch (currentMode) {
    case MODE_NORMAL: modeName = "正常閃爍"; break;
    case MODE_DOUBLE: modeName = "雙重閃爍"; break;
    case MODE_TRIPLE: modeName = "三重閃爍"; break;
    case MODE_BREATHING: modeName = "呼吸燈"; break;
    case MODE_RANDOM: modeName = "隨機模式"; break;
  }
  
  Serial.print("閃爍模式: ");
  Serial.println(modeName);
  Serial.print("閃爍速度: 等級 ");
  Serial.print(currentSpeed + 1);
  Serial.print(" (");
  Serial.print(BLINK_SPEEDS[currentSpeed]);
  Serial.println(" ms)");
  Serial.print("閃爍狀態: ");
  Serial.println(blinkEnabled ? "啟用" : "停用");
  Serial.print("LED 腳位: GPIO");
  Serial.println(LED_BUILTIN);
  Serial.println("========================================");
}
