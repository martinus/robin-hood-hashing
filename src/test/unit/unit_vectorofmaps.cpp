#include <robin_hood.h>

#include <app/CtorDtorVerifier.h>
#include <app/doctest.h>
#include <app/sfc64.h>

#include <ostream>
#include <vector>

TYPE_TO_STRING(robin_hood::unordered_flat_map<CtorDtorVerifier, CtorDtorVerifier>);
TYPE_TO_STRING(robin_hood::unordered_node_map<CtorDtorVerifier, CtorDtorVerifier>);

template <typename Map>
void fill(Map* map, sfc64* rng) {
    auto n = rng->uniform<size_t>(20);
    for (size_t i = 0; i < n; ++i) {
        auto a = rng->uniform<uint64_t>(20);
        auto b = rng->uniform<uint64_t>(20);
        (*map)[a] = b;
    }
}

TEST_CASE_TEMPLATE("vectormap", Map,
                   robin_hood::unordered_node_map<CtorDtorVerifier, CtorDtorVerifier>,
                   robin_hood::unordered_flat_map<CtorDtorVerifier, CtorDtorVerifier>) {
    sfc64 rng(32154);
    {
        std::vector<Map> maps;

        // copies
        for (size_t i = 0; i < 10; ++i) {
            Map m;
            fill(&m, &rng);
            maps.push_back(m);
        }

        // move
        for (size_t i = 0; i < 10; ++i) {
            Map m;
            fill(&m, &rng);
            maps.push_back(std::move(m));
        }

        // emplace
        for (size_t i = 0; i < 10; ++i) {
            maps.emplace_back();
            fill(&maps.back(), &rng);
        }
    }
    REQUIRE(CtorDtorVerifier::mapSize() == static_cast<size_t>(0));
}
