#include <robin_hood.h>

#if ROBIN_HOOD(CXX) >= ROBIN_HOOD(CXX17)

#    include <string>
#    include <string_view>

#    include <app/doctest.h>

TEST_CASE("unit_hash_string_view") {
    auto cstr = "The ships hung in the sky in much the same way that bricks don't.";
    REQUIRE(robin_hood::hash<std::string>{}(std::string{cstr}) ==
            robin_hood::hash<std::string_view>{}(std::string_view{cstr}));
}

TEST_CASE("unit_hash_u32string") {
    std::u32string str{};
    str.push_back(1);
    str.push_back(2);
    str.push_back(3);
    str.push_back(4);
    str.push_back(5);

    REQUIRE(robin_hood::hash<std::u32string>{}(str) ==
            robin_hood::hash<std::u32string_view>{}(str));
}

#endif
