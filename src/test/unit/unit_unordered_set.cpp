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

TEST_CASE_TEMPLATE("unordered_set_string", Set, robin_hood::unordered_flat_set<std::string>,
                   robin_hood::unordered_node_set<std::string>) {
    Set set;
    REQUIRE(set.begin() == set.end());

    set.emplace(static_cast<size_t>(2000), 'a');
    REQUIRE(set.size() == 1);

    REQUIRE(set.begin() != set.end());
    std::string& str = *set.begin();
    REQUIRE(str == std::string(static_cast<size_t>(2000), 'a'));

    auto it = set.begin();
    REQUIRE(++it == set.end());
}
