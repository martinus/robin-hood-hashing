#include <benchmark.h>

#include <fmt/streamstate.h>

#include <iomanip>

Benchmark::~Benchmark() {
    fmt::streamstate ss(std::cout);
    auto runtime_sec = std::chrono::duration<double>(clock::now() - mStartTime).count();
    std::cout.precision(3);
    std::cout << std::fixed << std::setw(16) << (runtime_sec / mCount * 1e9) << " ns/" << mOpName
              << "," << std::setw(7) << runtime_sec << " s total " << mMsg << std::endl;
}
