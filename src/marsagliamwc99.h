#pragma once

#include <timer.h>
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
private:
    // generates next random value
    inline unsigned next() {
        _z = 36969 * (_z & 65535) + (_z >> 16);
        _w = 18000 * (_w & 65535) + (_w >> 16);
        return (_z << 16) + _w;
    }

public:
    typedef unsigned result_type;
    MarsagliaMWC99()
        : _range(std::numeric_limits<unsigned>::max()),
        _w(521288629 + static_cast<unsigned>(ticks())),
        _z(362436069 + static_cast<unsigned>(ticks())) {
    }

    MarsagliaMWC99(size_t range)
        : _range(static_cast<unsigned>(range)),
        _w(521288629 + static_cast<unsigned>(ticks())),
        _z(362436069 + static_cast<unsigned>(ticks())) {
    }

    void seed(size_t val) {
        _z = 362436069;
        _w = static_cast<unsigned>(val);
    }

    // This is the heart of the generator.
    // It uses George Marsaglia's MWC algorithm to produce an unsigned integer.
    // @see https://groups.google.com/forum/?fromgroups=#!topic/sci.crypt/yoaCpGWKEk0
    //
    // This has a modulo bias.
    inline unsigned operator()() {
        return next() % _range;
    }

    /// Random float value between 0 and 1.
    inline float rand01() {
        // 1.0 / (2^32 - 1)
        return next() * 2.3283064370807973754314699618685e-10f;
    }

    /// Random float value between min and max.
    inline float operator()(float min_val, float max_val) {
        return (max_val - min_val) * rand01() + min_val;
    }

    // Generates using the given range. This has a modulo bias.
    inline unsigned operator()(size_t range) {
        return next() % static_cast<unsigned>(range);
    }

    inline static unsigned max() {
        return -1;
    }

    inline static unsigned min() {
        return 0;
    }

private:
    const unsigned _range;
    unsigned _z;
    unsigned _w;
};
