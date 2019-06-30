#include <robin_hood.h>

#include <app/doctest.h>

#include <memory>

TYPE_TO_STRING(robin_hood::unordered_flat_map<size_t, std::unique_ptr<int>>);
TYPE_TO_STRING(robin_hood::unordered_node_map<size_t, std::unique_ptr<int>>);

TEST_CASE_TEMPLATE("unique_ptr", Map, robin_hood::unordered_flat_map<size_t, std::unique_ptr<int>>,
                   robin_hood::unordered_node_map<size_t, std::unique_ptr<int>>) {
    Map m;
    REQUIRE(m.end() == m.find(123));
    REQUIRE(m.end() == m.begin());
#if ROBIN_HOOD(CXX) >= ROBIN_HOOD(CXX14)
    m[static_cast<size_t>(32)] = std::make_unique<int>(123);
#else
    m[static_cast<size_t>(32)] = std::unique_ptr<int>(new int(123));
#endif
    REQUIRE(m.end() != m.begin());
    REQUIRE(m.end() == m.find(123));
    REQUIRE(m.end() != m.find(32));

    m = Map();
    REQUIRE(m.end() == m.begin());
    REQUIRE(m.end() == m.find(123));
    REQUIRE(m.end() == m.find(32));

    Map mEmpty;
    Map m3(std::move(mEmpty));
    REQUIRE(m3.end() == m3.begin());
    REQUIRE(m3.end() == m3.find(123));
    REQUIRE(m3.end() == m3.find(32));
    m3[static_cast<size_t>(32)];
    REQUIRE(m3.end() != m3.begin());
    REQUIRE(m3.end() == m3.find(123));
    REQUIRE(m3.end() != m3.find(32));

    mEmpty = Map{};
    Map m4(std::move(mEmpty));
    REQUIRE(m4.count(123) == 0);
    REQUIRE(m4.end() == m4.begin());
    REQUIRE(m4.end() == m4.find(123));
    REQUIRE(m4.end() == m4.find(32));

    for (auto const& kv : m) {
        REQUIRE(kv.first == 32);
        REQUIRE(kv.second != nullptr);
        REQUIRE(*kv.second == 123);
    }
}

TEST_CASE_TEMPLATE("unique_ptr fill", Map,
                   robin_hood::unordered_flat_map<size_t, std::unique_ptr<int>>,
                   robin_hood::unordered_node_map<size_t, std::unique_ptr<int>>) {

    Map m;
    for (int i = 0; i < 1000; ++i) {
        // m.emplace(i % 500, std::make_unique<int>(i));
        m.emplace(static_cast<size_t>(123), new int(i));
    }
}
