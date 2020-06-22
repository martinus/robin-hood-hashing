#include <robin_hood.h>

#include <unit/include_only.h>

// this file makes sure robin_hood.h includes everything it needs
int include_only() {
    size_t s = 0;
    robin_hood::unordered_flat_map<int, int> map;
    map[123] = 43;
    s += map.size();
    robin_hood::unordered_flat_map<int, int> map2 = map;

    map.clear();
    s += map2.size();

    // NOLINTNEXTLINE(performance-unnecessary-copy-initialization)
    robin_hood::unordered_flat_map<int, int> map3 = map2;
    s += map3.size();

    robin_hood::unordered_node_map<int, float> muf;
    muf[321] = 432.43F;
    muf.emplace(111, 11.132F);
    for (auto const& kv : muf) {
        s += static_cast<size_t>(kv.first) + static_cast<size_t>(kv.second);
    }

    return static_cast<int>(s);
}
