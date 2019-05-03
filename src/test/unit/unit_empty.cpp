#include <robin_hood.h>

#include <app/doctest.h>

TYPE_TO_STRING(robin_hood::unordered_flat_map<int, int>);
TYPE_TO_STRING(robin_hood::unordered_node_map<int, int>);

TEST_CASE_TEMPLATE("empty map operations", Map, robin_hood::unordered_flat_map<int, int>,
                   robin_hood::unordered_node_map<int, int>) {
    Map m;

    REQUIRE(m.end() == m.find(123));
    REQUIRE(m.end() == m.begin());
    m[32];
    REQUIRE(m.end() != m.begin());
    REQUIRE(m.end() == m.find(123));
    REQUIRE(m.end() != m.find(32));

    m = Map();
    REQUIRE(m.end() == m.begin());
    REQUIRE(m.end() == m.find(123));
    REQUIRE(m.end() == m.find(32));

    Map m2(m);
    REQUIRE(m2.end() == m2.begin());
    REQUIRE(m2.end() == m2.find(123));
    REQUIRE(m2.end() == m2.find(32));
    m2[32];
    REQUIRE(m2.end() != m2.begin());
    REQUIRE(m2.end() == m2.find(123));
    REQUIRE(m2.end() != m2.find(32));

    Map mEmpty;
    Map m3(mEmpty);
    REQUIRE(m3.end() == m3.begin());
    REQUIRE(m3.end() == m3.find(123));
    REQUIRE(m3.end() == m3.find(32));
    m3[32];
    REQUIRE(m3.end() != m3.begin());
    REQUIRE(m3.end() == m3.find(123));
    REQUIRE(m3.end() != m3.find(32));

    Map m4(std::move(mEmpty));
    REQUIRE(m4.count(123) == 0);
    REQUIRE(m4.end() == m4.begin());
    REQUIRE(m4.end() == m4.find(123));
    REQUIRE(m4.end() == m4.find(32));
}
