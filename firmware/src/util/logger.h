#pragma once
#include <cstdint>

/**
 * logger.h — Structured serial logger.
 *
 * All output is prefixed: [LEVEL][module][uptime_ms]
 * Log level can be changed at runtime via POST /api/log-level.
 */

enum class LogLevel : uint8_t {
    ERROR = 0,
    WARN  = 1,
    INFO  = 2,
    DEBUG = 3,
};

class Logger {
public:
    Logger() = default;

    void setLevel(LogLevel level) { _level = level; }
    LogLevel getLevel() const { return _level; }

    void error(const char* module, const char* fmt, ...);
    void warn (const char* module, const char* fmt, ...);
    void info (const char* module, const char* fmt, ...);
    void debug(const char* module, const char* fmt, ...);

private:
    LogLevel _level = LogLevel::INFO;
    void _log(LogLevel level, const char* module, const char* fmt, va_list args);
    static const char* _levelStr(LogLevel l);
};

extern Logger Log;
