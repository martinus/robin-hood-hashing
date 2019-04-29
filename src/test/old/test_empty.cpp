#include "test_base.h"

#include <memory>

TEST_CASE("trivially copyable") {
    static_assert(
        std::is_trivially_copyable<robin_hood::unordered_map<int, int>::value_type>::value,
        "NOT is_trivially_copyable");
    static_assert(
        std::is_trivially_copyable<robin_hood::unordered_map<int, int>::value_type>::value,
        "NOT is_trivially_destructible");
    // static_assert(std::is_trivially_copyable<std::pair<int, int>>::value, "NOT
    // is_trivially_copyable"); static_assert(std::is_trivially_copyable<std::pair<int,
    // int>>::value, "NOT is_trivially_destructible");
}

size_t inline_only();

TEST_CASE("inline_only") {
    REQUIRE(1 == inline_only());
}