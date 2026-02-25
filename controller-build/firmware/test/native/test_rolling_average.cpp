/**
 * test_rolling_average.cpp — Unit tests for RollingAverage template.
 */

#include <unity.h>
#include "../../src/util/rolling_average.h"

void setUp()    {}
void tearDown() {}

void test_empty_average_returns_zero() {
    RollingAverage<float, 4> avg;
    TEST_ASSERT_EQUAL_FLOAT(0.0f, avg.average());
    TEST_ASSERT_EQUAL(0u, avg.count());
    TEST_ASSERT_FALSE(avg.full());
}

void test_single_sample() {
    RollingAverage<float, 4> avg;
    avg.push(10.0f);
    TEST_ASSERT_EQUAL_FLOAT(10.0f, avg.average());
    TEST_ASSERT_EQUAL(1u, avg.count());
}

void test_multiple_samples_correct_average() {
    RollingAverage<float, 4> avg;
    avg.push(10.0f);
    avg.push(20.0f);
    avg.push(30.0f);
    avg.push(40.0f);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 25.0f, avg.average());
    TEST_ASSERT_TRUE(avg.full());
}

void test_eviction_of_oldest_sample() {
    RollingAverage<float, 3> avg;
    avg.push(10.0f);
    avg.push(20.0f);
    avg.push(30.0f);
    // Now push 40 — evicts 10, window = {20, 30, 40}
    avg.push(40.0f);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 30.0f, avg.average());
    TEST_ASSERT_EQUAL(3u, avg.count());
}

void test_reset_clears_state() {
    RollingAverage<float, 4> avg;
    avg.push(100.0f);
    avg.push(200.0f);
    avg.reset();
    TEST_ASSERT_EQUAL_FLOAT(0.0f, avg.average());
    TEST_ASSERT_EQUAL(0u, avg.count());
    TEST_ASSERT_FALSE(avg.full());
}

void test_integer_type() {
    RollingAverage<int, 4> avg;
    avg.push(2);
    avg.push(4);
    avg.push(6);
    avg.push(8);
    // Integer division: 20/4 = 5
    TEST_ASSERT_EQUAL_INT(5, avg.average());
}

void test_window_size_one() {
    RollingAverage<float, 1> avg;
    avg.push(42.0f);
    TEST_ASSERT_EQUAL_FLOAT(42.0f, avg.average());
    avg.push(99.0f);
    TEST_ASSERT_EQUAL_FLOAT(99.0f, avg.average());
    TEST_ASSERT_EQUAL(1u, avg.count());
}

void test_large_window_partial_fill() {
    RollingAverage<float, 32> avg;
    for (int i = 1; i <= 10; ++i) avg.push(static_cast<float>(i));
    // Average of 1..10 = 5.5
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 5.5f, avg.average());
    TEST_ASSERT_EQUAL(10u, avg.count());
    TEST_ASSERT_FALSE(avg.full());
}

int main(int /*argc*/, char** /*argv*/) {
    UNITY_BEGIN();

    RUN_TEST(test_empty_average_returns_zero);
    RUN_TEST(test_single_sample);
    RUN_TEST(test_multiple_samples_correct_average);
    RUN_TEST(test_eviction_of_oldest_sample);
    RUN_TEST(test_reset_clears_state);
    RUN_TEST(test_integer_type);
    RUN_TEST(test_window_size_one);
    RUN_TEST(test_large_window_partial_fill);

    return UNITY_END();
}
