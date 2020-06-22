#include <robin_hood.h>

#include <app/doctest.h>

TYPE_TO_STRING(robin_hood::unordered_flat_map<int, int>);
TYPE_TO_STRING(robin_hood::unordered_node_map<int, int>);

TEST_CASE_TEMPLATE("assign to moved", Map, robin_hood::unordered_node_map<int, int>,
                   robin_hood::unordered_flat_map<int, int>) {

    Map a;
    a[1] = 2;
    Map moved = std::move(a);
    REQUIRE(moved.size() == 1U);

    Map c;
    c[3] = 4;

    // assign to a moved map
    a = c;
}

TEST_CASE_TEMPLATE("move to moved", Map, robin_hood::unordered_node_map<int, int>,
                   robin_hood::unordered_flat_map<int, int>) {

    Map a;
    a[1] = 2;
    Map moved = std::move(a);

    Map c;
    c[3] = 4;

    // assign to a moved map
    a = std::move(c);

    a[5] = 6;
    moved[6] = 7;
    REQUIRE(moved[6] == 7);
}

TEST_CASE_TEMPLATE("swapempty", Map, robin_hood::unordered_node_map<int, int>,
                   robin_hood::unordered_flat_map<int, int>) {

    {
        Map b;
        {
            Map a;
            b[1] = 2;

            a.swap(b);
            REQUIRE(a.end() != a.find(1));
            REQUIRE(b.end() == b.find(1));
        }
        REQUIRE(b.end() == b.find(1));
        b[2] = 3;
        REQUIRE(b.end() != b.find(2));
        REQUIRE(b.size() == 1);
    }

    {
        Map a;
        {
            Map b;
            b[1] = 2;

            a.swap(b);
            REQUIRE(a.end() != a.find(1));
            REQUIRE(b.end() == b.find(1));
        }
        REQUIRE(a.end() != a.find(1));
        a[2] = 3;
        REQUIRE(a.end() != a.find(2));
        REQUIRE(a.size() == 2);
    }

    {
        Map a;
        {
            Map b;
            a.swap(b);
            REQUIRE(a.end() == a.find(1));
            REQUIRE(b.end() == b.find(1));
        }
        REQUIRE(a.end() == a.find(1));
    }
}
