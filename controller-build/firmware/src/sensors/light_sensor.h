#pragma once
#include "sensor_hub.h"
#include <optional>

/**
 * light_sensor.h — AS7341 11-channel spectral light sensor wrapper.
 *
 * Note: Waveshare AS7341 breakout board requires an I2C level shifter
 * (3.3V ↔ 5V) when powered from VIN. Verify your breakout pull-ups
 * reference 3.3V before connecting directly to ESP32 I2C pins.
 */
#ifndef NATIVE_TEST
#include <Adafruit_AS7341.h>

class LightSensor {
public:
    LightSensor() = default;

    /** begin() — initialise AS7341. Returns false if not found on I2C bus. */
    bool begin();

    /** readSpectrum() — Read all 11 channels. Returns nullopt on failure. */
    std::optional<LightReading> readSpectrum();

private:
    Adafruit_AS7341 _as7341;
    bool            _valid = false;
};

extern LightSensor LightSensorDev;
#endif
