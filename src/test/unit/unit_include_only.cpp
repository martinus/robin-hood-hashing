#include <include_only.h>

#include <doctest.h>

TEST_CASE("include only") {
    REQUIRE(include_only() == 878);
}
