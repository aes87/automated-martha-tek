#include "relay_manager.h"
#include "relay_channel.h"

#ifdef NATIVE_TEST
// ── Stubs for native test builds ─────────────────────────────────────────────
#include <cstdio>
#include <cstring>

static void pinMode(int, int)    {}
static void digitalWrite(int, int) {}
static void pinMode_input(int)   {}

#define INPUT       0
#define OUTPUT      1
#define HIGH        1
#define LOW         0

static uint32_t _millis_stub = 0;
uint32_t millis() { return _millis_stub; }
void set_millis(uint32_t v) { _millis_stub = v; }

#define portMUX_TYPE       int
#define portMUX_INITIALIZER_UNLOCKED 0
#define taskENTER_CRITICAL(m) (void)(m)
#define taskEXIT_CRITICAL(m)  (void)(m)

#else
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#endif

#include "../../include/config.h"

// ── Static GPIO pin table ────────────────────────────────────────────────────
static const uint8_t RELAY_PINS[RELAY_CHANNEL_COUNT] = RELAY_PIN_TABLE;

// ─────────────────────────────────────────────────────────────────────────────

uint8_t RelayManager::_pinForChannel(RelayChannel ch) {
    return RELAY_PINS[static_cast<uint8_t>(ch)];
}

void RelayManager::begin() {
    _boot_ms = millis();
    _state   = RelayManagerState::BOOT_LOCKED;

#ifndef NATIVE_TEST
    if (!_mutex) {
        _mutex = xSemaphoreCreateMutex();
    }
#endif

    // Drive all relay pins HIGH immediately (active-LOW = OFF).
    // This overrides any pull-up or boot-state glitch.
    for (uint8_t i = 0; i < RELAY_CHANNEL_COUNT; ++i) {
        RelayChannel ch = static_cast<RelayChannel>(i);
        uint8_t pin = _pinForChannel(ch);
        pinMode(pin, OUTPUT);
        digitalWrite(pin, RELAY_ACTIVE_LOW ? HIGH : LOW);
        _relay[i] = false;
        _logChange(ch, false, false, RelaySource::BOOT_INIT, _boot_ms);
    }
}

void RelayManager::tick() {
    uint32_t now = millis();

    if (_state == RelayManagerState::BOOT_LOCKED) {
        if ((now - _boot_ms) >= BOOT_LOCK_MS) {
            _state = RelayManagerState::ARMED;
            // Re-apply all stored relay states (should all be OFF at this point)
            _applyPins();
        }
    }
}

bool RelayManager::set(RelayChannel channel, bool on, RelaySource source) {
    uint32_t now = millis();
    uint8_t  idx = static_cast<uint8_t>(channel);

    if (_state == RelayManagerState::BOOT_LOCKED) {
        // Log the rejection but don't crash
        _logChange(channel, _relay[idx], _relay[idx], RelaySource::BOOT_INIT, now);
        return false;
    }

    if (_state == RelayManagerState::MANUAL_MODE) {
        return false;  // GPIOs released; physical switches have control
    }

    // UVC extra boot guard
    if (channel == RelayChannel::UVC && _isUvcLocked(now)) {
        return false;
    }

    if (_relay[idx] == on) {
        return true;  // No change needed
    }

    bool prev = _relay[idx];
    _relay[idx] = on;
    _logChange(channel, prev, on, source, now);
    _applyPin(channel, on);
    return true;
}

bool RelayManager::get(RelayChannel channel) const {
    return _relay[static_cast<uint8_t>(channel)];
}

void RelayManager::setManualMode(bool enable) {
    if (enable == (_state == RelayManagerState::MANUAL_MODE)) return;

    if (enable) {
        _state = RelayManagerState::MANUAL_MODE;
        _releaseAllPins();
    } else {
        // Return to normal — re-apply stored states
        _state = RelayManagerState::ARMED;
        _applyPins();
    }
}

// ── Private helpers ───────────────────────────────────────────────────────────

void RelayManager::_applyPins() {
    for (uint8_t i = 0; i < RELAY_CHANNEL_COUNT; ++i) {
        _applyPin(static_cast<RelayChannel>(i), _relay[i]);
    }
}

void RelayManager::_applyPin(RelayChannel ch, bool on) {
#ifndef NATIVE_TEST
    uint8_t pin   = _pinForChannel(ch);
    int     level = RELAY_ACTIVE_LOW ? (on ? LOW : HIGH) : (on ? HIGH : LOW);
    pinMode(pin, OUTPUT);
    digitalWrite(pin, level);
#else
    (void)ch; (void)on;
#endif
}

void RelayManager::_releaseAllPins() {
#ifndef NATIVE_TEST
    for (uint8_t i = 0; i < RELAY_CHANNEL_COUNT; ++i) {
        uint8_t pin = _pinForChannel(static_cast<RelayChannel>(i));
        pinMode(pin, INPUT);  // High-Z; pull-ups on relay IN pins hold relays OFF
    }
#endif
}

void RelayManager::_logChange(RelayChannel ch, bool from, bool to,
                               RelaySource src, uint32_t ts) {
    RelayStateEntry& entry = _log[_log_head % LOG_SIZE];
    entry.channel      = ch;
    entry.state        = to;
    entry.source       = src;
    entry.timestamp_ms = ts;

    _log_head++;
    if (_log_count < LOG_SIZE) _log_count++;

#ifdef NATIVE_TEST
    // In native tests, print log entries so test failures are diagnosable
    if (from != to) {
        printf("[RELAY] %s %s -> %s (src=%d t=%u)\n",
               RELAY_CHANNEL_NAMES[static_cast<uint8_t>(ch)],
               from ? "ON" : "OFF",
               to   ? "ON" : "OFF",
               static_cast<int>(src),
               (unsigned)ts);
    }
#endif
    (void)from;  // suppress unused-parameter warning in release builds
}

bool RelayManager::_isUvcLocked(uint32_t now_ms) const {
    uint32_t uvc_unlock_ms = _boot_ms + BOOT_LOCK_MS + UVC_EXTRA_GUARD_MS;
    return (now_ms - _boot_ms) < (BOOT_LOCK_MS + UVC_EXTRA_GUARD_MS);
    (void)uvc_unlock_ms;
}
