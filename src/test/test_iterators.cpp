#include "test_base.h"

int clz(void const* data) {
    auto const n = robin_hood::detail::unaligned_load<size_t>(data);
    return ROBIN_HOOD_COUNT_TRAILING_ZEROES(n) / 8;
}

TEST_CASE("clz") {
    int constexpr const num_bytes = sizeof(size_t);
    std::array<uint8_t, num_bytes * 2 + 1> ary{};
    for (size_t data = 0; data < num_bytes; ++data) {
        uint8_t* d = ary.data() + data;
        for (int i = 0; i < num_bytes; ++i) {
            // different values in this byte
            d[i] = UINT8_C(1);
            REQUIRE(i == clz(d));
            d[i] = UINT8_C(0xff);
            REQUIRE(i == clz(d));
            d[i] = UINT8_C(0x80);
            REQUIRE(i == clz(d));

            // different values in the *next* byte
            d[i + 1] = UINT8_C(1);
            REQUIRE(i == clz(d));
            d[i + 1] = UINT8_C(0xff);
            REQUIRE(i == clz(d));
            d[i + 1] = UINT8_C(0x80);
            REQUIRE(i == clz(d));

            // clear all
            d[i] = UINT8_C(0);
            d[i + 1] = UINT8_C(0);
            REQUIRE(num_bytes == clz(d));
        }
    }
}

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

TEMPLATE_TEST_CASE("default constructed iterators", "",
                   (robin_hood::unordered_flat_map<uint64_t, size_t>),
                   (robin_hood::unordered_node_map<uint64_t, size_t>)) {
    TestType map;
    using It = typename TestType::iterator;
    using CIt = typename TestType::const_iterator;
    REQUIRE(It{} == It{});
    REQUIRE(CIt{} == CIt{});
    REQUIRE(It{} == CIt{});
    REQUIRE(CIt{} == CIt{});
}