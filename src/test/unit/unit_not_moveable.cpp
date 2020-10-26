#include <robin_hood.h>

#include <app/doctest.h>

class NoCopyMove {
public:
    NoCopyMove() noexcept = default;
    explicit NoCopyMove(size_t d) noexcept
        : mData(d) {}
    ~NoCopyMove() = default;

    NoCopyMove(NoCopyMove const&) = delete;
    NoCopyMove& operator=(NoCopyMove const&) = delete;

    NoCopyMove(NoCopyMove&&) = delete;
    NoCopyMove& operator=(NoCopyMove&&) = delete;

    ROBIN_HOOD(NODISCARD) size_t data() const {
        return mData;
    }

private:
    size_t mData{};
};

TYPE_TO_STRING(robin_hood::unordered_node_map<size_t, NoCopyMove>);

// doesn't work with robin_hood::unordered_flat_map<size_t, NoCopyMove> because not movable and not
// copyable
TEST_CASE_TEMPLATE("not moveable", Map, robin_hood::unordered_node_map<size_t, NoCopyMove>) {
    // it's ok because it is movable.
    Map m;
    for (size_t i = 0; i < 100; ++i) {
        m[i];
        m.emplace(std::piecewise_construct, std::forward_as_tuple(i * 100),
                  std::forward_as_tuple(i));
    }
    REQUIRE(m.size() == 199);

    // not copyable, because m is not copyable!
    // Map m2 = m;

    // not movable
    // Map m2 = std::move(m);
    // REQUIRE(m2.size() == 199);
    m.clear();
    REQUIRE(m.size() == 0);
}
