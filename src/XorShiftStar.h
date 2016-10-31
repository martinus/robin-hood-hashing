#pragma once

#include <limits>
#include <cstdint>

/// Extremely fast, simple, and quite good RNG
/// @see https://en.wikipedia.org/wiki/Xorshift#xorshift.2A
class XorShiftStar {
public:
    typedef uint32_t result_type;

    // random state created at https://www.random.org/cgi-bin/randbyte?nbytes=8&format=h
    XorShiftStar(uint64_t initialState = 0x853c49e6748fea9bULL)
        : mState(initialState) {
    }

    void seed(uint64_t state) {
        mState = state;
    }

    inline uint32_t operator()() {
        mState ^= mState >> 12; // a
        mState ^= mState << 25; // b
        mState ^= mState >> 27; // c
        return static_cast<uint32_t>(mState * 2685821657736338717ULL);
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
        return -1;
    }

    inline static uint32_t min() {
        return 0;
    }

private:
    uint64_t mState;
};
