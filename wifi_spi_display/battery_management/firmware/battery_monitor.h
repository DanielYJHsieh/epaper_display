/**
 * battery_monitor.h
 * 
 * 電池監控模組 - 負責 ADC 電壓讀取、電量估算、充電狀態檢測
 * 
 * 硬體需求:
 * - ADC (A0): 連接分壓電路（電池電壓 → 100kΩ → A0 → 33kΩ → GND）
 * - GPIO14 (D5): 連接 TP4056 CHRG 腳位（可選）
 * - GPIO12 (D6): 連接 TP4056 STDBY 腳位（可選）
 * 
 * 使用範例:
 *   BatteryMonitor battery;
 *   battery.begin();
 *   float voltage = battery.readVoltage();
 *   uint8_t percent = battery.getPercentage();
 * 
 * 作者: battery_management 專案團隊
 * 版本: 1.0
 * 日期: 2025-10-12
 */

#ifndef BATTERY_MONITOR_H
#define BATTERY_MONITOR_H

#include <Arduino.h>

class BatteryMonitor {
public:
    /**
     * 電池狀態列舉
     */
    enum BatteryState {
        BATTERY_FULL,        // 充滿 (>= 4.0V)
        BATTERY_GOOD,        // 良好 (3.5-4.0V)
        BATTERY_NORMAL,      // 正常 (3.3-3.5V)
        BATTERY_LOW,         // 低電量 (3.0-3.3V)
        BATTERY_CRITICAL,    // 極低 (< 3.0V)
        BATTERY_CHARGING     // 充電中
    };

    /**
     * 初始化電池監控
     * 設定 GPIO 為輸入、初始化濾波器
     */
    void begin();

    /**
     * 讀取電池電壓（經校正與濾波）
     * @return 電池電壓 (V)，範圍 2.75-4.2V
     */
    float readVoltage();

    /**
     * 獲取電量百分比
     * @return 電量百分比 0-100%
     */
    uint8_t getPercentage();

    /**
     * 獲取電池狀態
     * @return BatteryState 列舉值
     */
    BatteryState getState();

    /**
     * 檢查是否正在充電
     * @return true=充電中, false=電池供電
     */
    bool isCharging();

    /**
     * 取得狀態字串（用於序列埠輸出）
     * @param state 電池狀態
     * @return 狀態描述字串
     */
    const char* getStateString(BatteryState state);

    /**
     * 輸出 JSON 格式電池資訊（用於 WebSocket 傳輸）
     * 格式: {"voltage":3.85,"percentage":60,"state":"GOOD","charging":false}
     * @return JSON 字串
     */
    String toJSON();

    /**
     * 設定電壓校正係數（根據實測調整）
     * 預設值: 4.2 / 1023.0
     * @param calibration 校正係數
     */
    void setVoltageCalibration(float calibration);

private:
    // GPIO 定義
    static const int ADC_PIN = A0;
    static const int PIN_CHRG = 14;   // D5 - TP4056 CHRG
    static const int PIN_STDBY = 12;  // D6 - TP4056 STDBY

    // 電壓校正係數（可透過 setVoltageCalibration() 調整）
    float voltageCalibration = 4.2 / 1023.0;

    // 低通濾波器參數
    float filteredVoltage = 0.0;
    static constexpr float FILTER_ALPHA = 0.2; // 0-1，越小越平滑

    // 電壓與電量對應表（簡化線性模型）
    static constexpr float VOLTAGE_MIN = 3.0;   // 0%
    static constexpr float VOLTAGE_MAX = 4.2;   // 100%

    /**
     * 原始 ADC 讀取（未濾波）
     * @return 原始電壓值
     */
    float readRawVoltage();
};

#endif // BATTERY_MONITOR_H
