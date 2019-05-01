#include <robin_hood.h>

#include <app/benchmark.h>
#include <app/counter_defaults.h>
#include <app/sfc64.h>

#include <array>

TEST_CASE_TEMPLATE("count ctor & dtor" * doctest::test_suite("count") * doctest::skip(), Map,
                   robin_hood::unordered_flat_map<cnt, cnt, std::hash<cnt>>,
                   robin_hood::unordered_flat_map<cnt, cnt, robin_hood::hash<cnt>>,
                   robin_hood::unordered_node_map<cnt, cnt, std::hash<cnt>>,
                   robin_hood::unordered_node_map<cnt, cnt, robin_hood::hash<cnt>>,
                   std::unordered_map<cnt, cnt, std::hash<cnt>>,
                   std::unordered_map<cnt, cnt, robin_hood::hash<cnt>>) {

    Counter counts;
    { Map m; }
    counts.printCounts(std::string("ctor & dtor ") + doctest::detail::type_to_string<Map>());
}
