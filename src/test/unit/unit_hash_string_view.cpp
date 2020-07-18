#include <robin_hood.h>

#if ROBIN_HOOD(CXX) >= ROBIN_HOOD(CXX17)

#    include <iostream>
#    include <string>
#    include <string_view>

#    include <app/doctest.h>

TEST_CASE("unit_hash_string_view") {
    auto cstr = "The ships hung in the sky in much the same way that bricks don't.";
    REQUIRE(robin_hood::hash<std::string>{}(std::string{cstr}) ==
            robin_hood::hash<std::string_view>{}(std::string_view{cstr}));
}

#endif
