#include <app/PerformanceCounters.h>
#include <app/doctest.h>

#include <iostream>
#include <map>
#include <vector>

#include <app/sfc64.h>

namespace {

size_t do_something(size_t numIters) {
    sfc64 rng(231);
    for (size_t i = 0; i < numIters; ++i) {
        rng();
    }
    return rng.uniform<size_t>();
}

} // namespace

TEST_CASE("measure times new") {
    PerformanceCounters pc;

    auto swPageFaults = pc.monitor(PerformanceCounters::Event::page_faults);
    auto cycles = pc.monitor(PerformanceCounters::Event::cpu_cycles);
    auto contextSwitches = pc.monitor(PerformanceCounters::Event::context_switches);
    auto instructions = pc.monitor(PerformanceCounters::Event::instructions);
    auto branches = pc.monitor(PerformanceCounters::Event::branch_instructions);
    auto misses = pc.monitor(PerformanceCounters::Event::branch_misses);
    auto timeEnabled = pc.monitor(PerformanceCounters::Event::time_total_enabled);
    auto timeRunning = pc.monitor(PerformanceCounters::Event::time_total_running);

    pc.reset();
    pc.enable();
    size_t s = do_something(1);
    pc.disable();
    s += do_something(1);
    pc.enable();
    s += do_something(1);
    pc.disable();

    pc.fetch();
    std::cout << (static_cast<double>(*timeEnabled) * 1e-9) << " time enabled" << std::endl;
    std::cout << (static_cast<double>(*timeRunning) * 1e-9) << " time running" << std::endl;
    std::cout << *contextSwitches << " context-switches" << std::endl;
    std::cout << *swPageFaults << " page-faults" << std::endl;
    std::cout << (static_cast<double>(*cycles)) << " cycles" << std::endl;
    std::cout << (static_cast<double>(*instructions)) << " instructions" << std::endl;
    std::cout << *branches << " branches" << std::endl;
    std::cout << *misses << " branch-misses" << std::endl;
    std::cout << std::endl;
    std::cout << (static_cast<double>(*instructions) / static_cast<double>(*cycles))
              << " insn per cycle" << std::endl;

    std::cout << (static_cast<double>(*misses) * 100.0 / static_cast<double>(*branches))
              << "% of all branches" << std::endl;
    std::cout << s << std::endl;
}
