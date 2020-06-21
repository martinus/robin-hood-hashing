#include <robin_hood.h>

#include <app/CtorDtorVerifier.h>
#include <app/doctest.h>
#include <app/fmt/hex.h>
#include <app/hash/Bad.h>
#include <app/sfc64.h>

#include <iostream>
#include <unordered_set>

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

#if ROBIN_HOOD(BITNESS) == 64

namespace {

bool isColliding(uint64_t x) {
    return UINT64_C(0) == (x & UINT64_C(0x1fffff));
}

} // namespace

TEST_CASE("overflow_finder" * doctest::skip()) {
    robin_hood::unordered_set<uint64_t> set;

    // loop until we get an overflow
    // auto mix = UINT64_C(0x73e6d3e6f41f53a9);
    for (uint64_t i = 0; i < UINT64_C(1000000000); ++i) {
        auto s = i;
        auto h1 = robin_hood::hash<uint64_t>{}(s);
        auto h2 = h1;
        h1 >>= 32U;
        // robin_hood::hash<uint64_t>{}(s * mix);

        if (isColliding(h1)) {
            std::cout << i << ": " << fmt::hex(s) << " -> " << fmt::hex(h1) << " " << fmt::hex(h2)
                      << " " << isColliding(h2) << std::endl;
            set.insert(s);
        }
    }
}

TEST_CASE("overflow_finder_simple" * doctest::skip()) {
    robin_hood::unordered_set<uint64_t> set;
    size_t count = 0;
    uint64_t i = 0;
    try {
        while (count < 255U) {
            auto h = robin_hood::hash<uint64_t>{}(i);
            if (UINT64_C(0) == (h & UINT64_C(0x1fffff))) {
                ++count;
                set.insert(i);
            }
            ++i;
        }
    } catch (std::overflow_error&) {
        FAIL("i=" << i << ", count=" << count);
    }
}

#endif
