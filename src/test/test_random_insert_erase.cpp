#include "test_base.h"

#include <unordered_map>
#include <unordered_set>

// final step from MurmurHash3
inline uint64_t fmix64(uint64_t k) {
    k ^= k >> 33;
    k *= 0xff51afd7ed558ccdULL;
    k ^= k >> 33;
    k *= 0xc4ceb9fe1a85ec53ULL;
    k ^= k >> 33;
    return k;
}

// from boost::hash_combine, with additional fmix64 of value
inline uint64_t hash_combine(uint64_t seed, uint64_t value) {
    return seed ^ (fmix64(value) + 0x9e3779b9 + (seed << 6) + (seed >> 2));
}

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

// randomly modifies a map and then checks if an std::unordered_map gave the same result.
TEMPLATE_TEST_CASE("random insert & erase", "", FlatMap, NodeMap,
                   (std::unordered_map<uint64_t, uint64_t>)) {
    Rng rng(123);

    TestType map;
    for (size_t i = 1; i < 10000; ++i) {
        map[rng(i)] = i;
        map.erase(rng(i));
    }

    // number generated with std::unordered_map
    REQUIRE(hash(map) == UINT64_C(0xf081c4121e9cb110));
}

class CtorDtorVerifier {
public:
    CtorDtorVerifier(uint64_t val)
        : mVal(val) {
        REQUIRE(mConstructedAddresses.insert(this).second);
        if (mDoPrintDebugInfo) {
            std::cout << this << " ctor(uint64_t) " << mConstructedAddresses.size() << std::endl;
        }
    }

    CtorDtorVerifier()
        : mVal(static_cast<uint64_t>(-1)) {
        REQUIRE(mConstructedAddresses.insert(this).second);
        if (mDoPrintDebugInfo) {
            std::cout << this << " ctor() " << mConstructedAddresses.size() << std::endl;
        }
    }

    CtorDtorVerifier(const CtorDtorVerifier& o)
        : mVal(o.mVal) {
        REQUIRE(mConstructedAddresses.insert(this).second);
        if (mDoPrintDebugInfo) {
            std::cout << this << " ctor(const CtorDtorVerifier& o) " << mConstructedAddresses.size()
                      << std::endl;
        }
    }

    CtorDtorVerifier& operator=(CtorDtorVerifier&& o) {
        REQUIRE(1 == mConstructedAddresses.count(this));
        REQUIRE(1 == mConstructedAddresses.count(&o));
        mVal = std::move(o.mVal);
        return *this;
    }

    CtorDtorVerifier& operator=(const CtorDtorVerifier& o) {
        REQUIRE(1 == mConstructedAddresses.count(this));
        REQUIRE(1 == mConstructedAddresses.count(&o));
        mVal = o.mVal;
        return *this;
    }

    bool operator==(const CtorDtorVerifier& o) const {
        return mVal == o.mVal;
    }

    bool operator!=(const CtorDtorVerifier& o) {
        return mVal != o.mVal;
    }

    ~CtorDtorVerifier() {
        REQUIRE(1 == mConstructedAddresses.erase(this));
        if (mDoPrintDebugInfo) {
            std::cout << this << " dtor " << mConstructedAddresses.size() << std::endl;
        }
    }

    bool eq(const CtorDtorVerifier& o) const {
        return mVal == o.mVal;
    }

    uint64_t val() const {
        return mVal;
    }

    static size_t mapSize() {
        return mConstructedAddresses.size();
    }

    static void printMap() {
        std::cout << "data in map:" << std::endl;
        for (std::unordered_set<CtorDtorVerifier const*>::const_iterator
                 it = mConstructedAddresses.begin(),
                 end = mConstructedAddresses.end();
             it != end; ++it) {
            std::cout << "\t" << *it << std::endl;
        }
    }

    static bool contains(CtorDtorVerifier const* ptr) {
        return 1 == mConstructedAddresses.count(ptr);
    }

    static bool mDoPrintDebugInfo;

private:
    uint64_t mVal;
    static std::unordered_set<CtorDtorVerifier const*> mConstructedAddresses;
};

template <>
inline uint64_t hash_value(CtorDtorVerifier const& value) {
    return value.val();
}

