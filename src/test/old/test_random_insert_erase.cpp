
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
