#include <robin_hood.h>

#include <app/doctest.h>

TYPE_TO_STRING(robin_hood::unordered_flat_map<int, int>);
TYPE_TO_STRING(robin_hood::unordered_node_map<int, int>);

TEST_CASE_TEMPLATE("assign to moved", Map, robin_hood::unordered_node_map<int, int>,
                   robin_hood::unordered_flat_map<int, int>) {

    Map a;
    a[1] = 2;
    Map moved = std::move(a);

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
}