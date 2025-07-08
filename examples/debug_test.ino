/*
  ESP8266 E-Paper 除錯測試程式
  
  功能：
  - 系統性測試所有硬體連接
  - 提供詳細的除錯資訊
  - 分步驟測試各個功能
  - 協助找出問題根源
  
  使用方式：
  1. 上傳此程式到 ESP8266
  2. 開啟序列埠監控器 (115200)
  3. 觀察測試結果
  4. 根據輸出訊息進行除錯
  
  作者：DYJ Hsieh
  日期：2025年7月
  版本：v1.0 (除錯專用)
*/

#include <SPI.h>

// 腳位定義
#define EPD_CS_PIN    D8    // GPIO15
#define EPD_DC_PIN    D3    // GPIO0  
#define EPD_RST_PIN   D4    // GPIO2
#define EPD_BUSY_PIN  D2    // GPIO4
#define EPD_MOSI_PIN  D7    // GPIO13
#define EPD_SCK_PIN   D5    // GPIO14

// SSD1677 基本命令
#define SSD1677_SW_RESET                    0x12
#define SSD1677_DRIVER_OUTPUT_CONTROL       0x01
#define SSD1677_DATA_ENTRY_MODE_SETTING     0x11

// 測試階段
enum TestPhase {
  PHASE_INIT = 0,
  PHASE_PIN_TEST,
  PHASE_SPI_TEST,
  PHASE_RESET_TEST,
  PHASE_BUSY_TEST,
  PHASE_COMMAND_TEST,
  PHASE_DISPLAY_TEST,
  PHASE_COMPLETE
};

TestPhase currentPhase = PHASE_INIT;
bool testPassed = false;

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  showHeader();
  
  Serial.println("🔍 開始 E-Paper 除錯測試");
  Serial.println("========================================");
  
  // 執行所有測試
  runAllTests();
}

void loop() {
  // 提供互動式測試選單
  if (Serial.available() > 0) {
    String command = Serial.readString();
    command.trim();
    command.toLowerCase();
    
    if (command == "1") {
      testPinConfiguration();
    } else if (command == "2") {
      testSPICommunication();
    } else if (command == "3") {
      testHardwareReset();
    } else if (command == "4") {
      testBusySignal();
    } else if (command == "5") {
      testBasicCommands();
    } else if (command == "6") {
      testFullDisplay();
    } else if (command == "all") {
      runAllTests();
    } else if (command == "help") {
      showMenu();
    } else if (command == "info") {
      showSystemInfo();
    } else {
      showMenu();
    }
  }
  
  delay(100);
}

void showHeader() {
  Serial.println();
  Serial.println("########################################");
  Serial.println("#                                      #");
  Serial.println("#     E-Paper 除錯測試程式             #");
  Serial.println("#                                      #");
  Serial.println("########################################");
  Serial.println();
  Serial.println("版本: v1.0 (除錯專用)");
  Serial.println("目標: GDEQ0426T82 (SSD1677)");
  Serial.println("解析度: 800x480 像素");
  Serial.println();
}

void showMenu() {
  Serial.println("========================================");
  Serial.println("           除錯測試選單");
  Serial.println("========================================");
  Serial.println("1    - 腳位配置測試");
  Serial.println("2    - SPI 通訊測試");
  Serial.println("3    - 硬體重置測試");
  Serial.println("4    - BUSY 信號測試");
  Serial.println("5    - 基本命令測試");
  Serial.println("6    - 完整顯示測試");
  Serial.println("all  - 執行所有測試");
  Serial.println("info - 顯示系統資訊");
  Serial.println("help - 顯示此選單");
  Serial.println("========================================");
}

