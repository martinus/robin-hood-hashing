#include <robin_hood.h>

#include <doctest.h>

#include <iostream>
#include <string>
#include <type_traits>

template <typename K, typename V>
void nothrow_trivially(bool is_nothrow, bool is_trivially) {
    REQUIRE(std::is_nothrow_move_constructible<robin_hood::pair<K, V>>::value == is_nothrow);
    REQUIRE(std::is_nothrow_move_assignable<robin_hood::pair<K, V>>::value == is_nothrow);

    REQUIRE(std::is_trivially_copyable<robin_hood::pair<K, V>>::value == is_trivially);
    REQUIRE(std::is_trivially_destructible<robin_hood::pair<K, V>>::value == is_trivially);

    REQUIRE(sizeof(robin_hood::pair<K, V>) == sizeof(std::pair<K, V>));
    /*
    std::cout << std::is_trivially_copyable<std::pair<K, V>>::value << " "
              << std::is_trivially_destructible<std::pair<K, V>>::value << ", expected: " << result
              << std::endl;
    */
}

TEST_CASE("pair trivially copy/destructible") {
    nothrow_trivially<int, int>(true, true);
    nothrow_trivially<float, float>(true, true);
    nothrow_trivially<size_t, std::string>(true, false);
    nothrow_trivially<std::string, size_t>(true, false);
    nothrow_trivially<char const*, size_t>(true, true);
    nothrow_trivially<size_t, char const*>(true, true);
    nothrow_trivially<uint64_t, uint64_t>(true, true);
    nothrow_trivially<uint32_t, uint32_t>(true, true);
    nothrow_trivially<char, char>(true, true);
}
