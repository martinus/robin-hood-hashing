#include <robin_hood.h>

#include <app/doctest.h>

#include <array>
#include <iostream>
#include <unordered_map>

namespace {
struct Foo {
    std::array<int, 20> ary;
};
} // namespace

#define SHOW(...) std::cout << sizeof(__VA_ARGS__) << " bytes for " << #__VA_ARGS__ << std::endl

TEST_CASE("show datastructure sizes" * doctest::test_suite("show") * doctest::skip()) {
    SHOW(robin_hood::unordered_flat_map<uint32_t, uint32_t>);
    SHOW(robin_hood::unordered_flat_map<uint64_t, uint64_t>);
    SHOW(robin_hood::unordered_flat_map<uint32_t, Foo>);
    SHOW(robin_hood::unordered_flat_map<Foo, uint32_t>);
    SHOW(robin_hood::unordered_flat_map<Foo, Foo>);

    SHOW(robin_hood::unordered_node_map<uint32_t, uint32_t>);
    SHOW(robin_hood::unordered_node_map<uint64_t, uint64_t>);
    SHOW(robin_hood::unordered_node_map<uint32_t, Foo>);
    SHOW(robin_hood::unordered_node_map<Foo, uint32_t>);
    SHOW(robin_hood::unordered_node_map<Foo, Foo>);
}
