#include <robin_hood.h>

#include <app/doctest.h>

#include <vector>

TYPE_TO_STRING(robin_hood::unordered_flat_map<int, int>);
TYPE_TO_STRING(robin_hood::unordered_node_map<int, int>);

TEST_CASE_TEMPLATE("insert list", Map, robin_hood::unordered_flat_map<int, int>,
                   robin_hood::unordered_node_map<int, int>) {
    std::vector<typename Map::value_type> v;
    v.reserve(1000);
    for (int i = 0; i < 1000; ++i) {
        v.emplace_back(i, i);
    }

    Map map(v.begin(), v.end());
    REQUIRE(map.size() == v.size());
    for (const auto& kv : v) {
        REQUIRE(map.count(kv.first) == 1);
        auto it = map.find(kv.first);
        REQUIRE(it != map.end());
    }
}
