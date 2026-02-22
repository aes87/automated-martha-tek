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
// Moves Ch1/Ch2 to GPIO 32/33 — avoids UART0 boot-state glitch
// S3 also has native USB, eliminating CH340 moisture-risk issue

#undef  PIN_RELAY_FOGGER
#undef  PIN_RELAY_TUB_FAN

#define PIN_RELAY_FOGGER      32   // Ch1 — no UART attachment, safe boot state
#define PIN_RELAY_TUB_FAN     33   // Ch2 — no UART attachment, safe boot state

// S3 I2C pins remain the same (GPIO 21/22 available on S3)
// S3 ADC: GPIO 34 is not available on S3; remap to GPIO 7 (ADC1_CH6)
#undef  PIN_ADC_WATER_LEVEL
#define PIN_ADC_WATER_LEVEL   7

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
