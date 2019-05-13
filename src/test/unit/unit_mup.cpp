#include <app/fmt/mup.h>

#include <app/doctest.h>

#include <iostream>

TEST_CASE("mup format" * doctest::test_suite("show") * doctest::skip()) {
    using martinus::mup;

    double a = 1.23456789;
    double b = 1.0;
    double c = 1.23456789;
    double d = 1.0;
    for (size_t i = 1; i < 30; ++i) {
        std::cout << mup(a) << "s, " << mup(b) << "B, " << mup(c) << "J/s, " << mup(d) << "m/s"
                  << std::endl;
        a *= 10;
        b *= 10;
        c /= 10;
        d /= 10;
    }

    std::cout << mup(0) << "s" << std::endl;
}
