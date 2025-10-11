/**
 * power_manager.cpp
 * 
 * 電源管理模組實作
 */

#include "power_manager.h"

void PowerManager::begin(BatteryMonitor* batteryMonitor) {
    battery = batteryMonitor;
    lastActiveTime = millis();

    Serial.println("[PowerManager] 初始化完成");
    printWakeInfo();
}

bool PowerManager::shouldSleep() {
    // 1. 檢查電池狀態
    BatteryMonitor::BatteryState state = battery->getState();

    // 極低電量立即休眠
    if (state == BatteryMonitor::BATTERY_CRITICAL) {
        Serial.println("[PowerManager] ⚠️ 電量極低，強制休眠");
        return true;
    }

    // 2. 檢查活動超時
    uint32_t elapsed = getTimeSinceLastActivity();
    if (elapsed >= activeTimeout) {
        Serial.printf("[PowerManager] 活動超時 (%lu 秒)，準備休眠\n", elapsed);
        return true;
    }

    return false;
}

uint64_t PowerManager::getDefaultSleepTime() {
    BatteryMonitor::BatteryState state = battery->getState();

    switch (state) {
        case BatteryMonitor::BATTERY_CHARGING:
            Serial.println("[PowerManager] 充電中，休眠 30 分鐘");
            return 30 * MINUTE_US;

        case BatteryMonitor::BATTERY_FULL:
        case BatteryMonitor::BATTERY_GOOD:
            Serial.println("[PowerManager] 電量充足，休眠 1 小時");
            return 1 * HOUR_US;

        case BatteryMonitor::BATTERY_NORMAL:
            Serial.println("[PowerManager] 電量正常，休眠 2 小時");
            return 2 * HOUR_US;

        case BatteryMonitor::BATTERY_LOW:
            Serial.println("[PowerManager] 低電量，休眠 3 小時");
            return 3 * HOUR_US;

        case BatteryMonitor::BATTERY_CRITICAL:
            Serial.println("[PowerManager] 極低電量，不喚醒");
            return 0; // 不喚醒，需充電或手動重啟

        default:
            return 1 * HOUR_US; // 預設 1 小時
    }
}

void PowerManager::enterDeepSleep(uint64_t sleepTimeUs) {
    Serial.println("========================================");
    Serial.println("[PowerManager] 進入 Deep Sleep");

    if (sleepTimeUs == 0) {
        Serial.println("[PowerManager] 喚醒時間: 不喚醒（等待充電或手動重啟）");
    } else {
        uint32_t seconds = sleepTimeUs / SECOND_US;
        uint32_t minutes = seconds / 60;
        uint32_t hours = minutes / 60;

        Serial.print("[PowerManager] 喚醒時間: ");
        if (hours > 0) {
            Serial.printf("%lu 小時 %lu 分鐘\n", hours, minutes % 60);
        } else if (minutes > 0) {
            Serial.printf("%lu 分鐘 %lu 秒\n", minutes, seconds % 60);
        } else {
            Serial.printf("%lu 秒\n", seconds);
        }
    }

    // 更新 RTC 記憶體中的喚醒次數
    incrementWakeCount();

    // 顯示電池狀態
    Serial.println("[PowerManager] 休眠前電池狀態:");
    Serial.println(battery->toJSON());

    // 確保序列埠輸出完成
    Serial.flush();
    delay(100);

    // 進入 Deep Sleep
    ESP.deepSleep(sleepTimeUs);
}

void PowerManager::handleLowBattery() {
    Serial.println("========================================");
    Serial.println("[PowerManager] 🔋 低電量警告");
    Serial.printf("[PowerManager] 電池電壓: %.2fV\n", battery->readVoltage());
    Serial.println("[PowerManager] 進入省電模式（3 小時休眠）");

    // 可在此加入 WebSocket 警告發送
    // webSocket.sendTXT("{\"type\":\"warning\",\"msg\":\"Low battery\"}");

    // 進入長時間休眠
    enterDeepSleep(3 * HOUR_US);
}

void PowerManager::updateTimer() {
    lastActiveTime = millis();
}

void PowerManager::setActiveTimeout(uint32_t timeoutSeconds) {
    activeTimeout = timeoutSeconds;
    Serial.printf("[PowerManager] 活動超時設定: %lu 秒\n", activeTimeout);
}

uint32_t PowerManager::getTimeSinceLastActivity() {
    return (millis() - lastActiveTime) / 1000; // 轉換為秒
}

void PowerManager::printWakeInfo() {
    Serial.println("========================================");
    Serial.println("[PowerManager] 喚醒資訊");

    // 喚醒原因
    String resetReason = ESP.getResetReason();
    Serial.println("[PowerManager] 喚醒原因: " + resetReason);

    // 喚醒次數
    uint32_t wakeCount = getWakeCount();
    Serial.printf("[PowerManager] 累計喚醒次數: %lu\n", wakeCount);

    // 運行時間
    Serial.printf("[PowerManager] 系統運行時間: %lu 秒\n", millis() / 1000);

    Serial.println("========================================");
}

uint32_t PowerManager::getWakeCount() {
    uint32_t count = 0;
    ESP.rtcUserMemoryRead(0, &count, sizeof(count));
    return count;
}

void PowerManager::incrementWakeCount() {
    uint32_t count = getWakeCount();
    count++;
    ESP.rtcUserMemoryWrite(0, &count, sizeof(count));
}
