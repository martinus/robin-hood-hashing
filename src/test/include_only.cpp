#include "robin_hood.h"
#include <stdexcept>

// this file makes sure robin_hood.h includes everything it needs
size_t inline_only() {
    robin_hood::unordered_map<int, int> map;
    map[123] = 43;

    robin_hood::unordered_map<int, int> map2 = map;
    map.clear();
    return map.size() + map2.size();
}
