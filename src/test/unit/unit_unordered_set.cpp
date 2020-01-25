#include <robin_hood.h>

#include <app/doctest.h>

#include <iostream>

TYPE_TO_STRING(robin_hood::unordered_flat_map<uint64_t, void>);
TYPE_TO_STRING(robin_hood::unordered_node_map<uint64_t, void>);

TEST_CASE_TEMPLATE("unordered_set", Map, robin_hood::unordered_flat_map<uint64_t, void>,
                   robin_hood::unordered_node_map<uint64_t, void>) {
    REQUIRE(sizeof(typename Map::value_type) == sizeof(uint64_t));
    Map map;
    map.emplace(UINT64_C(123));
    REQUIRE(map.size() == 1U);

    map.insert(UINT64_C(333));
    REQUIRE(map.size() == 2U);

    map.erase(UINT64_C(222));
    REQUIRE(map.size() == 2U);

    map.erase(UINT64_C(123));
    REQUIRE(map.size() == 1U);
}
