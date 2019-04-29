#include "test_base.h"

#include <memory>

TEST_CASE("unique ptr") {
    robin_hood::unordered_map<int, std::unique_ptr<int>> map;
    map[321].reset(new int(123));
}

TEST_CASE("insert list") {
    std::vector<robin_hood::unordered_map<int, int>::value_type> v;
    v.emplace_back(1, 2);
    v.emplace_back(3, 4);

    robin_hood::unordered_map<int, int> map(v.begin(), v.end());
    REQUIRE(map.size() == 2);
}

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