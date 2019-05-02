#include <robin_hood.h>

#include <app/doctest.h>

#include <iostream>
#include <unordered_map>

namespace {
struct Foo {
    std::array<int, 20> ary;
};
} // namespace

/*
namespace std {
template <>
struct hash<Foo> {
    size_t operator()(Foo const&) const {
        return 0;
    }
};
} // namespace std
*/

TEST_CASE("datastructure sizes") {
#if ROBIN_HOOD_BITNESS == 64
    size_t flat_size = 6 * sizeof(size_t);
    size_t node_size = 8 * sizeof(size_t);
#else
    size_t flat_size = 7 * sizeof(size_t);
    size_t node_size = 9 * sizeof(size_t);
#endif
    REQUIRE(sizeof(robin_hood::unordered_flat_map<uint32_t, uint32_t>) == flat_size);
    REQUIRE(sizeof(robin_hood::unordered_flat_map<uint64_t, uint64_t>) == flat_size);
    REQUIRE(sizeof(robin_hood::unordered_flat_map<uint32_t, Foo>) == flat_size);
    REQUIRE(sizeof(robin_hood::unordered_flat_map<Foo, uint32_t>) == flat_size);
    REQUIRE(sizeof(robin_hood::unordered_flat_map<Foo, Foo>) == flat_size);

    REQUIRE(sizeof(robin_hood::unordered_node_map<uint32_t, uint32_t>) == node_size);
    REQUIRE(sizeof(robin_hood::unordered_node_map<uint64_t, uint64_t>) == node_size);
    REQUIRE(sizeof(robin_hood::unordered_node_map<uint32_t, Foo>) == node_size);
    REQUIRE(sizeof(robin_hood::unordered_node_map<Foo, uint32_t>) == node_size);
    REQUIRE(sizeof(robin_hood::unordered_node_map<Foo, Foo>) == node_size);
}
