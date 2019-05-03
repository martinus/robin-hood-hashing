#include <robin_hood.h>

#include <app/doctest.h>

TYPE_TO_STRING(robin_hood::unordered_flat_map<int, int>);
TYPE_TO_STRING(robin_hood::unordered_node_map<int, int>);

TEST_CASE_TEMPLATE("count", Map, robin_hood::unordered_flat_map<int, int>,
                   robin_hood::unordered_node_map<int, int>) {
    Map map;
    REQUIRE(map.count(123) == 0);
    REQUIRE(map.count(0) == 0);
    map[123];
    REQUIRE(map.count(123) == 1);
    REQUIRE(map.count(0) == 0);
}
