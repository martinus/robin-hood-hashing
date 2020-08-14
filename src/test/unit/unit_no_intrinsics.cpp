#define ROBIN_HOOD_DISABLE_INTRINSICS

#include <robin_hood.h>

#include <app/doctest.h>

#include <string>

TEST_CASE("disable_intrinsics") {
    robin_hood::unordered_flat_map<uint64_t, std::string> map;
    map[1] = "bug?";

    REQUIRE(map[1] == "bug?");
}
