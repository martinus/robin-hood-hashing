#pragma once

#include <limits>
#include <cstdint>

/// Extremely fast, simple, and quite good RNG
/// @see https://en.wikipedia.org/wiki/Xorshift#xorshift.2A
class XorShiftStar {
public:
    typedef uint64_t result_type;

    // random state created at https://www.random.org/cgi-bin/randbyte?nbytes=8&format=h
    XorShiftStar(uint64_t initialState = UINT64_C(0x853c49e6748fea9b))
        : mState(initialState) {
    }

    void seed(uint64_t state) {
        mState = state;
    }

    // 563.031 Million OPS for XorShiftStar
    // 548.254 ILP
    inline uint64_t operator()() {
        mState ^= mState >> 12; // a
        mState ^= mState << 25; // b
        mState ^= mState >> 27; // c
        return mState * UINT64_C(2685821657736338717);
    }

    /// Random float value between 0 and 1.
    inline double rand01() {
        // 1.0 / (2^32 - 1)
        return operator()() * 5.4210108624275221703311375920553e-20;
    }

    /// Random float value between min and max.
    inline double operator()(double min_val, double max_val) {
        return (max_val - min_val) * rand01() + min_val;
    }

    // Generates using the given range. This has a modulo bias.
    inline uint64_t operator()(uint64_t range) {
        return operator()() % range;
    }

    inline static uint64_t max() {
        return static_cast<uint64_t>(-1);
    }

    inline static uint64_t min() {
        return 0;
    }

private:
    uint64_t mState;
};
