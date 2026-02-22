#include "logger.h"
#include <cstdio>
#include <cstdarg>
#include <cstdint>

#ifdef NATIVE_TEST
static uint32_t millis() {
    return 0;  // In native tests millis() may be overridden by test harness
}
#else
#include <Arduino.h>
#endif

Logger Log;

static const char* _levelStr(LogLevel l) {
    switch (l) {
        case LogLevel::ERROR: return "ERROR";
        case LogLevel::WARN:  return "WARN ";
        case LogLevel::INFO:  return "INFO ";
        case LogLevel::DEBUG: return "DEBUG";
        default:              return "?    ";
    }
}

void Logger::_log(LogLevel level, const char* module, const char* fmt, va_list args) {
    if (static_cast<uint8_t>(level) > static_cast<uint8_t>(_level)) return;

    uint32_t t = millis();

    char buf[256];
    vsnprintf(buf, sizeof(buf), fmt, args);

#ifdef NATIVE_TEST
    printf("[%s][%-8s][%8u] %s\n", _levelStr(level), module, (unsigned)t, buf);
#else
    Serial.printf("[%s][%-8s][%8u] %s\n", _levelStr(level), module, t, buf);
#endif
}

void Logger::error(const char* module, const char* fmt, ...) {
    va_list a; va_start(a, fmt); _log(LogLevel::ERROR, module, fmt, a); va_end(a);
}
void Logger::warn(const char* module, const char* fmt, ...) {
    va_list a; va_start(a, fmt); _log(LogLevel::WARN, module, fmt, a); va_end(a);
}
void Logger::info(const char* module, const char* fmt, ...) {
    va_list a; va_start(a, fmt); _log(LogLevel::INFO, module, fmt, a); va_end(a);
}
void Logger::debug(const char* module, const char* fmt, ...) {
    va_list a; va_start(a, fmt); _log(LogLevel::DEBUG, module, fmt, a); va_end(a);
}
