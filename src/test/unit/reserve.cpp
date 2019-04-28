#include "robin_hood.h"

#include "doctest.h"

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
}