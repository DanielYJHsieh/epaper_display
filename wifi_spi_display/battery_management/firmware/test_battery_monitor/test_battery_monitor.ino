/**
 * test_battery_monitor.ino
 * 
 * 電池監控模組測試程式
 * 
 * 測試項目:
 * 1. ADC 電壓讀取精度
 * 2. 電量百分比計算
 * 3. 電池狀態判斷
 * 4. 充電檢測
 * 5. JSON 輸出格式
 * 
 * 硬體連接:
 * - A0: 分壓電路（電池 → 100kΩ → A0 → 33kΩ → GND）
 * - GPIO14 (D5): TP4056 CHRG（可選）
 * - GPIO12 (D6): TP4056 STDBY（可選）
 * 
 * 驗收標準:
 * - 電壓誤差 < 0.1V（與萬用表對比）
 * - 濾波後讀值穩定（波動 < 0.05V）
 * - 充電狀態正確識別
 */

#include "battery_monitor.h"

BatteryMonitor battery;

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("========================================");
    Serial.println("電池監控模組測試");
    Serial.println("========================================");
    
    // 初始化電池監控
    battery.begin();
    
    // 可選: 根據實測調整電壓校正係數
    // battery.setVoltageCalibration(4.25 / 1023.0);
    
    Serial.println("\n開始監控（每秒輸出一次）...\n");
}

void loop() {
    // 讀取電池資訊
    float voltage = battery.readVoltage();
    uint8_t percent = battery.getPercentage();
    BatteryMonitor::BatteryState state = battery.getState();
    const char* stateStr = battery.getStateString(state);
    bool charging = battery.isCharging();
    
    // 格式化輸出
    Serial.println("----------------------------------------");
    Serial.printf("電壓:   %.3f V\n", voltage);
    Serial.printf("電量:   %3d %%\n", percent);
    Serial.printf("狀態:   %s\n", stateStr);
    Serial.printf("充電:   %s\n", charging ? "是" : "否");
    
    // JSON 格式（用於 WebSocket 測試）
    Serial.println("\nJSON 格式:");
    Serial.println(battery.toJSON());
    
    // 狀態指示
    if (state == BatteryMonitor::BATTERY_CRITICAL) {
        Serial.println("\n⚠️  警告: 電量極低！");
    } else if (state == BatteryMonitor::BATTERY_LOW) {
        Serial.println("\n🔋 低電量警告");
    } else if (state == BatteryMonitor::BATTERY_CHARGING) {
        Serial.println("\n🔌 充電中...");
    }
    
    delay(1000);
}
