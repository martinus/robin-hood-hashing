#include <robin_hood.h>

#include <app/doctest.h>

#include <vector>

TYPE_TO_STRING(robin_hood::unordered_flat_map<int, int>);
TYPE_TO_STRING(robin_hood::unordered_node_map<int, int>);

TEST_CASE_TEMPLATE("load_factor", Map, robin_hood::unordered_flat_map<int, int>,
                   robin_hood::unordered_node_map<int, int>) {

    Map m;
    REQUIRE(static_cast<double>(m.load_factor()) == doctest::Approx(0.0));
    for (int i = 0; i < 10000; ++i) {
        m.emplace(i, i);
        REQUIRE(m.load_factor() > 0.0F);
        REQUIRE(m.load_factor() <= 0.8F);
    }
}
