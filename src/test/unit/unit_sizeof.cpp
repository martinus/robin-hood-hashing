#include <robin_hood.h>

#include <doctest.h>

#include <iostream>
#include <unordered_map>

namespace {
struct Foo {
    std::array<int, 20> ary;
};
} // namespace

namespace std {
template <>
struct hash<Foo> {
    size_t operator()(Foo const&) const {
        return 0;
    }
};
} // namespace std

TEST_CASE("datastructure sizes") {
    size_t s = 6 * sizeof(size_t);
    REQUIRE(sizeof(robin_hood::unordered_flat_map<uint32_t, uint32_t>) == s);
    REQUIRE(sizeof(robin_hood::unordered_flat_map<uint64_t, uint64_t>) == s);
    REQUIRE(sizeof(robin_hood::unordered_flat_map<uint32_t, Foo>) == s);
    REQUIRE(sizeof(robin_hood::unordered_flat_map<Foo, uint32_t>) == s);
    REQUIRE(sizeof(robin_hood::unordered_flat_map<Foo, Foo>) == s);

    s = 8 * sizeof(size_t);
    REQUIRE(sizeof(robin_hood::unordered_node_map<uint32_t, uint32_t>) == s);
    REQUIRE(sizeof(robin_hood::unordered_node_map<uint64_t, uint64_t>) == s);
    REQUIRE(sizeof(robin_hood::unordered_node_map<uint32_t, Foo>) == s);
    REQUIRE(sizeof(robin_hood::unordered_node_map<Foo, uint32_t>) == s);
    REQUIRE(sizeof(robin_hood::unordered_node_map<Foo, Foo>) == s);
}
