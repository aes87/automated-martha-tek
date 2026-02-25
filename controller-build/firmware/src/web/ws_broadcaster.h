#pragma once
/**
 * ws_broadcaster.h — WebSocket push broadcaster.
 *
 * Pushes a JSON SensorSnapshot to all connected WebSocket clients
 * every WS_BROADCAST_PERIOD_MS (2 seconds).
 *
 * WebSocket path: WS_PATH ("/ws")
 */
#ifndef NATIVE_TEST
#include <ESPAsyncWebServer.h>
#include "../sensors/sensor_hub.h"
#include "../relay/relay_manager.h"

class WsBroadcaster {
public:
    WsBroadcaster() = default;

    /** begin(server) — Register WebSocket handler on server at WS_PATH. */
    void begin(AsyncWebServer& server);

    /**
     * tick(snapshot, relay, now_ms) — Build and broadcast JSON to all clients.
     * Respects WS_BROADCAST_PERIOD_MS interval.
     */
    void tick(const SensorSnapshot& snapshot, const RelayManager& relay, uint32_t now_ms);

private:
    AsyncWebSocket* _ws = nullptr;
    uint32_t        _last_broadcast_ms = 0;

    void _buildJson(const SensorSnapshot& snap, const RelayManager& relay, char* buf, size_t len);
};

extern WsBroadcaster WsBroadcast;
#endif
