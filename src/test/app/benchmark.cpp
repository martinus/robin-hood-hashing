#include <app/benchmark.h>

#include <app/fmt/streamstate.h>

#include <iomanip>

namespace {

struct Metric {
    Metric(double x, double count, std::string unit)
        : mVal(x / count)
        , mUnit(unit) {}

    double const mVal;
    std::string const mUnit;
};

bool isValid(uint64_t const* val) {
    return val != nullptr && *val != std::numeric_limits<uint64_t>::max();
}

std::ostream& operator<<(std::ostream& os, Metric const& m) {
    if (m.mVal < 0.0) {
        // invalid data, don't show
        return os;
    }

    fmt::streamstate ss(std::cout);
    os.precision(3);
    os << std::fixed << m.mVal << " " << m.mUnit << "; ";
    return os;
}

Metric metric(uint64_t const* val, double count, std::string unit) {
    if (!isValid(val)) {
        return Metric(-1.0, count, unit);
    }
    return Metric(static_cast<double>(*val), count, unit);
}

Metric metric(double val, double count, std::string unit) {
    return Metric(val, count, unit);
}

} // namespace

Benchmark::~Benchmark() {
    auto runtime_sec = std::chrono::duration<double>(clock::now() - mStartTime).count();
    mPc.disable();
    mPc.fetch();
    double branchMissesPercent = -1;
    if (isValid(mMisses) && isValid(mBranches)) {
        branchMissesPercent = static_cast<double>(*mMisses) / static_cast<double>(*mBranches);
    }

    std::cout << metric(runtime_sec, mCount, "s")
              << metric(mContextSwitches, mCount, " context-switches")
              << metric(mSwPageFaults, mCount, " page-faults") << metric(mCycles, mCount, " cycles")
              << metric(mInstructions, mCount, " instructions")
              << metric(mBranches, mCount, " branches") << metric(mMisses, mCount, " branch-misses")
              << "(" << metric(branchMissesPercent, 0.01, "%") << ")" << std::endl;

    /*
    std::cout << std::fixed << std::setw(16) << (runtime_sec / mCount * 1e9) << " ns/" << mOpName
              << "," << std::setw(7) << runtime_sec << " s total " << mMsg << std::endl;
              */
}
