#include <robin_hood.h>

#include <app/doctest.h>

TEST_CASE("hash_bytes") {
    std::string str = "hello, world!";
    auto h1 = robin_hood::hash_bytes(str.data(), str.size());
    ++str.back();
    auto h2 = robin_hood::hash_bytes(str.data(), str.size());
    --str.back();
    ++str.front();
    auto h3 = robin_hood::hash_bytes(str.data(), str.size());
    REQUIRE(h1 != h2);
    REQUIRE(h1 != h3);
    REQUIRE(h2 != h3);
}
