#include <robin_hood.h>

#include <app/Counter.h>
#include <app/benchmark.h>
#include <app/counter_defaults.h>
#include <app/sfc64.h>

TEST_CASE_TEMPLATE("count one emplace" * doctest::test_suite("count") * doctest::skip(), Map,
                   robin_hood::unordered_flat_map<cnt, cnt, std::hash<cnt>>,
                   robin_hood::unordered_flat_map<cnt, cnt, robin_hood::hash<cnt>>,
                   robin_hood::unordered_node_map<cnt, cnt, std::hash<cnt>>,
                   robin_hood::unordered_node_map<cnt, cnt, robin_hood::hash<cnt>>,
                   std::unordered_map<cnt, cnt, std::hash<cnt>>,
                   std::unordered_map<cnt, cnt, robin_hood::hash<cnt>>) {
    Counter counts;
    Map map;
    size_t x = 1;
    map.emplace(std::piecewise_construct, std::forward_as_tuple(x, counts),
                std::forward_as_tuple(x, counts));
    counts.printCounts(std::string("one emplace ") + doctest::detail::type_to_string<Map>());
}
