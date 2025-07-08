/*
  ESP8266 多種 LED 閃爍模式
  
  功能：
  - 包含多種預設的閃爍模式
  - 可以循環切換不同模式
  - 每種模式都有獨特的閃爍規律
  - 支援自定義模式
  
  模式列表：
  1. 標準閃爍 - 規律的開關
  2. 快速閃爍 - 高頻率閃爍
  3. SOS 求救信號 - 短短短長長長短短短
  4. 心跳模式 - 模擬心跳節拍
  5. 隨機閃爍 - 隨機間隔閃爍
  6. 漸變閃爍 - 亮度漸變效果
  7. 警示閃爍 - 警示燈效果
  8. 節拍閃爍 - 音樂節拍效果
  
  作者：DYJ Hsieh
  日期：2025年7月
*/

// ============================================
// 包含函式庫
// ============================================

#include <Arduino.h>

// ============================================
// 模式定義
// ============================================

enum LedPattern {
  PATTERN_STANDARD,      // 標準閃爍
  PATTERN_FAST,          // 快速閃爍
  PATTERN_SOS,           // SOS 求救信號
  PATTERN_HEARTBEAT,     // 心跳模式
  PATTERN_RANDOM,        // 隨機閃爍
  PATTERN_FADE,          // 漸變閃爍
  PATTERN_WARNING,       // 警示閃爍
  PATTERN_BEAT,          // 節拍閃爍
  PATTERN_COUNT          // 模式總數
};

// ============================================
// 變數定義
// ============================================

LedPattern currentPattern = PATTERN_STANDARD;
unsigned long previousMillis = 0;
unsigned long patternStartTime = 0;
int patternStep = 0;
bool ledState = false;
int brightness = 0;
int fadeDirection = 1;

// SOS 模式的定義 (毫秒)
// SOS: ... --- ...
int sosPattern[] = {200, 200, 200, 200, 200, 600,   // ...
                    600, 200, 600, 200, 600, 600,   // ---
                    200, 200, 200, 200, 200, 1000}; // ...
int sosPatternLength = sizeof(sosPattern) / sizeof(sosPattern[0]);

// 心跳模式的定義
int heartbeatPattern[] = {100, 100, 100, 600};
int heartbeatPatternLength = sizeof(heartbeatPattern) / sizeof(heartbeatPattern[0]);

// 節拍模式的定義
int beatPattern[] = {150, 150, 150, 150, 400, 150, 150, 400};
int beatPatternLength = sizeof(beatPattern) / sizeof(beatPattern[0]);

// ============================================
// 初始化設定
// ============================================

void setup() {
  Serial.begin(115200);
  delay(100);
  
  showStartupInfo();
  
  // 設定 LED 腳位
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);  // 初始關閉
  
  // 初始化時間
  patternStartTime = millis();
  
  showCurrentPattern();
  showHelp();
}

// ============================================
// 主要迴圈
// ============================================

void loop() {
  // 處理序列埠指令
  handleCommands();
  
  // 執行目前的閃爍模式
  executeCurrentPattern();
  
  delay(1);  // 小延遲避免 CPU 過載
}

// ============================================
// 模式執行函數
// ============================================

void executeCurrentPattern() {
  unsigned long currentMillis = millis();
  
  switch (currentPattern) {
    case PATTERN_STANDARD:
      executeStandardPattern(currentMillis);
      break;
      
    case PATTERN_FAST:
      executeFastPattern(currentMillis);
      break;
      
    case PATTERN_SOS:
      executeSOSPattern(currentMillis);
      break;
      
    case PATTERN_HEARTBEAT:
      executeHeartbeatPattern(currentMillis);
      break;
      
    case PATTERN_RANDOM:
      executeRandomPattern(currentMillis);
      break;
      
    case PATTERN_FADE:
      executeFadePattern(currentMillis);
      break;
      
    case PATTERN_WARNING:
      executeWarningPattern(currentMillis);
      break;
      
    case PATTERN_BEAT:
      executeBeatPattern(currentMillis);
      break;
  }
}

// ============================================
// 個別模式實作
// ============================================

