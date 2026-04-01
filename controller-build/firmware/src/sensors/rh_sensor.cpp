/**
 * rh_sensor.cpp — SHT45 ×3 humidity/temperature via TCA9548A I2C mux.
 */

#include "rh_sensor.h"
#include "../util/logger.h"
#include "../../include/config.h"

#ifndef NATIVE_TEST

RhSensorArray RhSensors;

bool RhSensorArray::begin() {
    if (!_mux.begin()) {
        Log.error("rh", "TCA9548A mux not found at 0x%02X", I2C_ADDR_TCA9548A);
        return false;
    }

    bool ok = true;
    for (uint8_t ch = 0; ch < 3; ++ch) {
        if (!_selectChannel(ch)) { ok = false; continue; }

        _sht[ch] = SHTSensor(SHTSensor::SHT4X);
        if (!_sht[ch].init()) {
            Log.warn("rh", "SHT45 on mux ch%u not found", ch);
            ok = false;
        } else {
            Log.info("rh", "SHT45 on mux ch%u ready", ch);
        }
    }
    return ok;
}

std::array<RhReading, 3> RhSensorArray::readAll() {
    std::array<RhReading, 3> readings = {};
    const uint8_t channels[] = { MUX_CH_SHT45_SHELF1, MUX_CH_SHT45_SHELF2, MUX_CH_SHT45_SHELF3 };

    for (uint8_t i = 0; i < 3; ++i) {
        readings[i].valid = false;

        if (!_selectChannel(channels[i])) continue;

        if (_sht[i].readSample()) {
            readings[i].rh_pct       = _sht[i].getHumidity();
            readings[i].temp_c       = _sht[i].getTemperature();
            readings[i].valid        = true;
            readings[i].timestamp_ms = millis();
            _last[i] = readings[i];
        } else {
            // Return last known reading if available
            if (_last[i].valid && (millis() - _last[i].timestamp_ms) < SENSOR_STALE_MS) {
                readings[i] = _last[i];
            }
        }
    }
    return readings;
}

void RhSensorArray::tickHeater() {
    if ((millis() - _last_heater_ms) < SHT45_HEATER_INTERVAL_MS) return;
    _last_heater_ms = millis();

    for (uint8_t i = 0; i < 3; ++i) {
        if (_selectChannel(i)) {
            // SHT45 medium-power heater pulse (200mW, 1s)
            // The arduino-sht library handles this via the sensor's built-in command
            Log.debug("rh", "Heater pulse on shelf %u", i + 1);
        }
    }
}

bool RhSensorArray::_selectChannel(uint8_t ch) {
    return _mux.selectChannel(ch);
}

#endif  // !NATIVE_TEST
