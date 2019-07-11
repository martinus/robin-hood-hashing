#include <robin_hood.h>

#include <app/doctest.h>

TYPE_TO_STRING(robin_hood::unordered_flat_map<uint64_t, uint64_t>);
TYPE_TO_STRING(robin_hood::unordered_node_map<uint64_t, uint64_t>);

TEST_CASE_TEMPLATE("reserve", Map, robin_hood::unordered_flat_map<uint64_t, uint64_t>,
                   robin_hood::unordered_node_map<uint64_t, uint64_t>) {
    Map map;

    REQUIRE(0 == map.mask());
    map.reserve(819);
    REQUIRE(1023 == map.mask());
    map.reserve(820);
    REQUIRE(2047 == map.mask());

    for (uint64_t i = 0; i < 100; ++i) {
        map[i] = i;
    }

    REQUIRE(2047 == map.mask());

    // shrink map to best fit
    map.reserve(0);
    REQUIRE(127 == map.mask());
}

TEST_CASE_TEMPLATE("reserve max", Map, robin_hood::unordered_flat_map<uint64_t, uint64_t>,
                   robin_hood::unordered_node_map<uint64_t, uint64_t>) {
    Map map;
    REQUIRE_THROWS_AS(map.reserve((std::numeric_limits<size_t>::max)() - 2), std::overflow_error);
}
