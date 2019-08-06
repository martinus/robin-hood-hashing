#include <robin_hood.h>

#include <app/benchmark.h>
#include <app/doctest.h>
#include <app/sfc64.h>

TYPE_TO_STRING(robin_hood::unordered_flat_map<int, int>);
TYPE_TO_STRING(robin_hood::unordered_node_map<int, int>);

// benchmark adapted from https://github.com/attractivechaos/udb2
// this implementation should have less overhead, because sfc64 and it's uniform() is extremely
// fast.
TEST_CASE_TEMPLATE("bench_distinctness" * doctest::test_suite("bench") * doctest::skip(), Map,
                   robin_hood::unordered_flat_map<int, int>,
                   robin_hood::unordered_node_map<int, int>) {
    using mt = typename Map::mapped_type;

    static mt const upper = 50000000;
    static mt const lower = 10000000;
    static mt const num_steps = 5;
    static mt const step_width = (upper - lower) / num_steps;

    mt divisor = 0;
    std::string title;
    int required_checksum = 0;

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
        required_checksum = 1516798;
    }

    sfc64 rng(123);
    int checksum = 0;
    {
        Benchmark bench(title + " " + type_string(Map{}));
        mt count = 0;
        for (mt n = lower; n <= upper; n += step_width) {
            mt const max_rng = divisor == 0 ? (std::numeric_limits<mt>::max)() : n / divisor;
            Map map;
            for (mt i = 0; i < n; ++i) {
                checksum += map[rng.uniform(max_rng)]++;
            }
            count += n;
        }
        bench.count(count);
    }
    REQUIRE(checksum == required_checksum);
}
