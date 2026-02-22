#pragma once
#include "../util/rolling_average.h"
#include "../../include/config.h"
#include <cstdint>

/**
 * water_level.h — Reservoir water level via ADC on GPIO 34.
 *
 * HARDWARE SAFETY: GPIO 34 must be protected with a 1kΩ series resistor
 * and 3.3V Zener (or SMBJ3V3A TVS) before connecting any water-level
 * transmitter. See build guide C3. KIT0139 boards include this protection.
 *
 * Reads ADC voltage, converts to percentage using calibrated min/max,
 * applies a 32-sample rolling average, and provides hysteretic threshold
 * checks for the pump control loop.
 */

class WaterLevel {
public:
    WaterLevel() = default;

    /**
     * begin() — Initialise ADC calibration via esp_adc_cal_characterize().
     * Must be called once in setup() before any reads.
     */
    void begin();

    /**
     * tick() — Sample ADC and push into rolling average.
     * Call regularly (e.g. every SENSOR_TASK_PERIOD_MS).
     */
    void tick();

    /**
     * getLevelPercent() — Returns 0.0–100.0 from rolling average.
     * Returns -1.0 if fewer than 4 samples have been collected.
     */
    float getLevelPercent() const;

    /**
     * isBelowThreshold(low_pct) — True when rolling average is below low_pct
     * AND has been valid for at least one full window.
     * Used to trigger pump ON.
     */
    bool isBelowThreshold(float low_pct = WATER_LEVEL_LOW_PCT) const;

    /**
     * isAboveThreshold(high_pct) — True when rolling average is above high_pct.
     * Used to trigger pump OFF.
     */
    bool isAboveThreshold(float high_pct = WATER_LEVEL_HIGH_PCT) const;

    /**
     * setCalibration(min_mv, max_mv) — Override ADC voltage range at runtime.
     * Values are stored in NVS by ConfigStore.
     */
    void setCalibration(uint32_t min_mv, uint32_t max_mv);

    /** isValid() — True once the rolling average has at least 4 samples. */
    bool isValid() const { return _avg.count() >= 4; }

#ifdef NATIVE_TEST
    /** In native tests: inject a voltage reading directly (bypasses ADC). */
    void injectVoltage(uint32_t mv) { _pushVoltage(mv); }
#endif

private:
    RollingAverage<float, WATER_LEVEL_SAMPLES> _avg;
    uint32_t _min_mv = ADC_WATER_LEVEL_MIN_MV;
    uint32_t _max_mv = ADC_WATER_LEVEL_MAX_MV;

    void _pushVoltage(uint32_t mv_raw);
    uint32_t _readAdcMv() const;
};

extern WaterLevel WaterLevelSensor;
