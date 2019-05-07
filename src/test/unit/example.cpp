#include <app/PerformanceCounters.h>
#include <app/doctest.h>

#include <iostream>
#include <map>
#include <vector>

#include <app/sfc64.h>

namespace {

size_t do_something(size_t numIters) {
    sfc64 rng(231);
    std::map<uint64_t, uint64_t> map;
    for (size_t i = 0; i < numIters; ++i) {
        map[rng()] = 213;
    }
    return map.size();
}

} // namespace

TEST_CASE("measure times new") {
    PerformanceCounters pc;

    auto swPageFaults = pc.monitor(PERF_COUNT_SW_PAGE_FAULTS);
    auto cycles = pc.monitor(PERF_COUNT_HW_CPU_CYCLES);
    auto contextSwitches = pc.monitor(PERF_COUNT_SW_CONTEXT_SWITCHES);
    auto instructions = pc.monitor(PERF_COUNT_HW_INSTRUCTIONS);
    auto branches = pc.monitor(PERF_COUNT_HW_BRANCH_INSTRUCTIONS);
    auto misses = pc.monitor(PERF_COUNT_HW_BRANCH_MISSES);
    auto timeEnabled = pc.monitor(PerformanceCounters::PERF_TIME_ENABLED_NANOS);
    auto timeRunning = pc.monitor(PerformanceCounters::PERF_TIME_RUNNING_NANOS);

    pc.reset();
    pc.enable();
    size_t s = do_something(1000000);
    pc.disable();
    s += do_something(1000000);
    pc.enable();
    s += do_something(1000000);
    pc.disable();

    pc.fetch();
    std::cout << (static_cast<double>(*timeEnabled) * 1e-9) << " time enabled" << std::endl;
    std::cout << (static_cast<double>(*timeRunning) * 1e-9) << " time running" << std::endl;
    std::cout << *contextSwitches << " context-switches" << std::endl;
    std::cout << *swPageFaults << " page-faults" << std::endl;
    std::cout << (static_cast<double>(*cycles) / 1000000.0) << " cycles" << std::endl;
    std::cout << (static_cast<double>(*instructions) / 1000000.0) << " instructions" << std::endl;
    std::cout << *branches << " branches" << std::endl;
    std::cout << *misses << " branch-misses" << std::endl;
    std::cout << std::endl;
    std::cout << (static_cast<double>(*instructions) / static_cast<double>(*cycles))
              << " insn per cycle" << std::endl;

    std::cout << (static_cast<double>(*misses) * 100.0 / static_cast<double>(*branches))
              << "% of all branches" << std::endl;
    std::cout << s << std::endl;
}
