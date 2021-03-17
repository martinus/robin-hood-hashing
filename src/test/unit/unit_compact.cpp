#include <robin_hood.h>

#include <app/doctest.h>
TYPE_TO_STRING(robin_hood::unordered_flat_map<uint64_t, uint64_t>);
TYPE_TO_STRING(robin_hood::unordered_node_map<uint64_t, uint64_t>);

#if !ROBIN_HOOD(BROKEN_CONSTEXPR)
TEST_CASE_TEMPLATE("compact", Map, robin_hood::unordered_flat_map<uint64_t, uint64_t>,
                   robin_hood::unordered_node_map<uint64_t, uint64_t>) {
    Map map;

    REQUIRE(0 == map.mask());
    map.compact();
    REQUIRE(0 == map.mask());
    map.reserve(819);
    REQUIRE(1023 == map.mask());

    map.compact();

    // won't go back to zero
    REQUIRE(7U == map.mask());
    map[1U] = 2U;
    REQUIRE(7U == map.mask());

    map.compact();
    REQUIRE(7U == map.mask());

    map.clear();
    for (uint64_t i = 0; i < 100; ++i) {
        map[i] = i;
    }

    REQUIRE(127 == map.mask());
    for (uint64_t i = 0; i < 100; i += 2) {
        map.erase(i);
    }
    REQUIRE(127 == map.mask());
    map.compact();
    REQUIRE(63 == map.mask());
}
#endif
