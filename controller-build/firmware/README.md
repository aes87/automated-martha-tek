# Martha Tent Controller — Firmware

ESP32-based firmware for the automated Martha tent fruiting chamber.
Replaces ~$233 of off-the-shelf CO₂ and humidity controllers with a single board
providing real-time sensor data, closed-loop control, and a WiFi dashboard.

**Hardware prerequisite:** [DIY Controller Build Guide](../hardware/diy-controller-build-guide.md)
— read the hardware guide and complete all C1–C4 safety corrections before flashing.

---

## Features

| Feature | Detail |
|---------|--------|
| CO₂ / FAE control | SCD30 → Exhaust + Intake fans; hysteresis 800–950 ppm; 60s min run |
| Humidity control | 3× SHT45 via TCA9548A → Fogger + Tub fan; hysteresis 85–87% RH |
| UVC timer | Configurable duty cycle (default 1h on / 4h off); 10s boot guard |
| Grow lights timer | Configurable daily on/off window (default 08:00–20:00 UTC) |
| Substrate temps | 5× DS18B20 on 1-Wire |
| Spectral light | AS7341 11-channel |
| Pump control | ADC water level → pump relay; 20–80% hysteresis |
| Web dashboard | Live sensor cards, sparkline charts, relay toggles, config panel |
| OTA updates | ElegantOTA at `http://martha.local/update` |
| REST API | Full config read/write, relay override, log level |
| Watchdog | Hardware WDT (30s); config persists across reboots via NVS |

---

## Hardware

See the [build guide](../hardware/diy-controller-build-guide.md) for full BOM,
wiring diagrams, and safety corrections.

### GPIO Summary

| GPIO | Function | Notes |
|------|----------|-------|
| 4 | 1-Wire (DS18B20 ×5) | 2.2 kΩ pull-up + 100 nF decoupling |
| 16 | Relay Ch1 — Fogger | 10 kΩ pull-up to relay VCC |
| 17 | Relay Ch2 — Tub fan | 10 kΩ pull-up to relay VCC |
| 18 | Relay Ch3 — Exhaust fan | |
| 19 | Relay Ch4 — Intake fan | |
| 21 | I2C SDA | All I2C sensors |
| 22 | I2C SCL | All I2C sensors |
| 23 | Relay Ch5 — UVC lights | 10s boot guard |
| 25 | Relay Ch6 — Grow lights | |
| 26 | Relay Ch7 — Pump (optional) | |
| 27 | Relay Ch8 — Spare | |
| 34 | ADC — Water level | 1 kΩ + 3.3V Zener protection required |

### I2C Devices

| Device | Address | Purpose |
|--------|---------|---------|
| TCA9548A | 0x70 | I2C mux for 3× SHT45 |
| SHT45 ×3 | 0x44 | RH/temperature (mux ch 0/1/2) |
| SCD30 | 0x61 | CO₂ / temperature / humidity |
| AS7341 | 0x39 | Spectral light sensor |

---

## Build & Flash

### Prerequisites

```bash
pip install platformio
```

### Compile

```bash
cd firmware
pio run -e esp32dev          # ESP32 DevKit V1
pio run -e esp32s3           # ESP32-S3 variant
```

### Flash

```bash
pio run -e esp32dev --target upload
pio run -e esp32dev --target uploadfs   # Flash LittleFS web UI
```

### OTA Update

Navigate to `http://martha.local/update` and upload the compiled `.bin` from
`.pio/build/esp32dev/firmware.bin`.

---

## Unit Tests (no hardware required)

```bash
cd firmware
pio test -e native -v
```

Tests cover:
- Relay manager boot lock, UVC guard, manual mode
- Humidity loop hysteresis and cooldown
- CO₂ loop hysteresis and minimum run time
- VPD formula accuracy
- Rolling average correctness
- Water level ADC math and thresholds

---

## Initial Configuration

On first boot (or after factory reset), the ESP32 starts a WiFi AP:
- **SSID:** `Martha-Setup`
- **IP:** `192.168.4.1`

Connect and POST to `/api/config` with your WiFi credentials:

```bash
curl -X POST http://192.168.4.1/api/config \
  -H "Content-Type: application/json" \
  -d '{"wifi_ssid": "MyNetwork", "wifi_pass": "MyPassword"}'
```

After reboot it connects to your network and is reachable at `http://martha.local`.

---

## API Reference

| Method | Path | Description |
|--------|------|-------------|
| GET | `/api/status` | Full sensor snapshot + relay states |
| GET | `/api/config` | Current thresholds and schedules |
| POST | `/api/config` | Update config (persists to NVS) |
| POST | `/api/relay/:ch/set` | Manual relay override `{"state": true}` |
| POST | `/api/relay/manual` | Enter/exit manual mode `{"manual": true}` |
| POST | `/api/log-level` | Set log level `{"level": 0-3}` |
| GET | `/update` | ElegantOTA web UI |
| WS | `/ws` | Live sensor push (2s interval) |

Channel names for `:ch`: `Fogger`, `TubFan`, `Exhaust`, `Intake`, `UVC`, `Lights`, `Pump`, `Spare`

---

## Config Reference

| Field | Default | Description |
|-------|---------|-------------|
| `rh_on_pct` | 85.0 | Fogger turns ON below this RH % |
| `rh_hysteresis` | 2.0 | Fogger turns OFF above `rh_on + hysteresis` |
| `co2_on_ppm` | 950 | FAE turns ON above this CO₂ level |
| `co2_off_ppm` | 800 | FAE turns OFF below this CO₂ level |
| `timer.lights_on_minute` | 480 | Lights ON time (minutes from midnight UTC) |
| `timer.lights_off_minute` | 1200 | Lights OFF time |
| `timer.uvc_on_min` | 60 | UVC ON duration (minutes) |
| `timer.uvc_off_min` | 240 | UVC OFF duration (minutes) |
| `water_low_pct` | 20.0 | Pump ON below this water level % |
| `water_high_pct` | 80.0 | Pump OFF above this water level % |
| `rh_aggregation` | 0 | 0=average, 1=min, 2=max of shelf sensors |
| `timezone` | `"UTC0"` | POSIX TZ string |

---

## Adding a New Sensor

1. Create `src/sensors/my_sensor.h/.cpp` following the pattern of `co2_sensor.h`
2. Add a `MyReading` field to `SensorSnapshot` in `sensor_hub.h`
3. Instantiate and call your sensor from `SensorHub::_poll()` in `sensor_hub.cpp`
4. Add the reading to `WsBroadcaster::_buildJson()` and `/api/status` in `api.cpp`
5. Add native tests in `test/native/test_my_sensor.cpp`
6. Add the library to `platformio.ini` lib_deps

---

## Project Structure

```
firmware/
├── include/           config.h (pins, thresholds), hal.h (board variants)
├── src/
│   ├── relay/         RelayManager (safety-guarded 8-channel control)
│   ├── sensors/       SensorHub + individual drivers
│   ├── control/       humidity_loop, co2_loop, timer_scheduler, vpd
│   ├── web/           web_server, api, ws_broadcaster
│   ├── config/        config_store (NVS), defaults
│   └── util/          rolling_average, logger
├── data/              LittleFS web UI (index.html, app.js, style.css)
└── test/native/       Unity unit tests (run on PC)
```
