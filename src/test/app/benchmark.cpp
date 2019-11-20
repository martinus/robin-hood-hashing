#include <app/benchmark.h>
#include <app/fmt/mup.h>

#include <app/fmt/streamstate.h>

#include <iomanip>

namespace {

bool isValid(uint64_t const* val) {
    return val != nullptr && *val != std::numeric_limits<uint64_t>::max();
}

} // namespace

void Benchmark::showMetric(uint64_t const* const m, char const* name) const {
    if (!isValid(m)) {
        return;
    }
    std::cout << martinus::mup(static_cast<double>(*m) / mCount) << " " << name << "/" << mOpName
              << ";   ";
}

Benchmark::~Benchmark() {
    auto runtime_sec = std::chrono::duration<double>(clock::now() - mStartTime).count();
    mPc.disable();
    mPc.fetch();
    double branchMissesPercent = -1;
    if (isValid(mMisses) && isValid(mBranches)) {
        branchMissesPercent = static_cast<double>(*mMisses) / static_cast<double>(*mBranches);
    }
    double mInsPerCycle = -1;
    if (isValid(mInstructions) && isValid(mCycles)) {
        mInsPerCycle = static_cast<double>(*mInstructions) / static_cast<double>(*mCycles);
    }

    fmt::streamstate ss(std::cout);
    std::cout << martinus::mup(runtime_sec / mCount) << "s/" << mOpName << ";   ";
    showMetric(mContextSwitches, "context-switches");
    showMetric(mSwPageFaults, "page-faults");
    showMetric(mInstructions, "ins");
    showMetric(mCycles, "cyc");
    if (mInsPerCycle > 0) {
        std::cout << martinus::mup(mInsPerCycle) << " ins/cyc;   ";
    }
    showMetric(mBranches, "bra");
    showMetric(mMisses, "mis");
    if (branchMissesPercent > 0) {
        std::cout << "(" << std::setprecision(2) << std::fixed << (branchMissesPercent * 100)
                  << "%)   ";
    }

    std::cout << mMsg << std::endl;
}
