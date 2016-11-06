#pragma once

#include <limits>
#include <cstdint>

/// Extremely fast non-cryptograpic RNG. Period is 2^128 - 1.
///
/// @see https://github.com/rust-lang-nursery/rand/blob/master/src/lib.rs
class XorShiftRng {
public:
    typedef uint32_t result_type;

    XorShiftRng()
        : _x(0x193a6754)
        , _y(0xa8a7d469)
        , _z(0x97830e05)
        , _w(0x113ba7bb) {
        operator()();
    }

    void seed(uint32_t val) {
        _x = val;
        _y = 0xa8a7d469;
        _z = 0x97830e05;
        _w = 0x113ba7bb;
        operator()();
    }

    // 582.0 non ILP (instruction level parallelism)
    // 663.138 (648) remembering result
    inline uint32_t operator()() {
        // Extremely fast non-cryptograpic RNG. Period is 2^128 - 1.
        // see Marsaglia, George (July 2003). ["Xorshift RNGs"](http://www.jstatsoft.org/v08/i14/paper).
        // Journal of Statistical Software. Vol. 8 (Issue 14).
        const auto result = _result;
        const auto t = _x ^ (_x << 11);
        _x = _y;
        _y = _z;
        _z = _w;
        _result = (_w = _w ^ (_w >> 19) ^ (t ^ (t >> 8)));
        return result;
    }

    /// Random float value between 0 and 1.
    inline float rand01() {
        // 1.0 / (2^32 - 1)
        return operator()() * 2.3283064370807973754314699618685e-10f;
    }

    /// Random float value between min and max.
    inline float operator()(float min_val, float max_val) {
        return (max_val - min_val) * rand01() + min_val;
    }

    // Generates using the given range. This has a modulo bias.
    inline uint32_t operator()(size_t range) {
        return operator()() % static_cast<uint32_t>(range);
    }

    inline static uint32_t max() {
        return static_cast<uint32_t>(-1);
    }

    inline static uint32_t min() {
        return 0;
    }

private:
    uint32_t _x;
    uint32_t _y;
    uint32_t _z;
    uint32_t _w;
    uint32_t _result;
};
