#include "robin_hood.h"

#include "Counter.h"
#include "benchmark.h"
#include "sfc64.h"

#include <unordered_map>

#include "doctest.h"

using cnt = Counter::Obj;

TYPE_TO_STRING(robin_hood::unordered_flat_map<cnt, cnt, std::hash<cnt>>);
TYPE_TO_STRING(robin_hood::unordered_flat_map<cnt, cnt, robin_hood::hash<cnt>>);
TYPE_TO_STRING(robin_hood::unordered_node_map<cnt, cnt, std::hash<cnt>>);
TYPE_TO_STRING(robin_hood::unordered_node_map<cnt, cnt, robin_hood::hash<cnt>>);
TYPE_TO_STRING(std::unordered_map<cnt, cnt, std::hash<cnt>>);
TYPE_TO_STRING(std::unordered_map<cnt, cnt, robin_hood::hash<cnt>>);

TEST_CASE_TEMPLATE("count random insert erase" * doctest::test_suite("count") * doctest::skip(),
                   Map, robin_hood::unordered_flat_map<cnt, cnt, std::hash<cnt>>,
                   robin_hood::unordered_flat_map<cnt, cnt, robin_hood::hash<cnt>>,
                   robin_hood::unordered_node_map<cnt, cnt, std::hash<cnt>>,
                   robin_hood::unordered_node_map<cnt, cnt, robin_hood::hash<cnt>>,
                   std::unordered_map<cnt, cnt, std::hash<cnt>>,
                   std::unordered_map<cnt, cnt, robin_hood::hash<cnt>>) {
    sfc64 rng(123);

    Counter counts;

    size_t verifier = 0;
    Map map;
    for (size_t n = 1; n < 1000; ++n) {
        for (size_t i = 0; i < 1000; ++i) {
            map[counts(rng.uniform(n))] = counts(i);
            verifier += map.erase(counts(rng.uniform(n)));
        }
    }
    counts.printCounts(std::string("random insert erase ") +
                       doctest::detail::type_to_string<Map>());
}