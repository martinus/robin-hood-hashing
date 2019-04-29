#include <robin_hood.h>

#include <doctest.h>

#include <string>

TYPE_TO_STRING(robin_hood::unordered_flat_map<std::string, int>);
TYPE_TO_STRING(robin_hood::unordered_node_map<std::string, int>);

TEST_CASE_TEMPLATE("initializerlist std::string", Map,
                   robin_hood::unordered_flat_map<std::string, int>,
                   robin_hood::unordered_node_map<std::string, int>) {
    Map m1{{"duck", 17}, {"lion", 10}};
    Map m2 = {{"duck", 17}, {"lion", 10}};

    REQUIRE(m1.size() == 2);
    REQUIRE(m1["duck"] == 17);
    REQUIRE(m1["lion"] == 10);

    REQUIRE(m2.size() == 2);
    auto it = m2.find("duck");
    REQUIRE((it != m2.end() && it->second == 17));
    REQUIRE(m2["lion"] == 10);
}
