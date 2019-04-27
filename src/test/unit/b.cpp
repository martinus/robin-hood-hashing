#include <doctest.h>

#include <iostream>

TEST_CASE_TEMPLATE("b", T, std::string) {
    std::cout << std::hex;
}
