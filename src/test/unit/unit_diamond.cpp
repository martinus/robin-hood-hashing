#include <robin_hood.h>

#include <app/doctest.h>

// struct that provides both hash and equals operator. Can't directly use this in the robin_hood::map.
struct HashWithEqual {
    size_t operator()(int x) const {
        return static_cast<size_t>(x);
    }

    bool operator()(int a, int b) const {
        return a == b;
    }
};

// create simple wrapper classes, use these in the map
// see https://stackoverflow.com/a/28771920/48181
struct WrapHash : public HashWithEqual {};
struct WrapEquals : public HashWithEqual {};

TEST_CASE("bulkpoolallocator addOrFree") {
    robin_hood::unordered_flat_map<int, int, WrapHash, WrapEquals> map;
    map[1] = 2;
    REQUIRE(map.size() == 1);
    REQUIRE(map.find(1) != map.end());
}
