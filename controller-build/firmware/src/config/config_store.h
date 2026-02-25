#pragma once
#include "../control/timer_scheduler.h"
#include "../sensors/sensor_hub.h"
#include "defaults.h"
#include <cstdint>

/**
 * config_store.h — Preferences NVS wrapper (namespace "martha").
 *
 * Stores all user-configurable values that must survive reboots:
 *   WiFi SSID/password, control thresholds, schedules, calibration.
 *
 * exportJson() / importJson() bridge to the REST /api/config endpoints.
 * loadDefaults() is called on first boot when namespace is empty.
 */

struct MarthaConfig {
    // WiFi
    char wifi_ssid[64]     = {};
    char wifi_pass[64]     = {};

    // Control thresholds
    float rh_on_pct        = DEFAULT_RH_ON_PCT;
    float rh_hysteresis    = DEFAULT_RH_HYSTERESIS_PCT;
    float co2_on_ppm       = DEFAULT_CO2_ON_PPM;
    float co2_off_ppm      = DEFAULT_CO2_OFF_PPM;

    // Pump thresholds
    float water_low_pct    = DEFAULT_WATER_LOW_PCT;
    float water_high_pct   = DEFAULT_WATER_HIGH_PCT;

    // Schedules
    TimerConfig timer;

    // ADC calibration
    uint32_t adc_water_min_mv = DEFAULT_ADC_WATER_MIN_MV;
    uint32_t adc_water_max_mv = DEFAULT_ADC_WATER_MAX_MV;

    // RH aggregation (0=average, 1=min, 2=max)
    uint8_t rh_aggregation = DEFAULT_RH_AGGREGATION;

    // Timezone
    char timezone[48] = DEFAULT_TIMEZONE;

    // Log level
    uint8_t log_level = DEFAULT_LOG_LEVEL;

    // Per-probe shelf labels (null-terminated strings)
    char probe_labels[5][32] = {
        "Shelf1", "Shelf2", "Shelf3", "Shelf4", "Shelf5"
    };
};

#ifndef NATIVE_TEST
#include <Preferences.h>
#include <ArduinoJson.h>

class ConfigStore {
public:
    ConfigStore() = default;

    /** begin() — Open NVS namespace; load saved config or write defaults. */
    void begin();

    /** get() — Reference to in-memory config (read-only). */
    const MarthaConfig& get() const { return _cfg; }

    /** set() — Update in-memory config and persist to NVS. */
    void set(const MarthaConfig& cfg);

    /** loadDefaults() — Reset all values to compile-time defaults in NVS. */
    void loadDefaults();

    /**
     * exportJson(doc) — Serialise config to ArduinoJson document.
     * Used by GET /api/config.
     */
    void exportJson(JsonDocument& doc) const;

    /**
     * importJson(doc) — Update config from ArduinoJson document.
     * Used by POST /api/config. Returns false if validation fails.
     */
    bool importJson(const JsonDocument& doc);

private:
    Preferences  _prefs;
    MarthaConfig _cfg;

    void _load();
    void _save();
};

extern ConfigStore Config;
#endif
