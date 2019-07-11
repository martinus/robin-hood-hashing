#include <robin_hood.h>

#include <app/doctest.h>

TEST_CASE("default map load") {
    robin_hood::unordered_flat_map<uint64_t, uint64_t> defaultMap;
    REQUIRE(80 == defaultMap.calcMaxNumElementsAllowed(100));
}

template <size_t LF>
using MapLF = robin_hood::unordered_flat_map<uint64_t, uint64_t, std::hash<uint64_t>,
                                             std::equal_to<uint64_t>, LF>;
TEST_CASE("calcMaxNumElementsAllowed") {
    REQUIRE(75 == MapLF<75>{}.calcMaxNumElementsAllowed(100));

    // 256 * 0.8 * 204.8
    REQUIRE(204 == MapLF<80>{}.calcMaxNumElementsAllowed(256));

#if ROBIN_HOOD(BITNESS) == 64
    // exact value would be 4611686018427387904 * 0.8 = 3689348814741910323.2
    // this is quite close enough
    REQUIRE(3689348814741910320 == MapLF<80>{}.calcMaxNumElementsAllowed(4611686018427387904));

    // maximum possible value (2^64-1)*0,8 = 0xCCCCCCCCCCCCCCCC
    REQUIRE(UINT64_C(0xCCCCCCCCCCCCCCC0) ==
            MapLF<80>{}.calcMaxNumElementsAllowed(UINT64_C(0xFFFFFFFFFFFFFFFF)));
#else
    // exact value would be 2147483647 * 0.8 = 1717986917.6
    // this is quite close enough
    REQUIRE(1717986880 == MapLF<80>{}.calcMaxNumElementsAllowed(2147483647));

    // maximum possible value (2^64-1)*0,8 = 14757395258967641292
    REQUIRE(0xCCCCCC80 == MapLF<80>{}.calcMaxNumElementsAllowed(0xFFFFFFFF));
#endif
}
