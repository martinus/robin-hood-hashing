#include <robin_hood.h>

#include <app/doctest.h>
#include <iostream>

namespace {

struct Dummy {
    Dummy(size_t x)
        : val(x) {}

    Dummy() = default;

    size_t val{};

    bool operator==(Dummy const& other) const {
        return val == other.val;
    }
};

} // namespace

// this dummy hash needs to be named ::robin_hood::hash or otherwise the safety mixer would kick in
namespace robin_hood {

template <>
struct hash<Dummy> {
    size_t operator()(Dummy const& d) const {
        return d.val;
    }
};

} // namespace robin_hood

TEST_CASE("iterator_twice_bug") {
    robin_hood::unordered_flat_map<Dummy, int> map;

    map[31 + 1024 * 0] = 1;
    map[31 + 1024 * 1] = 3;

    // it points to 1055, the first element which has wrapped around
    auto it = map.begin();
    std::cout << it->first.val << " -> " << it->second << std::endl;

    // it points to the last element, 1024 which is in its original bucket
    ++it;
    std::cout << it->first.val << " -> " << it->second << std::endl;

    // backward shift deletion removes 1024 and wraps back 2048, so it now (again!) points to 2048
    // and NOT end
    it = map.erase(it);
    REQUIRE(it == map.end());
}
