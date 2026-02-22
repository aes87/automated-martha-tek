#include "timer_scheduler.h"
#include "../util/logger.h"

#ifdef NATIVE_TEST
#include <cstring>
static uint32_t _millis_val = 0;
uint32_t millis() { return _millis_val; }
void set_millis(uint32_t v) { _millis_val = v; }

static int _override_minute = -1;
void set_minute_of_day(int m) { _override_minute = m; }
#else
#include <Arduino.h>
#include <time.h>
#include <esp_sntp.h>
#endif

void TimerScheduler::begin(const char* timezone_str) {
#ifndef NATIVE_TEST
    configTzTime(timezone_str, NTP_SERVER);
    Log.info("timer", "NTP configured tz=%s server=%s", timezone_str, NTP_SERVER);
    // NTP sync happens asynchronously; _ntp_synced set in tick() once time is valid
#else
    (void)timezone_str;
#endif
}

void TimerScheduler::tick(RelayManager& relay, uint32_t now_ms) {
#ifndef NATIVE_TEST
    // Check NTP sync state
    if (!_ntp_synced) {
        struct tm ti;
        if (getLocalTime(&ti, 0)) {  // 0ms timeout — non-blocking
            // Year > 2020 means NTP has synced (not epoch 0)
            if (ti.tm_year > 120) {
                _ntp_synced = true;
                Log.info("timer", "NTP synced");
            }
        }
    }
#endif

    int minute = _minuteOfDay();
    _tickLights(relay, minute);
    _tickUvc(relay, now_ms);
}

void TimerScheduler::_tickLights(RelayManager& relay, int minute_of_day) {
    bool want_on;

    if (minute_of_day < 0) {
        // No time available — default OFF (safe)
        want_on = false;
    } else if (_cfg.lights_on_minute < _cfg.lights_off_minute) {
        // Normal case: on window doesn't cross midnight
        want_on = (minute_of_day >= _cfg.lights_on_minute &&
                   minute_of_day <  _cfg.lights_off_minute);
    } else {
        // Wraps midnight (e.g. 22:00–06:00)
        want_on = (minute_of_day >= _cfg.lights_on_minute ||
                   minute_of_day <  _cfg.lights_off_minute);
    }

    if (want_on != _lights_on) {
        _lights_on = want_on;
        relay.set(RelayChannel::LIGHTS, _lights_on, RelaySource::TIMER);
        Log.info("timer", "Lights %s (minute=%d)", _lights_on ? "ON" : "OFF", minute_of_day);
    }
}

void TimerScheduler::_tickUvc(RelayManager& relay, uint32_t now_ms) {
    uint32_t elapsed = now_ms - _uvc_last_flip_ms;
    uint32_t threshold_ms;

    if (_uvc_on) {
        threshold_ms = static_cast<uint32_t>(_cfg.uvc_on_min) * 60000UL;
        if (elapsed >= threshold_ms) {
            _uvc_on           = false;
            _uvc_last_flip_ms = now_ms;
            relay.set(RelayChannel::UVC, false, RelaySource::TIMER);
            Log.info("timer", "UVC OFF (ran %u min)", _cfg.uvc_on_min);
        }
    } else {
        threshold_ms = static_cast<uint32_t>(_cfg.uvc_off_min) * 60000UL;
        if (elapsed >= threshold_ms) {
            _uvc_on           = true;
            _uvc_last_flip_ms = now_ms;
            relay.set(RelayChannel::UVC, true, RelaySource::TIMER);
            Log.info("timer", "UVC ON (cycle start)");
        }
    }
}

int TimerScheduler::_minuteOfDay() const {
#ifdef NATIVE_TEST
    return _override_minute;
#else
    if (!_ntp_synced) return -1;
    struct tm ti;
    if (!getLocalTime(&ti, 0)) return -1;
    return ti.tm_hour * 60 + ti.tm_min;
#endif
}

void TimerScheduler::setConfig(const TimerConfig& cfg) {
    _cfg = cfg;
    // Reset UVC cycle on config change to avoid unexpected behavior
    _uvc_last_flip_ms = 0;
    Log.info("timer", "Config updated lights=%d-%d uvc=%dmin/%dmin",
             cfg.lights_on_minute, cfg.lights_off_minute,
             cfg.uvc_on_min, cfg.uvc_off_min);
}
