#include "hash_map.h"
#include "test_base.h"

#include <unordered_map>

template <typename T>
struct BadHash {
    size_t operator()(T const&) const {
        return 0;
    }
};

TEST_CASE("bug overflow") {
    robin_hood::unordered_flat_map<uint64_t, uint64_t, BadHash<uint64_t>> rh;
    std::unordered_map<uint64_t, uint64_t> uo;

    Rng rng(123);

    for (int i = 0; i < 100000; ++i) {
        try {
            auto key = rng();
            auto val = rng();
            rh.emplace(key, val);
            uo.emplace(key, val);
        } catch (std::overflow_error const&) {
        }

        INFO(i);

        // make sure both maps are the same
        REQUIRE(rh.size() == uo.size());
        REQUIRE(hash(rh) == hash(uo));
    }
}