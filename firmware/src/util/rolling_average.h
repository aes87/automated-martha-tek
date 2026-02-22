#pragma once
#include <cstddef>

/**
 * rolling_average.h — Fixed-size rolling (sliding window) average.
 *
 * Header-only template. Works in both native tests and on ESP32.
 *
 * Usage:
 *   RollingAverage<float, 32> adc;
 *   adc.push(3.14f);
 *   float avg = adc.average();
 */

template<typename T, size_t N>
class RollingAverage {
    static_assert(N > 0, "RollingAverage size must be > 0");

public:
    RollingAverage() : _sum(T{}), _count(0), _head(0) {}

    /** Add a new sample. Evicts oldest sample when window is full. */
    void push(T value) {
        if (_count == N) {
            // Evict oldest
            _sum -= _buf[_head];
        } else {
            _count++;
        }
        _buf[_head] = value;
        _sum += value;
        _head = (_head + 1) % N;
    }

    /** Returns the current rolling average. Returns T{} if empty. */
    T average() const {
        if (_count == 0) return T{};
        return _sum / static_cast<T>(_count);
    }

    /** Returns number of samples currently in the window. */
    size_t count() const { return _count; }

    /** Returns true if window is full (N samples). */
    bool full() const { return _count == N; }

    /** Reset — clear all samples. */
    void reset() {
        _sum   = T{};
        _count = 0;
        _head  = 0;
    }

private:
    T      _buf[N] = {};
    T      _sum;
    size_t _count;
    size_t _head;
};
