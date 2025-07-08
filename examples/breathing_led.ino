/*
  ESP8266 呼吸燈效果
  
  功能：
  - 使用 PWM 控制 LED 亮度
  - 創造平滑的呼吸效果
  - 可調整呼吸速度和亮度範圍
  
  技術說明：
  - 使用 analogWrite() 函數控制 PWM
  - ESP8266 PWM 範圍：0-1023
  - 板載 LED 為反向邏輯 (數值越大越暗)
  
  作者：DYJ Hsieh
  日期：2025年7月
*/

// ============================================
// 變數定義
// ============================================

int brightness = 0;        // 目前亮度 (0-255)
int fadeDirection = 1;     // 漸變方向 (1: 變亮, -1: 變暗)
int fadeStep = 2;          // 每次亮度變化量
int fadeDelay = 15;        // 漸變延遲 (毫秒)

// 可調整參數
int minBrightness = 0;     // 最小亮度
int maxBrightness = 255;   // 最大亮度
float breathingSpeed = 1.0; // 呼吸速度倍數

// ============================================
// 初始化設定
// ============================================

void setup() {
  Serial.begin(115200);
  delay(100);
  
  Serial.println("===============================");
  Serial.println("    ESP8266 呼吸燈效果");
  Serial.println("===============================");
  Serial.println("功能：平滑的 LED 亮度變化");
  Serial.println("指令：");
  Serial.println("  1-9: 調整速度");
  Serial.println("  +/-: 調整亮度範圍");
  Serial.println("  r:   重置設定");
  Serial.println("===============================");
  
  // 設定 LED 腳位為 PWM 輸出
  pinMode(LED_BUILTIN, OUTPUT);
  
  // 顯示初始設定
  showSettings();
}

// ============================================
// 主要迴圈
// ============================================

void loop() {
  // 處理序列埠指令
  handleCommands();
  
  // 執行呼吸燈效果
  breathingEffect();
  
  // 延遲
  delay(fadeDelay / breathingSpeed);
}

// ============================================
// 呼吸燈效果函數
// ============================================

void breathingEffect() {
  // 更新亮度
  brightness += fadeDirection * fadeStep;
  
  // 檢查邊界並改變方向
  if (brightness >= maxBrightness) {
    brightness = maxBrightness;
    fadeDirection = -1;
    Serial.println("🔆 達到最亮");
  } else if (brightness <= minBrightness) {
    brightness = minBrightness;
    fadeDirection = 1;
    Serial.println("🔅 達到最暗");
  }
  
  // 轉換為 ESP8266 PWM 值 (0-1023) 並反向
  int pwmValue = 1023 - map(brightness, 0, 255, 0, 1023);
  
  // 輸出到 LED
  analogWrite(LED_BUILTIN, pwmValue);
  
  // 定期顯示亮度資訊
  static unsigned long lastDisplay = 0;
  if (millis() - lastDisplay > 1000) {
    lastDisplay = millis();
    Serial.print("亮度: ");
    Serial.print(brightness);
    Serial.print("/");
    Serial.print(maxBrightness);
    Serial.print(" (PWM: ");
    Serial.print(pwmValue);
    Serial.println(")");
  }
}

// ============================================
// 指令處理
// ============================================

void handleCommands() {
  if (Serial.available() > 0) {
    char command = Serial.read();
    
    switch (command) {
      case '1': setSpeed(0.5); break;   // 最慢
      case '2': setSpeed(0.7); break;
      case '3': setSpeed(1.0); break;   // 預設
      case '4': setSpeed(1.3); break;
      case '5': setSpeed(1.5); break;   // 最快
      case '6': setSpeed(2.0); break;
      case '7': setSpeed(2.5); break;
      case '8': setSpeed(3.0); break;
      case '9': setSpeed(4.0); break;
      
      case '+': 
        adjustBrightness(10);
        break;
        
      case '-': 
        adjustBrightness(-10);
        break;
        
      case 'r':
        resetSettings();
        break;
        
      case 'h':
        showHelp();
        break;
        
      case 's':
        showSettings();
        break;
        
      default:
        if (command != '\n' && command != '\r') {
          Serial.println("未知指令: " + String(command));
        }
        break;
    }
  }
}

// ============================================
// 設定函數
// ============================================

void setSpeed(float speed) {
  breathingSpeed = speed;
  Serial.print("呼吸速度設定為: ");
  Serial.println(speed);
}

void adjustBrightness(int adjustment) {
  maxBrightness += adjustment;
  
  // 限制範圍
  if (maxBrightness > 255) maxBrightness = 255;
  if (maxBrightness < 50) maxBrightness = 50;
  
  Serial.print("最大亮度調整為: ");
  Serial.println(maxBrightness);
}

void resetSettings() {
  brightness = 0;
  fadeDirection = 1;
  fadeStep = 2;
  fadeDelay = 15;
  minBrightness = 0;
  maxBrightness = 255;
  breathingSpeed = 1.0;
  
  Serial.println("設定已重置為預設值");
  showSettings();
}

// ============================================
// 資訊顯示函數
// ============================================

void showSettings() {
  Serial.println("---------- 目前設定 ----------");
  Serial.print("呼吸速度: ");
  Serial.println(breathingSpeed);
  Serial.print("亮度範圍: ");
  Serial.print(minBrightness);
  Serial.print(" - ");
  Serial.println(maxBrightness);
  Serial.print("漸變步長: ");
  Serial.println(fadeStep);
  Serial.print("基礎延遲: ");
  Serial.print(fadeDelay);
  Serial.println(" ms");
  Serial.println("-----------------------------");
}

void showHelp() {
  Serial.println("===============================");
  Serial.println("           指令說明");
  Serial.println("===============================");
  Serial.println("速度控制:");
  Serial.println("  1-9    設定呼吸速度");
  Serial.println("         (1=最慢, 5=預設, 9=最快)");
  Serial.println();
  Serial.println("亮度控制:");
  Serial.println("  +      增加最大亮度");
  Serial.println("  -      減少最大亮度");
  Serial.println();
  Serial.println("其他:");
  Serial.println("  r      重置所有設定");
  Serial.println("  s      顯示目前設定");
  Serial.println("  h      顯示此說明");
  Serial.println("===============================");
}
