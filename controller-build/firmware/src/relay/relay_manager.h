#pragma once
#include "relay_channel.h"
#include <cstdint>
#include <cstddef>

/**
 * relay_manager.h — Safety-guarded relay controller.
 *
 * Safety contract:
 *  1. All relays are held OFF (HIGH with active-LOW logic) during the BOOT_LOCKED
 *     window to prevent GPIO boot-state glitches firing loads.
 *  2. UVC relay has an additional UVC_EXTRA_GUARD_MS lock window for UV safety.
 *  3. In MANUAL_MODE the manager releases GPIOs to INPUT (high-Z) so the physical
 *     DPDT panel switches take full control.
 *  4. Every state change is logged with timestamp, channel, old/new state, source.
 */

// In native test builds we stub Arduino/FreeRTOS types
#ifdef NATIVE_TEST
#include <cstdio>
using uint32_t = unsigned int;
// Minimal millis() stub resolved at link time in test harness
extern uint32_t millis();
#else
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#endif

enum class RelayManagerState : uint8_t {
    BOOT_LOCKED = 0,  // Within boot lock window; all commands rejected
    ARMED       = 1,  // Normal operation
    MANUAL_MODE = 2,  // Physical panel override active; GPIOs released
};

struct RelayStateEntry {
    RelayChannel channel;
    bool         state;       // true = ON, false = OFF
    RelaySource  source;
    uint32_t     timestamp_ms;
};

class RelayManager {
public:
    RelayManager() = default;

    /**
     * begin() — Configure all relay GPIOs as OUTPUT, drive all relays OFF,
     * and record the boot timestamp. Must be called once in setup().
     */
    void begin();

    /**
     * tick() — Must be called regularly (e.g. every 100ms in main loop or a
     * FreeRTOS task). Transitions state machine from BOOT_LOCKED → ARMED when
     * the guard window has elapsed.
     */
    void tick();

    /**
     * set(channel, on, source) — Request relay state change.
     * Returns false and logs a warning if called during BOOT_LOCKED window
     * (or UVC during its extended lock). Returns true if state was applied.
     */
    bool set(RelayChannel channel, bool on, RelaySource source = RelaySource::API);

    /** get(channel) — Returns the current commanded state of a relay. */
    bool get(RelayChannel channel) const;

    /**
     * setManualMode(enable) — When true, all GPIOs are set to INPUT (high-Z)
     * releasing control to the physical DPDT panel switches.
     */
    void setManualMode(bool enable);

    /** isArmed() — True once boot lock window has elapsed. */
    bool isArmed() const { return _state == RelayManagerState::ARMED; }

    /** isManualMode() — True when hardware DPDT override is active. */
    bool isManualMode() const { return _state == RelayManagerState::MANUAL_MODE; }

    /** getState() — Current state machine state. */
    RelayManagerState getState() const { return _state; }

    /** getBootTimestamp() — millis() value recorded in begin(). */
    uint32_t getBootTimestamp() const { return _boot_ms; }

    /** getLog() — Pointer to the recent state-change ring buffer. */
    const RelayStateEntry* getLog(size_t& out_count) const {
        out_count = _log_count;
        return _log;
    }

private:
    static constexpr size_t LOG_SIZE = 32;

    RelayManagerState _state      = RelayManagerState::BOOT_LOCKED;
    bool              _relay[RELAY_CHANNEL_COUNT] = {};  // All OFF
    uint32_t          _boot_ms    = 0;
    bool              _prev_manual = false;

    RelayStateEntry   _log[LOG_SIZE] = {};
    size_t            _log_head  = 0;
    size_t            _log_count = 0;

#ifndef NATIVE_TEST
    SemaphoreHandle_t _mutex = nullptr;
#endif

    void _applyPins();
    void _applyPin(RelayChannel ch, bool on);
    void _releaseAllPins();
    void _logChange(RelayChannel ch, bool from, bool to, RelaySource src, uint32_t ts);
    bool _isUvcLocked(uint32_t now_ms) const;

    /** Returns the GPIO number for a channel using RELAY_PIN_TABLE. */
    static uint8_t _pinForChannel(RelayChannel ch);
};
