#include <app/PerformanceCounters.h>

const uint64_t PerformanceCounters::no_data = static_cast<uint64_t>(-1);

#if defined(__linux__) && PERFORMANCE_COUNTERS_ENABLED()

#    include <robin_hood.h>

// currently only linux is supported.

#    include <cstdlib>
#    include <cstring>
#    include <iostream>
#    include <map>
#    include <vector>

#    include <linux/perf_event.h>
#    include <sys/ioctl.h>
#    include <sys/syscall.h>
#    include <unistd.h>

namespace {

uint64_t const* mon(PerformanceCounters* const pc, perf_sw_ids id) {
    return pc->monitor(PERF_TYPE_SOFTWARE, id);
}

uint64_t const* mon(PerformanceCounters* const pc, perf_hw_id id) {
    return pc->monitor(PERF_TYPE_HARDWARE, id);
}

} // namespace

uint64_t const* PerformanceCounters::monitor(Event e) {
    switch (e) {
    case Event::time_total_enabled:
        return &mTimeEnabledNanos;

    case Event::time_total_running:
        return &mTimeRunningNanos;

    case Event::cpu_cycles:
        return mon(this, PERF_COUNT_HW_CPU_CYCLES);

    case Event::instructions:
        return mon(this, PERF_COUNT_HW_INSTRUCTIONS);

    case Event::cache_references:
        return mon(this, PERF_COUNT_HW_CACHE_REFERENCES);

    case Event::cache_misses:
        return mon(this, PERF_COUNT_HW_CACHE_MISSES);

    case Event::branch_instructions:
        return mon(this, PERF_COUNT_HW_BRANCH_INSTRUCTIONS);

    case Event::branch_misses:
        return mon(this, PERF_COUNT_HW_BRANCH_MISSES);

    case Event::bus_cycles:
        return mon(this, PERF_COUNT_HW_BUS_CYCLES);

    case Event::stalled_cycles_frontend:
        return mon(this, PERF_COUNT_HW_STALLED_CYCLES_FRONTEND);

    case Event::stalled_cycles_backend:
        return mon(this, PERF_COUNT_HW_STALLED_CYCLES_BACKEND);

    case Event::ref_cpu_cycles:
        return mon(this, PERF_COUNT_HW_REF_CPU_CYCLES);

    case Event::cpu_clock:
        return mon(this, PERF_COUNT_SW_CPU_CLOCK);

    case Event::task_clock:
        return mon(this, PERF_COUNT_SW_TASK_CLOCK);

    case Event::page_faults:
        return mon(this, PERF_COUNT_SW_PAGE_FAULTS);

    case Event::context_switches:
        return mon(this, PERF_COUNT_SW_CONTEXT_SWITCHES);

    case Event::cpu_migrations:
        return mon(this, PERF_COUNT_SW_CPU_MIGRATIONS);

    case Event::page_faults_minor:
        return mon(this, PERF_COUNT_SW_PAGE_FAULTS_MIN);

    case Event::page_faults_major:
        return mon(this, PERF_COUNT_SW_PAGE_FAULTS_MAJ);

    case Event::alignment_faults:
        return mon(this, PERF_COUNT_SW_ALIGNMENT_FAULTS);

    case Event::emulation_faults:
        return mon(this, PERF_COUNT_SW_EMULATION_FAULTS);

#    if !defined(__clang__)
    default:
#        if ROBIN_HOOD(HAS_EXCEPTIONS)
        throw std::runtime_error("unknown event");
#        else
        abort();
#        endif
#    endif
    }
}

// start counting
void PerformanceCounters::enable() const {
    ioctl(mFd, PERF_EVENT_IOC_ENABLE, PERF_IOC_FLAG_GROUP); // NOLINT(hicpp-signed-bitwise)
}

// stop counting
void PerformanceCounters::disable() const {
    ioctl(mFd, PERF_EVENT_IOC_DISABLE, PERF_IOC_FLAG_GROUP); // NOLINT(hicpp-signed-bitwise)
}

void PerformanceCounters::reset() const {
    ioctl(mFd, PERF_EVENT_IOC_RESET, PERF_IOC_FLAG_GROUP); // NOLINT(hicpp-signed-bitwise)
}

uint64_t const* PerformanceCounters::monitor(uint32_t type, uint64_t eventid) {
    auto pea = perf_event_attr();
    memset(&pea, 0, sizeof(perf_event_attr));
    pea.type = type;
    pea.size = sizeof(perf_event_attr);
    pea.config = eventid;
    pea.disabled = 1; // start counter as disabled
    pea.exclude_kernel = 1;
    pea.exclude_hv = 1;

    // NOLINTNEXTLINE(hicpp-signed-bitwise)
    pea.read_format = PERF_FORMAT_GROUP | PERF_FORMAT_ID | PERF_FORMAT_TOTAL_TIME_ENABLED |
                      PERF_FORMAT_TOTAL_TIME_RUNNING;

    const int pid = 0;  // the current process
    const int cpu = -1; // all CPUs
    const unsigned long flags = 0;

    auto fd = static_cast<int>(syscall(__NR_perf_event_open, &pea, pid, cpu, mFd, flags));
    if (-1 == fd) {
        return &no_data;
    }
    if (-1 == mFd) {
        // first call: set to fd, and use this from now on
        mFd = fd;
    }
    uint64_t id = 0;
    // NOLINTNEXTLINE(hicpp-signed-bitwise)
    if (-1 == ioctl(fd, PERF_EVENT_IOC_ID, &id)) {
        // couldn't get id: return pointer to no_data
        return &no_data;
    }

    // insert into map, rely on the fact that map's references are constant.
    auto* ret = &mIdToValue[id];

    // prepare readformat with the correct size (after the insert)
    mReadFormat.resize(3 + 2 * mIdToValue.size());
    return ret;
}

void PerformanceCounters::fetch() {
    auto const numBytes = sizeof(uint64_t) * mReadFormat.size();
    auto ret = read(mFd, mReadFormat.data(), numBytes);
    if (ret <= 0 || (ret % 8) != 0) {
#    if ROBIN_HOOD(HAS_EXCEPTIONS)
        throw std::runtime_error("not enough bytes read - maybe monitor the same thing twice?");
#    else
        abort();
#    endif
    }

    // clear old data
    for (auto& id_value : mIdToValue) {
        id_value.second = no_data;
    }

    mTimeEnabledNanos = mReadFormat[1];
    mTimeRunningNanos = mReadFormat[2];

    for (uint64_t i = 0; i < mReadFormat[0]; i++) {
        auto val = mReadFormat[static_cast<size_t>(3 + i * 2 + 0)];
        auto id = mReadFormat[static_cast<size_t>(3 + i * 2 + 1)];
        auto it = mIdToValue.find(id);
        if (it != mIdToValue.end()) {
            it->second = val;
        }
    }
}

PerformanceCounters::~PerformanceCounters() {
    if (-1 != mFd) {
        close(mFd);
    }
}

#else

uint64_t const* PerformanceCounters::monitor(Event /*unused*/) {
    return &no_data;
}

uint64_t const* PerformanceCounters::monitor(uint32_t /*unused*/, uint64_t /*unused*/) {
    return &no_data;
}

void PerformanceCounters::enable() const {}
void PerformanceCounters::disable() const {}
void PerformanceCounters::reset() const {}
void PerformanceCounters::fetch() {}
PerformanceCounters::~PerformanceCounters() = default;

#endif
