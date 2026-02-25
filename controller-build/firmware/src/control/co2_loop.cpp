#include "co2_loop.h"
#include "../util/logger.h"

void Co2Loop::tick(const SensorSnapshot& snapshot,
                   RelayManager& relay,
                   uint32_t now_ms) {
    if (!snapshot.co2.valid) {
        Log.warn("co2", "No valid CO2 reading; holding FAE state");
        return;
    }

    float co2 = snapshot.co2.co2_ppm;
    bool  want_fae;

    if (_flushing) {
        // Currently running FAE: keep running if CO2 still above off threshold
        // AND minimum run time has not yet elapsed
        bool min_run_met = (now_ms - _fan_on_ms) >= FAE_MIN_RUN_MS;
        want_fae = (co2 > _off_ppm) || !min_run_met;
    } else {
        want_fae = (co2 > _on_ppm);
    }

    if (want_fae != _flushing) {
        _flushing = want_fae;
        if (_flushing) {
            _fan_on_ms = now_ms;
        }

        // Atomically switch Exhaust and Intake fans
        relay.set(RelayChannel::EXHAUST, _flushing, RelaySource::CO2);
        relay.set(RelayChannel::INTAKE,  _flushing, RelaySource::CO2);

        Log.info("co2", "FAE %s @ CO2=%.0fppm (on=%.0f off=%.0f)",
                 _flushing ? "ON" : "OFF", co2, _on_ppm, _off_ppm);
    }
}

void Co2Loop::setThresholds(float on_ppm, float off_ppm) {
    if (off_ppm >= on_ppm) {
        Log.warn("co2", "Invalid CO2 thresholds on=%.0f off=%.0f; ignored", on_ppm, off_ppm);
        return;
    }
    _on_ppm  = on_ppm;
    _off_ppm = off_ppm;
    Log.info("co2", "CO2 thresholds updated: ON>%.0f OFF<%.0f", on_ppm, off_ppm);
}
