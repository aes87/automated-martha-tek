#pragma once
#include <cmath>

/**
 * vpd.h — Vapour Pressure Deficit calculation (header-only).
 *
 * VPD (kPa) = SVP × (1 - RH/100)
 *
 * SVP (saturated vapour pressure) is calculated using the Magnus formula:
 *   SVP = 0.6108 × exp(17.27 × T / (T + 237.3))   (kPa, T in °C)
 *
 * Typical fruiting targets:
 *   Gourmet species:  0.4–0.8 kPa
 *   High-RH fruiting: 0.1–0.4 kPa
 */

/**
 * calcSVP(temp_c) — Saturated vapour pressure in kPa using Magnus formula.
 */
inline float calcSVP(float temp_c) {
    return 0.6108f * expf(17.27f * temp_c / (temp_c + 237.3f));
}

/**
 * calcVPD(temp_c, rh_pct) — Vapour Pressure Deficit in kPa.
 *
 * @param temp_c   Dry-bulb air temperature in °C
 * @param rh_pct   Relative humidity in percent (0–100)
 * @return VPD in kPa; returns 0 for invalid inputs
 */
inline float calcVPD(float temp_c, float rh_pct) {
    if (rh_pct < 0.0f || rh_pct > 100.0f) return 0.0f;
    float svp = calcSVP(temp_c);
    return svp * (1.0f - rh_pct / 100.0f);
}
