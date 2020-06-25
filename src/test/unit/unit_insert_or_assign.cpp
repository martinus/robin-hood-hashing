#include <robin_hood.h>

#include <app/doctest.h>

#include <string>

TYPE_TO_STRING(robin_hood::unordered_flat_map<std::string, std::string>);
TYPE_TO_STRING(robin_hood::unordered_node_map<std::string, std::string>);

TEST_CASE_TEMPLATE("insert_or_assign", Map,
                   robin_hood::unordered_flat_map<std::string, std::string>,
                   robin_hood::unordered_node_map<std::string, std::string>) {

    Map map;
    auto ret = map.insert_or_assign("a", "b");
    REQUIRE(ret.second);
    REQUIRE(ret.first == map.find("a"));
    REQUIRE(map.size() == 1);
    REQUIRE(map["a"] == "b");

    ret = map.insert_or_assign("c", "d");
    REQUIRE(ret.second);
    REQUIRE(ret.first == map.find("c"));
    REQUIRE(map.size() == 2);
    REQUIRE(map["c"] == "d");

    ret = map.insert_or_assign("c", "dd");
    REQUIRE_FALSE(ret.second);
    REQUIRE(ret.first == map.find("c"));
    REQUIRE(map.size() == 2);
    REQUIRE(map["c"] == "dd");

    std::string key = "a";
    std::string value = "bb";
    ret = map.insert_or_assign(key, value);
    REQUIRE_FALSE(ret.second);
    REQUIRE(ret.first == map.find("a"));
    REQUIRE(map.size() == 2);
    REQUIRE(map["a"] == "bb");

    key = "e";
    value = "f";
    ret = map.insert_or_assign(map.end(), key, value);
    REQUIRE(ret.second);
    REQUIRE(ret.first == map.find("e"));
    REQUIRE(map.size() == 3);
    REQUIRE(map["e"] == "f");

    ret = map.insert_or_assign(map.begin(), "e", "ff");
    REQUIRE_FALSE(ret.second);
    REQUIRE(ret.first == map.find("e"));
    REQUIRE(map.size() == 3);
    REQUIRE(map["e"] == "ff");
}
