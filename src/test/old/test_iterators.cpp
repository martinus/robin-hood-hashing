#include "test_base.h"

TEMPLATE_TEST_CASE("iterators", "", FlatMap, NodeMap) {
    for (size_t i = 0; i < 10; ++i) {
        TestType m;
        REQUIRE(m.begin() == m.end());

        REQUIRE(m.end() == m.find(132));

        m[1];
        REQUIRE(m.begin() != m.end());
        REQUIRE(++m.begin() == m.end());
        m.clear();

        REQUIRE(m.begin() == m.end());
    }
}

TEMPLATE_TEST_CASE("iterators brute force", "", (robin_hood::unordered_flat_map<uint64_t, size_t>),
                   (robin_hood::unordered_node_map<uint64_t, size_t>)) {
    size_t totals = 5000;
    uint64_t rng_seed = UINT64_C(123);
    Rng rng{rng_seed};
    uint64_t checksum = 0;

    TestType map;
    for (size_t n = 0; n < totals; ++n) {
        // insert a single element
        map[rng()];

        // then iterate everything
        uint64_t iter_sum = 0;
        for (auto const& keyVal : map) {
            iter_sum += keyVal.first;
        }
        checksum += iter_sum * n;
    }

    // now remove element after element until the map is empty
    rng.seed(rng_seed);

    for (size_t n = 0; n < totals; ++n) {
        map.erase(rng());

        // then iterate everything
        uint64_t iter_sum = 0;
        for (auto const& keyVal : map) {
            iter_sum += keyVal.first;
        }
        checksum += iter_sum * n;
    }
    REQUIRE(map.empty());
    REQUIRE(checksum == UINT64_C(0x76c1c392ca6abb88));
}
