#pragma once
#include "../relay/relay_manager.h"
#include <cstdint>

/**
 * timer_scheduler.h — NTP-synced daily on/off timer for UVC and Lights.
 *
 * Uses wall clock time when NTP is available; falls back to elapsed uptime
 * when NTP has not synced (provides at-least-once guarantees on schedule).
 *
 * UVC schedule: configurable duty cycle (default 1h on / 4h off).
 *   UVC on/off respects RelayManager's boot lock (no special logic needed here).
 *
 * Lights schedule: configurable daily on/off window (default 08:00–20:00 UTC).
 */

struct TimerConfig {
    // Lights: minutes from midnight for on/off (UTC)
    uint16_t lights_on_minute  = LIGHTS_ON_MINUTE;
    uint16_t lights_off_minute = LIGHTS_OFF_MINUTE;

    // UVC: duty cycle durations (minutes)
    uint16_t uvc_on_min  = UVC_ON_DURATION_MIN;
    uint16_t uvc_off_min = UVC_OFF_DURATION_MIN;
};

class TimerScheduler {
public:
    TimerScheduler() = default;

    /**
     * begin() — Initialise NTP sync. Call after WiFi connects.
     * timezone_str: POSIX TZ string e.g. "UTC0" or "EST5EDT,M3.2.0,M11.1.0"
     */
    void begin(const char* timezone_str = NTP_TIMEZONE);

    /**
     * tick(relay, now_ms) — Evaluate schedules and fire relay commands.
     * Call every CONTROL_TASK_PERIOD_MS.
     */
    void tick(RelayManager& relay, uint32_t now_ms);

    /** setConfig() — Update schedule at runtime (from /api/config). */
    void setConfig(const TimerConfig& cfg);

    const TimerConfig& getConfig() const { return _cfg; }

    /** isNtpSynced() — True once NTP has provided wall-clock time. */
    bool isNtpSynced() const { return _ntp_synced; }

private:
    TimerConfig _cfg;
    bool        _ntp_synced       = false;
    bool        _lights_on        = false;
    bool        _uvc_on           = false;
    uint32_t    _uvc_last_flip_ms = 0;

    // Returns current time as minutes since midnight (UTC), or -1 if unavailable
    int _minuteOfDay() const;

    void _tickLights(RelayManager& relay, int minute_of_day);
    void _tickUvc(RelayManager& relay, uint32_t now_ms);
};
