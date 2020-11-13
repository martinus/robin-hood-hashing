#include <robin_hood.h>

#include <app/doctest.h>

#include <vector>

TEST_CASE("memleak_reserve_flat") {
    robin_hood::unordered_flat_map<int, int> m;
    for (size_t i = 0; i < 10; ++i) {
        m.reserve(1000);
    }
}

TEST_CASE("memleak_reserve_node") {
    robin_hood::unordered_node_map<int, int> m;
    for (size_t i = 0; i < 10; ++i) {
        m.reserve(1000);
    }
}
