#pragma once
/**
 * config.h — Martha Tent Controller: pin definitions, I2C addresses,
 *             compile-time thresholds, and version info.
 *
 * Base values target the classic ESP32 DevKit V1 (legacy / fallback).
 * Building for ESP32-S3 DevKitC-1 (recommended)? Define BOARD_S3 — see
 * include/hal.h, which overrides all pins that are invalid on the S3.
 */

// ── Firmware version ──────────────────────────────────────────────────────────
#ifndef MARTHA_FW_VERSION
#define MARTHA_FW_VERSION "0.1.0"
#endif

// ── GPIO — Relay outputs (active LOW on standard PC817 relay modules) ─────────
// NOTE: These are DevKit V1 base values. BOARD_S3 overrides all of these in
// hal.h — do not use these numbers directly when targeting the ESP32-S3.
#define PIN_RELAY_FOGGER      16   // Ch1 — V1: UART2 RX; 10kΩ pull-up to VCC required
#define PIN_RELAY_TUB_FAN     17   // Ch2 — V1: UART2 TX; glitches LOW at boot on V1
#define PIN_RELAY_EXHAUST     18   // Ch3 — safe boot state HIGH (same on S3)
#define PIN_RELAY_INTAKE      19   // Ch4 — safe boot state HIGH (same on S3)
#define PIN_RELAY_UVC         23   // Ch5 — V1 only: UV safety boot guard in firmware
#define PIN_RELAY_LIGHTS      25   // Ch6 — V1: DAC1; non-deterministic until init
#define PIN_RELAY_PUMP        26   // Ch7 — V1: DAC2; non-deterministic until init
#define PIN_RELAY_SPARE       27   // Ch8 — general purpose

// ── GPIO — Sensors ────────────────────────────────────────────────────────────
// NOTE: PIN_I2C_SCL and PIN_ADC_WATER_LEVEL are overridden for BOARD_S3 in hal.h
#define PIN_ONE_WIRE          4    // DS18B20 ×5; 2.2kΩ pull-up + 100nF at junction (same on S3)
#define PIN_I2C_SDA           21   // I2C data — same on both V1 and S3
#define PIN_I2C_SCL           22   // I2C clock — V1 only; S3 uses GPIO 9 (see hal.h)
#define PIN_ADC_WATER_LEVEL   34   // V1: input-only GPIO; S3 uses GPIO 7 (see hal.h)

// ── I2C addresses ─────────────────────────────────────────────────────────────
#define I2C_ADDR_TCA9548A     0x70  // I2C mux (fixed address)
#define I2C_ADDR_SHT45        0x44  // All three SHT45s share this address; mux selects
#define I2C_ADDR_SCD30        0x61  // CO2/temp/RH sensor
#define I2C_ADDR_AS7341       0x39  // Spectral light sensor

// ── TCA9548A mux channel assignments ─────────────────────────────────────────
#define MUX_CH_SHT45_SHELF1   0    // SHT45 on shelf 1
#define MUX_CH_SHT45_SHELF2   1    // SHT45 on shelf 2
#define MUX_CH_SHT45_SHELF3   2    // SHT45 on shelf 3

// ── DS18B20 probe count ───────────────────────────────────────────────────────
#define DS18B20_PROBE_COUNT   5    // One per shelf + spare

// ── Relay polarity ────────────────────────────────────────────────────────────
// Set to 1 for active-LOW relay modules (standard PC817 boards).
// Set to 0 for active-HIGH modules.
#define RELAY_ACTIVE_LOW      1

// ── Boot safety timing (ms) ───────────────────────────────────────────────────
#define BOOT_LOCK_MS          5000   // All relays locked for 5s after boot
#define UVC_EXTRA_GUARD_MS    5000   // UVC locked for additional 5s (10s total)

// ── Humidity control thresholds ───────────────────────────────────────────────
#define RH_SETPOINT_PCT       85.0f  // Fogger turns ON below this
#define RH_HYSTERESIS_PCT     2.0f   // Fogger turns OFF above (setpoint + hysteresis)
#define HUMIDITY_COOLDOWN_MS  30000  // Minimum ms between fogger state changes

// ── CO2 / FAE control thresholds ─────────────────────────────────────────────
#define CO2_ON_PPM            950    // FAE fans turn ON above this
#define CO2_OFF_PPM           800    // FAE fans turn OFF below this
#define FAE_MIN_RUN_MS        60000  // FAE must run at least 60s per cycle

// ── Pump / water level thresholds ────────────────────────────────────────────
#define WATER_LEVEL_LOW_PCT   20.0f  // Pump turns ON below this
#define WATER_LEVEL_HIGH_PCT  80.0f  // Pump turns OFF above this
#define WATER_LEVEL_SAMPLES   32     // Rolling average sample count

// ── ADC calibration ───────────────────────────────────────────────────────────
#define ADC_ATTEN             ADC_ATTEN_DB_11   // 0–3.6V range
#define ADC_WIDTH             ADC_WIDTH_BIT_12  // 12-bit resolution
#define ADC_WATER_LEVEL_MIN_MV  200    // Voltage at 0% water level (calibrate in-situ)
#define ADC_WATER_LEVEL_MAX_MV  3100   // Voltage at 100% water level

// ── SHT45 heater schedule ─────────────────────────────────────────────────────
#define SHT45_HEATER_INTERVAL_MS  3600000  // Run on-chip heater every 60 min

// ── Sensor staleness timeout ──────────────────────────────────────────────────
#define SENSOR_STALE_MS       30000   // Mark reading stale if not updated in 30s
#define SCD30_STALE_ALERT_MS  300000  // Alert if SCD30 missing for 5 min

// ── FreeRTOS task configuration ───────────────────────────────────────────────
#define SENSOR_TASK_STACK     4096
#define SENSOR_TASK_PRIORITY  2
#define SENSOR_TASK_PERIOD_MS 2000

#define CONTROL_TASK_STACK    4096
#define CONTROL_TASK_PRIORITY 3
#define CONTROL_TASK_PERIOD_MS 1000

#define WS_BROADCAST_PERIOD_MS 2000

// ── Hardware watchdog timeout (seconds) ───────────────────────────────────────
#define WDT_TIMEOUT_S         30

// ── Light schedule defaults ───────────────────────────────────────────────────
// Times in minutes from midnight (UTC, adjusted by NTP)
#define LIGHTS_ON_MINUTE      (8 * 60)   // 08:00
#define LIGHTS_OFF_MINUTE     (20 * 60)  // 20:00

// ── UVC schedule defaults ─────────────────────────────────────────────────────
#define UVC_ON_DURATION_MIN   60    // 1 hour on
#define UVC_OFF_DURATION_MIN  240   // 4 hours off

// ── NTP / WiFi ────────────────────────────────────────────────────────────────
#define NTP_SERVER            "pool.ntp.org"
#define NTP_TIMEZONE          "UTC0"
#define WIFI_CONNECT_TIMEOUT_MS 30000
#define WIFI_AP_SSID          "Martha-Setup"
#define WIFI_AP_PASSWORD      ""    // Open AP for initial config

// ── mDNS hostname ─────────────────────────────────────────────────────────────
#define MDNS_HOSTNAME         "martha"

// ── WebSocket ─────────────────────────────────────────────────────────────────
#define WS_PATH               "/ws"

// ── NVS namespace ─────────────────────────────────────────────────────────────
#define NVS_NAMESPACE         "martha"

// ── Boot history ─────────────────────────────────────────────────────────────
#define BOOT_LOG_SIZE         5    // Store last N reset reasons in NVS
