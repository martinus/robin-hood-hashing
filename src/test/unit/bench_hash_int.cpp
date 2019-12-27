#include <robin_hood.h>

#include <app/benchmark.h>
#include <app/doctest.h>
#include <app/sfc64.h>
#include <thirdparty/nanobench/nanobench.h>

#include <array>
#include <atomic>
#include <bitset>
#include <chrono>
#include <iomanip>
#include <nmmintrin.h>
#include <thread>
#include <unordered_map>
#include <unordered_set>

#include <immintrin.h>

namespace {
template <typename T, typename R>
uint64_t ror(T v, R r) {
    return (v >> static_cast<int>(r)) |
           (v << (static_cast<int>(sizeof(T)) * 8 - static_cast<int>(r)));
}

} // namespace

// 17168: accepted=(0.00629202, 0.00146349)=0.00303452, best=(0.00672049, 0.00130806)=0.00296493
// UINT64_C(0xddd442448b71ddf0)
class Mul128Xor {
public:
    static constexpr char const* name = "Mul128Xor";

    Mul128Xor()
        : mFactors{0xddd442448b71ddf0} {}

    uint64_t operator()(uint64_t v) const {
        uint64_t h;
        uint64_t l = robin_hood::detail::umul128(v, mFactors[0], &h);
        return h ^ l;
    }
    std::array<uint64_t, 1> mFactors;
};
TYPE_TO_STRING(Mul128Xor);

class Mul128XorMul {
public:
    static constexpr char const* name = "Mul128XorMul";

    Mul128XorMul()
        : mFactors{0xea054c65f6328a3b} {}

    uint64_t operator()(uint64_t v) const noexcept {
        uint64_t h;
        uint64_t l = robin_hood::detail::umul128(v, mFactors[0], &h);
        return (h ^ l) * mFactors[0];
    }
    std::array<uint64_t, 1> mFactors;
};
TYPE_TO_STRING(Mul128XorMul);

class Mul {
public:
    static constexpr char const* name = "Mul";

    Mul()
        : mFactors{0x1123a060916bdddd} {}

    uint64_t operator()(uint64_t v) const {
        return v * mFactors[0];
    }
    std::array<uint64_t, 1> mFactors;
};
TYPE_TO_STRING(Mul);

class Mul128Add {
public:
    static constexpr char const* name = "Mul128Add";

    Mul128Add()
        : mFactors{0x1123a060916bdddd} {}

    uint64_t operator()(uint64_t v) const {
        uint64_t h;
        uint64_t l = robin_hood::detail::umul128(v, mFactors[0], &h);
        return h + l;
    }
    std::array<uint64_t, 1> mFactors;
};
TYPE_TO_STRING(Mul128Add);

class MartinusHash1 {
public:
    static constexpr char const* name = "MartinusHash1";

    MartinusHash1()
        : mFactors{0x15b3b38d7e675129} {}

    uint64_t operator()(uint64_t v) const {
        uint64_t h;
        uint64_t l = robin_hood::detail::umul128(v * mFactors[0], v ^ mFactors[0], &h);
        return h ^ l;
    }

    std::array<uint64_t, 1> mFactors;
};
TYPE_TO_STRING(MartinusHash1);

class MartinusHash2 {
public:
    static constexpr char const* name = "MartinusHash2";

    MartinusHash2()
        : mFactors{0x15b3b38d7e675129, 0xea054c65f6328a3b} {}

    uint64_t operator()(uint64_t v) const {
        uint64_t h;
        uint64_t l;

        l = robin_hood::detail::umul128(v * mFactors[0], v ^ ror(v, (mFactors[1] & 63)), &h);

        return h ^ l;
    }

    std::array<uint64_t, 2> mFactors;
};
TYPE_TO_STRING(MartinusHash2);

class Murmur {
public:
    static constexpr char const* name = "Murmur";

    Murmur()
        : mFactors{33, UINT64_C(0xff51afd7ed558ccd), 33, UINT64_C(0xc4ceb9fe1a85ec53), 33} {}

    uint64_t operator()(uint64_t h) const {
        h ^= h >> (mFactors[0] & 0x3f);
        h *= mFactors[1];
        h ^= h >> (mFactors[2] & 0x3f);
        h *= mFactors[3];
        h ^= h >> (mFactors[4] & 0x3f);
        return h;
    }

    std::array<uint64_t, 5> mFactors;
};
TYPE_TO_STRING(Murmur);

class Aes {
public:
    static constexpr char const* name = "Aes";
    Aes()
        : mFactors{0} {}

    uint64_t operator()(uint64_t key) const {
        __m128i hash_64 = _mm_set1_epi64x(static_cast<int64_t>(key));
        for (int i = 0; i < 10; ++i) {
            hash_64 = _mm_aesenc_si128(hash_64, _mm_set1_epi32(static_cast<int>(mFactors[0])));
        }

        uint64_t result;
        std::memcpy(&result, &hash_64, sizeof(uint64_t));
        return result;
    }

    std::array<uint64_t, 1> mFactors;
};
TYPE_TO_STRING(Aes);

class Crc {
public:
    static constexpr char const* name = "Crc";

    Crc()
        : mFactors{UINT64_C(0x1f260ee53711dc7b)} {}

    uint64_t operator()(uint64_t v) const {
        return _mm_crc32_u64(0, v) * mFactors[0];
    }

    std::array<uint64_t, 1> mFactors;
};
TYPE_TO_STRING(Crc);

