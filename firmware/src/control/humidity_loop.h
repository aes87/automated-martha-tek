#pragma once
#include "../sensors/sensor_hub.h"
#include "../relay/relay_manager.h"
#include <cstdint>

/**
 * humidity_loop.h — RH → Fogger + Tub Fan control loop.
 *
 * Reads SensorSnapshot.rh_aggregate_pct (aggregation mode configurable).
 * Hysteresis: fogger ON at RH < RH_SETPOINT_PCT, OFF at RH > (setpoint + hysteresis).
 * Fogger and Tub Fan always switch together atomically.
 * Minimum HUMIDITY_COOLDOWN_MS between state changes prevents relay chatter.
 */
class HumidityLoop {
public:
    HumidityLoop() = default;

    /**
     * tick(snapshot, relay, now_ms) — Evaluate control logic and apply relay commands.
     * Call from the control task every CONTROL_TASK_PERIOD_MS.
     */
    void tick(const SensorSnapshot& snapshot, RelayManager& relay, uint32_t now_ms);

    /** setThresholds() — Override default on/off thresholds at runtime. */
    void setThresholds(float on_rh, float hysteresis_pct);

    bool isFogging() const { return _fogging; }
    uint32_t lastChangeMs() const { return _last_change_ms; }

private:
    float    _on_rh       = RH_SETPOINT_PCT;
    float    _hysteresis  = RH_HYSTERESIS_PCT;
    bool     _fogging     = false;
    uint32_t _last_change_ms = 0;
};
