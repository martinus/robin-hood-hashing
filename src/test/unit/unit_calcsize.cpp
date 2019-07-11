#include <robin_hood.h>

#include <app/doctest.h>

TEST_CASE("calc_size") {
    robin_hood::unordered_flat_map<uint64_t, uint64_t> fm;
    // auto const ps = static_cast<size_t>(sizeof(uint64_t) * 2);
    REQUIRE(16 == sizeof(robin_hood::pair<uint64_t, uint64_t>));

    REQUIRE(9 == fm.calcNumBytesInfo(1));
    REQUIRE(8 + (1 + 16) * 1 == fm.calcNumBytesTotal(1));
    REQUIRE(8 + (1 + 16) * 2 == fm.calcNumBytesTotal(2));
    REQUIRE(8 + (1 + 16) * 4 == fm.calcNumBytesTotal(4));

    // should be _just_ below 2^32
    REQUIRE(8 + (1 + 16) * UINT64_C(252645134) == fm.calcNumBytesTotal(252645134));

    // just above 2^32: throws on 32bit, but not on 64bit.
#if ROBIN_HOOD(BITNESS) == 32
    REQUIRE_THROWS_AS((void)fm.calcNumBytesTotal(252645135), std::overflow_error);
#else
    REQUIRE(8 + (1 + 16) * UINT64_C(252645135) == fm.calcNumBytesTotal(252645135));
    REQUIRE(8 + (1 + 16) * UINT64_C(1085102592571150094) ==
            fm.calcNumBytesTotal(UINT64_C(1085102592571150094)));
#endif
}
