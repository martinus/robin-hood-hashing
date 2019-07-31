#define ROBIN_HOOD_LOG_ENABLED
#include <robin_hood.h>

#include <app/doctest.h>

#include <iostream>

namespace {
struct BadType {
    BadType(uint64_t val) noexcept
        : mVal(val) {}

    bool operator==(BadType const& o) const {
        return mVal == o.mVal;
    }

    uint64_t mVal;
};

} // namespace

// needs to be a robin_hood::hash, otherwise the bad-hash-prevention kicks in.
namespace robin_hood {

template <>
struct hash<BadType> {
    size_t operator()(BadType const& obj) const noexcept {
        return hash_int(static_cast<uint64_t>(obj.mVal)) << 8;
    }
};

} // namespace robin_hood

TEST_CASE("bad hash shrink") {
    robin_hood::unordered_flat_map<BadType, uint64_t> map;

    auto numElements = map.calcMaxNumElementsAllowed(256);
    for (size_t i = 0; i < numElements; ++i) {
        auto n = static_cast<uint64_t>(i);
        map.emplace(std::piecewise_construct, std::forward_as_tuple(n), std::forward_as_tuple(n));
        std::cout << map.load_factor() << std::endl;
    }

    map.rehash(0);
}
