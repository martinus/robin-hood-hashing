#include <robin_hood.h>

#include <app/doctest.h>

// struct that provides both hash and equals operator
struct HashWithEqual {
    size_t operator()(int x) const {
        return static_cast<size_t>(x);
    }

    bool operator()(int a, int b) const {
        return a == b;
    }
};

// make sure the map works with the same type (check that it handles the diamond problem)
TEST_CASE("diamond_problem") {
    robin_hood::unordered_flat_map<int, int, HashWithEqual, HashWithEqual> map;
    map[1] = 2;
    REQUIRE(map.size() == 1);
    REQUIRE(map.find(1) != map.end());
}
