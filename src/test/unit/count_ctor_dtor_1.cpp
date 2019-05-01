#include <robin_hood.h>

#include <app/benchmark.h>
#include <app/counter_defaults.h>
#include <app/sfc64.h>

#include <array>

TEST_CASE_TEMPLATE("count ctor-emplace-dtor" * doctest::test_suite("count") * doctest::skip(), Map,
                   robin_hood::unordered_flat_map<cnt, cnt, std::hash<cnt>>,
                   robin_hood::unordered_flat_map<cnt, cnt, robin_hood::hash<cnt>>,
                   robin_hood::unordered_node_map<cnt, cnt, std::hash<cnt>>,
                   robin_hood::unordered_node_map<cnt, cnt, robin_hood::hash<cnt>>,
                   std::unordered_map<cnt, cnt, std::hash<cnt>>,
                   std::unordered_map<cnt, cnt, robin_hood::hash<cnt>>) {

    Counter counts;
    {
        Map m;
        m.emplace(counts(1), counts(2));
    }
    counts.printCounts(std::string("ctor-emplace-dtor") + doctest::detail::type_to_string<Map>());
}
