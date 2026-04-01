#include "api.h"
#include "../sensors/sensor_hub.h"
#include "../sensors/water_level.h"
#include "../relay/relay_manager.h"
#include "../control/humidity_loop.h"
#include "../control/co2_loop.h"
#include "../control/timer_scheduler.h"
#include "../config/config_store.h"
#include "../util/logger.h"

#ifndef NATIVE_TEST
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>

// Module instances (defined in main.cpp or their respective .cpp files)
extern RelayManager   Relay;
extern SensorHub      Sensors;
extern HumidityLoop   HumLoop;
extern Co2Loop        CO2Loop;
extern TimerScheduler Scheduler;
extern WaterLevel     WaterLevelSensor;

// ── Helpers ───────────────────────────────────────────────────────────────────
static void sendJson(AsyncWebServerRequest* req, JsonDocument& doc, int code = 200) {
    String body;
    serializeJson(doc, body);
    req->send(code, "application/json", body);
}

static bool parseBody(AsyncWebServerRequest* req,
                      uint8_t* data, size_t len,
                      JsonDocument& out) {
    if (len > API_MAX_BODY_SIZE) {
        req->send(413, "application/json", "{\"error\":\"body too large\"}");
        return false;
    }
    DeserializationError err = deserializeJson(out, data, len);
    return (err == DeserializationError::Ok);
}

// ── GET /api/status ───────────────────────────────────────────────────────────
static void handleGetStatus(AsyncWebServerRequest* req) {
    SensorSnapshot snap;
    bool ok = Sensors.read(snap);

    JsonDocument doc;
    doc["ok"]      = ok;
    doc["uptime"]  = millis();
    doc["fw_ver"]  = MARTHA_FW_VERSION;

    // CO2
    auto co2 = doc["co2"].to<JsonObject>();
    co2["ppm"]   = snap.co2.co2_ppm;
    co2["temp"]  = snap.co2.temp_c;
    co2["rh"]    = snap.co2.rh_pct;
    co2["valid"] = snap.co2.valid;

    // Shelf humidity sensors
    auto rh_arr = doc["rh"].to<JsonArray>();
    for (int i = 0; i < 3; ++i) {
        auto entry = rh_arr.add<JsonObject>();
        entry["rh"]    = snap.rh[i].rh_pct;
        entry["temp"]  = snap.rh[i].temp_c;
        entry["valid"] = snap.rh[i].valid;
    }
    doc["rh_aggregate"] = snap.rh_aggregate_pct;

    // Substrate temps
    auto temps = doc["temps"].to<JsonArray>();
    for (int i = 0; i < DS18B20_PROBE_COUNT; ++i) {
        auto t = temps.add<JsonObject>();
        t["temp"]  = snap.temp_probe[i];
        t["valid"] = snap.temp_probe_valid[i];
    }

    // Water level
    doc["water_pct"]   = snap.water_level_pct;
    doc["water_valid"] = snap.water_level_valid;

    // Relay states
    auto relays = doc["relays"].to<JsonObject>();
    for (uint8_t i = 0; i < RELAY_CHANNEL_COUNT; ++i) {
        relays[RELAY_CHANNEL_NAMES[i]] = Relay.get(static_cast<RelayChannel>(i));
    }
    relays["armed"]       = Relay.isArmed();
    relays["manual_mode"] = Relay.isManualMode();

    sendJson(req, doc);
}

// ── GET /api/config ───────────────────────────────────────────────────────────
static void handleGetConfig(AsyncWebServerRequest* req) {
    JsonDocument doc;
    Config.exportJson(doc);
    sendJson(req, doc);
}

// ── POST /api/config (body handler) ──────────────────────────────────────────
static void handlePostConfigBody(AsyncWebServerRequest* req,
                                  uint8_t* data, size_t len,
                                  size_t /*index*/, size_t /*total*/) {
    JsonDocument doc;
    if (!parseBody(req, data, len, doc)) {
        req->send(400, "application/json", "{\"error\":\"invalid JSON\"}");
        return;
    }
    if (!Config.importJson(doc)) {
        req->send(422, "application/json", "{\"error\":\"validation failed\"}");
        return;
    }

    // Push updated config to running control loops (W2 fix)
    const MarthaConfig& cfg = Config.get();
    HumLoop.setThresholds(cfg.rh_on_pct, cfg.rh_hysteresis);
    CO2Loop.setThresholds(cfg.co2_on_ppm, cfg.co2_off_ppm);
    WaterLevelSensor.setCalibration(cfg.adc_water_min_mv, cfg.adc_water_max_mv);
    Scheduler.setConfig(cfg.timer);
    Sensors.setRhAggregation(static_cast<RhAggregation>(cfg.rh_aggregation));
    Log.setLevel(static_cast<LogLevel>(cfg.log_level));

    req->send(200, "application/json", "{\"ok\":true}");
}

