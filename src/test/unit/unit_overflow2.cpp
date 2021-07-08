//#define ROBIN_HOOD_TRACE_ENABLED
//#define ROBIN_HOOD_LOG_ENABLED
#include <robin_hood.h>

#include <app/doctest.h>

#include <fstream>
#include <iostream>

TEST_CASE("bug_overflow2") {
    robin_hood::unordered_set<uint32_t> set;

    std::ifstream fin("a.txt");
    if (fin.is_open()) {
        uint32_t num{};
        size_t i = 0;
        while (fin >> num) {
            ++i;
            set.insert(num);
            std::cout << i << ": " << num << std::endl;
        }
    }
}