class CrcMul128 {
public:
    static constexpr char const* name = "CrcMul128";

    CrcMul128()
        : mFactors{UINT64_C(1)} {}

    uint64_t operator()(uint64_t v) const {
        uint64_t h;
        uint64_t l = robin_hood::detail::umul128(_mm_crc32_u64(0, v), mFactors[0], &h);
        return l ^ h;
    }

    std::array<uint64_t, 1> mFactors;
};
TYPE_TO_STRING(CrcMul128);

class StronglyUniversal {
public:
    static constexpr char const* name = "StronglyUniversal";

    StronglyUniversal()
        : mFactors{UINT64_C(0x8532c5125ef9bbee), UINT64_C(0x61af3eb1d51b4e89),
                   UINT64_C(0x74d28e483bf43464), UINT64_C(0xf8eea90faec9c802),
                   UINT64_C(0x8f78acfe4dc2982c), UINT64_C(0x11ae6912bca65735)} {}
    uint64_t operator()(uint64_t x) const noexcept {
        auto lo = x & UINT64_C(0x00000000ffffffff);
        auto hi = x >> 32;

        auto r1 = (mFactors[0] * lo + mFactors[1] * hi + mFactors[2]) >> 32;
        auto r2 = (mFactors[3] * lo + mFactors[4] * hi + mFactors[5]) >> 32;
        return (r1 << 32) | r2;
    }

public:
    std::array<uint64_t, 6> mFactors;
};
TYPE_TO_STRING(StronglyUniversal);

class Rrmxmx {
public:
    static constexpr char const* name = "Rrmxmx";

    Rrmxmx()
        : mFactors{49, 24, 0x9FB21C651E98DF25L, 28, 0x9FB21C651E98DF25L, 28} {}

    uint64_t operator()(uint64_t v) const {
        v ^= ror(v, (mFactors[0] & 0x3f)) ^ ror(v, (mFactors[1] & 0x3f));
        v *= mFactors[2];
        v ^= v >> (mFactors[3] & 0x3f);
        v *= mFactors[4];
        return v ^ v >> (mFactors[5] & 0x3f);
    }
    std::array<uint64_t, 6> mFactors;

private:
};
TYPE_TO_STRING(Rrmxmx);

