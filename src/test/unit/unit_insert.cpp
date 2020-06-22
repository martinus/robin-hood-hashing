#include <robin_hood.h>

#include <app/doctest.h>

TYPE_TO_STRING(robin_hood::unordered_flat_map<int, int>);
TYPE_TO_STRING(robin_hood::unordered_node_map<int, int>);

TEST_CASE_TEMPLATE("insert", Map, robin_hood::unordered_flat_map<int, int>,
                   robin_hood::unordered_node_map<int, int>) {

    Map map;

    typename Map::value_type val(123, 321);
    map.insert(val);
    REQUIRE(map.size() == 1);

    REQUIRE(map[123] == 321);
}
