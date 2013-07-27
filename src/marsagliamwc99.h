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
  MarsagliaMWC99()
  : _range(std::numeric_limits<unsigned>::max()),
    _w(521288629 + ticks()),
    _z(362436069 + ticks())
  { }
    
  MarsagliaMWC99(unsigned range)
  : _range(range),
    _w(521288629 + ticks()),
    _z(362436069 + ticks())
  { }

  void seed(unsigned val) {
    _z = 362436069;
    _w = val;
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
  inline unsigned operator()(unsigned range) {
    return next() % range;
  }

private:
  const unsigned _range;
  unsigned _z;
  unsigned _w;
};
