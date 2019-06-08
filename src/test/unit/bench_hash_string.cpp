#include <robin_hood.h>

#include <app/benchmark.h>
#include <app/doctest.h>
#include <app/sfc64.h>

TEST_CASE("bench robin_hood::hash<std::string>" * doctest::test_suite("bench") * doctest::skip()) {
    size_t h = 0;
    sfc64 rng(123);
    robin_hood::hash<std::string> hasher;

    size_t len = 0;
    size_t iterations = 0;

    SUBCASE("7") {
        len = 7;
        iterations = 250058138U;
    }
    SUBCASE("8") {
        len = 8;
        iterations = 310857313U;
    }
    SUBCASE("13") {
        len = 13;
        iterations = 225620314U;
    }
    SUBCASE("100") {
        len = 100;
        iterations = 68319310U;
    }
    SUBCASE("1000") {
        len = 1000;
        iterations = 6458275U;
    }

    std::string str(len, 'x');
    for (auto& ch : str) {
        ch = rng.uniform<char>();
    }
    BENCHMARK("std::string length " + std::to_string(len), len * iterations, "B") {
        for (size_t i = 0; i < iterations; ++i) {
            // modify string to prevent optimization
            ++*robin_hood::detail::reinterpret_cast_no_cast_align_warning<uint64_t*>(&str[0]);
            h += hasher(str);
        }
    }

    // prevent optimization
    INFO(h);
    REQUIRE(h != 0);
}