void executeStandardPattern(unsigned long currentMillis) {
  if (currentMillis - previousMillis >= 1000) {
    previousMillis = currentMillis;
    toggleLED();
  }
}

void executeFastPattern(unsigned long currentMillis) {
  if (currentMillis - previousMillis >= 150) {
    previousMillis = currentMillis;
    toggleLED();
  }
}

void executeSOSPattern(unsigned long currentMillis) {
  if (currentMillis - previousMillis >= sosPattern[patternStep]) {
    previousMillis = currentMillis;
    
    // 奇數步驟點亮 LED，偶數步驟關閉 LED
    ledState = (patternStep % 2 == 0);
    setLED(ledState);
    
    patternStep++;
    if (patternStep >= sosPatternLength) {
      patternStep = 0;
      Serial.println("📡 SOS 信號傳送完成");
    }
  }
}

void executeHeartbeatPattern(unsigned long currentMillis) {
  if (currentMillis - previousMillis >= heartbeatPattern[patternStep]) {
    previousMillis = currentMillis;
    
    // 心跳：兩次快速閃爍然後暫停
    ledState = (patternStep % 2 == 0);
    setLED(ledState);
    
    patternStep++;
    if (patternStep >= heartbeatPatternLength) {
      patternStep = 0;
    }
  }
}

void executeRandomPattern(unsigned long currentMillis) {
  static unsigned long nextChange = 0;
  
  if (currentMillis >= nextChange) {
    toggleLED();
    // 隨機間隔：100ms 到 2000ms
    nextChange = currentMillis + random(100, 2000);
  }
}

void executeFadePattern(unsigned long currentMillis) {
  if (currentMillis - previousMillis >= 10) {
    previousMillis = currentMillis;
    
    brightness += fadeDirection * 3;
    
    if (brightness >= 255) {
      brightness = 255;
      fadeDirection = -1;
    } else if (brightness <= 0) {
      brightness = 0;
      fadeDirection = 1;
    }
    
    // 轉換為 ESP8266 PWM 值並反向
    int pwmValue = 1023 - map(brightness, 0, 255, 0, 1023);
    analogWrite(LED_BUILTIN, pwmValue);
  }
}

void executeWarningPattern(unsigned long currentMillis) {
  static int warningStep = 0;
  
  // 警示模式：快速閃爍3次，暫停1秒
  if (currentMillis - previousMillis >= (warningStep < 6 ? 100 : 1000)) {
    previousMillis = currentMillis;
    
    if (warningStep < 6) {
      ledState = (warningStep % 2 == 0);
      setLED(ledState);
    }
    
    warningStep++;
    if (warningStep >= 7) {
      warningStep = 0;
    }
  }
}

void executeBeatPattern(unsigned long currentMillis) {
  if (currentMillis - previousMillis >= beatPattern[patternStep]) {
    previousMillis = currentMillis;
    
    ledState = (patternStep % 2 == 0);
    setLED(ledState);
    
    patternStep++;
    if (patternStep >= beatPatternLength) {
      patternStep = 0;
    }
  }
}

// ============================================
// LED 控制函數
// ============================================

void toggleLED() {
  ledState = !ledState;
  setLED(ledState);
}

void setLED(bool state) {
  digitalWrite(LED_BUILTIN, state ? LOW : HIGH);  // ESP8266 反向邏輯
  
  // 顯示狀態（僅限標準和快速模式）
  if (currentPattern == PATTERN_STANDARD || currentPattern == PATTERN_FAST) {
    Serial.println(state ? "🔵 LED 開啟" : "⚫ LED 關閉");
  }
}

// ============================================
// 指令處理
// ============================================

void handleCommands() {
  if (Serial.available() > 0) {
    char command = Serial.read();
    
    switch (command) {
      case '1': case '2': case '3': case '4':
      case '5': case '6': case '7': case '8':
        switchToPattern((LedPattern)(command - '1'));
        break;
        
      case 'n':
        nextPattern();
        break;
        
      case 'h':
        showHelp();
        break;
        
      case 'i':
        showPatternInfo();
        break;
        
      case 's':
        showStatus();
        break;
        
      case 'r':
        resetPattern();
        break;
        
      default:
        if (command != '\n' && command != '\r') {
          Serial.println("未知指令: " + String(command));
          Serial.println("輸入 'h' 查看說明");
        }
        break;
    }
  }
}