namespace {

/*
clang-format off
|               ns/op |                op/s |    err% |          ins/op |          cyc/op |    IPC |         bra/op |   miss% |     total | benchmark
|--------------------:|--------------------:|--------:|----------------:|----------------:|-------:|---------------:|--------:|----------:|:----------
|                0.74 |    1,344,675,247.44 |    0.1% |            5.00 |            2.38 |  2.103 |           0.00 |    0.0% |      0.00 | `robin_hood::hash`
|                0.74 |    1,344,659,363.23 |    0.1% |            5.00 |            2.38 |  2.104 |           0.00 |    0.0% |      0.00 | `robin_hood::hash_int`
|                0.94 |    1,064,064,375.58 |    0.0% |            6.00 |            3.00 |  1.999 |           0.00 |    0.0% |      0.00 | `hash2`
|                1.88 |      532,042,371.66 |    0.0% |           17.00 |            6.00 |  2.831 |           0.00 |    0.0% |      0.00 | `stronglyuniversal`
|                1.25 |      798,206,087.40 |    0.0% |           12.00 |            4.00 |  2.998 |           0.00 |    0.0% |      0.00 | `murmurhash3`
|                1.13 |      886,007,536.69 |    0.1% |           11.00 |            3.61 |  3.049 |           0.00 |    0.0% |      0.00 | `rrmxmx`
|                1.40 |      713,724,930.41 |    0.4% |           13.00 |            4.47 |  2.908 |           0.00 |    0.0% |      0.00 | `twang_mix64`
|                0.94 |    1,064,695,420.19 |    0.0% |            4.00 |            3.00 |  1.333 |           0.00 |    0.0% |      0.00 | `crc`
|                0.94 |    1,064,450,686.64 |    0.0% |            5.00 |            3.00 |  1.666 |           0.00 |    0.0% |      0.00 | `perfecthash64`
clang-format on
*/

uint64_t hash2(uint64_t v) {
    uint64_t h;
    uint64_t l = robin_hood::detail::umul128(v, UINT64_C(0x6fe29d68f57b9eb5), &h);
    return static_cast<size_t>(h ^ l) * UINT64_C(0x6fe29d68f57b9eb5);
}

// 0.00010808544474592668395 chi^2 after 1984000000 evaluations
template <typename T>
size_t murmurhash3(T x) noexcept {
    uint64_t obj = static_cast<uint64_t>(x);
    uint64_t h = obj;
    h ^= h >> 33;
    h *= 0xff51afd7ed558ccd;
    h ^= h >> 33;
    h *= 0xc4ceb9fe1a85ec53;
    h ^= h >> 33;
    return static_cast<size_t>(h);
}

uint64_t stronglyuniversal(uint64_t x) noexcept {
    auto lo = x & UINT64_C(0x00000000ffffffff);
    auto hi = x >> 32;

    auto r1 = (UINT64_C(0x8532c5125ef9bbee) * lo + UINT64_C(0x61af3eb1d51b4e89) * hi +
               UINT64_C(0xf8eea90faec9c802)) >>
              32;
    auto r2 = (UINT64_C(0x74d28e483bf43464) * lo + UINT64_C(0x8f78acfe4dc2982c) * hi +
               UINT64_C(0x11ae6912bca65735)) >>
              32;
    return (r1 << 32) | r2;
}

/*
  0,01 │ 240:┌─→movabs     $0xb53168ca67169575,%rcx
 24,79 │     │  imul       %rax,%rcx
  0,28 │     │  movabs     $0xb53168ca67169575,%rsi
  0,01 │     │  xor        %rsi,%rax
       │     │  mov        %rcx,%rdx
 54,39 │     │  mulx       %rax,%rcx,%rbx
  0,00 │     │  mov        %rbx,%rax
 15,91 │     │  xor        %rcx,%rax
  4,56 │     └──jne        240
*/
uint64_t martinushash1(uint64_t v) noexcept {
    constexpr auto k = UINT64_C(0xb53168ca67169575);
    uint64_t h;
    uint64_t l = robin_hood::detail::umul128(v * k, v ^ k, &h);
    return h ^ l;
}

uint64_t martinushash2(uint64_t v) noexcept {
    static constexpr auto k = UINT64_C(0x9c7a6e64d27cb4ab);
    uint64_t h;
    uint64_t l;

    l = robin_hood::detail::umul128(v * k, v ^ ror(v, 49) ^ ror(v, 24), &h);
    v = h ^ l;

    return v;
}

// 64-bit perfect hash
uint64_t aes(uint64_t key) {
    __m128i hash_64 = _mm_set1_epi64x(static_cast<int64_t>(key));
    hash_64 = _mm_aesenc_si128(hash_64, _mm_set1_epi32(-559038737));
    hash_64 = _mm_aesenc_si128(hash_64, _mm_set1_epi32(-559038737));
    hash_64 = _mm_aesenc_si128(hash_64, _mm_set1_epi32(-559038737));

    uint64_t result;
    std::memcpy(&result, &hash_64, sizeof(uint64_t));
    return result;
}

// 4.2434522807416192894e-07 chi^2 after 1664000000 evaluations
uint64_t rrmxmx(uint64_t v) {
    v ^= ror(v, 49) ^ ror(v, 24);
    v *= 0x9FB21C651E98DF25L;
    v ^= v >> 28;
    v *= 0x9FB21C651E98DF25L;
    return v ^ v >> 28;
}

// 0.013322717782640111986 chi^2 after 1088000000 evaluations
uint64_t twang_mix64(uint64_t key) noexcept {
    key = (~key) + (key << 21); // key *= (1 << 21) - 1; key -= 1;
    key = key ^ (key >> 24);
    key = key + (key << 3) + (key << 8); // key *= 1 + (1 << 3) + (1 << 8)
    key = key ^ (key >> 14);
    key = key + (key << 2) + (key << 4); // key *= 1 + (1 << 2) + (1 << 4)
    key = key ^ (key >> 28);
    key = key + (key << 31); // key *= 1 + (1 << 31)
    return key;
}

/*
       │ 3f0:┌─→mov        %rax,%rdx
 47,99 │     │  crc32q     0x8(%rsp),%rdx
  0,04 │     │  movabs     $0x9236b9136cd64e6b,%rcx
 31,00 │     │  imul       %rcx,%rdx
 20,92 │     │  mov        %rdx,0x8(%rsp)
  0,02 │     └──jne        3f0
*/
uint64_t crc(uint64_t v) noexcept {
    return _mm_crc32_u64(0, v) * UINT64_C(0x9236b9136cd64e6b);
}

uint64_t crcmul128(uint64_t v) noexcept {
    uint64_t h;
    uint64_t l = robin_hood::detail::umul128(_mm_crc32_u64(0, v), UINT64_C(0x9236b9136cd64e6b), &h);
    return l ^ h;
}

template <size_t S>
std::ostream& operator<<(std::ostream& os, std::array<uint64_t, S> const& factors) {
    auto prefix = "";
    for (auto f : factors) {
        os << prefix << std::hex << "UINT64_C(0x" << f << ")" << std::dec;
        prefix = ", ";
    }
    return os;
}

} // namespace

TEST_CASE("bench_hash_int" * doctest::test_suite("nanobench") * doctest::skip()) {
    using namespace std::chrono_literals;
    // add a (neglectible) bit of randomization so the compiler can't optimize this away

    // static constexpr auto c = UINT64_C(0xb81fd30ffccab4af);
    ankerl::nanobench::Config cfg;
    // cfg.minEpochTime(200ms);

    sfc64 rng;
    auto n = rng();
    cfg.run("martinushash1", [&] { n = martinushash1(n); });
    cfg.run("martinushash2", [&] { n = martinushash2(n); });
    cfg.run("crc", [&] { n = crc(n); });
    cfg.run("stronglyuniversal", [&] { n = stronglyuniversal(n); });
    cfg.run("robin_hood::hash", [&] { n = robin_hood::hash<uint64_t>{}(n); });
    cfg.run("robin_hood::hash_int", [&] { n = robin_hood::hash_int(n); });
    cfg.run("hash2", [&] { n = hash2(n); });
    cfg.run("murmurhash3", [&] { n = murmurhash3(n); });
    cfg.run("rrmxmx", [&] { n = rrmxmx(n); });
    cfg.run("twang_mix64", [&] { n = twang_mix64(n); });
    cfg.run("aes", [&] { n = aes(n); });
    cfg.run("crcmul128", [&] { n = crcmul128(n); });

    ankerl::nanobench::doNotOptimizeAway(n);
}

