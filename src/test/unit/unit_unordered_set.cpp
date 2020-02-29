#include <robin_hood.h>

#include <app/doctest.h>

#include <iostream>

TYPE_TO_STRING(robin_hood::unordered_flat_set<uint64_t>);
TYPE_TO_STRING(robin_hood::unordered_node_set<uint64_t>);

TEST_CASE_TEMPLATE("unordered_set_asserts", Set, robin_hood::unordered_flat_set<uint64_t>,
                   robin_hood::unordered_node_set<uint64_t>) {
    static_assert(std::is_same<typename Set::key_type, uint64_t>::value, "key_type same");
    static_assert(std::is_same<typename Set::value_type, uint64_t>::value, "value_type same");
}

TEST_CASE_TEMPLATE("unordered_set", Set, robin_hood::unordered_flat_set<uint64_t>,
                   robin_hood::unordered_node_set<uint64_t>) {

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

TEST_CASE_TEMPLATE("unordered_set_eq", Set, robin_hood::unordered_flat_set<std::string>,
                   robin_hood::unordered_node_set<std::string>) {
    Set set1;
    Set set2;
    REQUIRE(set1.size() == set2.size());
    REQUIRE(set1 == set2);
    REQUIRE(set2 == set1);

    set1.emplace("asdf");
    // (asdf) == ()
    REQUIRE(set1.size() != set2.size());
    REQUIRE(set1 != set2);
    REQUIRE(set2 != set1);

    set2.emplace("huh");
    // (asdf) == (huh)
    REQUIRE(set1.size() == set2.size());
    REQUIRE(set1 != set2);
    REQUIRE(set2 != set1);

    set1.emplace("huh");
    // (asdf, huh) == (huh)
    REQUIRE(set1.size() != set2.size());
    REQUIRE(set1 != set2);
    REQUIRE(set2 != set1);

    set2.emplace("asdf");
    // (asdf, huh) == (asdf, huh)
    REQUIRE(set1.size() == set2.size());
    REQUIRE(set1 == set2);
    REQUIRE(set2 == set1);

    set1.erase("asdf");
    // (huh) == (asdf, huh)
    REQUIRE(set1.size() != set2.size());
    REQUIRE(set1 != set2);
    REQUIRE(set2 != set1);

    set2.erase("asdf");
    // (huh) == (huh)
    REQUIRE(set1.size() == set2.size());
    REQUIRE(set1 == set2);
    REQUIRE(set2 == set1);

    set1.clear();
    // () == (huh)
    REQUIRE(set1.size() != set2.size());
    REQUIRE(set1 != set2);
    REQUIRE(set2 != set1);

    set2.erase("huh");
    // () == ()
    REQUIRE(set1.size() == set2.size());
    REQUIRE(set1 == set2);
    REQUIRE(set2 == set1);
}
