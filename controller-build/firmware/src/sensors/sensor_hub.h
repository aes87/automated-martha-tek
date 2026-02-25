#pragma once
#include <cstdint>
#include <optional>

/**
 * sensor_hub.h — Aggregated sensor snapshot and FreeRTOS polling task.
 *
 * SensorSnapshot is a plain struct populated by the sensor task every
 * SENSOR_TASK_PERIOD_MS milliseconds. Readers access it under a mutex.
 */

struct RhReading {
    float    rh_pct;
    float    temp_c;
    bool     valid;
    uint32_t timestamp_ms;
};

struct Co2Reading {
    float    co2_ppm;
    float    temp_c;
    float    rh_pct;
    bool     valid;
    uint32_t timestamp_ms;
};

struct LightReading {
    uint16_t channels[11];  // AS7341 11-channel spectral data
    bool     valid;
    uint32_t timestamp_ms;
};

struct SensorSnapshot {
    // Three SHT45 shelf sensors (shelves 1–3)
    RhReading  rh[3];

    // SCD30 CO2 sensor
    Co2Reading co2;

    // DS18B20 substrate temperatures (up to 5 probes)
    float    temp_probe[5];
    bool     temp_probe_valid[5];
    uint32_t temp_probe_ts[5];

    // Water level ADC
    float    water_level_pct;
    bool     water_level_valid;
    uint32_t water_level_ts;

    // AS7341 spectral light sensor
    LightReading light;

    // Aggregated RH (configurable: min/avg/max across shelf sensors)
    float    rh_aggregate_pct;
    float    temp_aggregate_c;
};

/**
 * Aggregation mode for RH control loop.
 * AVERAGE: average of all valid shelf sensors (default)
 * MIN:     most-conservative (lowest RH) reading
 * MAX:     most-aggressive (highest RH) reading
 */
enum class RhAggregation : uint8_t { AVERAGE = 0, MIN = 1, MAX = 2 };

#ifndef NATIVE_TEST
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

class SensorHub {
public:
    SensorHub() = default;

    /** begin() — initialise all sensor drivers and create FreeRTOS polling task. */
    void begin();

    /**
     * read(out) — Copy the latest SensorSnapshot under mutex.
     * Returns true if snapshot has been populated at least once.
     */
    bool read(SensorSnapshot& out) const;

    /** setRhAggregation() — Controls which RH value is written to rh_aggregate_pct. */
    void setRhAggregation(RhAggregation mode) { _rh_mode = mode; }

private:
    static void _task(void* arg);
    void        _poll();
    void        _updateAggregate();

    SemaphoreHandle_t _mutex        = nullptr;
    TaskHandle_t      _task_handle  = nullptr;
    SensorSnapshot    _snapshot     = {};
    bool              _initialized  = false;
    RhAggregation     _rh_mode      = RhAggregation::AVERAGE;
};

extern SensorHub Sensors;

#endif  // !NATIVE_TEST
