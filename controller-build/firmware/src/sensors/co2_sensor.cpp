/**
 * co2_sensor.cpp — SCD30 CO2/temperature/humidity sensor driver.
 */

#include "co2_sensor.h"
#include "../util/logger.h"
#include "../../include/config.h"

#ifndef NATIVE_TEST

Co2Sensor CO2Sensor;

bool Co2Sensor::begin() {
    _scd30.begin(Wire, I2C_ADDR_SCD30);

    uint16_t err = _scd30.startPeriodicMeasurement(0);
    if (err != 0) {
        Log.error("co2", "SCD30 init failed (err=%u)", err);
        return false;
    }

    // Disable auto-calibration (indoor use — no periodic outdoor air exposure)
    _scd30.deactivateAutomaticSelfCalibration();

    Log.info("co2", "SCD30 initialised");
    return true;
}

std::optional<Co2Reading> Co2Sensor::read() {
    if (!isReady()) {
        // Check staleness
        if (_valid && (millis() - _last.timestamp_ms) > SENSOR_STALE_MS) {
            _valid = false;
            Log.warn("co2", "SCD30 reading stale (>%ums)", SENSOR_STALE_MS);
        }
        return _valid ? std::optional<Co2Reading>{_last} : std::nullopt;
    }

    float co2 = 0, temp = 0, rh = 0;
    uint16_t err = _scd30.readMeasurementData(co2, temp, rh);
    if (err != 0) {
        Log.warn("co2", "SCD30 read error=%u", err);
        return _valid ? std::optional<Co2Reading>{_last} : std::nullopt;
    }

    _last.co2_ppm      = co2;
    _last.temp_c       = temp;
    _last.rh_pct       = rh;
    _last.valid        = true;
    _last.timestamp_ms = millis();
    _valid = true;

    return _last;
}

bool Co2Sensor::isReady() {
    uint16_t ready = 0;
    _scd30.getDataReady(ready);
    return ready != 0;
}

#endif  // !NATIVE_TEST
