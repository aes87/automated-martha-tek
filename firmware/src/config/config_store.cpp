#include "config_store.h"
#include "../util/logger.h"

#ifndef NATIVE_TEST
#include <Preferences.h>
#include <ArduinoJson.h>
#include <cstring>

ConfigStore Config;

void ConfigStore::begin() {
    _prefs.begin(NVS_NAMESPACE, false);

    bool has_data = _prefs.isKey("rh_on");
    if (!has_data) {
        Log.info("cfg", "NVS empty; writing defaults");
        loadDefaults();
    } else {
        _load();
        Log.info("cfg", "Config loaded from NVS");
    }
}

void ConfigStore::loadDefaults() {
    _cfg = MarthaConfig{};  // Zero/default-initialise
    _save();
}

void ConfigStore::set(const MarthaConfig& cfg) {
    _cfg = cfg;
    _save();
}

void ConfigStore::_load() {
    _prefs.getString("wifi_ssid", _cfg.wifi_ssid, sizeof(_cfg.wifi_ssid));
    _prefs.getString("wifi_pass", _cfg.wifi_pass, sizeof(_cfg.wifi_pass));

    _cfg.rh_on_pct     = _prefs.getFloat("rh_on",    DEFAULT_RH_ON_PCT);
    _cfg.rh_hysteresis = _prefs.getFloat("rh_hyst",  DEFAULT_RH_HYSTERESIS_PCT);
    _cfg.co2_on_ppm    = _prefs.getFloat("co2_on",   DEFAULT_CO2_ON_PPM);
    _cfg.co2_off_ppm   = _prefs.getFloat("co2_off",  DEFAULT_CO2_OFF_PPM);
    _cfg.water_low_pct  = _prefs.getFloat("wl_low",  DEFAULT_WATER_LOW_PCT);
    _cfg.water_high_pct = _prefs.getFloat("wl_high", DEFAULT_WATER_HIGH_PCT);
    _cfg.adc_water_min_mv = _prefs.getUInt("adc_min", DEFAULT_ADC_WATER_MIN_MV);
    _cfg.adc_water_max_mv = _prefs.getUInt("adc_max", DEFAULT_ADC_WATER_MAX_MV);
    _cfg.rh_aggregation   = _prefs.getUChar("rh_agg", DEFAULT_RH_AGGREGATION);
    _cfg.log_level        = _prefs.getUChar("log_lvl", DEFAULT_LOG_LEVEL);

    _prefs.getString("timezone", _cfg.timezone, sizeof(_cfg.timezone));

    _cfg.timer.lights_on_minute  = _prefs.getUShort("l_on",   DEFAULT_LIGHTS_ON_MIN);
    _cfg.timer.lights_off_minute = _prefs.getUShort("l_off",  DEFAULT_LIGHTS_OFF_MIN);
    _cfg.timer.uvc_on_min        = _prefs.getUShort("uvc_on", DEFAULT_UVC_ON_MIN);
    _cfg.timer.uvc_off_min       = _prefs.getUShort("uvc_off",DEFAULT_UVC_OFF_MIN);

    // Probe labels
    for (int i = 0; i < 5; ++i) {
        char key[12];
        snprintf(key, sizeof(key), "probe_%d", i);
        _prefs.getString(key, _cfg.probe_labels[i], sizeof(_cfg.probe_labels[i]));
    }
}

void ConfigStore::_save() {
    _prefs.putString("wifi_ssid", _cfg.wifi_ssid);
    _prefs.putString("wifi_pass", _cfg.wifi_pass);

    _prefs.putFloat("rh_on",    _cfg.rh_on_pct);
    _prefs.putFloat("rh_hyst",  _cfg.rh_hysteresis);
    _prefs.putFloat("co2_on",   _cfg.co2_on_ppm);
    _prefs.putFloat("co2_off",  _cfg.co2_off_ppm);
    _prefs.putFloat("wl_low",   _cfg.water_low_pct);
    _prefs.putFloat("wl_high",  _cfg.water_high_pct);
    _prefs.putUInt("adc_min",   _cfg.adc_water_min_mv);
    _prefs.putUInt("adc_max",   _cfg.adc_water_max_mv);
    _prefs.putUChar("rh_agg",   _cfg.rh_aggregation);
    _prefs.putUChar("log_lvl",  _cfg.log_level);

    _prefs.putString("timezone", _cfg.timezone);

    _prefs.putUShort("l_on",    _cfg.timer.lights_on_minute);
    _prefs.putUShort("l_off",   _cfg.timer.lights_off_minute);
    _prefs.putUShort("uvc_on",  _cfg.timer.uvc_on_min);
    _prefs.putUShort("uvc_off", _cfg.timer.uvc_off_min);

    for (int i = 0; i < 5; ++i) {
        char key[12];
        snprintf(key, sizeof(key), "probe_%d", i);
        _prefs.putString(key, _cfg.probe_labels[i]);
    }
}

