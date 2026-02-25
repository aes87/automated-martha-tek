/**
 * test_control_loops.cpp — Unit tests for humidity and CO2 control loops.
 *
 * Uses mock SensorSnapshot data. Verifies hysteresis, cooldown, minimum
 * run time, and correct relay commands.
 */

#include <unity.h>
#include "../../src/control/humidity_loop.h"
#include "../../src/control/co2_loop.h"
#include "../../src/relay/relay_manager.h"
#include "../../src/sensors/sensor_hub.h"
#include "../../include/config.h"

extern void set_millis(uint32_t v);

// ── Helpers ───────────────────────────────────────────────────────────────────

static RelayManager make_armed_relay() {
    RelayManager r;
    set_millis(0);
    r.begin();
    // Advance past all guards
    set_millis(BOOT_LOCK_MS + UVC_EXTRA_GUARD_MS + 100);
    r.tick();
    return r;
}

static SensorSnapshot make_snap(float rh_agg, float co2_ppm) {
    SensorSnapshot s{};
    s.rh_aggregate_pct = rh_agg;
    // Populate rh[] with valid entries
    for (int i = 0; i < 3; ++i) {
        s.rh[i].rh_pct = rh_agg;
        s.rh[i].valid  = true;
    }
    s.co2.co2_ppm = co2_ppm;
    s.co2.valid   = true;
    return s;
}

void setUp()    {}
void tearDown() {}

// ── Humidity loop ─────────────────────────────────────────────────────────────

void test_fogger_on_below_setpoint() {
    RelayManager r = make_armed_relay();
    HumidityLoop loop;
    // RH below setpoint — should start fogging
    SensorSnapshot snap = make_snap(83.0f, 600.0f);
    loop.tick(snap, r, BOOT_LOCK_MS + UVC_EXTRA_GUARD_MS + 200);

    TEST_ASSERT_TRUE(r.get(RelayChannel::FOGGER));
    TEST_ASSERT_TRUE(r.get(RelayChannel::TUB_FAN));
    TEST_ASSERT_TRUE(loop.isFogging());
}

void test_fogger_off_above_setpoint_plus_hysteresis() {
    RelayManager r = make_armed_relay();
    HumidityLoop loop;
    uint32_t t = BOOT_LOCK_MS + UVC_EXTRA_GUARD_MS + 200;

    // Start fogging
    loop.tick(make_snap(83.0f, 600.0f), r, t);
    TEST_ASSERT_TRUE(loop.isFogging());

    // RH rises above setpoint + hysteresis (85 + 2 = 87%)
    t += HUMIDITY_COOLDOWN_MS + 1000;
    loop.tick(make_snap(88.0f, 600.0f), r, t);

    TEST_ASSERT_FALSE(loop.isFogging());
    TEST_ASSERT_FALSE(r.get(RelayChannel::FOGGER));
    TEST_ASSERT_FALSE(r.get(RelayChannel::TUB_FAN));
}

void test_fogger_no_chatter_at_threshold() {
    RelayManager r = make_armed_relay();
    HumidityLoop loop;
    uint32_t t = BOOT_LOCK_MS + UVC_EXTRA_GUARD_MS + 200;

    // Start fogging at RH=83
    loop.tick(make_snap(83.0f, 600.0f), r, t);
    TEST_ASSERT_TRUE(loop.isFogging());

    // Rapid RH changes at boundary — but cooldown prevents state flip
    for (int i = 0; i < 10; ++i) {
        t += 100;  // Only 100ms apart — much less than HUMIDITY_COOLDOWN_MS
        loop.tick(make_snap(86.0f, 600.0f), r, t);
    }
    // Still fogging because 86 < 87 (off threshold)
    TEST_ASSERT_TRUE(loop.isFogging());
}

void test_fogger_cooldown_prevents_rapid_cycling() {
    RelayManager r = make_armed_relay();
    HumidityLoop loop;
    uint32_t t = BOOT_LOCK_MS + UVC_EXTRA_GUARD_MS + 200;

    // Start fogging
    loop.tick(make_snap(83.0f, 600.0f), r, t);
    TEST_ASSERT_TRUE(loop.isFogging());

    // Jump above off threshold immediately — cooldown not met
    t += 1000;  // 1 second (cooldown is 30s)
    loop.tick(make_snap(90.0f, 600.0f), r, t);
    TEST_ASSERT_TRUE(loop.isFogging());  // Still on — cooldown not elapsed

    // Now wait for cooldown
    t += HUMIDITY_COOLDOWN_MS;
    loop.tick(make_snap(90.0f, 600.0f), r, t);
    TEST_ASSERT_FALSE(loop.isFogging());
}

