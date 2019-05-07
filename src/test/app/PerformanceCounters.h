#ifndef APP_PERFORMANCECOUNTERS_H
#define APP_PERFORMANCECOUNTERS_H

#include <linux/perf_event.h>
#include <sys/ioctl.h>

#include <cinttypes>
#include <cstddef>
#include <map>
#include <vector>

class PerformanceCounters {
public:
    static const uint64_t no_data;

    enum PerfTime { PERF_TIME_ENABLED_NANOS, PERF_TIME_RUNNING_NANOS };

    uint64_t const* monitor(perf_hw_id id);
    uint64_t const* monitor(perf_sw_ids id);
    uint64_t const* monitor(PerfTime id);

    // resets the counters
    void reset();

    // start counting
    inline void enable() {
        ioctl(mFd, PERF_EVENT_IOC_ENABLE, PERF_IOC_FLAG_GROUP);
    }

    // stop counting
    inline void disable() {
        ioctl(mFd, PERF_EVENT_IOC_DISABLE, PERF_IOC_FLAG_GROUP);
    }

    // fetch counting data into the monitors
    void fetch();

    ~PerformanceCounters();

private:
    uint64_t const* monitor(perf_type_id type, uint64_t eventid);

    std::map<uint64_t, uint64_t> mIdToValue{};
    std::vector<uint64_t> mReadFormat{};
    int mFd = -1;
    uint64_t mTimeEnabledNanos = 0;
    uint64_t mTimeRunningNanos = 0;
};

#endif
