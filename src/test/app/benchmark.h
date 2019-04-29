#ifndef BENCHMARK_H
#define BENCHMARK_H

#include <chrono>
#include <iostream>

// add explicit alignas() so -Wpadded doesn't warn
class Benchmark {
public:
    using clock =
        std::conditional<std::chrono::high_resolution_clock::is_steady,
                         std::chrono::high_resolution_clock, std::chrono::steady_clock>::type;

    Benchmark(std::string const& msg)
        : Benchmark(msg, 1, "op") {}

    template <typename T>
    Benchmark(std::string const& msg, T c, std::string const& opName)
        : mMsg(msg)
        , mCount(static_cast<double>(c))
        , mOpName(opName)
        , mStartTime(clock::now()) {}

    ~Benchmark();

    bool operator()() {
        if (mHasRun) {
            return false;
        }
        mHasRun = true;
        return true;
    }

    template <typename T>
    void count(T c) {
        mCount = c;
    }

private:
    std::string const mMsg;
    double mCount;
    std::string const mOpName;
    clock::time_point mStartTime;
    bool mHasRun = false;
};

#define BENCHMARK(x, count, opname) for (Benchmark b(x, count, opname); b();)

#endif
