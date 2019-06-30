#include <robin_hood.h>

#include <app/doctest.h>

TYPE_TO_STRING(robin_hood::unordered_flat_map<int, int>);
TYPE_TO_STRING(robin_hood::unordered_node_map<int, int>);

TEST_CASE_TEMPLATE("at", Map, robin_hood::unordered_flat_map<int, int>,
                   robin_hood::unordered_node_map<int, int>) {
    Map map;
    Map const& cmap = map;
    REQUIRE_THROWS_AS(map.at(123), std::out_of_range);
    REQUIRE_THROWS_AS(static_cast<void>(map.at(0)), std::out_of_range);
    REQUIRE_THROWS_AS(static_cast<void>(cmap.at(123)), std::out_of_range);
    REQUIRE_THROWS_AS(static_cast<void>(cmap.at(0)), std::out_of_range);
    map[123] = 333;
    REQUIRE(map.at(123) == 333);
    REQUIRE(cmap.at(123) == 333);
    REQUIRE_THROWS_AS(static_cast<void>(map.at(0)), std::out_of_range);
    REQUIRE_THROWS_AS(static_cast<void>(cmap.at(0)), std::out_of_range);
}