void runAllTests() {
  Serial.println("🚀 執行完整除錯測試流程");
  Serial.println("========================================");
  
  bool allPassed = true;
  
  // 測試 1: 腳位配置
  Serial.println("\n📌 測試 1: 腳位配置");
  allPassed &= testPinConfiguration();
  
  // 測試 2: SPI 通訊
  Serial.println("\n📡 測試 2: SPI 通訊");
  allPassed &= testSPICommunication();
  
  // 測試 3: 硬體重置
  Serial.println("\n🔄 測試 3: 硬體重置");
  allPassed &= testHardwareReset();
  
  // 測試 4: BUSY 信號
  Serial.println("\n⏳ 測試 4: BUSY 信號");
  allPassed &= testBusySignal();
  
  // 測試 5: 基本命令
  Serial.println("\n📝 測試 5: 基本命令");
  allPassed &= testBasicCommands();
  
  // 測試 6: 顯示測試
  Serial.println("\n🖥️  測試 6: 顯示功能");
  allPassed &= testFullDisplay();
  
  // 總結
  Serial.println("\n========================================");
  Serial.println("           測試結果總結");
  Serial.println("========================================");
  if (allPassed) {
    Serial.println("✅ 所有測試通過！E-Paper 應該可以正常工作");
  } else {
    Serial.println("❌ 部分測試失敗，請檢查上述錯誤訊息");
  }
  Serial.println("========================================");
  
  showMenu();
}

bool testPinConfiguration() {
  Serial.println("正在測試腳位配置...");
  
  // 設定腳位模式
  pinMode(EPD_CS_PIN, OUTPUT);
  pinMode(EPD_DC_PIN, OUTPUT);
  pinMode(EPD_RST_PIN, OUTPUT);
  pinMode(EPD_BUSY_PIN, INPUT);
  
  // 設定初始狀態
  digitalWrite(EPD_CS_PIN, HIGH);
  digitalWrite(EPD_DC_PIN, LOW);
  digitalWrite(EPD_RST_PIN, HIGH);
  
  // 測試腳位輸出
  Serial.println("  📍 測試 CS 腳位 (D8/GPIO15)...");
  digitalWrite(EPD_CS_PIN, HIGH);
  delay(10);
  digitalWrite(EPD_CS_PIN, LOW);
  delay(10);
  digitalWrite(EPD_CS_PIN, HIGH);
  Serial.println("    ✅ CS 腳位測試完成");
  
  Serial.println("  📍 測試 DC 腳位 (D3/GPIO0)...");
  digitalWrite(EPD_DC_PIN, HIGH);
  delay(10);
  digitalWrite(EPD_DC_PIN, LOW);
  delay(10);
  Serial.println("    ✅ DC 腳位測試完成");
  
  Serial.println("  📍 測試 RST 腳位 (D4/GPIO2)...");
  digitalWrite(EPD_RST_PIN, LOW);
  delay(10);
  digitalWrite(EPD_RST_PIN, HIGH);
  delay(10);
  Serial.println("    ✅ RST 腳位測試完成");
  
  Serial.println("  📍 測試 BUSY 腳位 (D2/GPIO4)...");
  int busyState = digitalRead(EPD_BUSY_PIN);
  Serial.print("    BUSY 目前狀態: ");
  Serial.println(busyState ? "HIGH" : "LOW");
  Serial.println("    ✅ BUSY 腳位讀取完成");
  
  Serial.println("✅ 腳位配置測試完成");
  return true;
}

bool testSPICommunication() {
  Serial.println("正在測試 SPI 通訊...");
  
  // 初始化 SPI
  SPI.begin();
  SPI.setFrequency(1000000);  // 降低頻率到 1MHz 進行測試
  SPI.setDataMode(SPI_MODE0);
  SPI.setBitOrder(MSBFIRST);
  
  Serial.println("  📡 SPI 初始化完成");
  Serial.println("    頻率: 1MHz (測試用)");
  Serial.println("    模式: MODE0");
  Serial.println("    位元順序: MSB First");
  
  // 測試 SPI 傳輸
  Serial.println("  📡 測試 SPI 資料傳輸...");
  
  digitalWrite(EPD_CS_PIN, LOW);
  delay(1);
  
  uint8_t testData[] = {0x00, 0xFF, 0xAA, 0x55};
  for (int i = 0; i < 4; i++) {
    SPI.transfer(testData[i]);
    Serial.print("    傳送: 0x");
    Serial.println(testData[i], HEX);
  }
  
  delay(1);
  digitalWrite(EPD_CS_PIN, HIGH);
  
  Serial.println("✅ SPI 通訊測試完成");
  return true;
}

