/**
 * sensor_hub.cpp — Aggregated sensor polling task implementation.
 *
 * Creates a FreeRTOS task that polls all sensors at SENSOR_TASK_PERIOD_MS
 * intervals and updates the SensorSnapshot under mutex.
 */

#include "sensor_hub.h"
#include "../util/logger.h"
#include "../../include/config.h"

#ifndef NATIVE_TEST

#include "co2_sensor.h"
#include "rh_sensor.h"
#include "temp_probe.h"
#include "light_sensor.h"
#include "water_level.h"

#include <algorithm>
#include <cfloat>

SensorHub Sensors;

void SensorHub::begin() {
    _mutex = xSemaphoreCreateMutex();

    // Initialise all sensor drivers
    CO2Sensor.begin();
    RhSensors.begin();
    TempProbeArray.begin();
    LightSensorDev.begin();

    // Create polling task
    xTaskCreatePinnedToCore(
        _task, "sensors",
        SENSOR_TASK_STACK, this,
        SENSOR_TASK_PRIORITY, &_task_handle,
        0  // Core 0 — sensor I/O separate from control on Core 1
    );

    Log.info("sensors", "SensorHub started (period=%dms)", SENSOR_TASK_PERIOD_MS);
}

bool SensorHub::read(SensorSnapshot& out) const {
    if (!_initialized) return false;
    xSemaphoreTake(_mutex, portMAX_DELAY);
    out = _snapshot;
    xSemaphoreGive(_mutex);
    return true;
}

void SensorHub::_task(void* arg) {
    auto* self = static_cast<SensorHub*>(arg);
    for (;;) {
        self->_poll();
        vTaskDelay(pdMS_TO_TICKS(SENSOR_TASK_PERIOD_MS));
    }
}

void SensorHub::_poll() {
    xSemaphoreTake(_mutex, portMAX_DELAY);

    // CO2 (SCD30)
    auto co2 = CO2Sensor.read();
    if (co2.has_value()) {
        _snapshot.co2 = co2.value();
    } else if ((millis() - _snapshot.co2.timestamp_ms) > SENSOR_STALE_MS) {
        _snapshot.co2.valid = false;
    }

    // RH × 3 (SHT45 via TCA9548A)
    auto rh_readings = RhSensors.readAll();
    for (int i = 0; i < 3; ++i) {
        _snapshot.rh[i] = rh_readings[i];
    }
    RhSensors.tickHeater();

    // Temperature probes (DS18B20)
    auto temps = TempProbeArray.readAll();
    for (int i = 0; i < DS18B20_PROBE_COUNT; ++i) {
        _snapshot.temp_probe[i]       = temps[i].temp_c;
        _snapshot.temp_probe_valid[i] = temps[i].valid;
        _snapshot.temp_probe_ts[i]    = temps[i].timestamp_ms;
    }

    // Light (AS7341)
    auto light = LightSensorDev.readSpectrum();
    if (light.has_value()) {
        _snapshot.light = light.value();
    }

    // Water level is handled separately via WaterLevelSensor.tick() in controlTask
    if (WaterLevelSensor.isValid()) {
        _snapshot.water_level_pct   = WaterLevelSensor.getLevelPercent();
        _snapshot.water_level_valid = true;
        _snapshot.water_level_ts    = millis();
    } else {
        _snapshot.water_level_valid = false;
    }

    _updateAggregate();
    _initialized = true;

    xSemaphoreGive(_mutex);
}

void SensorHub::_updateAggregate() {
    float sum = 0.0f, temp_sum = 0.0f;
    int   count = 0;
    float min_rh = FLT_MAX, max_rh = -FLT_MAX;

    for (int i = 0; i < 3; ++i) {
        if (!_snapshot.rh[i].valid) continue;
        float rh = _snapshot.rh[i].rh_pct;
        float tc = _snapshot.rh[i].temp_c;
        sum      += rh;
        temp_sum += tc;
        count++;
        if (rh < min_rh) min_rh = rh;
        if (rh > max_rh) max_rh = rh;
    }

    if (count == 0) {
        _snapshot.rh_aggregate_pct = 0.0f;
        _snapshot.temp_aggregate_c = 0.0f;
        return;
    }

    switch (_rh_mode) {
        case RhAggregation::MIN:     _snapshot.rh_aggregate_pct = min_rh;        break;
        case RhAggregation::MAX:     _snapshot.rh_aggregate_pct = max_rh;        break;
        case RhAggregation::AVERAGE:
        default:                     _snapshot.rh_aggregate_pct = sum / count;   break;
    }
    _snapshot.temp_aggregate_c = temp_sum / count;
}

#endif  // !NATIVE_TEST
