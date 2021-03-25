#include <robin_hood.h>

#include <app/checksum.h>
#include <app/doctest.h>
#include <app/hash/Bad.h>
#include <app/sfc64.h>

#include <unordered_map>

TYPE_TO_STRING(robin_hood::unordered_flat_map<uint64_t, uint64_t, hash::Bad<uint64_t>>);
TYPE_TO_STRING(robin_hood::unordered_node_map<uint64_t, uint64_t, hash::Bad<uint64_t>>);

// This test doesn't work as it was intended with the hash overflow protection
#if 0

#    if ROBIN_HOOD(HAS_EXCEPTIONS)

TEST_CASE_TEMPLATE("bug_overflow" * doctest::test_suite("stochastic"), Map,
                   robin_hood::unordered_flat_map<uint64_t, uint64_t, hash::Bad<uint64_t>>,
                   robin_hood::unordered_node_map<uint64_t, uint64_t, hash::Bad<uint64_t>>) {
    Map rh;
    std::unordered_map<uint64_t, uint64_t> uo;

    sfc64 rng;
    auto const bitmask = UINT64_C(0xfff);

    size_t num_throws = 0;
    for (int i = 0; i < 1000; ++i) {
        try {
            auto key = rng() & bitmask;
            auto val = rng() & bitmask;
            rh.emplace(key, val);
            uo.emplace(key, val);

            key = rng() & bitmask;
            rh.erase(key);
            uo.erase(key);
        } catch (std::overflow_error const&) {
            ++num_throws;
        }
        REQUIRE(rh.size() == uo.size());
    }

    // make sure both maps are the same
    REQUIRE(num_throws > 0);
    REQUIRE(checksum::map(rh) == checksum::map(uo));
}

#    endif

#endif
