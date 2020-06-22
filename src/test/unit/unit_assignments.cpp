#include <robin_hood.h>

#include <app/doctest.h>

#include <vector>

// creates a map with some data in it
template <class M>
M createMap(int numElements) {
    M m;
    for (int i = 0; i < numElements; ++i) {
        m[static_cast<typename M::key_type>((i + 123) * 7)] =
            static_cast<typename M::mapped_type>(i);
    }
    return m;
}

TYPE_TO_STRING(robin_hood::unordered_flat_map<int, int>);
TYPE_TO_STRING(robin_hood::unordered_node_map<int, int>);

TEST_CASE_TEMPLATE("copy and assign maps", Map, robin_hood::unordered_flat_map<int, int>,
                   robin_hood::unordered_node_map<int, int>) {

    { auto a = createMap<Map>(15); }

    { auto a = createMap<Map>(100); }

    {
        auto a = createMap<Map>(1);
        // NOLINTNEXTLINE(performance-unnecessary-copy-initialization)
        auto b = a;
        REQUIRE(a == b);
    }

    {
        Map a;
        REQUIRE(a.empty());
        a.clear();
        REQUIRE(a.empty());
    }

    {
        auto a = createMap<Map>(100);
        // NOLINTNEXTLINE(performance-unnecessary-copy-initialization)
        auto b = a;
        REQUIRE(b == a);
    }
    {
        Map a;
        a[123] = 321;
        a.clear();
        std::vector<Map> maps(10, a);

        for (size_t i = 0; i < maps.size(); ++i) {
            REQUIRE(maps[i].empty());
        }
    }

    {
        std::vector<Map> maps(10);
        REQUIRE(maps.size() == 10U);
    }

    {
        Map a;
        std::vector<Map> maps(12, a);
        REQUIRE(maps.size() == 12U);
    }

    {
        Map a;
        a[123] = 321;
        std::vector<Map> maps(10, a);
        a[123] = 1;

        for (size_t i = 0; i < maps.size(); ++i) {
            REQUIRE(maps[i].size() == 1);
            REQUIRE(maps[i].find(123)->second == 321);
        }
    }
}
