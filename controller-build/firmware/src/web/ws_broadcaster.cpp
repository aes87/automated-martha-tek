#include "ws_broadcaster.h"
#include "../util/logger.h"
#include "../../include/config.h"

#ifndef NATIVE_TEST
#include <ArduinoJson.h>

WsBroadcaster WsBroadcast;

void WsBroadcaster::begin(AsyncWebServer& server) {
    _ws = new AsyncWebSocket(WS_PATH);

    _ws->onEvent([](AsyncWebSocket* /*server*/,
                    AsyncWebSocketClient* client,
                    AwsEventType type,
                    void* /*arg*/,
                    uint8_t* /*data*/,
                    size_t /*len*/) {
        if (type == WS_EVT_CONNECT) {
            Log.info("ws", "Client #%u connected", client->id());
        } else if (type == WS_EVT_DISCONNECT) {
            Log.info("ws", "Client #%u disconnected", client->id());
        }
    });

    server.addHandler(_ws);
    Log.info("ws", "WebSocket handler registered at %s", WS_PATH);
}

void WsBroadcaster::tick(const SensorSnapshot& snapshot,
                          const RelayManager& relay,
                          uint32_t now_ms) {
    if (!_ws) return;
    if ((now_ms - _last_broadcast_ms) < WS_BROADCAST_PERIOD_MS) return;
    if (_ws->count() == 0) {
        _last_broadcast_ms = now_ms;
        return;
    }

    // Reuse a static buffer to avoid heap fragmentation
    static char buf[1024];
    _buildJson(snapshot, relay, buf, sizeof(buf));
    _ws->textAll(buf);
    _last_broadcast_ms = now_ms;
}

void WsBroadcaster::_buildJson(const SensorSnapshot& snap,
                                const RelayManager& relay,
                                char* buf, size_t len) {
    JsonDocument doc;

    doc["t"] = millis();

    // CO2
    doc["co2"]  = snap.co2.co2_ppm;
    doc["rh_a"] = snap.rh_aggregate_pct;

    // Shelf RH
    auto rh = doc["rh"].to<JsonArray>();
    for (int i = 0; i < 3; ++i) rh.add(snap.rh[i].rh_pct);

    // Temps
    auto tp = doc["tp"].to<JsonArray>();
    for (int i = 0; i < DS18B20_PROBE_COUNT; ++i) tp.add(snap.temp_probe[i]);

    // Water
    doc["wl"] = snap.water_level_pct;

    // Relay states (bitmask: bit N = channel N state)
    uint8_t mask = 0;
    for (uint8_t i = 0; i < RELAY_CHANNEL_COUNT; ++i) {
        if (relay.get(static_cast<RelayChannel>(i))) mask |= (1 << i);
    }
    doc["rl"] = mask;
    doc["am"] = relay.isArmed();
    doc["mm"] = relay.isManualMode();

    serializeJson(doc, buf, len);
}

#endif  // !NATIVE_TEST
