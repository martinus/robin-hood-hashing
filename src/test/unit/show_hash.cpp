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

namespace {

void showHash(size_t val) {
    auto id = hash::Identity<size_t>{}(val);
    auto sh = std::hash<size_t>{}(val);
    auto rh = robin_hood::hash<size_t>{}(val);
    auto fn = hash::FNV1a<size_t>{}(val);
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
        size_t x = ((0x23d7 + i) << (ROBIN_HOOD_BITNESS / 2)) + static_cast<size_t>(63);
        showHash(x);
    }

    for (size_t i = 1; i < 8; ++i) {
        showHash(i * (static_cast<size_t>(1) << (ROBIN_HOOD_BITNESS - 4)));
    }

    for (size_t i = 1; i != 0; i *= 2) {
        showHash(i);
        showHash(i + 1);
    }
}
