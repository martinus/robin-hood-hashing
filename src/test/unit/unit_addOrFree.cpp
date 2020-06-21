#include <robin_hood.h>

#include <app/doctest.h>

TEST_CASE("bulkpoolallocator addOrFree") {
    robin_hood::detail::BulkPoolAllocator<uint64_t> pool;

    // add data, some too small some large enough
    for (size_t i = 1; i < 10; ++i) {
        auto* x = malloc(i);
        pool.addOrFree(x, i);
    }

    for (size_t i = 0; i < 100; ++i) {
        REQUIRE(nullptr != pool.allocate());
    }

    // dtor should have freed everything
}
