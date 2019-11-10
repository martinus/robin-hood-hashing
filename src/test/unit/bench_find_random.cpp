#include <robin_hood.h>

#include <app/benchmark.h>
#include <app/doctest.h>
#include <app/sfc64.h>

#include <array>

TYPE_TO_STRING(robin_hood::unordered_flat_map<size_t, size_t>);
TYPE_TO_STRING(robin_hood::unordered_node_map<size_t, size_t>);

TEST_CASE_TEMPLATE("bench_find_random" * doctest::test_suite("bench") * doctest::skip(), Map,
                   robin_hood::unordered_flat_map<size_t, size_t>,
                   robin_hood::unordered_node_map<size_t, size_t>) {

    static constexpr size_t numTotal = 4;

    size_t requiredChecksum = 0;
    size_t numFound = 0;
    SUBCASE("0% found") {
        numFound = 0;
        requiredChecksum = 0;
    }
    SUBCASE("25% found") {
        numFound = 1;
        requiredChecksum = 24998621;
    }
    SUBCASE("50% found") {
        numFound = 2;
        requiredChecksum = 49997240;
    }
    SUBCASE("75% found") {
        numFound = 3;
        requiredChecksum = 74995861;
    }
    SUBCASE("100% found") {
        numFound = 4;
        requiredChecksum = 99994482;
    }

    std::string const title = "random find " + std::to_string(numFound * 100 / numTotal) +
                              "% success" + type_string(Map{});

    sfc64 rng(123);

    size_t checksum = 0;

    using Ary = std::array<bool, numTotal>;
    Ary insertRandom;
    insertRandom.fill(true);
    for (typename Ary::size_type i = 0; i < numFound; ++i) {
        insertRandom[i] = false;
    }

    sfc64 anotherUnrelatedRng(987654321);
    auto const anotherUnrelatedRngInitialState = anotherUnrelatedRng.state();
    sfc64 findRng(anotherUnrelatedRngInitialState);

    {
        static constexpr size_t numInserts = 200000;
        static constexpr size_t numFindsPerInsert = 500;
        static constexpr size_t numFindsPerIter = numFindsPerInsert * numTotal;

        Map map;
        size_t i = 0;
        size_t findCount = 0;

        BENCHMARK(title, numFindsPerIter * (numInserts / numTotal), "op") {
            do {
                // insert numTotal entries: some random, some sequential.
                std::shuffle(insertRandom.begin(), insertRandom.end(), rng);
                for (bool isRandomToInsert : insertRandom) {
                    auto val = anotherUnrelatedRng.uniform<size_t>();
                    if (isRandomToInsert) {
                        map[rng.uniform<size_t>()] = static_cast<size_t>(1);
                    } else {
                        map[val] = static_cast<size_t>(1);
                    }
                    ++i;
                }

                // the actual benchmark code which should be as fast as possible
                for (size_t j = 0; j < numFindsPerIter; ++j) {
                    if (++findCount > i) {
                        findCount = 0;
                        findRng.state(anotherUnrelatedRngInitialState);
                    }
                    auto it = map.find(findRng.uniform<size_t>());
                    if (it != map.end()) {
                        checksum += it->second;
                    }
                }
            } while (i < numInserts);
        }
    }
    REQUIRE(checksum == requiredChecksum);
}
