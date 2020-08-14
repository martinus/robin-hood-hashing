#include <robin_hood.h>

#include <app/doctest.h>

TEST_CASE("rotr") {
    auto x = uint64_t{2};
    REQUIRE(robin_hood::detail::rotr(x, 1U) == UINT64_C(1));
    REQUIRE(robin_hood::detail::rotr(x, 2U) == UINT64_C(0x8000000000000000));
    REQUIRE(robin_hood::detail::rotr(x, 63U) == UINT64_C(4));

    auto x32 = UINT32_C(0x46374f6e);
    REQUIRE(robin_hood::detail::rotr(x32, 17U) == UINT32_C(0xA7B7231B));
}
