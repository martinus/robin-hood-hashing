#ifndef BENCHMARK_H
#define BENCHMARK_H

#include <app/PerformanceCounters.h>

#include <chrono>
#include <iostream>
#include <string>

#if defined(__clang__)
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wpadded"
#endif

class Benchmark {
public:
    using clock =
        std::conditional<std::chrono::high_resolution_clock::is_steady,
                         std::chrono::high_resolution_clock, std::chrono::steady_clock>::type;

    Benchmark(std::string const& msg)
        : Benchmark(msg, 1, "op") {}

    template <typename T>
    Benchmark(std::string const& msg, T c, std::string const& opName)
        : mPc()
        , mMsg(msg)
        , mCount(static_cast<double>(c))
        , mOpName(opName)
        , mStartTime() {
        mSwPageFaults = mPc.monitor(PerformanceCounters::Event::page_faults);
        mCycles = mPc.monitor(PerformanceCounters::Event::cpu_cycles);
        mContextSwitches = mPc.monitor(PerformanceCounters::Event::context_switches);
        mInstructions = mPc.monitor(PerformanceCounters::Event::instructions);
        mBranches = mPc.monitor(PerformanceCounters::Event::branch_instructions);
        mMisses = mPc.monitor(PerformanceCounters::Event::branch_misses);

        // go!
        mStartTime = clock::now();
        mPc.enable();
    }

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
    PerformanceCounters mPc;
    uint64_t const* mSwPageFaults;
    uint64_t const* mCycles;
    uint64_t const* mContextSwitches;
    uint64_t const* mInstructions;
    uint64_t const* mBranches;
    uint64_t const* mMisses;
    std::string const mMsg;
    double mCount;
    std::string const mOpName;
    clock::time_point mStartTime;
    bool mHasRun = false;
};

#if defined(__clang__)
#    pragma clang diagnostic pop
#endif

#define BENCHMARK(x, count, opname) for (Benchmark b(x, count, opname); b();)

#endif
