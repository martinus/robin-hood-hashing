#include <robin_hood.h>

#include <app/doctest.h>

TYPE_TO_STRING(robin_hood::unordered_flat_map<uint64_t, uint64_t>);
TYPE_TO_STRING(robin_hood::unordered_node_map<uint64_t, uint64_t>);

TEST_CASE_TEMPLATE("iterators default constructed", Map,
                   robin_hood::unordered_flat_map<uint64_t, size_t>,
                   robin_hood::unordered_node_map<uint64_t, size_t>) {

    Map map;
    map[1] = 1U;

    auto it = map.begin();
    auto first = it++;
    REQUIRE(first == map.begin());
    REQUIRE(it == map.end());
}