namespace {

// calculates (n over k), see
// https://en.wikipedia.org/wiki/Binomial_coefficient#Computing_the_value_of_binomial_coefficients
double binomialCoefficient(size_t n, size_t k) {
    // since B is symmetric, optimize by setting k to the smallest
    k = std::min(k, n - k);

    double prod = 1.0;
    for (size_t i = 1; i <= k; ++i) {
        prod *= static_cast<double>(n + 1U - i) / static_cast<double>(i);
    }
    return prod;
}

std::vector<double> calculateBinomialDistributionProbabilities(size_t numberOfTrials,
                                                               double successProbabilityEachTrial) {

    std::vector<double> results;
    for (size_t k = 0; k <= numberOfTrials; ++k) {
        auto r = binomialCoefficient(numberOfTrials, k) *
                 std::pow(successProbabilityEachTrial, static_cast<double>(k)) *
                 std::pow(1 - successProbabilityEachTrial, static_cast<double>(numberOfTrials - k));
        results.push_back(r);
    }

    return results;
}

// Will bin in C++20 https://en.cppreference.com/w/cpp/numeric/popcount
template <typename T>
size_t popcount(T v) noexcept {
    // awesomely, this optimize to the popcnt instruction.
    return std::bitset<8 * sizeof(T)>(v).count();
}

// Mutates input by randomly flipping a few bits
template <size_t S>
void mutate(std::array<uint64_t, S>* vals, sfc64* rng, RandomBool* rbool) {
    do {
        uint64_t mask = 0;
        do {
            mask |= UINT64_C(1) << ((*rng)(62) + 1);
        } while (0 != (*rng)(3));
        (*vals)[rng->uniform<size_t>(vals->size())] ^= mask;
    } while ((*rbool)(*rng));
    if ((*rng)(10) == 0) {
        (*vals)[rng->uniform<size_t>(vals->size())] = (*rng)();
    }
}

template <size_t S>
void mutateBitFlipProbability(std::array<uint64_t, S>* vals, sfc64* rngPtr, RandomBool*) {
    // flip probability 0b111111: (1 - 1/64)^64 = 13% no-flip probability
    auto& rng = *rngPtr;

    // flip at least one bit
    (*vals)[rng(vals->size())] ^= UINT64_C(1) << rng(64);

    for (auto& v : *vals) {
        for (size_t b = 0; b < 64; ++b) {
            if (0 == (rng() & 0b111111)) {
                v ^= UINT64_C(1) << b;
            }
        }
    }
}

std::vector<uint64_t> generateUniqueLowEntrophyNumbers(size_t bits) {
    auto constexpr BitsInType = 64;

    robin_hood::unordered_flat_map<uint64_t, char> set;

    std::vector<uint64_t> vec;

    auto iters = 1U << bits;
    for (uint64_t i = 0; i < iters; ++i) {
        for (size_t b = bits - 1; b < BitsInType; ++b) {
            auto val = i ^ (1U << b);
            for (size_t r = 0; r < BitsInType; ++r) {
                auto n = ror(val, r);
                if (set.emplace(n, '\0').second) {
                    vec.push_back(n);
                }
                n = ~n;
                if (set.emplace(n, '\0').second) {
                    vec.push_back(n);
                }
            }
        }
    }

    return vec;
}

// see https://en.wikipedia.org/wiki/ANSI_escape_code#24-bit
class ColorTable {
public:
    ColorTable(std::array<uint8_t, 3 * 256> rgb)
        : mRgb(rgb) {}

    void bg(std::ostream& os, uint8_t colorIdx) const {
        write(os, 48, colorIdx);
    }

    void fg(std::ostream& os, uint8_t colorIdx) const {
        write(os, 38, colorIdx);
    }

    void ansiColors(std::ostream& os, uint8_t colorIdx) const {
        bg(os, colorIdx);
        fg(os, static_cast<uint8_t>(colorIdx + UINT8_C(127)));
    }

    // set background based on coloridx
private:
    void write(std::ostream& os, int what, uint8_t colorIdx) const {
        auto idx = static_cast<size_t>(colorIdx) * 3;
        os << "\x1b[" << what << ";2;" << static_cast<int>(mRgb[idx]) << ';'
           << static_cast<int>(mRgb[idx + 1]) << ';' << static_cast<int>(mRgb[idx + 2]) << 'm';
    }

