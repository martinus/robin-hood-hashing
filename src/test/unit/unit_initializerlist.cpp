#include <robin_hood.h>

#include <app/doctest.h>

#include <string>

TYPE_TO_STRING(robin_hood::unordered_flat_map<std::string, size_t>);
TYPE_TO_STRING(robin_hood::unordered_node_map<std::string, size_t>);

TEST_CASE_TEMPLATE("initializerlist std::string", Map,
                   robin_hood::unordered_flat_map<std::string, size_t>,
                   robin_hood::unordered_node_map<std::string, size_t>) {
    size_t const n1 = 17;
    size_t const n2 = 10;
    Map m1{{"duck", n1}, {"lion", n2}};
    Map m2 = {{"duck", n1}, {"lion", n2}};

    REQUIRE(m1.size() == 2);
    REQUIRE(m1["duck"] == n1);
    REQUIRE(m1["lion"] == n2);

    REQUIRE(m2.size() == 2);
    auto it = m2.find("duck");
    REQUIRE((it != m2.end() && it->second == n1));
    REQUIRE(m2["lion"] == n2);
}
