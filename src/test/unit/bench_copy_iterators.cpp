#include <robin_hood.h>

#include <app/benchmark.h>
#include <app/doctest.h>
#include <thirdparty/nanobench/nanobench.h>

#include <unordered_map>

TYPE_TO_STRING(robin_hood::unordered_flat_map<uint64_t, uint64_t>);
TYPE_TO_STRING(robin_hood::unordered_node_map<uint64_t, uint64_t>);

TEST_CASE_TEMPLATE("bench_copy_iterators" * doctest::test_suite("nanobench") * doctest::skip(), Map,
                   robin_hood::unordered_flat_map<uint64_t, uint64_t>,
                   robin_hood::unordered_node_map<uint64_t, uint64_t>) {

    Map a;
    for (size_t i = 0; i < 1000; ++i) {
        a.emplace(i, i);
    }

    ankerl::nanobench::Bench bench;
    Map b;
    bench.batch(a.size()).run("copy " + type_string(a), [&] {
        b = a;
        a = b;
    });
    REQUIRE(a.size() == b.size());
}