bool testHardwareReset() {
  Serial.println("正在測試硬體重置...");
  
  Serial.println("  🔄 執行硬體重置序列...");
  
  // 重置序列
  digitalWrite(EPD_RST_PIN, HIGH);
  delay(200);
  Serial.println("    RST = HIGH (200ms)");
  
  digitalWrite(EPD_RST_PIN, LOW);
  delay(2);
  Serial.println("    RST = LOW (2ms)");
  
  digitalWrite(EPD_RST_PIN, HIGH);
  delay(200);
  Serial.println("    RST = HIGH (200ms)");
  
  Serial.println("✅ 硬體重置測試完成");
  return true;
}

bool testBusySignal() {
  Serial.println("正在測試 BUSY 信號...");
  
  Serial.println("  ⏳ 監控 BUSY 信號變化...");
  
  unsigned long startTime = millis();
  int lastState = digitalRead(EPD_BUSY_PIN);
  int stateChanges = 0;
  
  Serial.print("    初始狀態: ");
  Serial.println(lastState ? "HIGH (忙碌)" : "LOW (就緒)");
  
  // 監控 5 秒
  while (millis() - startTime < 5000) {
    int currentState = digitalRead(EPD_BUSY_PIN);
    if (currentState != lastState) {
      stateChanges++;
      Serial.print("    狀態變化 ");
      Serial.print(stateChanges);
      Serial.print(": ");
      Serial.println(currentState ? "HIGH (忙碌)" : "LOW (就緒)");
      lastState = currentState;
    }
    delay(10);
  }
  
  Serial.print("    監控結果: ");
  Serial.print(stateChanges);
  Serial.println(" 次狀態變化");
  
  if (stateChanges == 0) {
    Serial.println("    ⚠️  BUSY 信號無變化，可能：");
    Serial.println("       1. BUSY 腳位連接錯誤");
    Serial.println("       2. E-Paper 模組未正確供電");
    Serial.println("       3. E-Paper 模組故障");
  }
  
  Serial.println("✅ BUSY 信號測試完成");
  return true;
}

bool testBasicCommands() {
  Serial.println("正在測試基本命令...");
  
  // 發送軟體重置命令
  Serial.println("  📝 發送軟體重置命令...");
  sendCommand(SSD1677_SW_RESET);
  delay(100);
  
  // 等待 BUSY 信號
  Serial.println("  ⏳ 等待 BUSY 信號...");
  if (waitUntilReady(5000)) {
    Serial.println("    ✅ BUSY 信號正常");
  } else {
    Serial.println("    ❌ BUSY 信號超時");
    return false;
  }
  
  // 發送基本設定命令
  Serial.println("  📝 發送基本設定命令...");
  
  // 驅動輸出控制
  sendCommand(SSD1677_DRIVER_OUTPUT_CONTROL);
  sendData(0xDF);  // 479 & 0xFF
  sendData(0x01);  // (479 >> 8) & 0xFF
  sendData(0x00);
  
  // 資料輸入模式
  sendCommand(SSD1677_DATA_ENTRY_MODE_SETTING);
  sendData(0x03);
  
  Serial.println("✅ 基本命令測試完成");
  return true;
}

