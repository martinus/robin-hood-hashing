#include <robin_hood.h>

#include <app/benchmark.h>
#include <app/doctest.h>
#include <app/sfc64.h>

TYPE_TO_STRING(robin_hood::unordered_flat_map<int, int>);
TYPE_TO_STRING(robin_hood::unordered_node_map<int, int>);

template <typename T>
uint64_t ror(T v, int r) {
    return (v >> r) | (v << (static_cast<int>(sizeof(T)) * 8 - r));
}

// benchmark adapted from https://github.com/attractivechaos/udb2
// this implementation should have less overhead, because sfc64 and it's uniform() is extremely
// fast.
TEST_CASE_TEMPLATE("bench_distinctness" * doctest::test_suite("bench") * doctest::skip(), Map,
                   robin_hood::unordered_flat_map<uint64_t, uint64_t>,
                   robin_hood::unordered_node_map<uint64_t, uint64_t>) {
    using mt = typename Map::mapped_type;

    static mt const upper = 50000000;
    static mt const lower = 10000000;
    static mt const num_steps = 5;
    static mt const step_width = (upper - lower) / num_steps;

    mt divisor = 0;
    std::string title;
    mt required_checksum = 0;

    SUBCASE("5%") {
        divisor = 20;
        title = "  5% distinct";
        required_checksum = 1799978125;
    }
    SUBCASE("25%") {
        divisor = 4;
        title = " 25% distinct";
        required_checksum = 359977283;
    }
    SUBCASE("50%") {
        divisor = 2;
        title = " 50% distinct";
        required_checksum = 179988809;
    }

    SUBCASE("100%") {
        divisor = 0;
        title = "100% distinct";
        required_checksum = 0;
    }

    sfc64 rng(123);
    mt checksum = 0;
    {
        Benchmark bench(title + " " + type_string(Map{}));
        mt count = 0;
        for (mt n = lower; n <= upper; n += step_width) {
            mt const max_rng =
                (divisor == static_cast<mt>(0)) ? (std::numeric_limits<mt>::max)() : n / divisor;
            Map map;
            for (mt i = 0; i < n; ++i) {
                auto num = rng.uniform(max_rng);
                num = ror(num, 17);
                checksum += map[num]++;
            }
            count += n;
        }
        bench.count(count);
    }
    REQUIRE(checksum == required_checksum);
}
