#include "test_base.h"

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

TEMPLATE_TEST_CASE("copy and assign maps", "", FlatMap, NodeMap) {
    using Map = TestType;

    { Map a = createMap<Map>(15); }

    { Map a = createMap<Map>(100); }

    {
        Map a = createMap<Map>(1);
        Map b = a;
        REQUIRE(a == b);
    }

    {
        Map a;
        REQUIRE(a.empty());
        a.clear();
        REQUIRE(a.empty());
    }

    {
        Map a = createMap<Map>(100);
        Map b = a;
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

    { std::vector<Map> maps(10); }

    {
        Map a;
        std::vector<Map> maps(10, a);
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

TEMPLATE_TEST_CASE("test assignment combinations", "", FlatMap, NodeMap) {
    using Map = TestType;

    {
        Map a;
        Map b;
        b = a;
    }
    {
        Map a;
        Map const& aConst = a;
        Map b;
        a[123] = 321;
        b = a;

        REQUIRE(a.find(123)->second == 321);
        REQUIRE(aConst.find(123)->second == 321);

        REQUIRE(b.find(123)->second == 321);
        a[123] = 111;
        REQUIRE(a.find(123)->second == 111);
        REQUIRE(aConst.find(123)->second == 111);
        REQUIRE(b.find(123)->second == 321);
        b[123] = 222;
        REQUIRE(a.find(123)->second == 111);
        REQUIRE(aConst.find(123)->second == 111);
        REQUIRE(b.find(123)->second == 222);
    }

    {
        Map a;
        Map b;
        a[123] = 321;
        a.clear();
        b = a;

        REQUIRE(a.size() == 0);
        REQUIRE(b.size() == 0);
    }

    {
        Map a;
        Map b;
        b[123] = 321;
        b = a;

        REQUIRE(a.size() == 0);
        REQUIRE(b.size() == 0);
    }
    {
        Map a;
        Map b;
        b[123] = 321;
        b.clear();
        b = a;

        REQUIRE(a.size() == 0);
        REQUIRE(b.size() == 0);
    }
    {
        Map a;
        a[1] = 2;
        Map b;
        b[3] = 4;
        b = a;

        REQUIRE(a.size() == 1);
        REQUIRE(b.size() == 1);
        REQUIRE(b.find(1)->second == 2);
        a[1] = 123;
        REQUIRE(a.size() == 1);
        REQUIRE(b.size() == 1);
        REQUIRE(b.find(1)->second == 2);
    }
    {
        Map a;
        a[1] = 2;
        a.clear();
        Map b;
        REQUIRE(a == b);
        b[3] = 4;
        REQUIRE(a != b);
        b = a;
        REQUIRE(a == b);
    }
    {
        Map a;
        a[1] = 2;
        Map b;
        REQUIRE(a != b);
        b[3] = 4;
        b.clear();
        REQUIRE(a != b);
        b = a;
        REQUIRE(a == b);
    }
    {
        Map a;
        a[1] = 2;
        a.clear();
        Map b;
        b[3] = 4;
        REQUIRE(a != b);
        b.clear();
        REQUIRE(a == b);
        b = a;
        REQUIRE(a == b);
    }
}

TEMPLATE_TEST_CASE("reserve", "", FlatMap, NodeMap) {
    TestType map;
    REQUIRE(0 == map.mask());
    map.reserve(819);
    REQUIRE(1023 == map.mask());
    map.reserve(820);
    REQUIRE(2047 == map.mask());
}

struct NonCopyable {
    ~NonCopyable() = default;
    NonCopyable(NonCopyable const&) = delete;
    NonCopyable& operator=(NonCopyable const&) = delete;

    NonCopyable(NonCopyable&&) = default;
    NonCopyable& operator=(NonCopyable&&) = default;
};

struct NonCopyMove {
    NonCopyMove(int) {}

    NonCopyMove(NonCopyMove const&) = delete;
    NonCopyMove& operator=(NonCopyMove const&) = delete;

    NonCopyMove(NonCopyMove&&) = delete;
    NonCopyMove& operator=(NonCopyMove&&) = delete;
};

TEST_CASE("noncopyable") {
    // it's ok because it is movable.
    robin_hood::unordered_flat_map<int, NonCopyable> m;
    m[1];
}

TEST_CASE("nonmovable") {
    // not ok: can't copy and can't move
    // robin_hood::unordered_flat_map<int, NonCopyMove> m2;
    // m2.emplace(std::piecewise_construct, std::forward_as_tuple(123), std::forward_as_tuple(333));

    // it's ok because unordered_map defaults to unordered_node_map
    robin_hood::unordered_map<int, NonCopyMove> m;
    m.emplace(std::piecewise_construct, std::forward_as_tuple(123), std::forward_as_tuple(333));
}