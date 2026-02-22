#pragma once
#include "sensor_hub.h"
#include <optional>
#include <array>

/**
 * rh_sensor.h — SHT45 ×3 relative humidity/temperature sensors via TCA9548A mux.
 *
 * The three SHT45s share I2C address 0x44; the TCA9548A selects which one
 * is active by enabling the corresponding mux channel.
 *
 * The SHT45 on-chip heater is scheduled to run once per SHT45_HEATER_INTERVAL_MS
 * to prevent condensation on the sensor die at high RH.
 */
#ifndef NATIVE_TEST
#include <SHTSensor.h>
#include <TCA9548.h>

class RhSensorArray {
public:
    RhSensorArray() = default;

    /** begin() — initialise TCA9548A and all three SHT45s. */
    bool begin();

    /**
     * readAll() — Read all three SHT45 sensors.
     * Returns array of 3 RhReading; individual entries have valid=false on failure.
     */
    std::array<RhReading, 3> readAll();

    /** tickHeater() — Call regularly; fires heater burst on schedule. */
    void tickHeater();

private:
    TCA9548   _mux;
    SHTSensor _sht[3];
    uint32_t  _last_heater_ms = 0;
    RhReading _last[3]        = {};

    bool _selectChannel(uint8_t ch);
};

extern RhSensorArray RhSensors;
#endif
