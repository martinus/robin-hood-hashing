#include <robin_hood.h>

#include <app/doctest.h>

#include <chrono>
#include <iomanip>
#include <iostream>

namespace {

inline uint32_t modify(uint32_t x) {
    return robin_hood::detail::rotr(x, 16U);
}

} // namespace

TEST_CASE("fill_x" * doctest::skip()) {
    std::cout << "fill_x" << std::endl;
    robin_hood::unordered_flat_set<uint32_t> data;

    // fill up with ever increasing xy data
    uint32_t x = 0;
    uint32_t y = 0;

    auto begin = std::chrono::steady_clock::now();
    size_t oldMask = 0;
    while (data.size() < 500000000) {
        data.insert(modify(x++));

        if (data.mask() != oldMask) {
            auto end = std::chrono::steady_clock::now();
            std::cout << std::setw(15) << std::chrono::nanoseconds(end - begin).count()
                      << " ns: " << data.size() << " " << (data.mask() + 1) << " " << x << " " << y
                      << std::endl;

            oldMask = data.mask();
        }
    }
    auto end = std::chrono::steady_clock::now();
    std::cout << std::setw(15) << std::chrono::nanoseconds(end - begin).count()
              << " ns: " << data.size() << " " << (data.mask() + 1) << " " << x << " " << y
              << std::endl;
}

TEST_CASE("fill_xy" * doctest::skip()) {
    std::cout << "fill_xy" << std::endl;
    robin_hood::unordered_flat_set<uint32_t> data;

    // fill up with ever increasing xy data
    uint32_t x = 0;
    uint32_t y = 0;

    auto begin = std::chrono::steady_clock::now();
    size_t oldMask = 0;
    while (data.size() < 500000000) {
        data.insert(modify(x | (y << 16U)));
        data.insert(modify(y | (x << 16U)));

        if (y == x) {
            ++x;
            y = 0;
        } else {
            ++y;
        }

        if (data.mask() != oldMask) {
            auto end = std::chrono::steady_clock::now();
            std::cout << std::setw(15) << std::chrono::nanoseconds(end - begin).count()
                      << " ns: " << data.size() << " " << (data.mask() + 1) << " " << x << " " << y
                      << std::endl;

            oldMask = data.mask();
        }
    }
    auto end = std::chrono::steady_clock::now();
    std::cout << std::setw(15) << std::chrono::nanoseconds(end - begin).count()
              << " ns: " << data.size() << " " << (data.mask() + 1) << " " << x << " " << y
              << std::endl;
}

TEST_CASE("fill_xyz" * doctest::skip()) {
    std::cout << "fill_xyz" << std::endl;
    robin_hood::unordered_flat_set<uint32_t> data;

    // fill up with ever increasing xy data
    uint32_t x = 0;
    uint32_t y = 0;
    uint32_t z = 0;

    auto begin = std::chrono::steady_clock::now();
    size_t oldMask = 0;
    while (data.size() < 500000000) {
        data.insert(modify(x | (y << 10U) | (z << 20U)));
        data.insert(modify(x | (z << 10U) | (y << 20U)));
        data.insert(modify(y | (x << 10U) | (z << 20U)));
        data.insert(modify(y | (z << 10U) | (x << 20U)));
        data.insert(modify(z | (x << 10U) | (y << 20U)));
        data.insert(modify(z | (y << 10U) | (x << 20U)));

        if (y == x) {
            if (z == x) {
                ++z;
                x = 0;
                y = 0;
            } else {
                ++x;
                y = 0;
            }
        } else {
            ++y;
        }

        if (data.mask() != oldMask) {
            auto end = std::chrono::steady_clock::now();
            std::cout << std::setw(15) << std::chrono::nanoseconds(end - begin).count()
                      << " ns: " << data.size() << " " << (data.mask() + 1) << " " << x << " " << y
                      << " " << z << std::endl;

            oldMask = data.mask();
        }
    }
    auto end = std::chrono::steady_clock::now();
    std::cout << std::setw(15) << std::chrono::nanoseconds(end - begin).count()
              << " ns: " << data.size() << " " << (data.mask() + 1) << " " << x << " " << y << " "
              << z << std::endl;
}
