#include <robin_hood.h>

#include <app/benchmark.h>
#include <app/doctest.h>
#include <app/sfc64.h>

#include <unordered_map>

TYPE_TO_STRING(robin_hood::unordered_flat_map<uint64_t, uint64_t>);
TYPE_TO_STRING(robin_hood::unordered_node_map<uint64_t, uint64_t>);

TEST_CASE_TEMPLATE("bench iterate" * doctest::test_suite("bench") * doctest::skip(), Map,
                   robin_hood::unordered_flat_map<uint64_t, uint64_t>,
                   robin_hood::unordered_node_map<uint64_t, uint64_t>) {

    sfc64 rng(123);

    static constexpr size_t num_iters = 50000;
    static constexpr size_t num_accesses = (num_iters * (num_iters + 1)) / 2;

    uint64_t result = 0;
    Map map;

    auto const state = rng.state();
    BENCHMARK("iterate while adding", num_accesses, "it") {
        for (size_t n = 0; n < num_iters; ++n) {
            map[rng()] = n;
            for (auto const& keyVal : map) {
                result += keyVal.second;
            }
        }
    }
    REQUIRE(result == 20833333325000);

    rng.state(state);
    BENCHMARK("iterate while removing", num_accesses, "it") {
        for (size_t n = 0; n < num_iters; ++n) {
            for (auto const& keyVal : map) {
                result += keyVal.second;
            }
            map.erase(rng());
        }
    }
    REQUIRE(result == 62499999975000);
}
