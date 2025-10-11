/**
 * test_power_manager.ino
 * 
 * 電源管理模組測試程式
 * 
 * 測試項目:
 * 1. Deep Sleep 進入與喚醒
 * 2. 定時喚醒準確度
 * 3. 喚醒次數統計
 * 4. 低電量保護
 * 
 * 硬體連接:
 * - GPIO16 (D0) **必須** 連接到 RST（Deep Sleep 喚醒用）
 * - A0: 分壓電路
 * 
 * 測試流程:
 * 1. 開機後顯示喚醒資訊
 * 2. 讀取電池狀態
 * 3. 等待 10 秒
 * 4. 進入 Deep Sleep（30 秒）
 * 5. 自動喚醒並重複
 * 
 * 驗收標準:
 * - 每 30 秒自動喚醒
 * - 喚醒次數正確累加
 * - Deep Sleep 電流 < 0.1mA
 */

#include "battery_monitor.h"
#include "power_manager.h"

BatteryMonitor battery;
PowerManager power;

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n\n");
    Serial.println("========================================");
    Serial.println("電源管理模組測試");
    Serial.println("========================================");
    
    // 初始化電池監控
    battery.begin();
    
    // 初始化電源管理
    power.begin(&battery);
    
    // 顯示電池狀態
    Serial.println("\n當前電池狀態:");
    Serial.printf("電壓: %.2fV\n", battery.readVoltage());
    Serial.printf("電量: %d%%\n", battery.getPercentage());
    Serial.printf("狀態: %s\n", battery.getStateString(battery.getState()));
    Serial.printf("充電: %s\n", battery.isCharging() ? "是" : "否");
    
    // 檢查是否為低電量（測試低電量保護）
    if (battery.getState() == BatteryMonitor::BATTERY_CRITICAL) {
        Serial.println("\n⚠️  電量過低，測試低電量保護...");
        power.handleLowBattery();
        return; // 不會執行到這裡（已進入 Deep Sleep）
    }
    
    // 倒數計時
    Serial.println("\n========================================");
    Serial.println("10 秒後進入 Deep Sleep (30 秒)");
    Serial.println("========================================");
    
    for (int i = 10; i > 0; i--) {
        Serial.printf("倒數: %d 秒...\n", i);
        delay(1000);
    }
    
    // 進入 Deep Sleep（30 秒）
    Serial.println("\n進入 Deep Sleep（30 秒後自動喚醒）");
    Serial.flush();
    
    power.enterDeepSleep(30 * 1000000ULL); // 30 秒（微秒）
}

void loop() {
    // 不會執行到這裡（Deep Sleep 會重啟系統）
}