std::unordered_set<CtorDtorVerifier const*> CtorDtorVerifier::mConstructedAddresses;
bool CtorDtorVerifier::mDoPrintDebugInfo = false;

namespace std {
template <>
struct hash<CtorDtorVerifier> {
    std::size_t operator()(const CtorDtorVerifier& t) const {
        // hash is bad on purpose
        const size_t bitmaskWithoutLastBits = ~static_cast<size_t>(5);
        return static_cast<size_t>(t.val()) & bitmaskWithoutLastBits;
    }
};

} // namespace std

using FlatMapVerifier =
    robin_hood::unordered_flat_map<CtorDtorVerifier, CtorDtorVerifier, std::hash<CtorDtorVerifier>>;
using NodeMapVerifier =
    robin_hood::unordered_node_map<CtorDtorVerifier, CtorDtorVerifier, std::hash<CtorDtorVerifier>>;

TEMPLATE_TEST_CASE("random insert & erase", "", FlatMapVerifier, NodeMapVerifier) {
    using Map = TestType;

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
        std::pair<typename Map::iterator, bool> it_inner =
            rhhs.insert(typename Map::value_type(i * 4, i));

        REQUIRE(it_inner.second);
        REQUIRE(it_inner.first->first.val() == i * 4);
        REQUIRE(it_inner.first->second.val() == i);

        typename Map::iterator found = rhhs.find(i * 4);
        REQUIRE(rhhs.end() != found);
        REQUIRE(found->second.val() == i);
        REQUIRE(rhhs.size() == static_cast<size_t>(2 + i));
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

    Rng gen(123);

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

template <size_t S>
struct ShiftHash {
    size_t operator()(uint64_t const& t) const {
        return static_cast<size_t>(t >> S);
    }
};

TEMPLATE_TEST_CASE("insertion with simple hash", "",
                   (robin_hood::unordered_flat_map<uint64_t, uint64_t, ShiftHash<1>>),
                   (robin_hood::unordered_node_map<uint64_t, uint64_t, ShiftHash<1>>)) {
    TestType map;

    for (uint64_t i = 0; i < 4; ++i) {
        map[i] = i;
    }
    for (uint64_t i = 0; i < 4; ++i) {
        map.erase(i);
    }
}

template <typename Collection>
void print_keys(Collection&& col) {
    std::vector<int> v;
    for (auto const& kv : col) {
        v.push_back(kv.first);
    }
    std::sort(v.begin(), v.end());
    std::cout << "{";
    const char* prefix = "";
    for (auto k : v) {
        std::cout << prefix << k;
        prefix = ",";
    }
    std::cout << "}";
}

TEST_CASE("random insertion brute force", "[!hide]") {
    // find a problem
    size_t min_ops = 10000;
    uint64_t const n = 500;

    Rng rng{312};
    while (true) {
        size_t op = 0;
        auto state = rng.state();
        try {
            std::unordered_map<int, int> suo;
            robin_hood::unordered_node_map<int, int> ruo;

            while (++op < min_ops) {
                auto const key = rng.uniform<int>(n);
                if (op & 1) {
                    suo.erase(key);
                    ruo.erase(key);
                } else {
                    suo[key] = key;
                    ruo[key] = key;
                }
                if (suo.size() != ruo.size()) {
                    std::cout << "error after " << op << " ops, rng{" << sfc64{state} << "}"
                              << std::endl;
                    min_ops = op;
                }
            }

        } catch (std::overflow_error const&) {
            std::cout << "error after " << op << " ops, rng{" << sfc64{state} << "}" << std::endl;
            min_ops = op;
        }
    }
}

TEMPLATE_TEST_CASE("testing at()", "", (robin_hood::unordered_flat_map<uint64_t, size_t>),
                   (robin_hood::unordered_node_map<uint64_t, size_t>)) {
    TestType map;
    map[123] = 321;

    REQUIRE_THROWS_AS(map.at(666), std::out_of_range);
    REQUIRE(map.at(123) == 321);

    // same with const map
    auto const& cmap = map;
    REQUIRE_THROWS_AS(cmap.at(666), std::out_of_range);
    REQUIRE(cmap.at(123) == 321);
}