#include <app/benchmark.h>
#include <app/fmt/mup.h>

#include <app/fmt/streamstate.h>

#include <iomanip>

namespace {

bool isValid(uint64_t const* val) {
    return val != nullptr && *val != std::numeric_limits<uint64_t>::max();
}

} // namespace

Benchmark::~Benchmark() {
    using martinus::mup;

    auto runtime_sec = std::chrono::duration<double>(clock::now() - mStartTime).count();
    mPc.disable();
    mPc.fetch();
    double branchMissesPercent = -1;
    if (isValid(mMisses) && isValid(mBranches)) {
        branchMissesPercent = static_cast<double>(*mMisses) / static_cast<double>(*mBranches);
    }

    // << mup(*mContextSwitches / mCount) << " context-switches/" << mOpName << ", "
    fmt::streamstate ss(std::cout);
    std::cout << mup(runtime_sec / mCount) << "s/" << mOpName << ",   "
              << mup(static_cast<double>(*mSwPageFaults) / mCount) << " page-faults/" << mOpName
              << ",   " << mup(static_cast<double>(*mCycles) / mCount) << " cycles/" << mOpName
              << ",   " << mup(static_cast<double>(*mInstructions) / mCount) << " instructions/"
              << mOpName << ",   " << mup(static_cast<double>(*mBranches) / mCount) << " branches/"
              << mOpName << ",   " << mup(static_cast<double>(*mMisses) / mCount)
              << " branch-misses/" << mOpName << " (";

    std::cout << std::setprecision(4) << std::fixed << (branchMissesPercent * 100) << "%) for "
              << mMsg << std::endl;
}