// ============================================
// 模式切換函數
// ============================================

void switchToPattern(LedPattern pattern) {
  if (pattern >= 0 && pattern < PATTERN_COUNT) {
    currentPattern = pattern;
    resetPatternVariables();
    showCurrentPattern();
  }
}

void nextPattern() {
  currentPattern = (LedPattern)((currentPattern + 1) % PATTERN_COUNT);
  resetPatternVariables();
  showCurrentPattern();
}

void resetPattern() {
  resetPatternVariables();
  Serial.println("🔄 模式已重置");
}

void resetPatternVariables() {
  previousMillis = 0;
  patternStartTime = millis();
  patternStep = 0;
  ledState = false;
  brightness = 0;
  fadeDirection = 1;
  digitalWrite(LED_BUILTIN, HIGH);  // 確保 LED 關閉
}

// ============================================
// 資訊顯示函數
// ============================================

void showStartupInfo() {
  Serial.println();
  Serial.println("########################################");
  Serial.println("#                                      #");
  Serial.println("#      ESP8266 多模式 LED 控制         #");
  Serial.println("#                                      #");
  Serial.println("########################################");
  Serial.println("版本: v3.0");
  Serial.println("特色: 8種不同的閃爍模式");
  Serial.println();
}

void showCurrentPattern() {
  String patternName = getPatternName(currentPattern);
  Serial.println("🎯 目前模式: " + patternName);
}

String getPatternName(LedPattern pattern) {
  switch (pattern) {
    case PATTERN_STANDARD: return "標準閃爍";
    case PATTERN_FAST: return "快速閃爍";
    case PATTERN_SOS: return "SOS 求救信號";
    case PATTERN_HEARTBEAT: return "心跳模式";
    case PATTERN_RANDOM: return "隨機閃爍";
    case PATTERN_FADE: return "漸變閃爍";
    case PATTERN_WARNING: return "警示閃爍";
    case PATTERN_BEAT: return "節拍閃爍";
    default: return "未知模式";
  }
}

void showHelp() {
  Serial.println("========================================");
  Serial.println("              指令說明");
  Serial.println("========================================");
  Serial.println("模式選擇:");
  Serial.println("  1      標準閃爍 (1秒間隔)");
  Serial.println("  2      快速閃爍 (150ms間隔)");
  Serial.println("  3      SOS 求救信號");
  Serial.println("  4      心跳模式");
  Serial.println("  5      隨機閃爍");
  Serial.println("  6      漸變閃爍 (PWM控制)");
  Serial.println("  7      警示閃爍");
  Serial.println("  8      節拍閃爍");
  Serial.println();
  Serial.println("控制指令:");
  Serial.println("  n      切換到下一個模式");
  Serial.println("  r      重置目前模式");
  Serial.println("  i      顯示模式詳細資訊");
  Serial.println("  s      顯示系統狀態");
  Serial.println("  h      顯示此說明");
  Serial.println("========================================");
}

void showPatternInfo() {
  Serial.println("========================================");
  Serial.println("              模式資訊");
  Serial.println("========================================");
  
  for (int i = 0; i < PATTERN_COUNT; i++) {
    Serial.print(i + 1);
    Serial.print(". ");
    Serial.print(getPatternName((LedPattern)i));
    
    if (i == currentPattern) {
      Serial.print(" ← 目前");
    }
    Serial.println();
  }
  
  Serial.println("========================================");
}

void showStatus() {
  Serial.println("========================================");
  Serial.println("              系統狀態");
  Serial.println("========================================");
  Serial.print("目前模式: ");
  Serial.println(getPatternName(currentPattern));
  Serial.print("運行時間: ");
  Serial.print((millis() - patternStartTime) / 1000);
  Serial.println(" 秒");
  Serial.print("LED 狀態: ");
  Serial.println(digitalRead(LED_BUILTIN) == LOW ? "開啟" : "關閉");
  Serial.print("可用記憶體: ");
  Serial.print(ESP.getFreeHeap());
  Serial.println(" bytes");
  Serial.println("========================================");
}
