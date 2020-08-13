#include <robin_hood.h>

#include <app/fmt/bin.h>
#include <app/fmt/hex.h>
#include <app/fmt/streamstate.h>
#include <app/hash/FNV1a.h>
#include <app/hash/Identity.h>

#include <app/doctest.h>

#include <bitset>
#include <iomanip>
#include <iostream>
#include <vector>

namespace {

template <typename T>
void showHash(T val) {
    auto id = hash::Identity<T>{}(val);
    auto sh = std::hash<T>{}(val);
    auto rh = robin_hood::hash<T>{}(val);
    auto fn = hash::FNV1a<T>{}(val);
    std::cout << fmt::hex(val) << " | " << fmt::hex(id) << " | " << fmt::hex(sh) << " | "
              << fmt::hex(fn) << " | " << fmt::hex(rh) << " | " << fmt::bin(rh) << std::endl;
}

} // namespace

TEST_CASE("show hash distribution" * doctest::test_suite("show") * doctest::skip()) {
    fmt::streamstate ss(std::cout);
    int s = sizeof(size_t) * 2;
    std::cout << std::left << std::setw(s) << "input"
              << " | " << std::setw(s) << "Identity"
              << " | " << std::setw(s) << "std::hash"
              << " | " << std::setw(s) << "FNV1a"
              << " | " << std::setw(s) << "robin_hood::hash"
              << " | " << std::setw(sizeof(size_t) * 8) << "robin_hood::hash binary" << std::endl;
    for (size_t i = 0; i < 100; ++i) {
        showHash(i);
    }

    for (size_t i = 0; i < 10; ++i) {
        size_t x = ((0x23d7U + i) << (ROBIN_HOOD(BITNESS) / 2U)) + static_cast<size_t>(63);
        showHash(x);
    }

    for (size_t i = 1; i < 8; ++i) {
        showHash(i * (static_cast<size_t>(1) << (ROBIN_HOOD(BITNESS) - 4U)));
    }

    for (size_t i = 1; i != 0; i *= 2) {
        showHash(i);
        showHash(i + 1);
    }

    std::vector<size_t> data(10, 0);
    for (auto const& x : data) {
        showHash(&x);
    }
}

TEST_CASE("cout_max_bucket_size" * doctest::test_suite("show") * doctest::skip()) {
    robin_hood::unordered_flat_set<uint64_t> set;

#if ROBIN_HOOD(BITNESS) == 64
    uint64_t size = 1000U;
    for (uint64_t i = 0U; i < size; ++i) {
        for (uint64_t j = 0U; j < size; ++j) {
            auto val = (i << 32U) | j;
            for (size_t mix = 0; mix < 10; ++mix) {
                val = robin_hood::hash<uint64_t>{}(val);
            }
            set.insert(val);
        }
    }
    REQUIRE(set.size() == size * size);
#endif
}
