#include <robin_hood.h>

#include <app/doctest.h>

TEST_CASE("unit_hash_char_types") {
    REQUIRE(123 != robin_hood::hash<wchar_t>{}(123));
}
