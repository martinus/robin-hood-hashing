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

#if defined(__GNUC__) && (__GNUG__ < 8) && !defined(__clang__)
namespace std {
template <>
struct hash<Foo> {
    size_t operator()(const Foo& o) const noexcept {
        size_t h = 0;
        auto hf = hash<int>{};
        for (auto a : o.ary) {
            h ^= hf(a);
        }
        return h;
    }
};
} // namespace std
#endif

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