    std::array<uint8_t, 3 * 256> mRgb;
};

ColorTable colorTableVic{
    {0,   18,  97,  1,   20,  98,  1,   21,  99,  1,   23,  100, 1,   24,  101, 1,   26,  102, 2,
     28,  103, 2,   29,  104, 2,   31,  105, 2,   32,  106, 2,   34,  107, 2,   35,  108, 2,   37,
     109, 2,   39,  110, 2,   40,  111, 2,   42,  112, 2,   43,  113, 2,   45,  114, 2,   46,  115,
     2,   48,  116, 2,   49,  117, 2,   51,  118, 2,   52,  119, 2,   54,  120, 2,   55,  121, 2,
     57,  122, 2,   58,  123, 3,   60,  124, 3,   62,  125, 3,   63,  126, 3,   65,  127, 3,   66,
     128, 3,   68,  129, 3,   69,  130, 3,   71,  131, 3,   73,  132, 3,   74,  133, 4,   76,  134,
     4,   77,  135, 4,   79,  136, 5,   81,  137, 5,   82,  138, 6,   84,  139, 6,   86,  140, 7,
     87,  141, 8,   89,  143, 9,   91,  144, 11,  93,  145, 12,  94,  146, 14,  96,  147, 16,  98,
     148, 17,  100, 150, 19,  102, 151, 21,  103, 152, 23,  105, 153, 25,  107, 154, 28,  109, 156,
     30,  111, 157, 32,  113, 158, 35,  115, 160, 37,  117, 161, 40,  119, 162, 43,  121, 164, 45,
     123, 165, 48,  125, 166, 51,  127, 168, 54,  129, 169, 57,  131, 171, 60,  133, 172, 63,  135,
     173, 66,  137, 175, 69,  139, 176, 72,  141, 178, 75,  144, 179, 78,  146, 180, 81,  148, 182,
     84,  150, 183, 87,  152, 185, 90,  154, 186, 93,  156, 187, 97,  158, 189, 100, 160, 190, 103,
     162, 192, 106, 164, 193, 109, 166, 194, 113, 168, 196, 116, 170, 197, 119, 172, 198, 122, 174,
     200, 125, 176, 201, 128, 178, 202, 132, 180, 204, 135, 182, 205, 138, 184, 206, 141, 186, 208,
     144, 188, 209, 148, 190, 210, 151, 192, 212, 154, 194, 213, 157, 196, 214, 160, 197, 216, 163,
     199, 217, 167, 201, 218, 170, 203, 220, 173, 205, 221, 176, 207, 222, 179, 209, 223, 182, 211,
     225, 186, 213, 226, 189, 214, 227, 192, 216, 228, 195, 218, 229, 198, 219, 230, 201, 221, 231,
     204, 223, 232, 207, 224, 232, 210, 225, 233, 213, 227, 233, 216, 228, 233, 219, 229, 233, 222,
     230, 233, 224, 230, 233, 226, 231, 232, 229, 231, 232, 231, 231, 231, 232, 231, 229, 234, 230,
     228, 235, 230, 226, 236, 229, 224, 237, 228, 222, 238, 227, 220, 238, 225, 218, 238, 224, 216,
     238, 222, 213, 238, 221, 211, 238, 219, 208, 238, 217, 205, 237, 215, 203, 237, 213, 200, 236,
     211, 197, 236, 209, 195, 235, 208, 192, 234, 206, 189, 233, 204, 186, 233, 202, 184, 232, 200,
     181, 231, 198, 178, 230, 196, 176, 229, 193, 173, 228, 191, 170, 228, 190, 168, 227, 188, 165,
     226, 186, 162, 225, 184, 160, 224, 182, 157, 223, 180, 154, 223, 178, 152, 222, 176, 149, 221,
     174, 147, 220, 172, 144, 219, 170, 141, 219, 168, 139, 218, 166, 136, 217, 164, 134, 216, 162,
     131, 215, 160, 129, 214, 159, 126, 214, 157, 124, 213, 155, 121, 212, 153, 119, 211, 151, 116,
     211, 149, 114, 210, 148, 112, 209, 146, 109, 208, 144, 107, 207, 142, 104, 207, 140, 102, 206,
     139, 100, 205, 137, 97,  204, 135, 95,  204, 133, 93,  203, 131, 90,  202, 130, 88,  201, 128,
     86,  201, 126, 83,  200, 124, 81,  199, 123, 79,  198, 121, 76,  198, 119, 74,  197, 117, 72,
     196, 116, 69,  195, 114, 67,  194, 112, 65,  194, 110, 63,  193, 109, 60,  192, 107, 58,  191,
     105, 56,  190, 103, 54,  190, 101, 51,  189, 100, 49,  188, 98,  47,  187, 96,  45,  186, 94,
     42,  184, 92,  40,  183, 90,  38,  182, 88,  36,  181, 85,  33,  179, 83,  31,  178, 81,  29,
     176, 79,  27,  175, 76,  24,  173, 74,  22,  171, 72,  20,  169, 69,  18,  167, 67,  16,  165,
     64,  15,  163, 62,  13,  161, 60,  11,  159, 57,  10,  156, 55,  9,   154, 53,  8,   152, 51,
     7,   150, 49,  7,   148, 47,  6,   145, 45,  6,   143, 43,  6,   141, 41,  6,   139, 39,  6,
     137, 38,  6,   135, 36,  6,   133, 34,  6,   131, 33,  6,   129, 31,  6,   127, 30,  6,   126,
     29,  6,   124, 27,  6,   122, 26,  6,   120, 24,  6,   118, 23,  6,   116, 21,  6,   115, 20,
     6,   113, 19,  7,   111, 17,  7,   109, 16,  7,   108, 14,  7,   106, 13,  7,   104, 12,  7,
     103, 10,  7,   101, 9,   7,   99,  7,   7,   98,  6,   7,   96,  4,   8,   94,  3,   8,   93,
     2,   8,   91,  1,   8,   89,  0,   8}};

template <typename Iter>
double geomean(Iter begin, Iter end) {
    double logSum = 0.0;
    size_t count = 0;
    while (begin != end) {
        logSum += std::log(*begin);
        ++count;
        ++begin;
    }
    return std::exp(logSum / static_cast<double>(count));
}

double geomean(std::initializer_list<double> l) {
    return geomean(l.begin(), l.end());
}

double geomean(std::vector<double> const& l) {
    return geomean(l.begin(), l.end());
}

} // namespace

