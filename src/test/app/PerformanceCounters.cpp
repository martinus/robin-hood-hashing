#include <app/PerformanceCounters.h>

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <map>
#include <vector>

#include <linux/perf_event.h>
#include <sys/ioctl.h>
#include <sys/syscall.h>
#include <unistd.h>

const uint64_t PerformanceCounters::no_data = static_cast<uint64_t>(-1);

uint64_t const* PerformanceCounters::monitor(Event e) {
    switch (e) {
    case Event::time_total_enabled:
        return &mTimeEnabledNanos;
    case Event::time_total_running:
        return mTimeRunningNanos;
        /*
    case cpu_cycles:

    case instructions:
    case cache_references:
    case cache_misses:
    case branch_instructions:
    case branch_misses:
    case bus_cycles:
    case stalled_cycles_frontend:
    case stalled_cycles_backend:
    case ref_cpu_cycles:
    case cpu_clock:
    case task_clock:
    case page_faults:
    case context_switches:
    case cpu_migrations:
    case page_faults_minor:
    case page_faults_major:
    case alignment_faults:
    case emulation_faults:
        * /
    }
    return monitor(PERF_TYPE_HARDWARE, id);
}

uint64_t const* PerformanceCounters::monitor(perf_sw_ids id) {
    return monitor(PERF_TYPE_SOFTWARE, id);
}

// start counting
void PerformanceCounters::enable() {
    ioctl(mFd, PERF_EVENT_IOC_ENABLE, PERF_IOC_FLAG_GROUP);
}

// stop counting
void PerformanceCounters::disable() {
    ioctl(mFd, PERF_EVENT_IOC_DISABLE, PERF_IOC_FLAG_GROUP);
}

uint64_t const* PerformanceCounters::monitor(PerfTime id) {
    switch (id) {
    case PERF_TIME_ENABLED_NANOS:
        return &mTimeEnabledNanos;
    case PERF_TIME_RUNNING_NANOS:
        return &mTimeRunningNanos;
    default:
        return nullptr;
    }
}

void PerformanceCounters::reset() {
    ioctl(mFd, PERF_EVENT_IOC_RESET, PERF_IOC_FLAG_GROUP);
}

uint64_t const* PerformanceCounters::monitor(perf_type_id type, uint64_t eventid) {
    perf_event_attr pea;
    memset(&pea, 0, sizeof(perf_event_attr));
    pea.type = type;
    pea.size = sizeof(perf_event_attr);
    pea.config = eventid;
    pea.disabled = 1; // start counter as disabled
    pea.exclude_kernel = 1;
    pea.exclude_hv = 1;
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
    if (-1 == ioctl(fd, PERF_EVENT_IOC_ID, &id)) {
        // couldn't get id: return pointer to no_data
        return &no_data;
    }

    // insert into map, rely on the fact that map's references are constant.
    auto ret = &mIdToValue[id];

    // prepare readformat with the correct size (after the insert)
    mReadFormat.resize(3 + 2 * mIdToValue.size());
    return ret;
}

void PerformanceCounters::fetch() {
    auto const numBytes = sizeof(uint64_t) * mReadFormat.size();
    auto ret = read(mFd, mReadFormat.data(), numBytes);
    if (ret <= 0 || (ret % 8) != 0) {
        throw std::runtime_error("not enough bytes read - maybe monitor the same thing twice?");
    }

    // clear old data
    for (auto& id_value : mIdToValue) {
        id_value.second = no_data;
    }

    mTimeEnabledNanos = mReadFormat[1];
    mTimeRunningNanos = mReadFormat[2];

    for (uint64_t i = 0; i < mReadFormat[0]; i++) {
        auto val = mReadFormat[3 + i * 2 + 0];
        auto id = mReadFormat[3 + i * 2 + 1];
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