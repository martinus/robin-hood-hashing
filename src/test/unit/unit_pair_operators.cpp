#include <robin_hood.h>

#include <app/doctest.h>

#include <iostream>

namespace {

template <typename A, typename B>
void check(A const& a1, B const& b1, A const& a2, B const& b2) {
    auto p1 = std::pair<A, B>(a1, b1);
    auto p2 = std::pair<A, B>(a2, b2);

    auto r1 = robin_hood::pair<A, B>(a1, b1);
    auto r2 = robin_hood::pair<A, B>(a2, b2);

    INFO("a1=" << a1 << ", b1=" << b1 << ", a2=" << a2 << ", b2=" << b2);
    REQUIRE((p1 == p2) == (r1 == r2));
    REQUIRE((p1 != p2) == (r1 != r2));
    REQUIRE((p1 < p2) == (r1 < r2));
    REQUIRE((p1 <= p2) == (r1 <= r2));
    REQUIRE((p1 > p2) == (r1 > r2));
    REQUIRE((p1 >= p2) == (r1 >= r2));
}

// to try all possible comination of pair comparisons, we use 2 bits per number*4 = 8 bits total
// to cover all possible combinations try all combinations
template <typename Op>
void allCombinations(Op op) {
    for (uint64_t x = 0; x < 256; ++x) {
        auto d = x & 3U;
        auto c = (x >> 2U) & 3U;
        auto b = (x >> 4U) & 3U;
        auto a = (x >> 6U) & 3U;
        op(a, b, c, d);
    }
}

} // namespace

TEST_CASE("pair_ops") {
    allCombinations([](uint64_t a, uint64_t b, uint64_t c, uint64_t d) { check(a, b, c, d); });
}

// make sure only operator< and operator== is required, nothing else
class A {
public:
    explicit A(uint64_t val)
        : mVal(val) {}

    bool operator==(A const& other) const noexcept {
        return mVal == other.mVal;
    }

    bool operator<(A const& other) const noexcept {
        return mVal < other.mVal;
    }

private:
    uint64_t mVal;
};

class B {
public:
    explicit B(uint64_t val)
        : mVal(val) {}

    bool operator==(B const& other) const noexcept {
        return mVal == other.mVal;
    }

    bool operator<(B const& other) const noexcept {
        return mVal < other.mVal;
    }

private:
    uint64_t mVal;
};

TEST_CASE("pair_minimal_ops") {
    allCombinations([](uint64_t a, uint64_t b, uint64_t c, uint64_t d) {
        check<A, B>(A{a}, B{b}, A{c}, B{d});
    });
}
