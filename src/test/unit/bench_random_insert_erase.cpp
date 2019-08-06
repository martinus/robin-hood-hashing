#include <robin_hood.h>

#include <app/benchmark.h>
#include <app/sfc64.h>

#include <app/doctest.h>

TYPE_TO_STRING(robin_hood::unordered_flat_map<int, int>);
TYPE_TO_STRING(robin_hood::unordered_node_map<int, int>);
TYPE_TO_STRING(robin_hood::unordered_flat_map<uint64_t, uint64_t>);
TYPE_TO_STRING(robin_hood::unordered_node_map<uint64_t, uint64_t>);

TEST_CASE_TEMPLATE("bench_random_insert_erase" * doctest::test_suite("bench") * doctest::skip(),
                   Map, robin_hood::unordered_flat_map<int, int>,
                   robin_hood::unordered_node_map<int, int>,
                   robin_hood::unordered_flat_map<uint64_t, uint64_t>,
                   robin_hood::unordered_node_map<uint64_t, uint64_t>) {
    sfc64 rng(123);

    using kt = typename Map::key_type;
    using mt = typename Map::mapped_type;

    kt max_n = 15000;
    mt max_i = 15000;

    BENCHMARK("Random insert erase", max_n * max_i, "op") {
        size_t verifier = 0;
        Map map;
        for (kt n = 1; n < max_n; ++n) {
            for (mt i = 0; i < max_i; ++i) {
                map[rng.uniform(n)] = i;
                verifier += map.erase(rng.uniform(n));
            }
        }
        REQUIRE(verifier == 112533302);
    }
}
