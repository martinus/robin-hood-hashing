#include <robin_hood.h>

#include <app/doctest.h>

#include <array>
#include <iostream>
#include <unordered_map>

namespace {
struct Foo {
    int a0{};
    int a1{};
};
} // namespace

namespace robin_hood {

template <>
struct hash<Foo> {
    size_t operator()(Foo const& o) const noexcept {
        auto hf = hash<int>{};
        return hf(o.a0) ^ hf(o.a1);
    }
};

} // namespace robin_hood

#define SHOW(...) std::cout << sizeof(__VA_ARGS__) << " bytes for " << #__VA_ARGS__ << std::endl

TEST_CASE("show_datastructure_sizes" * doctest::test_suite("show") * doctest::skip()) {
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

    // use hash
    Foo f;
    std::cout << robin_hood::hash<Foo>{}(f) << std::endl;
}
