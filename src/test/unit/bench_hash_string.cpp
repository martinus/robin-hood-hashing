#include <robin_hood.h>

#include <app/benchmark.h>
#include <app/doctest.h>
#include <app/sfc64.h>
#include <thirdparty/nanobench/nanobench.h>

TEST_CASE("bench_hash_string" * doctest::test_suite("nanobench") * doctest::skip()) {
    size_t h = 0;
    sfc64 rng(123);
    robin_hood::hash<std::string> hasher;

    size_t len = 0;

    SUBCASE("7") {
        len = 7;
    }
    SUBCASE("8") {
        len = 8;
    }
    SUBCASE("13") {
        len = 13;
    }
    SUBCASE("100") {
        len = 100;
    }
    SUBCASE("1000") {
        len = 1000;
    }
    SUBCASE("10000") {
        len = 10000;
    }

    std::string str(len, 'x');
    for (auto& ch : str) {
        ch = rng.uniform<char>();
    }

    ankerl::nanobench::Bench bench;
    bench.unit("B")
        .batch(str.size())
        .run("std::string " + std::to_string(len),
             [&] {
                 // modify string to prevent optimization
                 ++*robin_hood::detail::reinterpret_cast_no_cast_align_warning<uint64_t*>(&str[0]);
                 h += hasher(str);
             })
        .doNotOptimizeAway(h);
}
