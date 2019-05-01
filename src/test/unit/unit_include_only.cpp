#include <unit/include_only.h>

#include <app/doctest.h>

TEST_CASE("include only") {
    REQUIRE(include_only() == 878);
}
