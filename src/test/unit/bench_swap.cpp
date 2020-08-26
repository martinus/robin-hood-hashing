#include <robin_hood.h>

#include <app/benchmark.h>
#include <app/doctest.h>
#include <thirdparty/nanobench/nanobench.h>

TYPE_TO_STRING(robin_hood::unordered_flat_map<uint64_t, uint64_t>);
TYPE_TO_STRING(robin_hood::unordered_node_map<uint64_t, uint64_t>);

TEST_CASE_TEMPLATE("bench_swap" * doctest::test_suite("nanobench") * doctest::skip(), Map,
                   robin_hood::unordered_flat_map<uint64_t, uint64_t>,
                   robin_hood::unordered_node_map<uint64_t, uint64_t>) {

    Map a;
    Map b;
    ankerl::nanobench::Rng rng(123);

    ankerl::nanobench::Bench bench;
    for (size_t i = 0; i < 10; ++i) {
        for (size_t j = 0; j < 10U * (1U << i); ++j) {
            a[rng()];
            b[rng()];
        }
        bench.complexityN(a.size()).run("swap " + type_string(a), [&] { std::swap(a, b); });
    }
    ankerl::nanobench::doNotOptimizeAway(&a);
    ankerl::nanobench::doNotOptimizeAway(&b);
    std::cout << bench.complexityBigO() << std::endl;
}
