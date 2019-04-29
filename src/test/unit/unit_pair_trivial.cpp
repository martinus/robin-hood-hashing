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

    /*
    std::cout << std::is_trivially_copyable<std::pair<K, V>>::value << " "
              << std::is_trivially_destructible<std::pair<K, V>>::value << ", expected: " << result
              << std::endl;
    */
}

template <typename K, typename V>
void yes() {
    check<K, V>(true);
}

template <typename K, typename V>
void no() {
    check<K, V>(false);
}

TEST_CASE("pair trivially copy/destructible") {
    yes<int, int>();
    yes<float, float>();
    no<size_t, std::string>();
    no<std::string, size_t>();
    yes<char const*, size_t>();
    yes<size_t, char const*>();
    yes<uint64_t, uint64_t>();
    yes<uint32_t, uint32_t>();
}
