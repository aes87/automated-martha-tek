#pragma once
/**
 * defaults.h — Compile-time defaults for NVS-stored configuration.
 *
 * These values are written to NVS on first boot (when namespace is empty).
 * All values here must agree with config.h thresholds.
 */

// Humidity control
#define DEFAULT_RH_ON_PCT          85.0f
#define DEFAULT_RH_HYSTERESIS_PCT   2.0f

// CO2 control
#define DEFAULT_CO2_ON_PPM         950.0f
#define DEFAULT_CO2_OFF_PPM        800.0f

// Lights schedule (minutes from midnight UTC)
#define DEFAULT_LIGHTS_ON_MIN      480   // 08:00
#define DEFAULT_LIGHTS_OFF_MIN     1200  // 20:00

// UVC duty cycle (minutes)
#define DEFAULT_UVC_ON_MIN         60
#define DEFAULT_UVC_OFF_MIN        240

// Pump / water level
#define DEFAULT_WATER_LOW_PCT      20.0f
#define DEFAULT_WATER_HIGH_PCT     80.0f

// ADC water level calibration (mV)
#define DEFAULT_ADC_WATER_MIN_MV   200
#define DEFAULT_ADC_WATER_MAX_MV   3100

// RH aggregation (0=AVERAGE, 1=MIN, 2=MAX)
#define DEFAULT_RH_AGGREGATION     0

// NTP timezone (POSIX TZ string)
#define DEFAULT_TIMEZONE           "UTC0"

// Log level (0=ERROR, 1=WARN, 2=INFO, 3=DEBUG)
#define DEFAULT_LOG_LEVEL          2
