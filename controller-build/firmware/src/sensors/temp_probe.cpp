/**
 * temp_probe.cpp — DS18B20 substrate temperature probes via 1-Wire.
 */

#include "temp_probe.h"
#include "../util/logger.h"

#ifndef NATIVE_TEST

TempProbes TempProbeArray;

uint8_t TempProbes::begin() {
    _dt.begin();
    _dt.setResolution(12);  // 12-bit = ~0.0625°C, ~750ms conversion

    _count = std::min(static_cast<uint8_t>(_dt.getDeviceCount()),
                      static_cast<uint8_t>(DS18B20_PROBE_COUNT));

    // Store ROM addresses (sorted ascending for deterministic ordering)
    for (uint8_t i = 0; i < _count; ++i) {
        _dt.getAddress(_roms[i], i);
    }

    Log.info("temp", "%u DS18B20 probes found on GPIO %d", _count, PIN_ONE_WIRE);
    return _count;
}

std::array<TempProbeReading, DS18B20_PROBE_COUNT> TempProbes::readAll() {
    std::array<TempProbeReading, DS18B20_PROBE_COUNT> readings = {};

    _dt.requestTemperatures();  // Blocking: ~750ms at 12-bit

    for (uint8_t i = 0; i < DS18B20_PROBE_COUNT; ++i) {
        readings[i].valid = false;

        if (i >= _count) continue;

        float temp = _dt.getTempC(_roms[i]);
        if (temp == DEVICE_DISCONNECTED_C) {
            Log.warn("temp", "Probe %u disconnected", i);
            continue;
        }

        readings[i].temp_c       = temp;
        readings[i].valid        = true;
        readings[i].timestamp_ms = millis();
        memcpy(readings[i].rom, _roms[i], 8);
    }

    return readings;
}

const uint8_t* TempProbes::getRom(uint8_t idx) const {
    if (idx >= _count) return nullptr;
    return _roms[idx];
}

#endif  // !NATIVE_TEST
