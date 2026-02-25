#pragma once
#include "../sensors/sensor_hub.h"
#include "../relay/relay_manager.h"
#include <cstdint>

/**
 * co2_loop.h — CO2 → FAE (Exhaust + Intake fan) control loop.
 *
 * Hysteresis: FAE ON at CO2 > CO2_ON_PPM, OFF at CO2 < CO2_OFF_PPM.
 * Minimum run time: FAE_MIN_RUN_MS per cycle (prevents premature shutdown).
 * Exhaust and Intake fans always switch together atomically.
 */
class Co2Loop {
public:
    Co2Loop() = default;

    /**
     * tick(snapshot, relay, now_ms) — Evaluate control logic and apply relay commands.
     * Call from the control task every CONTROL_TASK_PERIOD_MS.
     */
    void tick(const SensorSnapshot& snapshot, RelayManager& relay, uint32_t now_ms);

    /** setThresholds() — Override default on/off CO2 thresholds at runtime. */
    void setThresholds(float on_ppm, float off_ppm);

    bool isFlushing() const { return _flushing; }
    uint32_t lastOnMs()  const { return _fan_on_ms; }

private:
    float    _on_ppm    = static_cast<float>(CO2_ON_PPM);
    float    _off_ppm   = static_cast<float>(CO2_OFF_PPM);
    bool     _flushing  = false;
    uint32_t _fan_on_ms = 0;  // millis() when fans were last turned on
};
