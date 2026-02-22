# mushroom-firmware

ESP32 PlatformIO firmware for the Martha Tent Controller.
Lives at `/workspace/projects/mushroom-firmware/` in the workspace,
synced to `aes87/automated-martha-tek/firmware/` on GitHub.

## Build Commands

```bash
# Compile for ESP32 hardware
pio run -e esp32dev

# Compile for ESP32-S3 (BOARD_S3 remaps all invalid/reserved pins — see hal.h)
pio run -e esp32s3

# Run unit tests on PC (no ESP32 needed)
pio test -e native -v

# Flash firmware
pio run -e esp32dev --target upload

# Flash LittleFS web UI
pio run -e esp32dev --target uploadfs

# Serial monitor
pio device monitor --baud 115200
```

## Test Commands

```bash
# Run all native tests
pio test -e native

# Run a single test file
pio test -e native --filter test_relay_manager
pio test -e native --filter test_vpd
pio test -e native --filter test_rolling_average
pio test -e native --filter test_water_level
pio test -e native --filter test_control_loops
```

## OTA Update

```bash
# Via curl (replace IP with ESP32 address or use martha.local)
curl -F "firmware=@.pio/build/esp32dev/firmware.bin" http://martha.local/update
```

## Project Structure

```
include/         config.h (pins, thresholds, timing), hal.h (board variants)
src/
  relay/         RelayManager — boot-safe relay control
  sensors/       SensorHub + CO2, RH, light, temp, water drivers
  control/       humidity_loop, co2_loop, timer_scheduler, vpd.h
  web/           web_server, api, ws_broadcaster
  config/        config_store (NVS), defaults.h
  util/          rolling_average.h (template), logger
data/            LittleFS web UI (index.html, app.js, style.css)
test/native/     Unity unit tests — all run on PC
test/embedded/   Hardware bring-up checklist
```

## Key Safety Rules (DO NOT CHANGE without understanding)

1. **Boot lock** — all relays held OFF for 5s (`BOOT_LOCK_MS`) after `begin()`
2. **UVC extra guard** — UVC relay locked for additional 5s beyond boot lock (`UVC_EXTRA_GUARD_MS`)
3. **GPIO 16/17** — UART2 on DevKit V1; need 10 kΩ pull-ups on relay IN pins to relay VCC
4. **GPIO 34** — input-only ADC; needs 1 kΩ + 3.3V Zener before connecting any sensor
5. **Active-LOW relays** — `RELAY_ACTIVE_LOW 1`; HIGH = relay OFF, LOW = relay ON
6. **Manual mode** — `setManualMode(true)` sets all GPIO to INPUT (high-Z); pull-ups hold relays OFF

## Adding a New Sensor

1. Create `src/sensors/my_sensor.h/.cpp` following `co2_sensor.h` pattern
2. Add a `MyReading` field to `SensorSnapshot` in `sensor_hub.h`
3. Call your sensor from `SensorHub::_poll()` in `sensor_hub.cpp`
4. Add reading to `WsBroadcaster::_buildJson()` and `/api/status` endpoint
5. Write native unit tests in `test/native/test_my_sensor.cpp`
6. Add library to `platformio.ini` lib_deps

## Syncing to GitHub

```bash
# Sync workspace project to automated-martha-tek repo
rsync -av --delete /workspace/projects/mushroom-firmware/ \
    /tmp/automated-martha-tek/firmware/

# Push to GitHub
cd /tmp/automated-martha-tek
git add firmware/
git commit -m "Update firmware"
git push
```

## Environment Notes

- Build flags include `-Wall -Wextra -Werror` — zero warnings policy
- Native tests use `NATIVE_TEST` macro to exclude hardware-specific code
- `set_millis(uint32_t)` in relay_manager.cpp and timer_scheduler.cpp controls virtual time in tests
- Logger outputs to `Serial.printf()` on ESP32, `printf()` in native builds
