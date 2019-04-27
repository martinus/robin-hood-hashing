#include "test_base.h"

#include <memory>

TEMPLATE_TEST_CASE("find with empty map", "", FlatMap, NodeMap) {
    using Map = TestType;
    Map m;

    REQUIRE(m.end() == m.find(123));
    REQUIRE(m.end() == m.begin());
    m[32];
    REQUIRE(m.end() != m.begin());
    REQUIRE(m.end() == m.find(123));
    REQUIRE(m.end() != m.find(32));

    m = Map();
    REQUIRE(m.end() == m.begin());
    REQUIRE(m.end() == m.find(123));
    REQUIRE(m.end() == m.find(32));

    Map m2(m);
    REQUIRE(m2.end() == m2.begin());
    REQUIRE(m2.end() == m2.find(123));
    REQUIRE(m2.end() == m2.find(32));
    m2[32];
    REQUIRE(m2.end() != m2.begin());
    REQUIRE(m2.end() == m2.find(123));
    REQUIRE(m2.end() != m2.find(32));

    Map mEmpty;
    Map m3(mEmpty);
    REQUIRE(m3.end() == m3.begin());
    REQUIRE(m3.end() == m3.find(123));
    REQUIRE(m3.end() == m3.find(32));
    m3[32];
    REQUIRE(m3.end() != m3.begin());
    REQUIRE(m3.end() == m3.find(123));
    REQUIRE(m3.end() != m3.find(32));
}

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