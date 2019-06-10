#include <app/checksum.h>
#include <app/doctest.h>
#include <app/sfc64.h>

// make sure RNG is deterministic
TEST_CASE("rng_is_deterministic") {
    sfc64 rng(123);

    uint64_t h = 0;
    for (size_t i = 1; i < 10000; ++i) {
        h = checksum::combine(h, rng(i));
        h = checksum::combine(h, rng(i));
        h = checksum::combine(h, rng(i));
    }
    REQUIRE(h == UINT64_C(6668772197474419138));
    REQUIRE(rng() == UINT64_C(0x4001279087d5fe55));
}
