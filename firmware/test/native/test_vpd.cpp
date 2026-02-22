/**
 * test_vpd.cpp — Unit tests for VPD formula accuracy.
 */

#include <unity.h>
#include "../../src/control/vpd.h"

void setUp()    {}
void tearDown() {}

// Reference values computed independently using the Magnus formula
// SVP = 0.6108 * exp(17.27 * T / (T + 237.3))  [kPa]
// VPD = SVP * (1 - RH/100)

void test_vpd_at_25c_85rh() {
    // SVP(25°C) ≈ 3.168 kPa; VPD = 3.168 * 0.15 ≈ 0.475 kPa
    float vpd = calcVPD(25.0f, 85.0f);
    TEST_ASSERT_FLOAT_WITHIN(0.05f, 0.475f, vpd);
}

void test_vpd_at_22c_90rh() {
    // SVP(22°C) ≈ 2.645 kPa; VPD = 2.645 * 0.10 ≈ 0.265 kPa
    float vpd = calcVPD(22.0f, 90.0f);
    TEST_ASSERT_FLOAT_WITHIN(0.05f, 0.265f, vpd);
}

void test_vpd_at_100rh_is_zero() {
    float vpd = calcVPD(25.0f, 100.0f);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, vpd);
}

void test_vpd_invalid_rh_above_100() {
    float vpd = calcVPD(25.0f, 101.0f);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, vpd);
}

void test_vpd_invalid_rh_below_0() {
    float vpd = calcVPD(25.0f, -1.0f);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, vpd);
}

void test_svp_increases_with_temperature() {
    TEST_ASSERT_GREATER_THAN(calcSVP(20.0f), calcSVP(25.0f));
    TEST_ASSERT_GREATER_THAN(calcSVP(25.0f), calcSVP(30.0f));
}

void test_vpd_at_fruiting_target() {
    // Typical fruiting: 22°C, 85% RH → VPD should be in 0.3–0.5 kPa range
    float vpd = calcVPD(22.0f, 85.0f);
    TEST_ASSERT_GREATER_THAN(0.3f, vpd);
    TEST_ASSERT_LESS_THAN(0.5f, vpd);
}

int main(int /*argc*/, char** /*argv*/) {
    UNITY_BEGIN();

    RUN_TEST(test_vpd_at_25c_85rh);
    RUN_TEST(test_vpd_at_22c_90rh);
    RUN_TEST(test_vpd_at_100rh_is_zero);
    RUN_TEST(test_vpd_invalid_rh_above_100);
    RUN_TEST(test_vpd_invalid_rh_below_0);
    RUN_TEST(test_svp_increases_with_temperature);
    RUN_TEST(test_vpd_at_fruiting_target);

    return UNITY_END();
}