class EvalStrictAvalancheCriterion {
    static constexpr int BitsInType = 64;

public:
    explicit EvalStrictAvalancheCriterion(size_t bits)
        : mBinomialProbs(calculateBinomialDistributionProbabilities(BitsInType, 0.5))
        , mNumbers(generateUniqueLowEntrophyNumbers(bits)) {}

    template <typename Hasher>
    double operator()(Hasher const& hash) const {
        return operator()(hash, false);
    }

    template <typename Hasher>
    void show(Hasher const& hash) const {
        operator()(hash, true);
    }

private:
    template <typename Hasher>
    double operator()(Hasher const& hash, bool show) const {
        std::array<uint32_t, BitsInType + 1U> popCounts{};

        for (auto n : mNumbers) {
            // calc popcount with singly flipped bit
            auto h = hash(n);
            for (size_t flip = 0; flip < BitsInType; ++flip) {
                auto hashAfterBitflip = hash(n ^ (1U << flip));
                ++popCounts[popcount(h ^ hashAfterBitflip)];
            }
        }

        auto totalIters = static_cast<double>(mNumbers.size() * BitsInType);
        double chiPopcount = 0.0;
        double chiCount = 0.0;
        for (size_t i = 0; i < popCounts.size(); ++i) {
            if (mBinomialProbs[i] * totalIters > 5.0) {
                auto expected = mBinomialProbs[i] * totalIters;
                auto actual = static_cast<double>(popCounts[i]);

                auto diff = actual - expected;
                chiPopcount += (diff * diff) / expected;
                ++chiCount;
            }
        }

        if (show) {
            printf("EvalStrictAvalancheCriterion:\n");
            for (size_t i = 0; i < popCounts.size(); ++i) {
                auto expected = mBinomialProbs[i] * totalIters;
                auto actual = static_cast<double>(popCounts[i]);

                auto chi = 0.0;
                if (mBinomialProbs[i] * totalIters > 5.0) {
                    auto diff = actual - expected;
                    chi = (diff * diff) / expected;
                }
                printf("%4ld: %13.1f %13.1f: %8.3f\n", i, expected, actual, chi);
            }
        }
        return chiPopcount / chiCount;
    }
    std::vector<double> mBinomialProbs;
    std::vector<uint64_t> mNumbers;
};

class EvalBitFlipProbability {
    static constexpr int BitsInType = 64;

public:
    explicit EvalBitFlipProbability(size_t bits)
        : mNumbers(generateUniqueLowEntrophyNumbers(bits)) {}

    template <typename Hasher>
    double operator()(Hasher const& hash) const {
        return eval(hash, false);
    }

    template <typename Hasher>
    void show(Hasher const& hash) const {
        eval(hash, true);
    }

private:
    template <typename Hasher>
    double eval(Hasher const& hash, bool printStatus) const {
        std::array<size_t, BitsInType * BitsInType> countBitFlips{};

        for (auto n : mNumbers) {
            auto h = hash(n);
            for (size_t i = 0; i < BitsInType; ++i) {
                auto hashAfterBitflip = hash(n ^ (1U << i));
                auto flipped = h ^ hashAfterBitflip;
                auto location = &countBitFlips[i * BitsInType];
                for (size_t j = 0; j < BitsInType; ++j) {
                    // fully branch-less code, without any side effects in the inner loop. This
                    // is pretty important so this is fast. The compiler can convert this into a
                    // vpbroadcastq instruction, which makes this whole program ~20 times
                    // faster.
                    //
                    // I played a bit with the code, and this seems to be fastest
                    location[j] += (flipped >> j) & UINT64_C(1);
                }
            }
        }

        if (printStatus) {
            printf("EvalBitFlipProbability:\n");
            auto prob = static_cast<double>(mNumbers.size());
            for (size_t i = 0; i < BitsInType; ++i) {
                for (size_t j = 0; j < BitsInType; ++j) {
                    auto color = static_cast<uint8_t>(std::round(
                        255.0 * static_cast<double>(countBitFlips[(i + 1) * BitsInType - j - 1]) /
                        prob));
                }
                printf("\n");
            }
            printf("\n");
        }

        // ideally, each bit is flipped with probability of 1/2
        double expectedFlipsPerBit = static_cast<double>(mNumbers.size() / 2);
        double chi = 0.0;
        for (auto count : countBitFlips) {
            auto diff = (static_cast<double>(count) - expectedFlipsPerBit) / expectedFlipsPerBit;
            chi += diff * diff;
        }
        return chi / static_cast<double>(countBitFlips.size());
    }

    std::vector<uint64_t> mNumbers;
};

class EvalBitFlipSAC {
    static constexpr int BitsInType = 64;
    using BitFlips = std::array<size_t, BitsInType * BitsInType>;
    using PopCounts = std::array<size_t, BitsInType + 1U>;

public:
    explicit EvalBitFlipSAC(size_t bits)
        : mBinomialProbs(calculateBinomialDistributionProbabilities(BitsInType, 0.5))
        , mNumbers(generateUniqueLowEntrophyNumbers(bits)) {}

    template <typename Hasher>
    double operator()(Hasher const& hash) const {
        return eval(hash, false);
    }

