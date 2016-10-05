#pragma once

#include <limits>

/// MarsagliaMWC99 is a simple random number generator based on 
/// George Marsaglia's MWC (multiply with carry) generator.
/// Although it is very simple, it passes Marsaglia's DIEHARD
/// series of random number generator tests. It is exceptionally fast.
/// 
/// @see http://www.codeproject.com/Articles/25172/Simple-Random-Number-Generation
/// @see http://www.bobwheeler.com/statistics/Password/MarsagliaPost.txt
/// @see http://mathforum.org/kb/message.jspa?messageID=1524861
class MarsagliaMWC99 {
public:
    typedef uint32_t result_type;

    MarsagliaMWC99()
        : _w(521288629 + static_cast<uint32_t>(ticks()))
        , _z(362436069 + static_cast<uint32_t>(ticks())) {
    }

    MarsagliaMWC99(size_t range)
        : _w(521288629 + static_cast<uint32_t>(ticks()))
        , _z(362436069 + static_cast<uint32_t>(ticks())) {
    }

    void seed(size_t val) {
        _z = 362436069;
        _w = static_cast<uint32_t>(val);
    }

    // This is the heart of the generator.
    // It uses George Marsaglia's MWC algorithm to produce an uint32_t integer.
    // @see https://groups.google.com/forum/?fromgroups=#!topic/sci.crypt/yoaCpGWKEk0
    //
    // This has a modulo bias.
    inline uint32_t operator()() {
        _z = 36969 * (_z & 65535) + (_z >> 16);
        _w = 18000 * (_w & 65535) + (_w >> 16);
        return (_z << 16) + _w;
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
    uint32_t _z;
    uint32_t _w;
};
