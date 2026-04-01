/**
 * test_clock.cpp — Single millis()/set_millis() definition for native tests.
 *
 * All source files that need millis() under NATIVE_TEST use
 * `extern uint32_t millis()` and link against this single definition.
 * This avoids ODR violations from multiple millis() stubs.
 */

#ifdef NATIVE_TEST
#include <cstdint>

static uint32_t _millis_val = 0;

uint32_t millis() { return _millis_val; }
void set_millis(uint32_t v) { _millis_val = v; }

#endif
