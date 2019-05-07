#include <asm/unistd.h>
#include <errno.h>
#include <inttypes.h>
#include <linux/hw_breakpoint.h>
#include <linux/perf_event.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/syscall.h>
#include <unistd.h>

#include <app/doctest.h>

#include <iostream>
#include <map>
#include <unordered_map>
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

class PerformanceCounters {
public:
    enum PerfTime { PERF_TIME_ENABLED_NANOS, PERF_TIME_RUNNING_NANOS };

    uint64_t const* monitor(perf_hw_id id) {
        return monitor(PERF_TYPE_HARDWARE, id);
    }

    uint64_t const* monitor(perf_sw_ids id) {
        return monitor(PERF_TYPE_SOFTWARE, id);
    }

    uint64_t const* monitor(PerfTime id) {
        switch (id) {
        case PERF_TIME_ENABLED_NANOS:
            return &mTimeEnabledNanos;
        case PERF_TIME_RUNNING_NANOS:
            return &mTimeRunningNanos;
        default:
            return nullptr;
        }
    }

    void reset() {
        ioctl(mFd, PERF_EVENT_IOC_RESET, PERF_IOC_FLAG_GROUP);
    }

    void enable() {
        ioctl(mFd, PERF_EVENT_IOC_ENABLE, PERF_IOC_FLAG_GROUP);
    }

    void disable() {
        ioctl(mFd, PERF_EVENT_IOC_DISABLE, PERF_IOC_FLAG_GROUP);
    }

    void fetch() {
        auto const numBytes = sizeof(uint64_t) * mReadFormat.size();
        auto ret = read(mFd, mReadFormat.data(), numBytes);
        if (ret % 8 != 0) {
            throw std::runtime_error("not enough bytes read - maybe monitor the same thing twice?");
        }
        /*
        if (ret != static_cast<ssize_t>(numBytes)) {
            std::cout << mReadFormat[0] << std::endl;
            REQUIRE(ret == static_cast<ssize_t>(numBytes));
            throw std::runtime_error("not enough bytes read - maybe monitor the same thing twice?");
        }
*/
        // clear old data
        for (auto& id_value : mIdToValue) {
            id_value.second = 0;
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

    ~PerformanceCounters() {
        if (mFd != -1) {
            close(mFd);
        }
    }

private:
    uint64_t const* monitor(perf_type_id type, uint64_t eventid) {
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
        if (mFd == -1) {
            // first call: set to fd, and use this from now on
            mFd = fd;
        }
        uint64_t id;
        ioctl(fd, PERF_EVENT_IOC_ID, &id);

        // insert into map, rely on the fact that map's references are constant.
        auto ret = &mIdToValue[id];

        // prepare readformat with the correct size (after the insert)
        mReadFormat.resize(3 + 2 * mIdToValue.size());
        return ret;
    }

    std::map<uint64_t, uint64_t> mIdToValue{};
    std::vector<uint64_t> mReadFormat{};
    int mFd = -1;
    uint64_t mTimeEnabledNanos = 0;
    uint64_t mTimeRunningNanos = 0;
};

} // namespace

TEST_CASE("measure times new") {
    PerformanceCounters pc;

    auto swPageFaults = pc.monitor(PERF_COUNT_SW_PAGE_FAULTS);
    auto cycles = pc.monitor(PERF_COUNT_HW_CPU_CYCLES);
    // auto contextSwitches = pc.monitor(PERF_COUNT_SW_CONTEXT_SWITCHES);
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
    // std::cout << *contextSwitches << " context-switches" << std::endl;
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
