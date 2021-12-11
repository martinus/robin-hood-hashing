#include <robin_hood.h>

#include <app/doctest.h>

#include <string>

#if !ROBIN_HOOD(BROKEN_CONSTEXPR)
struct RegularType {
    // cppcheck-suppress passedByValue
    RegularType(std::size_t i_, std::string s_) noexcept
        : s(std::move(s_))
        , i(i_) {}

    friend bool operator==(const RegularType& r1, const RegularType& r2) {
        return r1.i == r2.i && r1.s == r2.s;
    }

private:
    std::string s;
    std::size_t i;
};

TYPE_TO_STRING(robin_hood::unordered_flat_map<std::string, RegularType>);
TYPE_TO_STRING(robin_hood::unordered_node_map<std::string, RegularType>);

TEST_CASE_TEMPLATE("try_emplace", Map, robin_hood::unordered_flat_map<std::string, RegularType>,
                   robin_hood::unordered_node_map<std::string, RegularType>) {

    Map map;
    auto ret = map.try_emplace("a", 1U, "b");
    REQUIRE(ret.second);
    REQUIRE(ret.first == map.find("a"));
    REQUIRE(ret.first->second == RegularType(1U, "b"));
    REQUIRE(map.size() == 1);

    ret = map.try_emplace("c", 2U, "d");
    REQUIRE(ret.second);
    REQUIRE(ret.first == map.find("c"));
    REQUIRE(ret.first->second == RegularType(2U, "d"));
    REQUIRE(map.size() == 2U);

    std::string key = "c";
    ret = map.try_emplace(key, 3U, "dd");
    REQUIRE_FALSE(ret.second);
    REQUIRE(ret.first == map.find("c"));
    REQUIRE(ret.first->second == RegularType(2U, "d"));
    REQUIRE(map.size() == 2U);

    key = "a";
    RegularType value(3U, "dd");
    ret = map.try_emplace(key, value);
    REQUIRE_FALSE(ret.second);
    REQUIRE(ret.first == map.find("a"));
    REQUIRE(ret.first->second == RegularType(1U, "b"));
    REQUIRE(map.size() == 2U);

    auto pos = map.try_emplace(map.end(), "e", 67U, "f");
    REQUIRE(pos == map.find("e"));
    REQUIRE(pos->second == RegularType(67U, "f"));
    REQUIRE(map.size() == 3U);

    key = "e";
    RegularType value2(66U, "ff");
    pos = map.try_emplace(map.begin(), key, value2);
    REQUIRE(pos == map.find("e"));
    REQUIRE(pos->second == RegularType(67U, "f"));
    REQUIRE(map.size() == 3U);
}
#endif
