#include "test_base.h"

TEST_CASE("Api demonstration", "[demo]") {
    robin_hood::unordered_map<std::string, int> map = {{"duck", 17}, {"lion", 10}};

    REQUIRE(map.size() == 2);
    REQUIRE(map["duck"] == 17);
    REQUIRE(map["lion"] == 10);
}

TEST_CASE("api flat", "[demo]") {
    robin_hood::unordered_flat_map<int, int> map;
    map[123] = 7;
    map[42] = 543;

    // iterators won't be stable
    REQUIRE(map.size() == 2);
}

TEST_CASE("api node", "[demo]") {
    robin_hood::unordered_node_map<int, int> map;
    map[123] = 7;
    map[42] = 543;

    // iterators *will* be stable
    REQUIRE(map.size() == 2);
}
