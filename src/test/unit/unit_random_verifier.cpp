#include <robin_hood.h>

#include <app/CtorDtorVerifier.h>
#include <app/checksum.h>
#include <app/doctest.h>
#include <app/sfc64.h>

// verified with
// #include <unordered_map>
// std::unordered_map<CtorDtorVerifier, CtorDtorVerifier, robin_hood::hash<CtorDtorVerifier>>

TYPE_TO_STRING(robin_hood::unordered_flat_map<CtorDtorVerifier, CtorDtorVerifier>);
TYPE_TO_STRING(robin_hood::unordered_node_map<CtorDtorVerifier, CtorDtorVerifier>);

TEST_CASE_TEMPLATE("random_insert_erase_with_Verifier", Map,
                   robin_hood::unordered_node_map<CtorDtorVerifier, CtorDtorVerifier>,
                   robin_hood::unordered_flat_map<CtorDtorVerifier, CtorDtorVerifier>) {
    sfc64 rng(1780);

    REQUIRE(CtorDtorVerifier::mapSize() == static_cast<size_t>(0));
    Map map;
    for (size_t i = 1; i < 10000; ++i) {
        auto v = rng(i);
        auto k = rng(i);
        map[k] = v;
        map.erase(rng(i));
    }

    REQUIRE(CtorDtorVerifier::mapSize() == 6606);
    REQUIRE(checksum::map(map) == UINT64_C(0x22E9B522B3B36762));
    map.clear();

    REQUIRE(CtorDtorVerifier::mapSize() == 0);
    REQUIRE(checksum::map(map) == UINT64_C(0x9E3779F8));
}