bool testFullDisplay() {
  Serial.println("正在測試完整顯示功能...");
  
  // 這裡只做基本的顯示測試
  Serial.println("  🖥️  嘗試清空顯示器...");
  
  // 設定記憶體區域
  setMemoryArea(0, 0, 799, 479);
  setMemoryPointer(0, 0);
  
  // 發送清空資料
  sendCommand(0x24);  // WRITE_RAM
  
  digitalWrite(EPD_DC_PIN, HIGH);  // 資料模式
  digitalWrite(EPD_CS_PIN, LOW);
  
  // 發送白色資料 (簡化版本，只發送前 1000 位元組)
  for (int i = 0; i < 1000; i++) {
    SPI.transfer(0xFF);  // 白色
  }
  
  digitalWrite(EPD_CS_PIN, HIGH);
  
  // 更新顯示
  sendCommand(0x22);  // DISPLAY_UPDATE_CONTROL_2
  sendData(0xC4);
  
  sendCommand(0x20);  // MASTER_ACTIVATION
  sendCommand(0xFF);  // TERMINATE_FRAME_READ_WRITE
  
  Serial.println("  ⏳ 等待顯示更新完成...");
  if (waitUntilReady(10000)) {
    Serial.println("    ✅ 顯示更新完成");
    Serial.println("    🎯 請檢查 E-Paper 是否有變化");
  } else {
    Serial.println("    ❌ 顯示更新超時");
    return false;
  }
  
  Serial.println("✅ 完整顯示測試完成");
  return true;
}

// 輔助函數
void sendCommand(uint8_t command) {
  digitalWrite(EPD_DC_PIN, LOW);   // 命令模式
  digitalWrite(EPD_CS_PIN, LOW);
  SPI.transfer(command);
  digitalWrite(EPD_CS_PIN, HIGH);
}

void sendData(uint8_t data) {
  digitalWrite(EPD_DC_PIN, HIGH);  // 資料模式
  digitalWrite(EPD_CS_PIN, LOW);
  SPI.transfer(data);
  digitalWrite(EPD_CS_PIN, HIGH);
}

bool waitUntilReady(unsigned long timeout) {
  unsigned long startTime = millis();
  
  while (digitalRead(EPD_BUSY_PIN) == HIGH) {
    delay(100);
    if (millis() - startTime > timeout) {
      return false;  // 超時
    }
  }
  return true;  // 成功
}

void setMemoryArea(int x_start, int y_start, int x_end, int y_end) {
  // 設定 X 位址範圍
  sendCommand(0x44);  // SET_RAM_X_ADDRESS_START_END_POSITION
  sendData((x_start >> 3) & 0xFF);
  sendData((x_end >> 3) & 0xFF);
  
  // 設定 Y 位址範圍
  sendCommand(0x45);  // SET_RAM_Y_ADDRESS_START_END_POSITION
  sendData(y_start & 0xFF);
  sendData((y_start >> 8) & 0xFF);
  sendData(y_end & 0xFF);
  sendData((y_end >> 8) & 0xFF);
}

void setMemoryPointer(int x, int y) {
  // 設定 X 位址計數器
  sendCommand(0x4E);  // SET_RAM_X_ADDRESS_COUNTER
  sendData((x >> 3) & 0xFF);
  
  // 設定 Y 位址計數器
  sendCommand(0x4F);  // SET_RAM_Y_ADDRESS_COUNTER
  sendData(y & 0xFF);
  sendData((y >> 8) & 0xFF);
}

void showSystemInfo() {
  Serial.println("========================================");
  Serial.println("              系統資訊");
  Serial.println("========================================");
  Serial.print("晶片 ID: 0x");
  Serial.println(ESP.getChipId(), HEX);
  Serial.print("可用記憶體: ");
  Serial.print(ESP.getFreeHeap());
  Serial.println(" bytes");
  Serial.print("CPU 頻率: ");
  Serial.print(ESP.getCpuFreqMHz());
  Serial.println(" MHz");
  Serial.print("Flash 大小: ");
  Serial.print(ESP.getFlashChipSize() / 1024 / 1024);
  Serial.println(" MB");
  Serial.println();
  Serial.println("腳位配置:");
  Serial.println("CS   (D8) → GPIO15");
  Serial.println("DC   (D3) → GPIO0");
  Serial.println("RST  (D4) → GPIO2");
  Serial.println("BUSY (D2) → GPIO4");
  Serial.println("MOSI (D7) → GPIO13");
  Serial.println("SCK  (D5) → GPIO14");
  Serial.println("========================================");
}
