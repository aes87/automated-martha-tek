#include "water_level.h"
#include "../util/logger.h"
#include <cstdint>
#include <algorithm>

#ifdef NATIVE_TEST
// ADC not available in native test builds; injectVoltage() is used instead.
static void adc_init() {}
static uint32_t adc_read_mv() { return 0; }
#else
#include <Arduino.h>
#include <esp_adc_cal.h>
static esp_adc_cal_characteristics_t _adc_chars;

static void adc_init() {
    adc1_config_width(ADC_WIDTH);
    adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN);  // GPIO34 = ADC1_CH6
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN, ADC_WIDTH, 1100, &_adc_chars);
}

static uint32_t adc_read_mv() {
    uint32_t raw = adc1_get_raw(ADC1_CHANNEL_6);
    return esp_adc_cal_raw_to_voltage(raw, &_adc_chars);
}
#endif

WaterLevel WaterLevelSensor;

void WaterLevel::begin() {
#ifndef NATIVE_TEST
    adc_init();
#endif
    _avg.reset();
    Log.info("water", "WaterLevel init min=%umV max=%umV", _min_mv, _max_mv);
}

void WaterLevel::tick() {
#ifndef NATIVE_TEST
    uint32_t mv = _readAdcMv();
    _pushVoltage(mv);
#endif
}

void WaterLevel::_pushVoltage(uint32_t mv_raw) {
    // Clamp to calibrated range
    uint32_t clamped = std::max(_min_mv, std::min(_max_mv, mv_raw));
    float pct = 0.0f;
    if (_max_mv > _min_mv) {
        pct = 100.0f * static_cast<float>(clamped - _min_mv)
                     / static_cast<float>(_max_mv - _min_mv);
    }
    _avg.push(pct);
}

uint32_t WaterLevel::_readAdcMv() const {
#ifndef NATIVE_TEST
    return adc_read_mv();
#else
    return 0;
#endif
}

float WaterLevel::getLevelPercent() const {
    if (_avg.count() < 4) return -1.0f;
    return _avg.average();
}

bool WaterLevel::isBelowThreshold(float low_pct) const {
    if (!isValid()) return false;
    return _avg.average() < low_pct;
}

bool WaterLevel::isAboveThreshold(float high_pct) const {
    if (!isValid()) return false;
    return _avg.average() > high_pct;
}

void WaterLevel::setCalibration(uint32_t min_mv, uint32_t max_mv) {
    if (max_mv <= min_mv) {
        Log.warn("water", "Invalid calibration min=%u max=%u; ignored", min_mv, max_mv);
        return;
    }
    _min_mv = min_mv;
    _max_mv = max_mv;
    _avg.reset();
    Log.info("water", "Calibration updated min=%umV max=%umV", min_mv, max_mv);
}
