#include <robin_hood.h>

#include <app/doctest.h>

#include <ostream>

TEST_CASE("copyassignment") {
    robin_hood::unordered_map<std::string, std::string> map;
    robin_hood::unordered_map<std::string, std::string> tmp;

    map.emplace("a", "b");
    map = tmp;
    map.emplace("c", "d");

    REQUIRE(map.size() == 1);
    REQUIRE(map["c"] == "d");
    REQUIRE(map.size() == 1);

    REQUIRE(tmp.size() == 0);

    map["e"] = "f";
    REQUIRE(map.size() == 2);
    REQUIRE(tmp.size() == 0);

    tmp["g"] = "h";
    REQUIRE(map.size() == 2);
    REQUIRE(tmp.size() == 1);
}
