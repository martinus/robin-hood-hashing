#include <robin_hood.h>

#include <app/doctest.h>

namespace {

struct MyException : public std::exception {
    MyException(int /* unused */, std::string const& /* unused */) {}
};

} // namespace

TEST_CASE("assertNotNull") {
    REQUIRE_THROWS_AS(
        (robin_hood::detail::assertNotNull<std::runtime_error, uint64_t>(nullptr, "exception")),
        std::runtime_error);

    uint64_t x = 123;
    auto* a = robin_hood::detail::assertNotNull<std::bad_alloc, uint64_t>(&x);
    REQUIRE(a == &x);

    REQUIRE_THROWS_AS(
        (robin_hood::detail::assertNotNull<MyException, uint64_t>(nullptr, 123, "asdf")),
        MyException);
}
