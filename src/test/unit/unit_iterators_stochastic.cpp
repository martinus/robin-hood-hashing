#include <app/checksum.h>
#include <robin_hood.h>
#include <sfc64.h>

#include <doctest.h>

#include <random>
#include <unordered_map>

TYPE_TO_STRING(robin_hood::unordered_flat_map<uint64_t, uint64_t>);
TYPE_TO_STRING(robin_hood::unordered_node_map<uint64_t, uint64_t>);

// this test is first run with std::unordered_map for reference
TEST_CASE_TEMPLATE("iterators stochastic" * doctest::test_suite("stochastic"), Map,
                   robin_hood::unordered_flat_map<uint64_t, uint64_t>,
                   robin_hood::unordered_node_map<uint64_t, uint64_t>) {

    size_t totals = 1000;

    std::random_device rd;
    auto seed = std::uniform_int_distribution<uint64_t>{}(rd);
    INFO("seed=" << seed);
    sfc64 rng(seed);

    Map rh;
    std::unordered_map<uint64_t, uint64_t> uo;
    for (size_t n = 0; n < totals; ++n) {
        // insert a single element
        auto k = rng();
        auto v = rng();
        rh[k] = v;
        uo[k] = v;

        REQUIRE(rh.size() == uo.size());
        REQUIRE(checksum::map(rh) == checksum::map(uo));
    }

    // now remove element after element until the map is empty
    rng.seed(seed);

    for (size_t n = 0; n < totals; ++n) {
        // insert a single element
        auto k = rng();
        rng(); // discard value
        rh.erase(k);
        uo.erase(k);

        // then iterate everything
        REQUIRE(rh.size() == uo.size());
        REQUIRE(checksum::map(rh) == checksum::map(uo));
    }
}
