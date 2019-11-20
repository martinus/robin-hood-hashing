#include <robin_hood.h>

#include <app/doctest.h>
#include <iostream>

namespace {

// adding a small wrapper around size_t so I can implement a robin_hood::hash for this, to
// circumvent the safety mixer
class Dummy {
public:
    explicit Dummy(size_t x) noexcept
        : mVal(x) {}

    ROBIN_HOOD(NODISCARD) bool operator==(Dummy const& other) const noexcept {
        return mVal == other.mVal;
    }

    ROBIN_HOOD(NODISCARD) size_t val() const noexcept {
        return mVal;
    }

private:
    size_t mVal{};
};

} // namespace

// this dummy hash needs to be named ::robin_hood::hash or otherwise the safety mixer would kick in
namespace robin_hood {

template <>
struct hash<Dummy> {
    size_t operator()(Dummy const& d) const {
        return d.val();
    }
};

} // namespace robin_hood

TEST_CASE("iterator_twice_bug") {
    robin_hood::unordered_flat_map<Dummy, size_t> map;

    auto a = 31U + 1024U * 0U;
    auto b = 31U + 1024U * 1U;

    map[Dummy(a)] = 1U;
    map[Dummy(b)] = 3U;

    // it points to 1055, the first element which has wrapped around
    auto it = map.begin();
    REQUIRE(it->first.val() == a);

    // it points to the last element, 1024 which is in its original bucket
    ++it;
    REQUIRE(it->first.val() == b);

    // backward shift deletion removes 1024 and wraps back 2048, so it now (again!) points to 2048
    // and NOT end
    it = map.erase(it);
    REQUIRE(it == map.end());
}