    template <typename Hasher>
    void show(Hasher const& hash) const {
        eval(hash, true);
    }

private:
    template <typename Hasher>
    double eval(Hasher const& hash, bool printStatus) const {
        BitFlips countBitFlips{};
        PopCounts popCounts{};

        for (auto n : mNumbers) {
            auto h = hash(n);
            for (size_t i = 0; i < BitsInType; i += 2) {
                auto flipped1 = h ^ hash(n ^ (1U << i));
                auto flipped2 = h ^ hash(n ^ (1U << (i + 1)));

                ++popCounts[popcount(flipped1)];
                ++popCounts[popcount(flipped2)];

                for (size_t j = 0; j < BitsInType; ++j) {
                    // fully branch-less code, without any side effects in the inner loop. This
                    // is pretty important so this is fast. The compiler can convert this into a
                    // vpbroadcastq instruction, which makes this whole program ~20 times
                    // faster.
                    //
                    // I played a bit with the code, and this seems to be fastest
                    countBitFlips[i * BitsInType + j] += (flipped1 >> j) & UINT64_C(1);
                    countBitFlips[(i + 1) * BitsInType + j] += (flipped2 >> j) & UINT64_C(1);
                }
            }
        }

        if (printStatus) {
            printBitFlipProbabilities(countBitFlips);
            printPopcountProbabilities(popCounts, 5.0);
        }

        // geometric mean
        return geomean({chiBitFlip(countBitFlips), chiPopcount(popCounts, 5.0)});
    }

    // chi^2 for the bit flips. Ideally, each bit is flipped with probability of 1/2
    double chiBitFlip(BitFlips const& countBitFlips) const {
        double expectedFlipsPerBit = static_cast<double>(mNumbers.size() / 2);
        double chi = 0.0;
        for (auto count : countBitFlips) {
            auto diff = (static_cast<double>(count) - expectedFlipsPerBit) / expectedFlipsPerBit;
            chi += diff * diff;
        }
        return chi / static_cast<double>(countBitFlips.size());
    }

    // chi^2 for the distribution of popcount, the "strict avalanche criteria"
    double chiPopcount(std::array<size_t, BitsInType + 1U> const& popCounts,
                       double minCountThreshold = 5.0) const {
        auto totalPopcount = static_cast<double>(mNumbers.size() * BitsInType);
        double chi = 0.0;
        double chiCount = 0.0;
        for (size_t i = 0; i < popCounts.size(); ++i) {
            if (mBinomialProbs[i] * totalPopcount >= minCountThreshold) {
                auto expected = mBinomialProbs[i] * totalPopcount;
                auto actual = static_cast<double>(popCounts[i]);

                auto diff = actual - expected;
                chi += (diff * diff) / expected;
                ++chiCount;
            }
        }

        return chi / chiCount;
    }

    void printBitFlipProbabilities(BitFlips const& countBitFlips) const {
        printf("BitFlipProbabilities:\n");
        auto prob = static_cast<double>(mNumbers.size());
        for (size_t i = 0; i < BitsInType; ++i) {
            for (size_t j = 0; j < BitsInType; ++j) {
#if 0                
                printf("%3.0f",
                       100.0 * static_cast<double>(countBitFlips[(i + 1) * BitsInType - j - 1]) /
                           prob);
#endif
                auto prob01 =
                    static_cast<double>(countBitFlips[(i + 1) * BitsInType - j - 1]) / prob;
                auto color = static_cast<uint8_t>(std::round(255.0 * prob01));
                colorTableVic.ansiColors(std::cout, color);
                std::cout << std::setw(3) << static_cast<int>(std::round(100.0 * prob01))
                          << "\x1b[0m";
            }
            printf("\n");
        }
        printf("\n");
    }

    void printPopcountProbabilities(PopCounts const& popCounts, double minCountThreshold) const {
        printf("PopcountProbabilities:\n");
        auto totalPopcount = static_cast<double>(mNumbers.size() * BitsInType);
        for (size_t i = 0; i < popCounts.size(); ++i) {
            auto expected = mBinomialProbs[i] * totalPopcount;
            auto actual = static_cast<double>(popCounts[i]);

            printf("%4ld: %13.1f %13.1f: ", i, expected, actual);
            auto chi = 0.0;
            if (mBinomialProbs[i] * totalPopcount >= minCountThreshold) {
                auto diff = actual - expected;
                chi = (diff * diff) / expected;
                printf("%8.3f\n", chi);
            } else {
                printf("   --\n");
            }
        }
    }

    std::vector<double> mBinomialProbs;
    std::vector<uint64_t> mNumbers;
};

enum class Acceptance { rejected, accepted, new_best };

template <typename Individual>
class MarkovChainMonteCarlo {
public:
    MarkovChainMonteCarlo(double beta)
        : mBeta(beta) {}

    Acceptance accept(Individual const& newIndividual, std::vector<double> newCosts) {
        auto acceptedCost = geomean(mAcceptedCost);
        auto newCost = geomean(newCosts);

        ++mTrial;
        auto threshold = std::log(mRng.uniform01()) / mBeta;
        if (newCost <= acceptedCost - threshold) {
            mAccepted = newIndividual;
            mAcceptedCost = newCosts;

            if (newCost < geomean(mBestCost)) {
                mBest = newIndividual;
                mBestCost = newCosts;
                return Acceptance::new_best;
            }

            return Acceptance::accepted;
        }

        return Acceptance::rejected;
    }

    std::vector<double> bestCost() const {
        return mBestCost;
    }

    Individual const& best() const {
        return mBest;
    }

    std::vector<double> acceptedCost() const {
        return mAcceptedCost;
    }

