#include "robin_hood.h"

#include "doctest.h"

// not copyable, but movable.
struct NoCopy {
    NoCopy() {}
    NoCopy(int) {}

    ~NoCopy() = default;
    NoCopy(NoCopy const&) = delete;
    NoCopy& operator=(NoCopy const&) = delete;

    NoCopy(NoCopy&&) = default;
    NoCopy& operator=(NoCopy&&) = default;
};

TYPE_TO_STRING(robin_hood::unordered_flat_map<int, NoCopy>);
TYPE_TO_STRING(robin_hood::unordered_node_map<int, NoCopy>);

TEST_CASE_TEMPLATE("not copyable", Map, robin_hood::unordered_flat_map<int, NoCopy>,
                   robin_hood::unordered_node_map<int, NoCopy>) {
    // it's ok because it is movable.
    Map m;
    for (int i = 0; i < 100; ++i) {
        m[i];
        m.emplace(std::piecewise_construct, std::forward_as_tuple(i * 100),
                  std::forward_as_tuple(i));
    }
    REQUIRE(m.size() == 199);

    // not copyable, because m is not copyable!
    // Map m2 = m;

    // movable works
    Map m2 = std::move(m);
    REQUIRE(m2.size() == 199);
    m.clear();
    REQUIRE(m.size() == 0);
}
