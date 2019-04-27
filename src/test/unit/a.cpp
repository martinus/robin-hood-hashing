#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>
#include <iostream>
TEST_CASE("a") {
    for (size_t i = 0; i < 1234; ++i) {
        REQUIRE(i >= 0);
    }
    std::cout << std::hex << std::setfill('0');
}
