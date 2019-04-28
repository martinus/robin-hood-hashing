#ifndef BENCHMARK_H
#define BENCHMARK_H

#include <chrono>
#include <iostream>

class Benchmark {
public:
    using clock =
        std::conditional<std::chrono::high_resolution_clock::is_steady,
                         std::chrono::high_resolution_clock, std::chrono::steady_clock>::type;

    template <typename T>
    Benchmark(std::string const& msg, T divisor, std::string const& opName)
        : mMsg(msg)
        , mDivisor(static_cast<double>(divisor))
        , mOpName(opName)
        , mStartTime(clock::now()) {}

    ~Benchmark() {
        auto runtime_sec = std::chrono::duration<double>(clock::now() - mStartTime).count();
        std::cerr << (runtime_sec / mDivisor * 1e9) << " ns/" << mOpName << " (" << runtime_sec
                  << " s total) " << mMsg << std::endl;
    }

    bool operator()() {
        if (mHasRun) {
            return false;
        }
        mHasRun = true;
        return true;
    }

private:
    std::string const mMsg;
    double const mDivisor;
    std::string const mOpName;
    clock::time_point mStartTime;
    bool mHasRun = false;
};

#define BENCHMARK(x, divisor, opname) for (Benchmark b(x, divisor, opname); b();)

#endif