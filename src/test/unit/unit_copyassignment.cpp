#include <robin_hood.h>

#include <app/doctest.h>

TEST_CASE("copyassignment") {
    robin_hood::unordered_map<std::string, std::string> map;
    robin_hood::unordered_map<std::string, std::string> tmp;
    map.emplace("a", "b");
    map = tmp;
    map.emplace("c", "d");
}
