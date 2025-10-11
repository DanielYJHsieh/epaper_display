/**
 * power_manager.h
 * 
 * 電源管理模組 - 負責休眠決策、定時喚醒、低電量保護
 * 
 * 硬體需求:
 * - GPIO16 (D0) 必須連接到 RST 腳位（Deep Sleep 喚醒用）
 * - BatteryMonitor 實例（提供電池狀態）
 * 
 * 使用範例:
 *   PowerManager power;
 *   power.begin(&batteryMonitor);
 *   if (power.shouldSleep()) {
 *       uint64_t sleepTime = power.getDefaultSleepTime();
 *       power.enterDeepSleep(sleepTime);
 *   }
 * 
 * 作者: battery_management 專案團隊
 * 版本: 1.0
 * 日期: 2025-10-12
 */

#ifndef POWER_MANAGER_H
#define POWER_MANAGER_H

#include <Arduino.h>
#include "battery_monitor.h"

class PowerManager {
public:
    /**
     * 初始化電源管理
     * @param batteryMonitor 電池監控器指標
     */
    void begin(BatteryMonitor* batteryMonitor);

    /**
     * 判斷是否應該進入休眠
     * 根據電池狀態與活動超時判斷
     * @return true=需要休眠, false=繼續運行
     */
    bool shouldSleep();

    /**
     * 進入 Deep Sleep 模式
     * ⚠️ 注意: GPIO16 (D0) 必須連接到 RST！
     * @param sleepTimeUs 休眠時間（微秒），0=不喚醒（直到充電或手動 RST）
     */
    void enterDeepSleep(uint64_t sleepTimeUs);

    /**
     * 根據電池狀態自動決定休眠時間
     * - 充電中: 30 分鐘
     * - 正常/良好: 1 小時
     * - 低電量: 3 小時
     * - 極低: 不喚醒
     * @return 建議休眠時間（微秒）
     */
    uint64_t getDefaultSleepTime();

    /**
     * 處理低電量情況
     * 發送警告並進入長時間休眠
     */
    void handleLowBattery();

    /**
     * 更新活動計時器（重置超時）
     * 在 WebSocket 連接、接收資料等活動時呼叫
     */
    void updateTimer();

    /**
     * 設定活動超時時間（秒）
     * 預設 30 秒
     * @param timeoutSeconds 超時時間（秒）
     */
    void setActiveTimeout(uint32_t timeoutSeconds);

    /**
     * 獲取自上次活動以來的時間（秒）
     * @return 經過時間（秒）
     */
    uint32_t getTimeSinceLastActivity();

    /**
     * 印出喚醒原因與統計資訊
     */
    void printWakeInfo();

private:
    BatteryMonitor* battery;

    // 上次活動時間戳記
    unsigned long lastActiveTime = 0;

    // 活動超時閾值（秒）
    uint32_t activeTimeout = 30;

    // Deep Sleep 喚醒 GPIO
    static const int WAKE_PIN = 16; // D0 (必須連接 RST)

    // 時間常數（微秒）
    static constexpr uint64_t SECOND_US = 1000000ULL;
    static constexpr uint64_t MINUTE_US = 60 * SECOND_US;
    static constexpr uint64_t HOUR_US = 60 * MINUTE_US;

    /**
     * 從 RTC 記憶體讀取喚醒次數
     * @return 喚醒次數
     */
    uint32_t getWakeCount();

    /**
     * 更新 RTC 記憶體中的喚醒次數
     */
    void incrementWakeCount();
};

#endif // POWER_MANAGER_H
