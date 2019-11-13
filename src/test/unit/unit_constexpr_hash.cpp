#include <robin_hood.h>

#include <app/doctest.h>
#include <app/fmt/hex.h>

#include <iostream>
#include <limits>

#if defined(_MSC_VER)
// warning C4307: '*': integral constant overflow
#    define ROBIN_HOOD_NO_WARN_INTEGRAL_CONSTANT_BEGIN() \
        __pragma(warning(push)) __pragma(warning(disable : 4307))
#    define ROBIN_HOOD_NO_WARN_INTEGRAL_CONSTANT_END() __pragma(warning(pop))
#else
#    define ROBIN_HOOD_NO_WARN_INTEGRAL_CONSTANT_BEGIN()
#    define ROBIN_HOOD_NO_WARN_INTEGRAL_CONSTANT_END()
#endif

ROBIN_HOOD_NO_WARN_INTEGRAL_CONSTANT_BEGIN()

// NOLINTNEXTLINE(modernize-concat-nested-namespaces)
namespace robin_hood {

// contains C++11 compatible compile time calculations.
namespace compiletime {
namespace hash {

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

constexpr uint64_t fmix(uint64_t h) {
    return xoshi_r(mul_m(xoshi_r(h)));
}

constexpr uint64_t mix(uint64_t h, uint64_t k) {
    return mul_m(h ^ (mul_m(xoshi_r(mul_m(k)))));
}

// Control function for the mixing
constexpr uint64_t block(const char* data, size_t n, uint64_t h) {
    return n >= 8 ? block(data + 8, n - 8, mix(h, fetch(data))) : fmix(rest(data, n, h));
}

constexpr uint64_t calc(const char* data, size_t n, uint64_t seed) {
#if defined(_MSC_VER)
#    pragma warning(push)
#    pragma warning(disable : 4307)
#endif
    return block(data, n, seed ^ mul_m(n));
#if defined(_MSC_VER)
#    pragma warning(pop)
#endif
}

} // namespace hash

constexpr const char* forward(const char* data, size_t len) {
    return len == 0 || *data == '\0' ? data : forward(data + 1, len - 1);
}

constexpr size_t strlen(char const* data, size_t offset = 0, size_t level = 64) noexcept {
    return data[offset] == '\0'
               ? offset
               : level == 0 ? offset + 1
                            : strlen(data, strlen(data, offset + 1, level - 1), level - 1);
}

constexpr size_t hash_bytes(char const* data) {
    return static_cast<size_t>(hash::calc(data, strlen(data), UINT64_C(0xe17a1465)));
}

} // namespace compiletime
} // namespace robin_hood

TEST_CASE("constexpr_strlen") {
    constexpr auto x = "x";
    constexpr auto hx = robin_hood::compiletime::hash_bytes(x);
    REQUIRE(hx == robin_hood::hash_bytes(x, strlen(x)));
    REQUIRE(strlen(x) == robin_hood::compiletime::strlen(x));

    constexpr auto hello = "hello";
    constexpr auto hhello = robin_hood::compiletime::hash_bytes(hello);
    REQUIRE(hhello == robin_hood::hash_bytes(hello, strlen(hello)));
    REQUIRE(strlen(hello) == robin_hood::compiletime::strlen(hello));

    constexpr auto wasdf = "kkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkk"
                           "k kkkkkkkkkkkkkkkkkkkkkkakkkkkkkkkkk askdf kskdfk askdf kksakd "
                           "fksk dksakdfkaskd fkskadf kskxxxdfk skad fkksdfk ksdfk ksadkfk s "
                           "kdfkasdfk ksadfk ksakskdkasdfkaksdfk askdfk aksdfksadfksaskdfkas";
    constexpr auto hwasdf = robin_hood::compiletime::hash_bytes(wasdf);
    REQUIRE(hwasdf == robin_hood::hash_bytes(wasdf, strlen(wasdf)));
    REQUIRE(strlen(wasdf) == robin_hood::compiletime::strlen(wasdf));

    constexpr auto txt =
        "012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789"
        "012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789"
        "012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789"
        "012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789"
        "012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789"
        "012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789"
        "012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789"
        "012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789"
        "012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789"
        "012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789"
        "012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789"
        "012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789"
        "012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789"
        "012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789"
        "012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789"
        "012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789"
        "012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789"
        "012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789"
        "012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789"
        "012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789"
        "012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789"
        "012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789"
        "012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789"
        "012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789"
        "012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789"
        "012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789"
        "012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789"
        "012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789"
        "012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789"
        "012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789"
        "012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789"
        "012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789"
        "012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789"
        "012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789"
        "012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789"
        "012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789"
        "012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789"
        "012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789"
        "012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789"
        "012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789"
        "012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789"
        "01234567890123456789012345678901234567890123456789012345678901234567890123456789012345678"
        "9";

    constexpr auto htxt = robin_hood::compiletime::hash_bytes(txt);
    REQUIRE(htxt == robin_hood::hash_bytes(txt, strlen(txt)));
    REQUIRE(strlen(txt) == robin_hood::compiletime::strlen(txt));
}

TEST_CASE("constexpr_hash_usage") {
    std::string str = "affaseaseasasd fsdflaksdlflsdf";
    switch (robin_hood::hash_bytes(str.data(), str.size())) {
    case robin_hood::compiletime::hash_bytes("affaseaseasasd fsdflaksdlflsdf"): // ok
        break;

    default:
        FAIL("wrong case");
        break;
    }
}

#define ROBIN_HOOD_HASH_CHECK(str) \
    REQUIRE(robin_hood::hash<std::string>{}(str) == robin_hood::compiletime::hash_bytes(str))

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

ROBIN_HOOD_NO_WARN_INTEGRAL_CONSTANT_END()