// ── POST /api/relay/:ch/set (body handler) ────────────────────────────────────
static void handleRelaySetBody(AsyncWebServerRequest* req,
                                uint8_t* data, size_t len,
                                size_t /*index*/, size_t /*total*/) {
    const String& ch_str = req->pathArg(0);
    RelayChannel ch = RelayChannel::COUNT;

    // Match channel by name (case-insensitive)
    for (uint8_t i = 0; i < RELAY_CHANNEL_COUNT; ++i) {
        if (ch_str.equalsIgnoreCase(RELAY_CHANNEL_NAMES[i])) {
            ch = static_cast<RelayChannel>(i);
            break;
        }
    }
    // Also accept numeric index (but only if the string is actually a number)
    if (ch == RelayChannel::COUNT && ch_str.length() > 0) {
        bool is_numeric = true;
        for (unsigned int ci = 0; ci < ch_str.length(); ++ci) {
            if (ch_str[ci] < '0' || ch_str[ci] > '9') { is_numeric = false; break; }
        }
        if (is_numeric) {
            uint8_t idx = static_cast<uint8_t>(ch_str.toInt());
            if (idx < RELAY_CHANNEL_COUNT) ch = static_cast<RelayChannel>(idx);
        }
    }

    if (ch == RelayChannel::COUNT) {
        req->send(404, "application/json", "{\"error\":\"unknown channel\"}");
        return;
    }

    JsonDocument doc;
    if (!parseBody(req, data, len, doc) || !doc["state"].is<bool>()) {
        req->send(400, "application/json", "{\"error\":\"expected {state: bool}\"}");
        return;
    }

    bool state = doc["state"].as<bool>();
    bool ok = Relay.set(ch, state, RelaySource::API);
    String resp = ok ? "{\"ok\":true}" : "{\"ok\":false,\"error\":\"relay locked\"}";
    req->send(ok ? 200 : 503, "application/json", resp);
}

// ── POST /api/relay/manual (body handler) ────────────────────────────────────
static void handleRelayManualBody(AsyncWebServerRequest* req,
                                   uint8_t* data, size_t len,
                                   size_t /*index*/, size_t /*total*/) {
    JsonDocument doc;
    if (!parseBody(req, data, len, doc) || !doc["manual"].is<bool>()) {
        req->send(400, "application/json", "{\"error\":\"expected {manual: bool}\"}");
        return;
    }
    Relay.setManualMode(doc["manual"].as<bool>());
    req->send(200, "application/json", "{\"ok\":true}");
}

// ── POST /api/log-level (body handler) ───────────────────────────────────────
static void handleLogLevelBody(AsyncWebServerRequest* req,
                                uint8_t* data, size_t len,
                                size_t /*index*/, size_t /*total*/) {
    JsonDocument doc;
    if (!parseBody(req, data, len, doc) || !doc["level"].is<int>()) {
        req->send(400, "application/json", "{\"error\":\"expected {level: 0-3}\"}");
        return;
    }
    int lvl = doc["level"].as<int>();
    if (lvl < 0 || lvl > 3) {
        req->send(400, "application/json", "{\"error\":\"level must be 0-3\"}");
        return;
    }
    Log.setLevel(static_cast<LogLevel>(lvl));
    req->send(200, "application/json", "{\"ok\":true}");
}

// ── Route registration ────────────────────────────────────────────────────────
void apiRegisterRoutes(AsyncWebServer& server) {
    server.on("/api/status", HTTP_GET, handleGetStatus);
    server.on("/api/config", HTTP_GET, handleGetConfig);

    server.on("/api/config", HTTP_POST,
        [](AsyncWebServerRequest*){},  // header handler (unused)
        nullptr,
        handlePostConfigBody);

    server.on("/api/relay/manual", HTTP_POST,
        [](AsyncWebServerRequest*){},
        nullptr,
        handleRelayManualBody);

    // /api/relay/:ch/set — path parameter
    server.on("^\\/api\\/relay\\/([a-zA-Z0-9]+)\\/set$", HTTP_POST,
        [](AsyncWebServerRequest*){},
        nullptr,
        handleRelaySetBody);

    server.on("/api/log-level", HTTP_POST,
        [](AsyncWebServerRequest*){},
        nullptr,
        handleLogLevelBody);

    Log.info("api", "REST routes registered");
}

#endif  // !NATIVE_TEST
