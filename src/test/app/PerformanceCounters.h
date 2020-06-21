#ifndef APP_PERFORMANCECOUNTERS_H
#define APP_PERFORMANCECOUNTERS_H

// enable/disable performance counters here
#define PERFORMANCE_COUNTERS_ENABLED() 1

#include <cinttypes>
#include <cstddef>
#include <map>
#include <vector>

#if defined(__clang__)
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wpadded"
#endif

// API to C++ Performance counters.
class PerformanceCounters {
public:
    static const uint64_t no_data;

    enum class Event {
        // time_enabled can be used to calculate estimated totals if the PMU is overcommitted and
        // multiplexing is
        // happening.
        time_total_enabled,

        // time_enabled can be used to calculate estimated totals if the PMU is overcommitted and
        // multiplexing is
        // happening.
        time_total_running,

        // Total cycles.  Be wary of what happens during CPU frequency scaling.
        cpu_cycles,

        // Retired instructions. Can be affected by e.g. hardware interrupt counts.
        instructions,

        // Cache accesses. Usually this indicates Last Level Cache accesses, but
        // depends on your CPU.
        cache_references,

        // Cache misses. Usually indicates Last Level Cache misses, use in combination
        // with cache_references event to calculate cache miss rate.
        cache_misses,

        // Retired branch instructions.
        branch_instructions,

        // Mispredicted branch instructions.
        branch_misses,

        // Bus cycles, can be different from total cycles.
        bus_cycles,

        // Stalled cycles during issue.
        stalled_cycles_frontend,

        // stalled cycles during retirement.
        stalled_cycles_backend,

        // total cyles, not affected by CPU frequency scaling
        ref_cpu_cycles,

        // CPU clock, a high-resolution per-CPU timer.
        cpu_clock,

        // Clock count specific to the task that is running.
        task_clock,

        // Number of page faults.
        page_faults,

        // Until Linux 2.6.34, these were all reported as user-space
        // events, after that they are reported as happening in the kernel.
        context_switches,

        // number of times the process has migrated to a new CPU.
        cpu_migrations,

        // Number of minor page faults. These did not require disk I/O to handle.
        page_faults_minor,

        // Number of major page faults. These required disk I/O to handle.
        page_faults_major,

        // Number of alignment faults. These happen when unaligned memory accesses happen; the
        // kernel can handle these but it reduces performance.  This happens only on some
        // architectures (never on x86).
        alignment_faults,

        // Number of emulation faults. The kernel sometimes traps on unimplemented instructions and
        // emulates them for user space.  This can negatively impact performance.
        emulation_faults
    };

    uint64_t const* monitor(Event e);

    // resets the counters
    void reset() const;

    // start counting
    void enable() const;

    // stop counting
    void disable() const;

    // fetch counting data into the monitors
    void fetch();

    ~PerformanceCounters();

    uint64_t const* monitor(uint32_t type, uint64_t eventid);

private:
#if defined(__linux__) && PERFORMANCE_COUNTERS_ENABLED()
    std::map<uint64_t, uint64_t> mIdToValue{};
    std::vector<uint64_t> mReadFormat{};
    uint64_t mTimeEnabledNanos = 0;
    uint64_t mTimeRunningNanos = 0;
    int mFd = -1;
#endif
};

#if defined(__clang__)
#    pragma clang diagnostic pop
#endif

#endif