    Individual const& accepted() const {
        return mAccepted;
    }

    size_t trials() const {
        return mTrial;
    }

private:
    sfc64 mRng{};
    size_t mTrial = 0;
    double mBeta = 10.0;

    Individual mBest{};
    std::vector<double> mBestCost{std::numeric_limits<double>::max()};

    Individual mAccepted{};
    std::vector<double> mAcceptedCost{std::numeric_limits<double>::max()};
};

template <typename T>
std::ostream& operator<<(std::ostream& os, MarkovChainMonteCarlo<T> const& mcmc) {
    // printf("%ld: %.5f %.5f: ", mcmc.trials(), mcmc.acceptedCost(), mcmc.bestCost());
    os << mcmc.trials() << ": accepted=(";
    auto prefix = "";
    for (auto c : mcmc.acceptedCost()) {
        os << prefix << c;
        prefix = ", ";
    }
    os << ")=" << geomean(mcmc.acceptedCost()) << ", best=(";
    prefix = "";
    for (auto c : mcmc.bestCost()) {
        os << prefix << c;
        prefix = ", ";
    }
    os << ")=" << geomean(mcmc.bestCost()) << " " << mcmc.best().mFactors;
    return os;
}

class Dummy {
public:
    static constexpr char const* name = "Dummy";
    uint64_t operator()(uint64_t n) const {
        return n;
    }
};

class EvalSpread {
public:
    EvalSpread(size_t bits, size_t minTableSize, size_t maxTableSize)
        : mNumbers(generateUniqueLowEntrophyNumbers(bits))
        , mHashes(mNumbers.size())
        , mMinTableSize(minTableSize)
        , mMaxTableSize(maxTableSize) {}

    template <typename Hasher>
    double operator()(Hasher const& hash) {
        for (size_t i = 0; i < mNumbers.size(); ++i) {
            mHashes[i] = hash(mNumbers[i]);
        }

        double chiLogSum = 0.0;
        double chiCount = 0;

        for (size_t rot = 0; rot < 64; rot += 2) {
            mTable.clear();
            mTable.resize(mMaxTableSize, 0);
            auto mask = mMaxTableSize - 1;
            for (auto n : mHashes) {
                ++mTable[ror(n, rot) & mask];
            }

            chiLogSum += std::log(calcChiSquared());
            ++chiCount;
            while (mTable.size() > mMinTableSize) {
                // half mTable's size: add upper half to lower half, then decrease size by half.
                // So we are discarding the highest bit.
                for (size_t to = 0, from = mTable.size() / 2; from < mTable.size(); ++to, ++from) {
                    mTable[to] += mTable[from];
                }
                mTable.resize(mTable.size() / 2);

                chiLogSum += std::log(calcChiSquared());
                ++chiCount;
            }
        }
        // printf("%12.1f %s\n", std::exp(chiLogSum / chiCount), Hasher::name);
        return std::exp(chiLogSum / chiCount);
    }

    template <typename Hasher>
    void show(Hasher const&) {}

private:
    double calcChiSquared() const {
        double expectedPerEntry =
            static_cast<double>(mHashes.size()) / static_cast<double>(mTable.size());

        double chi = 0.0;
        for (auto count : mTable) {
            auto diff = (static_cast<double>(count) - expectedPerEntry) / expectedPerEntry;
            chi += static_cast<double>(diff * diff);
        }
        return chi;
    }
    std::vector<uint64_t> mNumbers{};
    std::vector<uint64_t> mHashes{};
    std::vector<size_t> mTable{};
    size_t mMinTableSize{};
    size_t mMaxTableSize{};
};

TEST_CASE_TEMPLATE("hash_constant_optimizer" * doctest::skip(), Hasher, Mul128XorMul, Mul128Add,
                   Murmur, Mul128Xor, Aes, CrcMul128, StronglyUniversal, Rrmxmx, MartinusHash1,
                   MartinusHash2) {
    Hasher op;

    sfc64 rng;
    //#if 0
    for (auto& f : op.mFactors) {
        f = rng();
    }
    //#endif

    RandomBool rbool;
    // Eval<uint64_t> eval(12, 12 * 100000);

    // EvalHashSpread<uint64_t> eval(13);
    // EvalSpread evalSpread(8, 8, 0b1000000000000000);
    // EvalStrictAvalancheCriterion evalSac(10);
    // EvalBitFlipProbability evalBitFlips(10);

    EvalBitFlipSAC evalBitflipSac(13);
    MarkovChainMonteCarlo<Hasher> mcmc(1000);

    size_t iter = 0;
    while (true) {
        ++iter;
        // std::vector<double> costs{evalSpread(op), evalSac(op), evalBitFlips(op)};
        auto begin = std::chrono::high_resolution_clock::now();
        std::vector<double> costs{evalBitflipSac(op)};
        auto end = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration<double>(end - begin).count();
        switch (mcmc.accept(op, costs)) {
        case Acceptance::rejected:
            break;

        case Acceptance::accepted:
            std::cout << mcmc << " // " << Hasher::name << " (" << elapsed << " sec)" << std::endl;
            break;

        case Acceptance::new_best:
            evalBitflipSac.show(op);
            std::cout << mcmc << " // " << Hasher::name << " (" << elapsed << " sec) NEW BEST"
                      << std::endl;
            break;
        }
        op = mcmc.accepted();
        mutateBitFlipProbability(&op.mFactors, &rng, &rbool);
    }
}
