#include "humidity_loop.h"
#include "../util/logger.h"

void HumidityLoop::tick(const SensorSnapshot& snapshot,
                        RelayManager& relay,
                        uint32_t now_ms) {
    // Use the pre-aggregated RH value from SensorHub
    float rh = snapshot.rh_aggregate_pct;

    // Check if any RH sensor is valid
    bool any_valid = false;
    for (int i = 0; i < 3; ++i) {
        if (snapshot.rh[i].valid) { any_valid = true; break; }
    }
    if (!any_valid) {
        // No valid sensor data — fail safe: leave fogger in current state
        Log.warn("humidity", "No valid RH sensors; holding fogger state");
        return;
    }

    bool want_fog;
    if (_fogging) {
        // Currently fogging: stop when RH rises above setpoint + hysteresis
        want_fog = (rh < (_on_rh + _hysteresis));
    } else {
        // Not fogging: start when RH drops below setpoint
        want_fog = (rh < _on_rh);
    }

    // Apply cooldown — don't change state more often than HUMIDITY_COOLDOWN_MS
    if (want_fog != _fogging) {
        if ((now_ms - _last_change_ms) < HUMIDITY_COOLDOWN_MS) {
            // Cooldown not elapsed; hold current state
            return;
        }
        _fogging        = want_fog;
        _last_change_ms = now_ms;

        // Atomically switch both Fogger and Tub Fan
        relay.set(RelayChannel::FOGGER,  _fogging, RelaySource::HUMIDITY);
        relay.set(RelayChannel::TUB_FAN, _fogging, RelaySource::HUMIDITY);

        Log.info("humidity", "Fogger %s @ RH=%.1f%% (threshold=%.1f%%)",
                 _fogging ? "ON" : "OFF", rh, _on_rh);
    }
}

void HumidityLoop::setThresholds(float on_rh, float hysteresis_pct) {
    _on_rh      = on_rh;
    _hysteresis = hysteresis_pct;
    Log.info("humidity", "Thresholds updated: ON<%.1f OFF>%.1f", on_rh, on_rh + hysteresis_pct);
}
