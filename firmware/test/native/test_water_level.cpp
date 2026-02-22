/**
 * test_water_level.cpp — Unit tests for WaterLevel ADC math and hysteresis.
 *
 * Uses WaterLevel::injectVoltage() to bypass the hardware ADC.
 */

#include <unity.h>
#include "../../src/sensors/water_level.h"
#include "../../include/config.h"

// Each test creates a fresh WaterLevel instance
static WaterLevel wl;

void setUp() {
    wl = WaterLevel{};
    wl.begin();
}
void tearDown() {}

// ── ADC voltage-to-level mapping ──────────────────────────────────────────────

void test_min_voltage_gives_zero_percent() {
    for (int i = 0; i < 32; ++i) wl.injectVoltage(ADC_WATER_LEVEL_MIN_MV);
    TEST_ASSERT_FLOAT_WITHIN(1.0f, 0.0f, wl.getLevelPercent());
}

void test_max_voltage_gives_100_percent() {
    for (int i = 0; i < 32; ++i) wl.injectVoltage(ADC_WATER_LEVEL_MAX_MV);
    TEST_ASSERT_FLOAT_WITHIN(1.0f, 100.0f, wl.getLevelPercent());
}

void test_midpoint_voltage_gives_50_percent() {
    uint32_t mid = (ADC_WATER_LEVEL_MIN_MV + ADC_WATER_LEVEL_MAX_MV) / 2;
    for (int i = 0; i < 32; ++i) wl.injectVoltage(mid);
    TEST_ASSERT_FLOAT_WITHIN(2.0f, 50.0f, wl.getLevelPercent());
}

void test_below_min_voltage_clamps_to_zero() {
    for (int i = 0; i < 32; ++i) wl.injectVoltage(0);  // Below calibrated min
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 0.0f, wl.getLevelPercent());
}

void test_above_max_voltage_clamps_to_100() {
    for (int i = 0; i < 32; ++i) wl.injectVoltage(4000);  // Above calibrated max
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 100.0f, wl.getLevelPercent());
}

// ── isValid() requires >= 4 samples ──────────────────────────────────────────

void test_not_valid_until_4_samples() {
    for (int i = 0; i < 3; ++i) {
        wl.injectVoltage(1500);
        TEST_ASSERT_FALSE(wl.isValid());
    }
    wl.injectVoltage(1500);
    TEST_ASSERT_TRUE(wl.isValid());
}

void test_getLevelPercent_returns_negative_when_invalid() {
    wl.injectVoltage(1500);  // Only 1 sample — not valid
    TEST_ASSERT_EQUAL_FLOAT(-1.0f, wl.getLevelPercent());
}

// ── Hysteresis thresholds ─────────────────────────────────────────────────────

void test_below_threshold_at_low_level() {
    // Fill reservoir to ~10% (below WATER_LEVEL_LOW_PCT = 20%)
    uint32_t mv_10pct = ADC_WATER_LEVEL_MIN_MV +
        (ADC_WATER_LEVEL_MAX_MV - ADC_WATER_LEVEL_MIN_MV) / 10;
    for (int i = 0; i < 32; ++i) wl.injectVoltage(mv_10pct);
    TEST_ASSERT_TRUE(wl.isBelowThreshold(WATER_LEVEL_LOW_PCT));
    TEST_ASSERT_FALSE(wl.isAboveThreshold(WATER_LEVEL_HIGH_PCT));
}

void test_above_threshold_at_high_level() {
    // Fill to ~90% (above WATER_LEVEL_HIGH_PCT = 80%)
    uint32_t mv_90pct = ADC_WATER_LEVEL_MIN_MV +
        (uint32_t)((ADC_WATER_LEVEL_MAX_MV - ADC_WATER_LEVEL_MIN_MV) * 0.9f);
    for (int i = 0; i < 32; ++i) wl.injectVoltage(mv_90pct);
    TEST_ASSERT_FALSE(wl.isBelowThreshold(WATER_LEVEL_LOW_PCT));
    TEST_ASSERT_TRUE(wl.isAboveThreshold(WATER_LEVEL_HIGH_PCT));
}

void test_not_valid_returns_false_for_both_thresholds() {
    wl.injectVoltage(0);  // Only 1 sample
    TEST_ASSERT_FALSE(wl.isBelowThreshold(WATER_LEVEL_LOW_PCT));
    TEST_ASSERT_FALSE(wl.isAboveThreshold(WATER_LEVEL_HIGH_PCT));
}

// ── Calibration override ──────────────────────────────────────────────────────

void test_custom_calibration_remaps_range() {
    wl.setCalibration(500, 2500);
    uint32_t mid = (500 + 2500) / 2;  // 1500 mV = 50% in new range
    for (int i = 0; i < 32; ++i) wl.injectVoltage(mid);
    TEST_ASSERT_FLOAT_WITHIN(2.0f, 50.0f, wl.getLevelPercent());
}

int main(int /*argc*/, char** /*argv*/) {
    UNITY_BEGIN();

    RUN_TEST(test_min_voltage_gives_zero_percent);
    RUN_TEST(test_max_voltage_gives_100_percent);
    RUN_TEST(test_midpoint_voltage_gives_50_percent);
    RUN_TEST(test_below_min_voltage_clamps_to_zero);
    RUN_TEST(test_above_max_voltage_clamps_to_100);
    RUN_TEST(test_not_valid_until_4_samples);
    RUN_TEST(test_getLevelPercent_returns_negative_when_invalid);
    RUN_TEST(test_below_threshold_at_low_level);
    RUN_TEST(test_above_threshold_at_high_level);
    RUN_TEST(test_not_valid_returns_false_for_both_thresholds);
    RUN_TEST(test_custom_calibration_remaps_range);

    return UNITY_END();
}
