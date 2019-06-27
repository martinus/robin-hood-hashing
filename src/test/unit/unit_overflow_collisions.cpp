#include <robin_hood.h>

#include <app/CtorDtorVerifier.h>
#include <app/doctest.h>
#include <app/hash/Bad.h>

TEST_CASE_TEMPLATE(
    "collisions", Map,
    robin_hood::unordered_flat_map<CtorDtorVerifier, CtorDtorVerifier, hash::Bad<CtorDtorVerifier>>,
    robin_hood::unordered_node_map<CtorDtorVerifier, CtorDtorVerifier,
                                   hash::Bad<CtorDtorVerifier>>) {

    static const uint64_t max_val = 127;

    // CtorDtorVerifier::mDoPrintDebugInfo = true;
    {
        Map m;
        for (uint64_t i = 0; i < max_val; ++i) {
            INFO(i);
            m[i];
        }
        REQUIRE(m.size() == max_val);
        REQUIRE_THROWS_AS(m[max_val], std::overflow_error);
        REQUIRE(m.size() == max_val);
    }
    if (0 != CtorDtorVerifier::mapSize()) {
        CtorDtorVerifier::printMap();
    }
    REQUIRE(CtorDtorVerifier::mapSize() == 0);

    {
        Map m;
        for (uint64_t i = 0; i < max_val; ++i) {
            REQUIRE(m.insert(typename Map::value_type(i, i)).second);
        }
        REQUIRE(m.size() == max_val);
        REQUIRE_THROWS_AS(m.insert(typename Map::value_type(max_val, max_val)),
                          std::overflow_error);
        REQUIRE(m.size() == max_val);
    }
    if (0 != CtorDtorVerifier::mapSize()) {
        CtorDtorVerifier::printMap();
    }
    REQUIRE(CtorDtorVerifier::mapSize() == 0);
}
