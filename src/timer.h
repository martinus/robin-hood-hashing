#pragma once

#include <Windows.h>
#ifdef max
#undef max
#endif
#ifdef min 
#undef min
#endif

inline size_t ticks() {
  LARGE_INTEGER s;
  QueryPerformanceCounter(&s);
  return s.LowPart;
}

class Timer {
public:
  inline Timer() {
    restart();
  }

  inline void restart() {
    QueryPerformanceCounter(&_start);
  }

  inline LONGLONG elapsed_ticks() const {
    LARGE_INTEGER stop;
    QueryPerformanceCounter(&stop);
    return stop.QuadPart - _start.QuadPart;
  }

  inline LONGLONG frequency() const {
    LARGE_INTEGER freq;
    QueryPerformanceFrequency(&freq);
    return freq.QuadPart;
  }


  inline double elapsed() const {
    return elapsed_ticks()/(double)frequency();
  }

private:
  LARGE_INTEGER _start;
};
