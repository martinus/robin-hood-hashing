#include <robin_hood.h>

#include <app/benchmark.h>
#include <app/counter_defaults.h>
#include <app/sfc64.h>

#include <array>

TEST_CASE_TEMPLATE("count find random" * doctest::test_suite("count") * doctest::skip(), Map,
                   robin_hood::unordered_flat_map<cnt, cnt, std::hash<cnt>>,
                   robin_hood::unordered_flat_map<cnt, cnt, robin_hood::hash<cnt>>,
                   robin_hood::unordered_node_map<cnt, cnt, std::hash<cnt>>,
                   robin_hood::unordered_node_map<cnt, cnt, robin_hood::hash<cnt>>,
                   std::unordered_map<cnt, cnt, std::hash<cnt>>,
                   std::unordered_map<cnt, cnt, robin_hood::hash<cnt>>) {

    static constexpr size_t numTotal = 4;
    static constexpr size_t numInserts = 20000;
    static constexpr size_t numFindsPerInsert = 50;
    static constexpr size_t numFindsPerIter = numFindsPerInsert * numTotal;

    Counter counts;

    sfc64 rng(123);

    sfc64 anotherUnrelatedRng(987654321);
    auto const anotherUnrelatedRngInitialState = anotherUnrelatedRng.state();
    sfc64 findRng(anotherUnrelatedRngInitialState);

    for (size_t numFound = 0; numFound <= numTotal; ++numFound) {
        auto insertRandom = std::array<bool, numTotal>();
        insertRandom.fill(true);
        for (size_t i = 0; i < numFound; ++i) {
            insertRandom[i] = false;
        }

        Map map;
        size_t i = 0;
        size_t findCount = 0;

        do {
            // insert numTotal entries: some random, some sequential.
            std::shuffle(insertRandom.begin(), insertRandom.end(), rng);
            for (bool isRandomToInsert : insertRandom) {
                auto val = anotherUnrelatedRng.uniform<size_t>();
                if (isRandomToInsert) {
                    map[counts(rng.uniform<size_t>())] = counts(1);
                } else {
                    map[counts(val)] = counts(i);
                }
                ++i;
            }

            // the actual benchmark code which sohould be as fast as possible
            for (size_t j = 0; j < numFindsPerIter; ++j) {
                if (++findCount > i) {
                    findCount = 0;
                    findRng.state(anotherUnrelatedRngInitialState);
                }
                // cast to void to get rid of nodiscard warning
                static_cast<void>(map.find(counts(findRng.uniform<size_t>())));
            }
        } while (i < numInserts);
    }
    counts.printCounts(std::string("find random ") + doctest::detail::type_to_string<Map>());
}
