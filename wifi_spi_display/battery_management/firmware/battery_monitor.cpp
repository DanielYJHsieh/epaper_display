/**
 * battery_monitor.cpp
 * 
 * 電池監控模組實作
 */

#include "battery_monitor.h"

void BatteryMonitor::begin() {
    // 設定充電狀態檢測 GPIO 為輸入上拉
    pinMode(PIN_CHRG, INPUT_PULLUP);
    pinMode(PIN_STDBY, INPUT_PULLUP);

    // 初始化濾波器（讀取初始電壓）
    filteredVoltage = readRawVoltage();

    Serial.println("[BatteryMonitor] 初始化完成");
    Serial.printf("[BatteryMonitor] 初始電壓: %.2fV\n", filteredVoltage);
}

float BatteryMonitor::readRawVoltage() {
    int adcValue = analogRead(ADC_PIN);
    float rawVoltage = adcValue * voltageCalibration;
    return rawVoltage;
}

float BatteryMonitor::readVoltage() {
    float rawVoltage = readRawVoltage();

    // 指數移動平均濾波（EMA）
    filteredVoltage = (FILTER_ALPHA * rawVoltage) + 
                      ((1.0 - FILTER_ALPHA) * filteredVoltage);

    return filteredVoltage;
}

uint8_t BatteryMonitor::getPercentage() {
    float voltage = readVoltage();

    // 邊界檢查
    if (voltage >= VOLTAGE_MAX) return 100;
    if (voltage <= VOLTAGE_MIN) return 0;

    // 線性插值: (V - Vmin) / (Vmax - Vmin) * 100
    float percent = (voltage - VOLTAGE_MIN) / (VOLTAGE_MAX - VOLTAGE_MIN) * 100.0;

    return (uint8_t)percent;
}

BatteryMonitor::BatteryState BatteryMonitor::getState() {
    // 充電檢測優先
    if (isCharging()) {
        return BATTERY_CHARGING;
    }

    float voltage = readVoltage();

    // 根據電壓分級
    if (voltage >= 4.0)  return BATTERY_FULL;
    if (voltage >= 3.5)  return BATTERY_GOOD;
    if (voltage >= 3.3)  return BATTERY_NORMAL;
    if (voltage >= 3.0)  return BATTERY_LOW;
    return BATTERY_CRITICAL;
}

bool BatteryMonitor::isCharging() {
    // TP4056 充電邏輯（低電平有效）
    bool chrg = (digitalRead(PIN_CHRG) == LOW);   // 充電中
    bool stdby = (digitalRead(PIN_STDBY) == LOW); // 充滿

    return chrg || stdby;
}

const char* BatteryMonitor::getStateString(BatteryState state) {
    switch (state) {
        case BATTERY_FULL:      return "FULL";
        case BATTERY_GOOD:      return "GOOD";
        case BATTERY_NORMAL:    return "NORMAL";
        case BATTERY_LOW:       return "LOW";
        case BATTERY_CRITICAL:  return "CRITICAL";
        case BATTERY_CHARGING:  return "CHARGING";
        default:                return "UNKNOWN";
    }
}

String BatteryMonitor::toJSON() {
    float voltage = readVoltage();
    uint8_t percent = getPercentage();
    BatteryState state = getState();
    const char* stateStr = getStateString(state);
    bool charging = isCharging();

    // 組合 JSON 字串
    String json = "{";
    json += "\"voltage\":" + String(voltage, 2) + ",";
    json += "\"percentage\":" + String(percent) + ",";
    json += "\"state\":\"" + String(stateStr) + "\",";
    json += "\"charging\":" + String(charging ? "true" : "false");
    json += "}";

    return json;
}

void BatteryMonitor::setVoltageCalibration(float calibration) {
    voltageCalibration = calibration;
    Serial.printf("[BatteryMonitor] 電壓校正係數更新: %.6f\n", calibration);
}
