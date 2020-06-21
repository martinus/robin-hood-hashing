#include <robin_hood.h>

#include <app/doctest.h>

// not copyable, but movable.
class NoCopy {
public:
    NoCopy() noexcept = default;
    explicit NoCopy(size_t d) noexcept
        : mData(d) {}

    ~NoCopy() = default;
    NoCopy(NoCopy const&) = delete;
    NoCopy& operator=(NoCopy const&) = delete;

    NoCopy(NoCopy&&) = default;
    NoCopy& operator=(NoCopy&&) = default;

    ROBIN_HOOD(NODISCARD) size_t data() const {
        return mData;
    }

private:
    size_t mData{};
};

TYPE_TO_STRING(robin_hood::unordered_flat_map<size_t, NoCopy>);
TYPE_TO_STRING(robin_hood::unordered_node_map<size_t, NoCopy>);

TEST_CASE_TEMPLATE("not copyable", Map, robin_hood::unordered_flat_map<size_t, NoCopy>,
                   robin_hood::unordered_node_map<size_t, NoCopy>) {
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

    // movable works
    Map m2 = std::move(m);
    REQUIRE(m2.size() == 199);
    m = Map{};
    REQUIRE(m.size() == 0);
}
