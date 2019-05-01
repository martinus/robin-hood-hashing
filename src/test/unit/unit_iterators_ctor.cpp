#include <robin_hood.h>

#include <app/doctest.h>

TYPE_TO_STRING(robin_hood::unordered_flat_map<uint64_t, uint64_t>);
TYPE_TO_STRING(robin_hood::unordered_node_map<uint64_t, uint64_t>);

TEST_CASE_TEMPLATE("iterators default constructed", Map,
                   robin_hood::unordered_flat_map<uint64_t, size_t>,
                   robin_hood::unordered_node_map<uint64_t, size_t>) {
    using It = typename Map::iterator;
    using CIt = typename Map::const_iterator;
    REQUIRE(It{} == It{});
    REQUIRE(CIt{} == CIt{});
    REQUIRE(It{} == CIt{});
    REQUIRE(CIt{} == CIt{});
}
