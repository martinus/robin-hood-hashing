#pragma once

#include <limits>
#include <cstdint>

/// Extremely fast and good RNG.
/// @see http://www.pcg-random.org/
class Pcg32 {
public:
    typedef uint32_t result_type;

    // random state created at https://www.random.org/cgi-bin/randbyte?nbytes=8&format=h
    Pcg32(uint64_t initialState = UINT64_C(0x853c49e6748fea9b), uint64_t sequenceSelectionConstant = UINT64_C(0xda3e39cb94b95bdb))
        : mState(0)
        , mInc((sequenceSelectionConstant << 1u) | 1u) {

        // run once, see https://github.com/imneme/pcg-c-basic/blob/master/pcg_basic.c#L46
        operator()();
        mState += initialState;
        operator()();
    }

    void seed(uint64_t state) {
        mState = state;
    }

    inline uint32_t operator()() {
        const uint64_t oldstate = mState;

        // Advance internal state
        mState = oldstate * UINT64_C(6364136223846793005) + mInc;

        // Calculate output function (XSH RR), uses old state for max ILP
        const uint32_t xorshifted = static_cast<uint32_t>(((oldstate >> 18u) ^ oldstate) >> 27u);
        const uint32_t rot = static_cast<uint32_t>(oldstate >> 59u);

        return (xorshifted >> rot) | (xorshifted << ((0-rot) & 31));
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
    uint64_t mState;
    const uint64_t mInc;
};
