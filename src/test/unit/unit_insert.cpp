#include <dense_flat_map.h>

#include <app/doctest.h>

TYPE_TO_STRING(ankerl::dense_flat_map<unsigned int, int>);

TEST_CASE("insert") {
    using Map = ankerl::dense_flat_map<unsigned int, int>;
    auto map = Map();
    typename Map::value_type val(123U, 321);
    map.insert(val);
    REQUIRE(map.size() == 1);

    REQUIRE(map[123U] == 321);
}