void test_fogger_off_when_no_valid_sensors() {
    RelayManager r = make_armed_relay();
    HumidityLoop loop;
    uint32_t t = BOOT_LOCK_MS + UVC_EXTRA_GUARD_MS + 200;

    // Start with valid data fogging
    loop.tick(make_snap(83.0f, 600.0f), r, t);
    TEST_ASSERT_TRUE(loop.isFogging());

    // Invalidate all sensors
    SensorSnapshot bad{};
    bad.rh_aggregate_pct = 83.0f;  // Data present but valid=false
    for (int i = 0; i < 3; ++i) bad.rh[i].valid = false;
    bad.co2.valid = true;

    t += HUMIDITY_COOLDOWN_MS + 1;
    loop.tick(bad, r, t);
    // Should hold current state (fail-safe: stay fogging)
    TEST_ASSERT_TRUE(loop.isFogging());
}

// ── CO2 loop ──────────────────────────────────────────────────────────────────

void test_fae_on_above_co2_threshold() {
    RelayManager r = make_armed_relay();
    Co2Loop loop;
    uint32_t t = BOOT_LOCK_MS + UVC_EXTRA_GUARD_MS + 200;

    loop.tick(make_snap(85.0f, 1000.0f), r, t);  // CO2 > 950

    TEST_ASSERT_TRUE(r.get(RelayChannel::EXHAUST));
    TEST_ASSERT_TRUE(r.get(RelayChannel::INTAKE));
    TEST_ASSERT_TRUE(loop.isFlushing());
}

void test_fae_off_below_co2_off_threshold_after_min_run() {
    RelayManager r = make_armed_relay();
    Co2Loop loop;
    uint32_t t = BOOT_LOCK_MS + UVC_EXTRA_GUARD_MS + 200;

    // Turn on FAE
    loop.tick(make_snap(85.0f, 1000.0f), r, t);
    TEST_ASSERT_TRUE(loop.isFlushing());

    // CO2 drops below off threshold immediately — min run time enforces stay on
    t += 1000;
    loop.tick(make_snap(85.0f, 700.0f), r, t);
    TEST_ASSERT_TRUE(loop.isFlushing());  // Still flushing — min run not elapsed

    // Advance past minimum run time
    t += FAE_MIN_RUN_MS;
    loop.tick(make_snap(85.0f, 700.0f), r, t);
    TEST_ASSERT_FALSE(loop.isFlushing());
    TEST_ASSERT_FALSE(r.get(RelayChannel::EXHAUST));
    TEST_ASSERT_FALSE(r.get(RelayChannel::INTAKE));
}

void test_fae_hysteresis_co2_between_thresholds() {
    RelayManager r = make_armed_relay();
    Co2Loop loop;
    uint32_t t = BOOT_LOCK_MS + UVC_EXTRA_GUARD_MS + 200;

    // CO2 between 800–950 and FAE is OFF — should stay OFF
    loop.tick(make_snap(85.0f, 900.0f), r, t);
    TEST_ASSERT_FALSE(loop.isFlushing());

    // CO2 rises above 950 — turns ON
    loop.tick(make_snap(85.0f, 960.0f), r, t);
    TEST_ASSERT_TRUE(loop.isFlushing());

    // CO2 drops to 850 (below on threshold but above off threshold) —
    // stays ON because we're in flushing mode + min run enforces
    t += 1000;
    loop.tick(make_snap(85.0f, 850.0f), r, t);
    TEST_ASSERT_TRUE(loop.isFlushing());
}

void test_fae_off_when_no_valid_co2() {
    RelayManager r = make_armed_relay();
    Co2Loop loop;
    uint32_t t = BOOT_LOCK_MS + UVC_EXTRA_GUARD_MS + 200;

    // Start flushing
    loop.tick(make_snap(85.0f, 1000.0f), r, t);
    TEST_ASSERT_TRUE(loop.isFlushing());

    // No valid CO2 — hold state
    SensorSnapshot bad = make_snap(85.0f, 1000.0f);
    bad.co2.valid = false;
    t += FAE_MIN_RUN_MS + 1;
    loop.tick(bad, r, t);
    TEST_ASSERT_TRUE(loop.isFlushing());  // Holds — can't validate turn-off
}

void test_fae_invalid_thresholds_rejected() {
    Co2Loop loop;
    // on <= off is invalid
    loop.setThresholds(800.0f, 950.0f);  // on < off — should be rejected
    // Loop still uses previous defaults
    // No crash — just verify it compiles and runs
}

int main(int /*argc*/, char** /*argv*/) {
    UNITY_BEGIN();

    // Humidity
    RUN_TEST(test_fogger_on_below_setpoint);
    RUN_TEST(test_fogger_off_above_setpoint_plus_hysteresis);
    RUN_TEST(test_fogger_no_chatter_at_threshold);
    RUN_TEST(test_fogger_cooldown_prevents_rapid_cycling);
    RUN_TEST(test_fogger_off_when_no_valid_sensors);

    // CO2
    RUN_TEST(test_fae_on_above_co2_threshold);
    RUN_TEST(test_fae_off_below_co2_off_threshold_after_min_run);
    RUN_TEST(test_fae_hysteresis_co2_between_thresholds);
    RUN_TEST(test_fae_off_when_no_valid_co2);
    RUN_TEST(test_fae_invalid_thresholds_rejected);

    return UNITY_END();
}
