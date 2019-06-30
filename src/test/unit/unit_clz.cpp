#include <robin_hood.h>

#include <app/doctest.h>

#include <array>

namespace {

int clz(void const* data) {
    auto const n = robin_hood::detail::unaligned_load<size_t>(data);
    return ROBIN_HOOD_COUNT_TRAILING_ZEROES(n) / 8;
}

} // namespace

TEST_CASE("clz") {
    int constexpr const num_bytes = sizeof(size_t);
    auto ary = std::array<uint8_t, num_bytes * 2 + 1>();
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
