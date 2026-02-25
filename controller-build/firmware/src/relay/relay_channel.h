#pragma once
#include <cstdint>

/**
 * relay_channel.h — Typed relay channel enum and per-channel metadata.
 *
 * RelayChannel values are indices into the relay pin table (RELAY_PIN_TABLE).
 * Always use the enum — never raw GPIO numbers in control logic.
 */

enum class RelayChannel : uint8_t {
    FOGGER   = 0,  // Ch1 — GPIO 16 (V1) / GPIO 32 (S3)
    TUB_FAN  = 1,  // Ch2 — GPIO 17 (V1) / GPIO 33 (S3)
    EXHAUST  = 2,  // Ch3 — GPIO 18
    INTAKE   = 3,  // Ch4 — GPIO 19
    UVC      = 4,  // Ch5 — GPIO 23; 5s extra boot guard
    LIGHTS   = 5,  // Ch6 — GPIO 25 (V1)
    PUMP     = 6,  // Ch7 — GPIO 26 (V1); optional reservoir top-off
    SPARE    = 7,  // Ch8 — GPIO 27
    COUNT    = 8   // Sentinel — keep last
};

static constexpr uint8_t RELAY_CHANNEL_COUNT = static_cast<uint8_t>(RelayChannel::COUNT);

/** Human-readable channel names (index matches RelayChannel enum value). */
static const char* const RELAY_CHANNEL_NAMES[RELAY_CHANNEL_COUNT] = {
    "Fogger",
    "TubFan",
    "Exhaust",
    "Intake",
    "UVC",
    "Lights",
    "Pump",
    "Spare"
};

/** Source identifier for relay state-change log entries. */
enum class RelaySource : uint8_t {
    BOOT_INIT   = 0,  // Initial state set during setup()
    HUMIDITY    = 1,  // Humidity control loop
    CO2         = 2,  // CO2/FAE control loop
    TIMER       = 3,  // Timer scheduler (UVC, lights)
    PUMP_CTRL   = 4,  // Water level / pump controller
    API         = 5,  // REST API manual override
    MANUAL_MODE = 6,  // Physical DPDT failsafe switch
    WATCHDOG    = 7,  // Watchdog recovery path
};
