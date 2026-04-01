/**
 * light_sensor.cpp — AS7341 11-channel spectral light sensor driver.
 */

#include "light_sensor.h"
#include "../util/logger.h"

#ifndef NATIVE_TEST

LightSensor LightSensorDev;

bool LightSensor::begin() {
    if (!_as7341.begin()) {
        Log.warn("light", "AS7341 not found on I2C bus");
        return false;
    }

    _as7341.setATIME(100);
    _as7341.setASTEP(999);
    _as7341.setGain(AS7341_GAIN_256X);

    _valid = true;
    Log.info("light", "AS7341 initialised");
    return true;
}

std::optional<LightReading> LightSensor::readSpectrum() {
    if (!_valid) return std::nullopt;

    if (!_as7341.readAllChannels()) {
        Log.warn("light", "AS7341 read failed");
        return std::nullopt;
    }

    LightReading reading = {};
    reading.channels[0]  = _as7341.getChannel(AS7341_CHANNEL_415nm_F1);
    reading.channels[1]  = _as7341.getChannel(AS7341_CHANNEL_445nm_F2);
    reading.channels[2]  = _as7341.getChannel(AS7341_CHANNEL_480nm_F3);
    reading.channels[3]  = _as7341.getChannel(AS7341_CHANNEL_515nm_F4);
    reading.channels[4]  = _as7341.getChannel(AS7341_CHANNEL_555nm_F5);
    reading.channels[5]  = _as7341.getChannel(AS7341_CHANNEL_590nm_F6);
    reading.channels[6]  = _as7341.getChannel(AS7341_CHANNEL_630nm_F7);
    reading.channels[7]  = _as7341.getChannel(AS7341_CHANNEL_680nm_F8);
    reading.channels[8]  = _as7341.getChannel(AS7341_CHANNEL_CLEAR);
    reading.channels[9]  = _as7341.getChannel(AS7341_CHANNEL_NIR);
    reading.channels[10] = 0;  // Flicker detection (not a standard channel)
    reading.valid        = true;
    reading.timestamp_ms = millis();

    return reading;
}

#endif  // !NATIVE_TEST
