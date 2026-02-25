#pragma once
/**
 * api.h — REST API endpoint registration.
 *
 * Endpoints:
 *   GET  /api/status          — SensorSnapshot + relay states as JSON
 *   GET  /api/config          — Current thresholds and schedules
 *   POST /api/config          — Update config (ArduinoJson body; persists to NVS)
 *   POST /api/relay/:ch/set   — Manual relay override {"state": true/false}
 *   POST /api/relay/manual    — Enter/exit manual mode {"manual": true/false}
 *   GET  /api/ota             — ElegantOTA web UI
 *   POST /api/log-level       — Set log level {"level": 0-3}
 */
#ifndef NATIVE_TEST
#include <ESPAsyncWebServer.h>

/** apiRegisterRoutes(server) — Register all /api/* routes on server. */
void apiRegisterRoutes(AsyncWebServer& server);
#endif
