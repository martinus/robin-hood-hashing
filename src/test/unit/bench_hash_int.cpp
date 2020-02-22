#include <robin_hood.h>

#include <app/benchmark.h>
#include <app/doctest.h>
#include <app/sfc64.h>
#include <thirdparty/nanobench/nanobench.h>

TEST_CASE_TEMPLATE("bench_hash_int" * doctest::test_suite("nanobench") * doctest::skip(), T,
                   uint64_t, int32_t) {
    sfc64 rng(123);

    // add a (neglectible) bit of randomization so the compiler can't optimize this away
    ankerl::nanobench::Bench bench;
    robin_hood::hash<T> hasher;
    T i = 0;
    size_t a = 0;
    bench.run("robin_hood::hash " + type_string(i), [&] { a += hasher(i++); }).doNotOptimizeAway(a);
}
