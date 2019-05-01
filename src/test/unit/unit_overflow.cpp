#include <robin_hood.h>

#include <app/checksum.h>
#include <app/doctest.h>
#include <app/sfc64.h>

#include <unordered_map>

struct BadHash {
    template <typename T>
    size_t operator()(T const&) const {
        return 0;
    }
};

TYPE_TO_STRING(robin_hood::unordered_flat_map<uint64_t, uint64_t, BadHash>);
TYPE_TO_STRING(robin_hood::unordered_node_map<uint64_t, uint64_t, BadHash>);

TEST_CASE_TEMPLATE("bug overflow" * doctest::test_suite("stochastic"), Map,
                   robin_hood::unordered_flat_map<uint64_t, uint64_t, BadHash>,
                   robin_hood::unordered_node_map<uint64_t, uint64_t, BadHash>) {
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
