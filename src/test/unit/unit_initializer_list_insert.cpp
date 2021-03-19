#include <robin_hood.h>

#include <app/doctest.h>

TYPE_TO_STRING(robin_hood::unordered_flat_map<int, int>);
TYPE_TO_STRING(robin_hood::unordered_node_map<int, int>);

TEST_CASE_TEMPLATE("insert_initializer_list", Map, robin_hood::unordered_flat_map<int, int>,
                   robin_hood::unordered_node_map<int, int>) {
    auto m = Map();
    m.insert({{1, 2}, {3, 4}, {5, 6}});
    REQUIRE(m.size() == 3U);
    REQUIRE(m[1] == 2);
    REQUIRE(m[3] == 4);
    REQUIRE(m[5] == 6);
}

TEST_CASE_TEMPLATE("insert_initializer_list_string", Map,
                   robin_hood::unordered_flat_map<int, std::string>,
                   robin_hood::unordered_node_map<int, std::string>) {
    auto m = Map();
    m.insert({{1, "a"}, {3, "b"}, {5, "c"}});
    REQUIRE(m.size() == 3U);
    REQUIRE(m[1] == "a");
    REQUIRE(m[3] == "b");
    REQUIRE(m[5] == "c");
}
