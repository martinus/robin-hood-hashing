#include <app/checksum.h>
#include <robin_hood.h>
#include <sfc64.h>

#include <doctest.h>

#include <unordered_map>

TYPE_TO_STRING(robin_hood::unordered_flat_map<uint64_t, uint64_t>);
TYPE_TO_STRING(robin_hood::unordered_node_map<uint64_t, uint64_t>);

// this test is first run with std::unordered_map for reference
TEST_CASE_TEMPLATE("iterators stochastic" * doctest::test_suite("stochastic"), Map,
                   robin_hood::unordered_flat_map<uint64_t, uint64_t>,
                   robin_hood::unordered_node_map<uint64_t, uint64_t>) {

    size_t totals = 1000;
    uint64_t seed = 9999;
    INFO("seed=" << seed);
    sfc64 rng(seed);

    uint64_t cks = 0;

    Map map;
    for (size_t n = 0; n < totals; ++n) {
        // insert a single element
        auto k = rng();
        auto v = rng();
        map[k] = v;

        // then iterate everything
        cks = checksum::combine(cks, checksum::map(map));
    }

    // now remove element after element until the map is empty
    rng.seed(seed);

    for (size_t n = 0; n < totals; ++n) {
        // insert a single element
        auto k = rng();
        rng(); // discard value
        map.erase(k);

        // then iterate everything
        cks = checksum::combine(cks, checksum::map(map));
    }
    REQUIRE(map.empty());
    REQUIRE(cks == UINT64_C(11980746028390242716));
}
