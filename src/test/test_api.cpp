#include "catch.hpp"
#include "robin_hood.h"

TEST_CASE("Api demonstration") {
	robin_hood::unordered_map<int, int> map;
	map[123] = 7;
	map[42] = 543;

	REQUIRE(map.size() == 2);
}

TEST_CASE("api flat") {
	robin_hood::flat_map<int, int> map;
	map[123] = 7;
	map[42] = 543;

	// iterators won't be stable
	REQUIRE(map.size() == 2);
}

TEST_CASE("api node") {
	robin_hood::node_map<int, int> map;
	map[123] = 7;
	map[42] = 543;

	// iterators *will* be stable
	REQUIRE(map.size() == 2);
}
