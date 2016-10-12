#pragma once

#include <chrono>

class Timer {
public:
    inline Timer() {
        restart();
    }

    inline void restart() {
        _start = std::chrono::high_resolution_clock::now();
    }

    inline double elapsed() const {
        auto stop = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duration = stop - _start;
        return duration.count();
    }

    inline double elapsed_restart() {
        auto stop = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duration = stop - _start;
        _start = stop;
        return duration.count();
    }

private:
    std::chrono::time_point<std::chrono::high_resolution_clock> _start;
};
