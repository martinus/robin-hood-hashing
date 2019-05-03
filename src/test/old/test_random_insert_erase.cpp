#include "test_base.h"

#include <unordered_map>
#include <unordered_set>

template <class T>
inline uint64_t hash_value(T const& value) {
    return value;
}

// calculates a hash of any iterable map. Order is irrelevant for the hash's result, as it simply
// xors the elements together.
template <typename M>
inline uint64_t hash(const M& map) {
    uint64_t combined_hash = 1;

    size_t numElements = 0;
    for (auto const& entry : map) {
        auto entry_hash = hash_combine(hash_value(entry.first), hash_value(entry.second));

        combined_hash ^= entry_hash;
        ++numElements;
    }

    return hash_combine(combined_hash, numElements);
}

template <>
inline uint64_t hash_value(CtorDtorVerifier const& value) {
    return value.val();
}

template <class M>
void fill(M& map, size_t num, bool isExisting = true) {
    if (isExisting) {
        for (size_t i = 0; i < num; ++i) {
            map[static_cast<typename M::key_type>(i)];
        }
    } else {
        for (size_t i = 0; i < num; ++i) {
            map[static_cast<typename M::key_type>(i + num)];
        }
    }
}

TEMPLATE_TEST_CASE("assign, iterate, clear", "", FlatMapVerifier, NodeMapVerifier) {
    using Map = TestType;
    {
        Map m;
        fill(m, 1);
        for (typename Map::const_iterator it = m.begin(); it != m.end(); ++it) {
            REQUIRE(CtorDtorVerifier::contains(&it->first));
            REQUIRE(CtorDtorVerifier::contains(&it->second));
        }
        // dtor
    }
    if (0 != CtorDtorVerifier::mapSize()) {
        CtorDtorVerifier::printMap();
    }
    REQUIRE(CtorDtorVerifier::mapSize() == 0);
}

// make sure RNG is deterministic
TEST_CASE("rng") {
    Rng rng(123);
    uint64_t h = 0;
    for (size_t i = 1; i < 10000; ++i) {
        h = hash_combine(h, rng(i));
        h = hash_combine(h, rng(i));
        h = hash_combine(h, rng(i));
    }
    REQUIRE(h == UINT64_C(0x3d7c20fe1135733e));
    REQUIRE(rng() == UINT64_C(0x4001279087d5fe55));
}

TEMPLATE_TEST_CASE("random insert & erase with Verifier", "", FlatMapVerifier, NodeMapVerifier,
                   (std::unordered_map<CtorDtorVerifier, CtorDtorVerifier>)) {
    Rng rng(123);

    REQUIRE(CtorDtorVerifier::mapSize() == 0);
    TestType map;
    for (size_t i = 1; i < 10000; ++i) {
        auto v = rng(i);
        auto k = rng(i);
        map[k] = v;
        map.erase(rng(i));
    }

    std::cout << "map.size()=" << map.size() << std::endl;
    INFO("map size is " << CtorDtorVerifier::mapSize());
    REQUIRE(CtorDtorVerifier::mapSize() == 6572);
    REQUIRE(hash(map) == UINT64_C(0xd665be038c0ed434));
    map.clear();

    REQUIRE(CtorDtorVerifier::mapSize() == 0);
    REQUIRE(hash(map) == UINT64_C(0x9e3779f8));
}

TEMPLATE_TEST_CASE("randins", "", (std::unordered_map<uint64_t, uint64_t>)) {
    Rng rng(123);

    TestType map;
    for (size_t i = 1; i < 10; ++i) {
        auto v = rng(i);
        auto k = rng(i);
        map[k] = v;
        map.erase(rng(i));
    }

    std::cout << "map.size()=" << map.size() << std::endl;
    map.clear();
}

// Dummy hash for testing collisions. Make sure to use robin_hood::hash, because this doesn't
// use a bad_hash_prevention multiplier.
namespace robin_hood {

template <>
struct hash<CtorDtorVerifier> {
    std::size_t operator()(const CtorDtorVerifier&) const {
        return 123;
    }
};

} // namespace robin_hood

TEMPLATE_TEST_CASE("collisions", "",
                   (robin_hood::unordered_flat_map<CtorDtorVerifier, CtorDtorVerifier>),
                   (robin_hood::unordered_node_map<CtorDtorVerifier, CtorDtorVerifier>)) {

    static const uint64_t max_val = 127;

    // CtorDtorVerifier::mDoPrintDebugInfo = true;
    {
        TestType m;
        for (uint64_t i = 0; i < max_val; ++i) {
            INFO(i);
            m[i];
        }
        REQUIRE(m.size() == max_val);
        REQUIRE_THROWS_AS(m[max_val], std::overflow_error);
        REQUIRE(m.size() == max_val);
    }
    if (0 != CtorDtorVerifier::mapSize()) {
        CtorDtorVerifier::printMap();
    }
    REQUIRE(CtorDtorVerifier::mapSize() == 0);

    {
        TestType m;
        for (uint64_t i = 0; i < max_val; ++i) {
            REQUIRE(m.insert(typename TestType::value_type(i, i)).second);
        }
        REQUIRE(m.size() == max_val);
        REQUIRE_THROWS_AS(m.insert(typename TestType::value_type(max_val, max_val)),
                          std::overflow_error);
        REQUIRE(m.size() == max_val);
    }
    if (0 != CtorDtorVerifier::mapSize()) {
        CtorDtorVerifier::printMap();
    }
    REQUIRE(CtorDtorVerifier::mapSize() == 0);
}

TEMPLATE_TEST_CASE("erase iterator", "", FlatMapVerifier, NodeMapVerifier) {
    {
        TestType map;
        for (uint64_t i = 0; i < 100; ++i) {
            map[i * 101] = i * 101;
        }

        typename TestType::const_iterator it = map.find(20 * 101);
        REQUIRE(map.size() == 100);
        REQUIRE(map.end() != map.find(20 * 101));
        it = map.erase(it);
        REQUIRE(map.size() == 99);
        REQUIRE(map.end() == map.find(20 * 101));

        it = map.begin();
        size_t currentSize = map.size();
        while (it != map.end()) {
            it = map.erase(it);
            currentSize--;
            REQUIRE(map.size() == currentSize);
        }
        REQUIRE(map.size() == static_cast<size_t>(0));
    }
    REQUIRE(CtorDtorVerifier::mapSize() == static_cast<size_t>(0));
}

TEMPLATE_TEST_CASE("test vector", "", FlatMapVerifier, NodeMapVerifier) {
    {
        std::vector<TestType> maps;
        for (size_t i = 0; i < 10; ++i) {
            TestType m;
            fill(m, 100);
            maps.push_back(m);
        }
    }
    REQUIRE(CtorDtorVerifier::mapSize() == static_cast<size_t>(0));
}

TEMPLATE_TEST_CASE("maps of maps", "", FlatMapVerifier, NodeMapVerifier) {
    {
        robin_hood::unordered_map<CtorDtorVerifier, TestType> maps;
        for (uint64_t i = 0; i < 10; ++i) {
            fill(maps[i], 100);
        }

        robin_hood::unordered_map<CtorDtorVerifier, TestType> maps2;
        maps2 = maps;
        REQUIRE(maps2 == maps);
    }
    REQUIRE(CtorDtorVerifier::mapSize() == static_cast<size_t>(0));
}
