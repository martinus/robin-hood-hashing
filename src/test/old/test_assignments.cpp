#include "test_base.h"

#include <vector>

// creates a map with some data in it
template <class M>
M createMap(int numElements) {
    M m;
    for (int i = 0; i < numElements; ++i) {
        m[static_cast<typename M::key_type>((i + 123) * 7)] =
            static_cast<typename M::mapped_type>(i);
    }
    return m;
}

struct NonCopyable {
    ~NonCopyable() = default;
    NonCopyable(NonCopyable const&) = delete;
    NonCopyable& operator=(NonCopyable const&) = delete;

    NonCopyable(NonCopyable&&) = default;
    NonCopyable& operator=(NonCopyable&&) = default;
};

struct NonCopyMove {
    NonCopyMove(int) {}

    NonCopyMove(NonCopyMove const&) = delete;
    NonCopyMove& operator=(NonCopyMove const&) = delete;

    NonCopyMove(NonCopyMove&&) = delete;
    NonCopyMove& operator=(NonCopyMove&&) = delete;
};

TEST_CASE("noncopyable") {
    // it's ok because it is movable.
    robin_hood::unordered_flat_map<int, NonCopyable> m;
    m[1];
}

TEST_CASE("nonmovable") {
    // not ok: can't copy and can't move
    // robin_hood::unordered_flat_map<int, NonCopyMove> m2;
    // m2.emplace(std::piecewise_construct, std::forward_as_tuple(123), std::forward_as_tuple(333));

    // it's ok because unordered_map defaults to unordered_node_map
    robin_hood::unordered_map<int, NonCopyMove> m;
    m.emplace(std::piecewise_construct, std::forward_as_tuple(123), std::forward_as_tuple(333));
}