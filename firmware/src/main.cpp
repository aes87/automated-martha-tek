/**
 * main.cpp — Martha Tent Controller firmware entry point.
 *
 * Setup flow:
 *   1. Serial + logger init
 *   2. RelayManager::begin() — all relays OFF, boot lock starts
 *   3. I2C + 1-Wire bus init
 *   4. SensorHub::begin() — creates sensor polling FreeRTOS task
 *   5. NVS ConfigStore::begin() — loads persisted config
 *   6. WiFi connect (STA) or AP fallback
 *   7. mDNS + NTP
 *   8. WebServer + WebSocket + OTA init
 *   9. Hardware watchdog init
 *  10. Control FreeRTOS task started
 *  11. loop() feeds watchdog + RelayManager::tick()
 */

#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <esp_task_wdt.h>

#include "../include/config.h"
#include "../include/hal.h"

#include "util/logger.h"
#include "relay/relay_manager.h"
#include "relay/relay_channel.h"
#include "sensors/sensor_hub.h"
#include "sensors/water_level.h"
#include "control/humidity_loop.h"
#include "control/co2_loop.h"
#include "control/timer_scheduler.h"
#include "config/config_store.h"
#include "web/web_server.h"
#include "web/ws_broadcaster.h"

// ── Module-level instances ────────────────────────────────────────────────────
RelayManager  Relay;
SensorHub     Sensors;
WaterLevel    WaterLevelSensor;

HumidityLoop  HumLoop;
Co2Loop       CO2Loop;
TimerScheduler Scheduler;

// ── Control task ──────────────────────────────────────────────────────────────
static void controlTask(void* /*arg*/) {
    esp_task_wdt_add(nullptr);  // Register this task with the hardware watchdog

    for (;;) {
        uint32_t now = millis();

        // Feed watchdog
        esp_task_wdt_reset();

        // Advance relay manager state machine (BOOT_LOCKED → ARMED)
        Relay.tick();

        // Read sensor snapshot
        SensorSnapshot snap;
        if (Sensors.read(snap)) {
            HumLoop.tick(snap, Relay, now);
            CO2Loop.tick(snap, Relay, now);
        }

        // Pump control
        WaterLevelSensor.tick();
        if (WaterLevelSensor.isValid()) {
            if (WaterLevelSensor.isBelowThreshold()) {
                Relay.set(RelayChannel::PUMP, true, RelaySource::PUMP_CTRL);
            } else if (WaterLevelSensor.isAboveThreshold()) {
                Relay.set(RelayChannel::PUMP, false, RelaySource::PUMP_CTRL);
            }
        }

        // Timer-based channels (UVC, Lights)
        Scheduler.tick(Relay, now);

        // WebSocket broadcast
        SensorSnapshot snap_ws;
        if (Sensors.read(snap_ws)) {
            WsBroadcast.tick(snap_ws, Relay, now);
        }

        vTaskDelay(pdMS_TO_TICKS(CONTROL_TASK_PERIOD_MS));
    }
}

// ── WiFi connect helper ───────────────────────────────────────────────────────
static void wifiConnect() {
    const MarthaConfig& cfg = Config.get();

    if (cfg.wifi_ssid[0] == '\0') {
        Log.warn("wifi", "No SSID configured; starting AP mode");
        goto start_ap;
    }

    Log.info("wifi", "Connecting to SSID: %s", cfg.wifi_ssid);
    WiFi.mode(WIFI_STA);
    WiFi.begin(cfg.wifi_ssid, cfg.wifi_pass);

    {
        uint32_t t0 = millis();
        while (WiFi.status() != WL_CONNECTED) {
            if ((millis() - t0) > WIFI_CONNECT_TIMEOUT_MS) {
                Log.warn("wifi", "Connection timeout; starting AP mode");
                goto start_ap;
            }
            delay(500);
        }
    }

    Log.info("wifi", "Connected! IP: %s", WiFi.localIP().toString().c_str());
    return;

start_ap:
    WiFi.mode(WIFI_AP);
    WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PASSWORD[0] ? WIFI_AP_PASSWORD : nullptr);
    Log.info("wifi", "AP started: %s  IP: %s",
             WIFI_AP_SSID, WiFi.softAPIP().toString().c_str());
}

// ── setup() ───────────────────────────────────────────────────────────────────
void setup() {
    Serial.begin(115200);
    delay(100);

    Log.info("main", "Martha Tent Controller v%s booting", MARTHA_FW_VERSION);

    // 1. Relay safety — arm all pins OUTPUT HIGH immediately
    Relay.begin();

    // 2. I2C bus
    Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL, I2C_CLOCK_HZ);
    Log.info("main", "I2C init SDA=%d SCL=%d", PIN_I2C_SDA, PIN_I2C_SCL);

    // 3. Water level ADC
    WaterLevelSensor.begin();

    // 4. NVS config
    Config.begin();

    // 5. Apply loaded config to control loops
    const MarthaConfig& cfg = Config.get();
    HumLoop.setThresholds(cfg.rh_on_pct, cfg.rh_hysteresis);
    CO2Loop.setThresholds(cfg.co2_on_ppm, cfg.co2_off_ppm);
    WaterLevelSensor.setCalibration(cfg.adc_water_min_mv, cfg.adc_water_max_mv);
    Scheduler.setConfig(cfg.timer);
    Log.setLevel(static_cast<LogLevel>(cfg.log_level));

    // 6. Sensor hub (starts FreeRTOS polling task)
    Sensors.setRhAggregation(static_cast<RhAggregation>(cfg.rh_aggregation));
    Sensors.begin();

    // 7. WiFi
    wifiConnect();

    // 8. mDNS
    if (MDNS.begin(MDNS_HOSTNAME)) {
        Log.info("wifi", "mDNS: http://%s.local", MDNS_HOSTNAME);
        MDNS.addService("http", "tcp", 80);
    }

    // 9. NTP
    Scheduler.begin(cfg.timezone);

    // 10. Web server + WebSocket + OTA
    webServerBegin();

    // 11. Hardware watchdog (30s timeout, panic on trigger)
    esp_task_wdt_config_t wdt_cfg = {
        .timeout_ms     = WDT_TIMEOUT_S * 1000U,
        .idle_core_mask = 0,
        .trigger_panic  = true,
    };
    esp_task_wdt_reconfigure(&wdt_cfg);

    // 12. Control FreeRTOS task
    xTaskCreatePinnedToCore(
        controlTask,
        "ctrl",
        CONTROL_TASK_STACK,
        nullptr,
        CONTROL_TASK_PRIORITY,
        nullptr,
        1  // Core 1 (Core 0 used by WiFi/BT stack)
    );

    Log.info("main", "Boot complete");
}

// ── loop() ────────────────────────────────────────────────────────────────────
void loop() {
    // Main loop is intentionally minimal — all work happens in FreeRTOS tasks.
    // ElegantOTA requires loop() to call its handler for OTA progress.
#ifndef NATIVE_TEST
    // ElegantOTA.loop() if using legacy (non-async) mode — check library version
#endif
    delay(10);
}
