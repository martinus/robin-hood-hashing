#include <robin_hood.h>

#include <app/Counter.h>
#include <app/benchmark.h>
#include <app/sfc64.h>

#include <unordered_map>

#include <app/counter_defaults.h>

TEST_CASE_TEMPLATE("count random insert erase" * doctest::test_suite("count") * doctest::skip(),
                   Map, robin_hood::unordered_flat_map<cnt, cnt, std::hash<cnt>>,
                   robin_hood::unordered_flat_map<cnt, cnt, robin_hood::hash<cnt>>,
                   robin_hood::unordered_node_map<cnt, cnt, std::hash<cnt>>,
                   robin_hood::unordered_node_map<cnt, cnt, robin_hood::hash<cnt>>,
                   std::unordered_map<cnt, cnt, std::hash<cnt>>,
                   std::unordered_map<cnt, cnt, robin_hood::hash<cnt>>) {
    sfc64 rng(123);

    Counter counts;

    Map map;
    for (size_t n = 100; n < 10000; ++n) {
        for (size_t i = 0; i < 100; ++i) {
            map[counts(rng.uniform(n))] = counts(i);
            map.erase(counts(rng.uniform(n)));
        }
    }
    counts.printCounts(std::string("random insert erase ") +
                       doctest::detail::type_to_string<Map>());
}
