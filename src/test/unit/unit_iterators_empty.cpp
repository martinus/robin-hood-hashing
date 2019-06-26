#include <robin_hood.h>

#include <app/doctest.h>

TYPE_TO_STRING(robin_hood::unordered_flat_map<uint64_t, uint64_t>);
TYPE_TO_STRING(robin_hood::unordered_node_map<uint64_t, uint64_t>);

TEST_CASE_TEMPLATE("iterators empty", Map, robin_hood::unordered_flat_map<uint64_t, size_t>,
                   robin_hood::unordered_node_map<uint64_t, size_t>) {
    for (size_t i = 0; i < 10; ++i) {
        Map m;
        REQUIRE(m.begin() == m.end());

        REQUIRE(m.end() == m.find(132));

        m[1];
        REQUIRE(m.begin() != m.end());
        REQUIRE(++m.begin() == m.end());
        m.clear();

        REQUIRE(m.begin() == m.end());
    }
}
