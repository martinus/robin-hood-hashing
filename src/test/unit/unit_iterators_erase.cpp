#include <robin_hood.h>

#include <app/CtorDtorVerifier.h>
#include <app/doctest.h>

#include <unordered_set>

TYPE_TO_STRING(robin_hood::unordered_flat_map<CtorDtorVerifier, CtorDtorVerifier>);
TYPE_TO_STRING(robin_hood::unordered_node_map<CtorDtorVerifier, CtorDtorVerifier>);

TEST_CASE_TEMPLATE("erase iterator", Map,
                   robin_hood::unordered_node_map<CtorDtorVerifier, CtorDtorVerifier>,
                   robin_hood::unordered_flat_map<CtorDtorVerifier, CtorDtorVerifier>) {
    {
        Map map;
        for (uint64_t i = 0; i < 100; ++i) {
            map[i * 101] = i * 101;
        }

        typename Map::const_iterator it = map.find(20 * 101);
        REQUIRE(map.size() == 100);
        REQUIRE(map.end() != map.find(20 * 101));
        it = map.erase(it);
        REQUIRE(map.size() == 99);
        REQUIRE(map.end() == map.find(20 * 101));

        it = map.begin();
        size_t currentSize = map.size();
        std::unordered_set<uint64_t> keys;
        while (it != map.end()) {
            REQUIRE(keys.emplace(it->first.val()).second);
            it = map.erase(it);
            currentSize--;
            REQUIRE(map.size() == currentSize);
        }
        REQUIRE(map.size() == static_cast<size_t>(0));
    }
    REQUIRE(CtorDtorVerifier::mapSize() == static_cast<size_t>(0));
}
