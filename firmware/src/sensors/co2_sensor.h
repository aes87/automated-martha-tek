#pragma once
#include "sensor_hub.h"
#include <optional>

/**
 * co2_sensor.h — SCD30 CO2/temperature/humidity sensor wrapper.
 *
 * Auto-calibration is disabled for indoor use — the SCD30's auto-calibration
 * assumes periodic exposure to ~400 ppm outdoor air, which doesn't apply here.
 */
#ifndef NATIVE_TEST
#include <SensirionI2cScd30.h>

class Co2Sensor {
public:
    Co2Sensor() = default;

    /** begin() — initialise SCD30. Returns false if sensor not found. */
    bool begin();

    /**
     * read() — Return latest measurement.
     * Returns nullopt if sensor has not produced a valid reading, or if
     * more than SENSOR_STALE_MS has passed since the last valid reading.
     */
    std::optional<Co2Reading> read();

    /** isReady() — True if the SCD30 has a new measurement available. */
    bool isReady();

private:
    SensirionI2cScd30 _scd30;
    Co2Reading        _last  = {};
    bool              _valid = false;
};

extern Co2Sensor CO2Sensor;
#endif
