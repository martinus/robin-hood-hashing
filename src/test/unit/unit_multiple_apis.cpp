#include <robin_hood.h>

#include <app/CtorDtorVerifier.h>
#include <app/doctest.h>
#include <app/randomseed.h>
#include <app/sfc64.h>

#include <unordered_map>
#include <utility>

TYPE_TO_STRING(robin_hood::unordered_flat_map<CtorDtorVerifier, CtorDtorVerifier>);
TYPE_TO_STRING(robin_hood::unordered_node_map<CtorDtorVerifier, CtorDtorVerifier>);

TEST_CASE_TEMPLATE("multiple_different_APIs" * doctest::test_suite("stochastic"), Map,
                   robin_hood::unordered_flat_map<CtorDtorVerifier, CtorDtorVerifier>,
                   robin_hood::unordered_node_map<CtorDtorVerifier, CtorDtorVerifier>) {
    Map rhhs;
    REQUIRE(rhhs.size() == static_cast<size_t>(0));
    std::pair<typename Map::iterator, bool> it_outer =
        rhhs.insert(typename Map::value_type{UINT64_C(32145), UINT64_C(123)});
    REQUIRE(it_outer.second);
    REQUIRE(it_outer.first->first.val() == 32145);
    REQUIRE(it_outer.first->second.val() == 123);
    REQUIRE(rhhs.size() == 1);

    const uint64_t times = 10000;
    for (uint64_t i = 0; i < times; ++i) {
        INFO(i);
        std::pair<typename Map::iterator, bool> it_inner =
            rhhs.insert(typename Map::value_type(i * 4, i));

        REQUIRE(it_inner.second);
        REQUIRE(it_inner.first->first.val() == i * 4);
        REQUIRE(it_inner.first->second.val() == i);

        typename Map::iterator found = rhhs.find(i * 4);
        REQUIRE(rhhs.end() != found);
        REQUIRE(found->second.val() == i);
        REQUIRE(rhhs.size() == 2 + i);
    }

    // check if everything can be found
    for (uint64_t i = 0; i < times; ++i) {
        typename Map::iterator found = rhhs.find(i * 4);
        REQUIRE(rhhs.end() != found);
        REQUIRE(found->second.val() == i);
        REQUIRE(found->first.val() == i * 4);
    }

    // check non-elements
    for (uint64_t i = 0; i < times; ++i) {
        typename Map::iterator found = rhhs.find((i + times) * 4);
        REQUIRE(rhhs.end() == found);
    }

    // random test against std::unordered_map
    rhhs.clear();
    std::unordered_map<uint64_t, uint64_t> uo;

    auto seed = randomseed();
    INFO("seed=" << seed);
    sfc64 gen(seed);

    for (uint64_t i = 0; i < times; ++i) {
        auto r = gen(times / 4);
        auto rhh_it = rhhs.insert(typename Map::value_type(r, r * 2));
        auto uo_it = uo.insert(std::make_pair(r, r * 2));
        REQUIRE(rhh_it.second == uo_it.second);
        REQUIRE(rhh_it.first->first.val() == uo_it.first->first);
        REQUIRE(rhh_it.first->second.val() == uo_it.first->second);
        REQUIRE(rhhs.size() == uo.size());

        r = gen(times / 4);
        typename Map::iterator rhhsIt = rhhs.find(r);
        auto uoIt = uo.find(r);
        REQUIRE((rhhs.end() == rhhsIt) == (uo.end() == uoIt));
        if (rhhs.end() != rhhsIt) {
            REQUIRE(rhhsIt->first.val() == uoIt->first);
            REQUIRE(rhhsIt->second.val() == uoIt->second);
        }
    }

    uo.clear();
    rhhs.clear();
    for (uint64_t i = 0; i < times; ++i) {
        const auto r = gen(times / 4);
        rhhs[r] = r * 2;
        uo[r] = r * 2;
        REQUIRE(rhhs.find(r)->second.val() == uo.find(r)->second);
        REQUIRE(rhhs.size() == uo.size());
    }

    std::size_t numChecks = 0;
    for (typename Map::const_iterator it = rhhs.begin(); it != rhhs.end(); ++it) {
        REQUIRE(uo.end() != uo.find(it->first.val()));
        ++numChecks;
    }
    REQUIRE(rhhs.size() == numChecks);

    numChecks = 0;
    const Map& constRhhs = rhhs;
    for (const typename Map::value_type& vt : constRhhs) {
        REQUIRE(uo.end() != uo.find(vt.first.val()));
        ++numChecks;
    }
    REQUIRE(rhhs.size() == numChecks);
}
