#include <robin_hood.h>

#include <app/benchmark.h>
#include <app/doctest.h>

#include <unordered_map>

TYPE_TO_STRING(robin_hood::unordered_flat_map<uint64_t, uint64_t>);
TYPE_TO_STRING(robin_hood::unordered_node_map<uint64_t, uint64_t>);

TEST_CASE_TEMPLATE("bench copy by iterating" * doctest::test_suite("bench") * doctest::skip(), Map,
                   robin_hood::unordered_flat_map<uint64_t, uint64_t>,
                   robin_hood::unordered_node_map<uint64_t, uint64_t>) {

    static constexpr size_t size = 800000;
    Map map;
    for (size_t i = 0; i < size; ++i) {
        map.emplace(i, i);
    }
    std::cout << map.load_factor() << std::endl;

    BENCHMARK("copy", map.size(), "op") {
        Map map_copy(map.begin(), map.end());
        REQUIRE(map_copy.size() == map.size());
    }
}
