#include <robin_hood.h>

#include <memory>

#include <app/doctest.h>

template <typename Ptr>
void check(Ptr const& ptr) {
    REQUIRE(robin_hood::hash<Ptr>{}(ptr) ==
            robin_hood::hash<decltype(std::declval<Ptr>().get())>{}(ptr.get()));
}

TEST_CASE("unit_hash_smart_ptr") {
    check(std::unique_ptr<uint64_t>{});
    check(std::unique_ptr<uint64_t>{new uint64_t{123}});

    check(std::shared_ptr<uint64_t>{});
    check(std::shared_ptr<uint64_t>{new uint64_t{123}});
}
