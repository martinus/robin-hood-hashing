

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
