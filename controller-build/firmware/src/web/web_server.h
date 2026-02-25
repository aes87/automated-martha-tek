#pragma once
/**
 * web_server.h — ESPAsyncWebServer initialisation.
 *
 * Serves LittleFS /data/ directory as static files.
 * Mounts all REST API routes (see api.h).
 * Mounts ElegantOTA web UI.
 * Starts WebSocket broadcaster (see ws_broadcaster.h).
 */
#ifndef NATIVE_TEST
#include <ESPAsyncWebServer.h>

extern AsyncWebServer WebServer;

/** webServerBegin() — Initialise and start the web server. */
void webServerBegin();
#endif
