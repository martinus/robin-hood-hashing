#include "test_base.h"

template <typename A, typename B>
void check() {
    static_assert(std::is_nothrow_move_constructible<robin_hood::pair<A, B>>::value,
                  "is_nothrow_move_constructible");
    static_assert(std::is_nothrow_move_assignable<robin_hood::pair<A, B>>::value,
                  "is_nothrow_move_assignable");
    std::cout << sizeof(robin_hood::pair<A, B>) << " sizeof robin_hood::pair, "
              << sizeof(std::pair<A, B>) << " sizeof std::pair" << std::endl;
}

TEST_CASE("pair") {
    check<int, int>();
    check<uint8_t, uint8_t>();
    check<uint64_t, uint64_t>();
    check<std::vector<std::unique_ptr<int>>, uint64_t>();
}