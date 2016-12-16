#pragma once

#include <cstdint>
#include <utility>

/// Extremely fast RNG that passes the BigCrush test.
/// It has a period of 2^128 - 1, and is even faster than MarsagliaMWC99.
/// 
/// @see http://xoroshiro.di.unimi.it/xoroshiro128plus.c
class XoRoShiRo128Plus {
public:
    typedef uint64_t result_type;
    typedef std::pair<uint64_t, uint64_t> state_type;

    // random state created at https://www.random.org/cgi-bin/randbyte?nbytes=8&format=h
    XoRoShiRo128Plus(uint64_t initialState = UINT64_C(0x853c49e6748fea9b)) {
        seed(initialState);
    }

    XoRoShiRo128Plus(const state_type& state)
        : mState(state) {
    }

    // Seeds the random number generator with a 64 bit value.
    // This uses splitmix64 RNG to provide two 64bit state seeds.
    inline void seed(uint64_t seed) {
        mState.first = splitmix64(seed);
        mState.second = splitmix64(seed);
    }

    // Generates a new 64bit random number. This is the core of
    // the generator, used in the other operations as well.
    inline uint64_t operator()() {
        const uint64_t s0 = mState.first;
        uint64_t s1 = mState.second;
        const uint64_t result = s0 + s1;

        s1 ^= s0;
        mState.first = rotl(s0, 55) ^ s1 ^ (s1 << 14); // a, b
        mState.second = rotl(s1, 36); // c

        return result;
    }

    // Uniform random double value in the interval [0..1).
    // See http://xoroshiro.di.unimi.it/ "Generating uniform doubles in the unit interval"
    inline double rand01() {
        union {
            uint64_t i;
            double d;
        } x;

        x.i = UINT64_C(0x3FF) << 52 | operator()() >> 12;
        return x.d - 1.0;

        // 1.0 / (2^64 - 1)
        //return operator()() * 5.4210108624275221703311375920553e-20;
    }

    /// Random float value between min and max.
    inline double operator()(double min_val, double max_val) {
        return (max_val - min_val) * rand01() + min_val;
    }

    // Generates using the given range. This has a modulo bias.
    // see https://github.com/imneme/pcg-c-basic/blob/master/pcg_basic.c#L79
    inline uint64_t operator()(uint64_t bound) {
        const uint64_t threshold = (0 - bound) % bound;

        for (;;) {
            const uint64_t r = operator()();
            if (r >= threshold) {
                return r % bound;
            }
        }
    }

    inline static uint64_t max() {
        return static_cast<uint64_t>(-1);
    }

    inline static uint64_t min() {
        return 0;
    }

    inline const state_type& state() const {
        return mState;
    }

    void state(const state_type& newState) {
        mState = newState;
    }

private:
    static inline uint64_t rotl(const uint64_t x, int k) {
        return (x << k) | (x >> (64 - k));
    }

    // Simple RNG using a 64 bit seed. This RNG is just used to provide a seed.
    // see http://xoroshiro.di.unimi.it/splitmix64.c
    // Updates state, and return the new random number based on it.
    static inline uint64_t splitmix64(uint64_t& state) {
        uint64_t z = (state += UINT64_C(0x9E3779B97F4A7C15));
        z = (z ^ (z >> 30)) * UINT64_C(0xBF58476D1CE4E5B9);
        z = (z ^ (z >> 27)) * UINT64_C(0x94D049BB133111EB);
        return z ^ (z >> 31);
    }

    // internal state of the generator, to provide a 2^128-1 period.
    state_type mState;
};
