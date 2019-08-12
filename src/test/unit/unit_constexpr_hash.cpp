#include <robin_hood.h>

#include <app/doctest.h>
#include <app/fmt/hex.h>

#include <iostream>

namespace robin_hood {
namespace compiletime_hash {

constexpr uint64_t xoshi_r(uint64_t h) {
    return h ^ (h >> 47U);
}

constexpr uint64_t mul_m(uint64_t h) {
    return h * UINT64_C(0xc6a4a7935bd1e995);
}

// reads N bytes, in little endian order
constexpr uint64_t fetch_le(char const* data, size_t n) {
    return n == 0 ? UINT64_C(0)
                  : (fetch_le(data + 1, n - 1) << 8U) | static_cast<uint64_t>(data[0]);
}

constexpr uint64_t fetch(char const* data, size_t n = 8) {
    return fetch_le(data, n);
}

// if 1-7 bytes remain, fetch them and mix them with the hash
constexpr uint64_t rest(const char* data, size_t n, uint64_t h) {
    return n == 0 ? h : mul_m(h ^ fetch(data, n));
}

// potentially fetches remaining 1-7 bytes
constexpr uint64_t fmix(uint64_t h) {
    return xoshi_r(mul_m(xoshi_r(h)));
}

constexpr uint64_t mix(uint64_t h, uint64_t k) {
    return mul_m(h ^ (mul_m(xoshi_r(mul_m(k)))));
}

// Control function for the mixing
constexpr uint64_t hash_block(const char* data, size_t n, uint64_t h) {
    return n >= 8 ? hash_block(data + 8, n - 8, mix(h, fetch(data))) : fmix(rest(data, n, h));
}

constexpr uint64_t hash_bytes(const char* data, size_t n) {
    return hash_block(data, n, UINT64_C(0xe17a1465) ^ mul_m(n));
}

} // namespace compiletime_hash

template <size_t N>
// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays,hicpp-avoid-c-arrays,modernize-avoid-c-arrays)
constexpr size_t hash_bytes(char const (&data)[N]) {
    return static_cast<size_t>(compiletime_hash::hash_bytes(data, N - 1));
}

} // namespace robin_hood

template <size_t S>
size_t forceCompileTime() {
    return S;
}

#define ROBIN_HOOD_HASH_CHECK(str) \
    REQUIRE(robin_hood::hash<std::string>{}(str) == forceCompileTime<robin_hood::hash_bytes(str)>())

TEST_CASE("constexpr_hash") {
    ROBIN_HOOD_HASH_CHECK("This is my test string. It's rather long, but that's ok.!");
    ROBIN_HOOD_HASH_CHECK("This is my test string. It's rather long, but that's ok.");
    ROBIN_HOOD_HASH_CHECK("This is my test string. It's rather long, but that's ok");
    ROBIN_HOOD_HASH_CHECK("This is my test string. It's rather long, but that's o");
    ROBIN_HOOD_HASH_CHECK("This is my test string. It's rather long, but that's ");
    ROBIN_HOOD_HASH_CHECK("This is my test string. It's rather long, but that's");
    ROBIN_HOOD_HASH_CHECK("This is my test string. It's rather long, but that'");
    ROBIN_HOOD_HASH_CHECK("This is my test string. It's rather long, but that");
    ROBIN_HOOD_HASH_CHECK("This is my test string. It's rather long, but tha");
    ROBIN_HOOD_HASH_CHECK("This is my test string. It's rather long, but th");
    ROBIN_HOOD_HASH_CHECK("This is my test string. It's rather long, but t");
    ROBIN_HOOD_HASH_CHECK("This is my test string. It's rather long, but ");
    ROBIN_HOOD_HASH_CHECK("This is my test string. It's rather long, but");
    ROBIN_HOOD_HASH_CHECK("This is my test string. It's rather long, bu");
    ROBIN_HOOD_HASH_CHECK("This is my test string. It's rather long, b");
    ROBIN_HOOD_HASH_CHECK("This is my test string. It's rather long, ");
    ROBIN_HOOD_HASH_CHECK("This is my test string. It's rather long,");
    ROBIN_HOOD_HASH_CHECK("This is my test string. It's rather long");
    ROBIN_HOOD_HASH_CHECK("This is my test string. It's rather lon");
    ROBIN_HOOD_HASH_CHECK("This is my test string. It's rather lo");
    ROBIN_HOOD_HASH_CHECK("This is my test string. It's rather l");
    ROBIN_HOOD_HASH_CHECK("This is my test string. It's rather ");
    ROBIN_HOOD_HASH_CHECK("This is my test string. It's rather");
    ROBIN_HOOD_HASH_CHECK("This is my test string. It's rathe");
    ROBIN_HOOD_HASH_CHECK("This is my test string. It's rath");
    ROBIN_HOOD_HASH_CHECK("This is my test string. It's rat");
    ROBIN_HOOD_HASH_CHECK("This is my test string. It's ra");
    ROBIN_HOOD_HASH_CHECK("This is my test string. It's r");
    ROBIN_HOOD_HASH_CHECK("This is my test string. It's ");
    ROBIN_HOOD_HASH_CHECK("This is my test string. It's");
    ROBIN_HOOD_HASH_CHECK("This is my test string. It'");
    ROBIN_HOOD_HASH_CHECK("This is my test string. It");
    ROBIN_HOOD_HASH_CHECK("This is my test string. I");
    ROBIN_HOOD_HASH_CHECK("This is my test string. ");
    ROBIN_HOOD_HASH_CHECK("This is my test string.");
    ROBIN_HOOD_HASH_CHECK("This is my test string");
    ROBIN_HOOD_HASH_CHECK("This is my test strin");
    ROBIN_HOOD_HASH_CHECK("This is my test stri");
    ROBIN_HOOD_HASH_CHECK("This is my test str");
    ROBIN_HOOD_HASH_CHECK("This is my test st");
    ROBIN_HOOD_HASH_CHECK("This is my test s");
    ROBIN_HOOD_HASH_CHECK("This is my test ");
    ROBIN_HOOD_HASH_CHECK("This is my test");
    ROBIN_HOOD_HASH_CHECK("This is my tes");
    ROBIN_HOOD_HASH_CHECK("This is my te");
    ROBIN_HOOD_HASH_CHECK("This is my t");
    ROBIN_HOOD_HASH_CHECK("This is my ");
    ROBIN_HOOD_HASH_CHECK("This is my");
    ROBIN_HOOD_HASH_CHECK("This is m");
    ROBIN_HOOD_HASH_CHECK("This is ");
    ROBIN_HOOD_HASH_CHECK("This is");
    ROBIN_HOOD_HASH_CHECK("This i");
    ROBIN_HOOD_HASH_CHECK("This ");
    ROBIN_HOOD_HASH_CHECK("This");
    ROBIN_HOOD_HASH_CHECK("Thi");
    ROBIN_HOOD_HASH_CHECK("Th");
    ROBIN_HOOD_HASH_CHECK("T");
    ROBIN_HOOD_HASH_CHECK("");
}
