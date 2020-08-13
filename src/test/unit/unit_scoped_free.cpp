#include <robin_hood.h>

#include <app/doctest.h>

namespace foo {

inline void free(void* /*unused*/) {
    FAIL("should never be called!");
}

inline void* malloc(size_t /*unused*/) {
    FAIL("should never be called!");
    return nullptr;
}

inline void* calloc(size_t /*unused*/, size_t /*unused*/) {
    FAIL("should never be called!");
    return nullptr;
}

inline void* realloc(void* /*unused*/, size_t /*unused*/) {
    FAIL("should never be called!");
    return nullptr;
}

inline void* reallocarray(void* /*unused*/, size_t /*unused*/, size_t /*unused*/) {
    FAIL("should never be called!");
    return nullptr;
}

enum bar { BAR_A, BAR_B };

TEST_CASE("scoped_free") {
    robin_hood::unordered_set<foo::bar> set{};
    robin_hood::unordered_map<foo::bar, int> map{};
}

} // namespace foo
