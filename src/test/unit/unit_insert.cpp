#include <robin_hood.h>

#include <app/doctest.h>

TYPE_TO_STRING(robin_hood::unordered_flat_map<unsigned int, int>);
TYPE_TO_STRING(robin_hood::unordered_node_map<unsigned int, int>);

TEST_CASE_TEMPLATE("insert", Map, robin_hood::unordered_flat_map<unsigned int, int>,
                   robin_hood::unordered_node_map<unsigned int, int>) {

    Map map;

    typename Map::value_type val(123U, 321);
    map.insert(val);
    REQUIRE(map.size() == 1);

    REQUIRE(map[123U] == 321);
}
