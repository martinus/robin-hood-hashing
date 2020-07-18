#include <robin_hood.h>

#include <app/doctest.h>

#include <cstring>

static bool disable_string_operator = true;

struct MyHash {
    using is_transparent = void;

    size_t operator()(const std::string& str) const {
        if (disable_string_operator) {
            throw std::runtime_error("No hashing of string is allowed");
        }
        return robin_hood::hash_bytes(str.c_str(), str.size());
    }

    size_t operator()(const char* str) const noexcept {
        return robin_hood::hash_bytes(str, std::strlen(str));
    }
};

struct MyEqual {
    using is_transparent = int;

    bool operator()(const char* lhs, const std::string& rhs) const noexcept {
        return std::strcmp(lhs, rhs.c_str()) == 0;
    }

    bool operator()(const std::string& lhs, const std::string& rhs) const {
        if (disable_string_operator) {
            throw std::runtime_error("No comparison of string to string is allowed");
        }
        return lhs == rhs;
    }
};

TYPE_TO_STRING(robin_hood::unordered_flat_map<std::string, uint64_t, MyHash, MyEqual>);
TYPE_TO_STRING(robin_hood::unordered_node_map<std::string, uint64_t, MyHash, MyEqual>);

TEST_CASE_TEMPLATE("heterogeneous", Map,
                   robin_hood::unordered_flat_map<std::string, uint64_t, MyHash, MyEqual>,
                   robin_hood::unordered_node_map<std::string, uint64_t, MyHash, MyEqual>) {
    static_assert(Map::is_transparent, "not transparent");
    Map map;
    const Map& cmap = map;
    REQUIRE(map.count("123") == 0);
    REQUIRE(map.count("0") == 0);
    REQUIRE(!map.contains("123"));
    REQUIRE(!map.contains("0"));
    REQUIRE(map.find("123") == map.end());
    REQUIRE(map.find("0") == map.end());
    REQUIRE(cmap.find("123") == map.cend());
    REQUIRE(cmap.find("0") == map.cend());
    disable_string_operator = false;
    map["123"];
    disable_string_operator = true;
    REQUIRE(map.count("123") == 1);
    REQUIRE(map.count("0") == 0);
    REQUIRE(map.contains("123"));
    REQUIRE(!map.contains("0"));
    REQUIRE(map.find("123") == map.begin());
    REQUIRE(map.find("0") == map.end());
    REQUIRE(cmap.find("123") == map.cbegin());
    REQUIRE(cmap.find("0") == map.cend());
}

#if 0
TEST_CASE_TEMPLATE("heterogeneous_emplace", Map,
                   robin_hood::unordered_flat_map<std::string, uint64_t, MyHash, MyEqual>,
                   robin_hood::unordered_node_map<std::string, uint64_t, MyHash, MyEqual>) {
    disable_string_operator = false;
    Map map;
    map.emplace("asdf", 100U);

    REQUIRE(map.size() == 1);
    REQUIRE(map.count("asdf") == 1U);
}
#endif
