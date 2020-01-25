#include <robin_hood.h>

#include <app/doctest.h>

#include <iostream>

TYPE_TO_STRING(robin_hood::unordered_flat_set<uint64_t>);
TYPE_TO_STRING(robin_hood::unordered_node_set<uint64_t>);

TEST_CASE_TEMPLATE("unordered_set", Set, robin_hood::unordered_flat_set<uint64_t>,
                   robin_hood::unordered_node_set<uint64_t>) {

    REQUIRE(sizeof(typename Set::value_type) == sizeof(uint64_t));

    Set set;
    set.emplace(UINT64_C(123));
    REQUIRE(set.size() == 1U);

    set.insert(UINT64_C(333));
    REQUIRE(set.size() == 2U);

    set.erase(UINT64_C(222));
    REQUIRE(set.size() == 2U);

    set.erase(UINT64_C(123));
    REQUIRE(set.size() == 1U);
}
