#include "web_server.h"
#include "api.h"
#include "ws_broadcaster.h"
#include "../util/logger.h"

#ifndef NATIVE_TEST
#include <LittleFS.h>
#include <ESPAsyncWebServer.h>
#include <ElegantOTA.h>

AsyncWebServer WebServer(80);

void webServerBegin() {
    // Mount LittleFS
    if (!LittleFS.begin(true)) {
        Log.warn("web", "LittleFS mount failed; serving API only");
    } else {
        Log.info("web", "LittleFS mounted");
    }

    // Serve static files from /data/
    WebServer.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");

    // Register REST API routes
    apiRegisterRoutes(WebServer);

    // ElegantOTA
    ElegantOTA.begin(&WebServer);
    Log.info("web", "ElegantOTA registered at /update");

    // WebSocket
    WsBroadcast.begin(WebServer);

    // 404 handler
    WebServer.onNotFound([](AsyncWebServerRequest* req) {
        req->send(404, "text/plain", "Not found");
    });

    WebServer.begin();
    Log.info("web", "HTTP server started on port 80");
}
#endif
