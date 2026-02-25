#pragma once
/**
 * hal.h — Board Hardware Abstraction Layer
 *
 * Provides alternate pin mappings for different ESP32 board variants.
 * Include this AFTER config.h to override defaults for your hardware revision.
 *
 * Usage:
 *   Build with -DBOARD_S3 to target the ESP32-S3 variant.
 *   Build with -DBOARD_V1 (default) for DevKit V1.
 */

#include "config.h"

#ifdef BOARD_S3
// ── ESP32-S3 DevKitC-1 variant ───────────────────────────────────────────────
// The ESP32-S3 skips GPIO 22-25 entirely (those numbers don't exist in the
// silicon) and GPIO 26-32 are internally wired to the QSPI flash inside the
// WROOM-1 module. All base-config pins that land on those ranges are remapped
// here to valid, exposed GPIO on the 38-pin DevKitC-1 header.
//
// Base config (V1) → S3 corrections:
//   Ch1 Fogger:    16 → 38  (16 was UART2 RX on V1; 38 is clean on S3)
//   Ch2 Tub fan:   17 → 39  (17 was UART2 TX on V1; 39 is clean on S3)
//   Ch5 UVC:       23 → 40  (23 does not exist on ESP32-S3)
//   Ch6 Lights:    25 → 41  (25 does not exist on ESP32-S3)
//   Ch7 Pump:      26 → 42  (26 is internal QSPI flash on ESP32-S3)
//   Ch8 Spare:     27 → 47  (27 is internal QSPI flash on ESP32-S3)
//   I2C SCL:       22 →  9  (22 does not exist on ESP32-S3; 9 is default SCL)
//   ADC water:     34 →  7  (34 is input-only on V1; 7 = ADC1_CH6 on S3)
//
// Unchanged (same GPIO valid on both):
//   Ch3 Exhaust: 18, Ch4 Intake: 19, I2C SDA: 21

#undef  PIN_RELAY_FOGGER
#undef  PIN_RELAY_TUB_FAN
#undef  PIN_RELAY_UVC
#undef  PIN_RELAY_LIGHTS
#undef  PIN_RELAY_PUMP
#undef  PIN_RELAY_SPARE
#undef  PIN_I2C_SCL
#undef  PIN_ADC_WATER_LEVEL

#define PIN_RELAY_FOGGER      38   // Ch1 — clean GPIO; safe boot state
#define PIN_RELAY_TUB_FAN     39   // Ch2 — clean GPIO; safe boot state
#define PIN_RELAY_UVC         40   // Ch5 — boot guard in firmware (10s total)
#define PIN_RELAY_LIGHTS      41   // Ch6 — safe boot state
#define PIN_RELAY_PUMP        42   // Ch7 — safe boot state
#define PIN_RELAY_SPARE       47   // Ch8 — safe boot state
#define PIN_I2C_SCL            9   // Default SCL in ESP32-S3 Arduino framework
#define PIN_ADC_WATER_LEVEL    7   // ADC1_CH6; 1kΩ + Zener protection required

#endif // BOARD_S3

// ── Derived pin table (runtime array for RelayManager) ────────────────────────
// Indexed by RelayChannel enum value (0-7)
#define RELAY_PIN_TABLE  { \
    PIN_RELAY_FOGGER,   \
    PIN_RELAY_TUB_FAN,  \
    PIN_RELAY_EXHAUST,  \
    PIN_RELAY_INTAKE,   \
    PIN_RELAY_UVC,      \
    PIN_RELAY_LIGHTS,   \
    PIN_RELAY_PUMP,     \
    PIN_RELAY_SPARE     \
}

// ── I2C speed ─────────────────────────────────────────────────────────────────
#define I2C_CLOCK_HZ    100000   // 100 kHz standard mode; SCD30 requires ≤100 kHz
