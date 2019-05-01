#include <robin_hood.h>

#include <app/benchmark.h>
#include <app/doctest.h>
#include <app/sfc64.h>

TEST_CASE_TEMPLATE("bench robin_hood::hash" * doctest::test_suite("bench") * doctest::skip(), T,
                   uint64_t, int32_t) {
    sfc64 rng(123);

    // add a (neglectible) bit of randomization so the compiler can't optimize this away
    T const numIters = 2000000000;
    size_t a = 0;
    BENCHMARK("robin_hood::hash", numIters, "op") {
        robin_hood::hash<T> hasher;
        for (T i = 0; i < numIters; ++i) {
            a += hasher(i);
        }
    }
    INFO(a);
}
