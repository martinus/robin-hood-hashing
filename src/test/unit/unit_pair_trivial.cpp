#include <robin_hood.h>

#include <doctest.h>

#include <iostream>
#include <string>

template <typename K, typename V>
void check(bool result) {
    REQUIRE(std::is_trivially_copyable<
                typename robin_hood::unordered_flat_map<K, V>::value_type>::value == result);
    REQUIRE(std::is_trivially_destructible<
                typename robin_hood::unordered_node_map<K, V>::value_type>::value == result);
}

template <typename K, typename V>
void yes() {
    check<K, V>(true);
}

template <typename K, typename V>
void no() {
    check<K, V>(false);
}

// std::pair is not trivially copyable, but robin_hood::pair is.
TEST_CASE("pair trivially copy/destructible") {
    yes<int, int>();
    yes<float, double>();
    no<int, std::string>();
    no<std::string, int>();
    yes<char const*, int>();
    yes<int, char const*>();
    yes<uint64_t, int32_t>();
    yes<int32_t, uint64_t>();
}