void ConfigStore::exportJson(JsonDocument& doc) const {
    doc["rh_on_pct"]     = _cfg.rh_on_pct;
    doc["rh_hysteresis"] = _cfg.rh_hysteresis;
    doc["co2_on_ppm"]    = _cfg.co2_on_ppm;
    doc["co2_off_ppm"]   = _cfg.co2_off_ppm;
    doc["water_low_pct"] = _cfg.water_low_pct;
    doc["water_high_pct"]= _cfg.water_high_pct;
    doc["rh_aggregation"]= _cfg.rh_aggregation;
    doc["log_level"]     = _cfg.log_level;
    doc["timezone"]      = _cfg.timezone;
    doc["wifi_ssid"]     = _cfg.wifi_ssid;
    // wifi_pass intentionally omitted from export

    auto timer = doc["timer"].to<JsonObject>();
    timer["lights_on_minute"]  = _cfg.timer.lights_on_minute;
    timer["lights_off_minute"] = _cfg.timer.lights_off_minute;
    timer["uvc_on_min"]        = _cfg.timer.uvc_on_min;
    timer["uvc_off_min"]       = _cfg.timer.uvc_off_min;

    auto adc = doc["adc"].to<JsonObject>();
    adc["water_min_mv"] = _cfg.adc_water_min_mv;
    adc["water_max_mv"] = _cfg.adc_water_max_mv;

    auto labels = doc["probe_labels"].to<JsonArray>();
    for (int i = 0; i < 5; ++i) labels.add(_cfg.probe_labels[i]);
}

bool ConfigStore::importJson(const JsonDocument& doc) {
    MarthaConfig c = _cfg;  // Start from current config

    if (doc["rh_on_pct"].is<float>())      c.rh_on_pct     = doc["rh_on_pct"].as<float>();
    if (doc["rh_hysteresis"].is<float>())  c.rh_hysteresis = doc["rh_hysteresis"].as<float>();
    if (doc["co2_on_ppm"].is<float>())     c.co2_on_ppm    = doc["co2_on_ppm"].as<float>();
    if (doc["co2_off_ppm"].is<float>())    c.co2_off_ppm   = doc["co2_off_ppm"].as<float>();
    if (doc["water_low_pct"].is<float>())  c.water_low_pct = doc["water_low_pct"].as<float>();
    if (doc["water_high_pct"].is<float>()) c.water_high_pct= doc["water_high_pct"].as<float>();
    if (doc["rh_aggregation"].is<int>())   c.rh_aggregation= doc["rh_aggregation"].as<uint8_t>();
    if (doc["log_level"].is<int>())        c.log_level     = doc["log_level"].as<uint8_t>();
    if (doc["timezone"].is<const char*>()) {
        strlcpy(c.timezone, doc["timezone"].as<const char*>(), sizeof(c.timezone));
    }
    if (doc["wifi_ssid"].is<const char*>()) {
        strlcpy(c.wifi_ssid, doc["wifi_ssid"].as<const char*>(), sizeof(c.wifi_ssid));
    }
    if (doc["wifi_pass"].is<const char*>()) {
        strlcpy(c.wifi_pass, doc["wifi_pass"].as<const char*>(), sizeof(c.wifi_pass));
    }

    // Timer
    if (doc["timer"]["lights_on_minute"].is<int>())
        c.timer.lights_on_minute = doc["timer"]["lights_on_minute"].as<uint16_t>();
    if (doc["timer"]["lights_off_minute"].is<int>())
        c.timer.lights_off_minute = doc["timer"]["lights_off_minute"].as<uint16_t>();
    if (doc["timer"]["uvc_on_min"].is<int>())
        c.timer.uvc_on_min = doc["timer"]["uvc_on_min"].as<uint16_t>();
    if (doc["timer"]["uvc_off_min"].is<int>())
        c.timer.uvc_off_min = doc["timer"]["uvc_off_min"].as<uint16_t>();

    // ADC calibration
    if (doc["adc"]["water_min_mv"].is<int>())
        c.adc_water_min_mv = doc["adc"]["water_min_mv"].as<uint32_t>();
    if (doc["adc"]["water_max_mv"].is<int>())
        c.adc_water_max_mv = doc["adc"]["water_max_mv"].as<uint32_t>();

    // Probe labels
    if (doc["probe_labels"].is<JsonArrayConst>()) {
        auto arr = doc["probe_labels"].as<JsonArrayConst>();
        for (int i = 0; i < 5 && i < (int)arr.size(); ++i) {
            if (arr[i].is<const char*>()) {
                strlcpy(c.probe_labels[i], arr[i].as<const char*>(), sizeof(c.probe_labels[i]));
            }
        }
    }

    // Validation
    if (c.rh_on_pct < 50.0f || c.rh_on_pct > 99.0f)            return false;
    if (c.co2_on_ppm <= c.co2_off_ppm)                           return false;
    if (c.water_low_pct >= c.water_high_pct)                     return false;
    if (c.timer.lights_on_minute > 1439)                         return false;
    if (c.timer.lights_off_minute > 1439)                        return false;

    set(c);
    return true;
}

#endif  // !NATIVE_TEST
