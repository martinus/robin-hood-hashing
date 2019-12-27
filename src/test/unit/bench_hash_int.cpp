#include <robin_hood.h>

#include <app/benchmark.h>
#include <app/doctest.h>
#include <app/sfc64.h>
#include <thirdparty/nanobench/nanobench.h>

#include <array>
#include <atomic>
#include <bitset>
#include <chrono>
#include <nmmintrin.h>
#include <thread>
#include <unordered_map>
#include <unordered_set>

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
|                0.94 |    1,064,420,041.79 |    0.0% |            8.00 |            3.00 |  2.664 |           0.00 |    0.0% |      0.00 | `rrmxmx2`
clang-format on
*/

uint64_t hash2(uint64_t v) {
    uint64_t h;
    uint64_t l = robin_hood::detail::umul128(v, UINT64_C(0x6fe29d68f57b9eb5), &h);
    return static_cast<size_t>(h ^ l) * UINT64_C(0x6fe29d68f57b9eb5);
}

// 9.22003e+06 4194303 1.37442e-06
// 1: 17.78865 17.78865: UINT64_C(0xf89f199221d6a52d) // Mul128XorMul NEW BEST
// 2714: 3.72236 3.03246: UINT64_C(0xa213cc9a3de2354f) // Mul128XorMul
// 16909: accepted=(0.0390154, 3.6235e-07)=0.0001189, best=(0.0390154, 3.6235e-07)=0.0001189
// UINT64_C(0xea054c65f6328a3b) // Mul128XorMul
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

//      1948: 0.01520657130295807 0.01520657130295807: {UINT64_C(0x910edbb7b4890982)} NEW BEST
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

//      2309: 0.00000063829856838 0.00000047778365759: {UINT64_C(0xa63a23d7401a5955),
//      UINT64_C(0x20bf219160352d85), UINT64_C(0x5dd176afd0ea0fa2), UINT64_C(0xda1f6577232c3a15),
//      UINT64_C(0xeb255b15985c6919)}

//      3899: 0.00000042940968891 0.00000042940968891: {UINT64_C(0xbe95d8ba65d96550),
//      UINT64_C(0xc3e77698e4873d23), UINT64_C(0x575b506c1ed73860),
//      UINT64_C(0x53017331e6283637), UINT64_C(0xc22a41df2e52096)} NEW BEST
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

class MurmurMin {
public:
    static constexpr char const* name = "MurmurMin";

    MurmurMin()
        : mFactors{UINT64_C(23), UINT64_C(0x7313c3a1d7524819), UINT64_C(33)} {}

    uint64_t operator()(uint64_t h) const {
        h ^= h >> (mFactors[0] & 0x3f);
        h *= mFactors[1];
        h ^= h >> (mFactors[2] & 0x3f);
        return h;
    }

    std::array<uint64_t, 3> mFactors;
};

#include <immintrin.h>

// 9.40042e+06 4194303 1.30147e-06
// 1: 17.75349 17.75349: UINT64_C(0x7b) // Aes NEW BEST
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

// 64-bit perfect hash
uint64_t perfecthash64(uint64_t key) {
    __m128i hash_64 = _mm_set1_epi64x(static_cast<int64_t>(key));
    hash_64 = _mm_aesenc_si128(hash_64, _mm_set1_epi32(-559038737));

    uint64_t result;
    std::memcpy(&result, &hash_64, sizeof(uint64_t));
    return result;
}

template <typename T, typename R>
uint64_t ror(T v, R r) {
    return (v >> static_cast<int>(r)) |
           (v << (static_cast<int>(sizeof(T)) * 8 - static_cast<int>(r)));
}

// 4.2434522807416192894e-07 chi^2 after 1664000000 evaluations
uint64_t rrmxmx(uint64_t v) {
    v ^= ror(v, 49) ^ ror(v, 24);
    v *= 0x9FB21C651E98DF25L;
    v ^= v >> 28;
    v *= 0x9FB21C651E98DF25L;
    return v ^ v >> 28;
}

class Rrmxmx {
public:
    static constexpr char const* name = "Rrmxmx";

    Rrmxmx()
        : mFactors{} {}

    uint64_t operator()(uint64_t v) {
        v ^= ror(v, (mFactors[0] & 0x3f)) ^ ror(v, (mFactors[1] & 0x3f));
        v *= mFactors[2];
        v ^= v >> (mFactors[3] & 0x3f);
        v *= mFactors[4];
        return v ^ v >> (mFactors[5] & 0x3f);
    }
    std::array<uint64_t, 6> mFactors;
};

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

// 9.0263e+06 3fffff 2.22708e-06
// 538: 18.25007 18.25007: UINT64_C(0x1f260ee53711dc7b) // Crc NEW BEST
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

// 0.00043386126810133004853 chi^2 after 672000000 evaluations
uint64_t crc(uint64_t x) {
    return _mm_crc32_u64(0, x) * UINT64_C(0x138bab81b6dfe7e7);
}

uint64_t rrmxmx2(uint64_t v) {
    return _mm_crc32_u64(0, v) * UINT64_C(0xff51afd7ed558ccd);
}

// 0.00049922812968725236316 chi^2 after 40960000 evaluations
// {UINT64_C(0x269b5aabc0e73efc), UINT64_C(0xd9039e2e3707f0d6), UINT64_C(0xe40b16eeca9363e4),
// UINT64_C(0x6a4f61306846bc14), UINT64_C(0xc12fdce47bb81b86), UINT64_C(0xc8767f69daf4abf)}

// 0.00045966155970463413701 chi^2 after 40960000 evaluations
// {UINT64_C(0x169b5aabc0e73efc), UINT64_C(0xd9239e263707f384), UINT64_C(0xe40b16eeeb9541a4),
// UINT64_C(0x6a4f61306846bc14), UINT64_C(0xdb2fdce47bf89b86), UINT64_C(0xc8767f48caf4ab7)}

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
    // add a (neglectible) bit of randomization so the compiler can't optimize this away

    // static constexpr auto c = UINT64_C(0xb81fd30ffccab4af);
    ankerl::nanobench::Config cfg;
    robin_hood::hash<uint64_t> hasher;

    auto modify = [](uint64_t n) -> uint64_t {
        // n *= 0xb81fd30ffccab4af;
        // n ^= n >> 29;
        ++n;
        return n;
    };

    sfc64 rng;
    auto n = rng();
    cfg.run("robin_hood::hash", [&] {
        n = modify(n);
        ankerl::nanobench::doNotOptimizeAway(hasher(n));
    });

    cfg.run("robin_hood::hash_int", [&] {
        n = modify(n);
        ankerl::nanobench::doNotOptimizeAway(robin_hood::hash_int(n));
    });

    cfg.run("hash2", [&] {
        n = modify(n);
        ankerl::nanobench::doNotOptimizeAway(hash2(n));
    });

    StronglyUniversal su;
    cfg.run("stronglyuniversal", [&] {
        n = modify(n);
        ankerl::nanobench::doNotOptimizeAway(su(n));
    });

    cfg.run("murmurhash3", [&] {
        n = modify(n);
        ankerl::nanobench::doNotOptimizeAway(murmurhash3(n));
    });

    cfg.run("rrmxmx", [&] {
        n = modify(n);
        ankerl::nanobench::doNotOptimizeAway(rrmxmx(n));
    });

    cfg.run("twang_mix64", [&] {
        n = modify(n);
        ankerl::nanobench::doNotOptimizeAway(twang_mix64(n));
    });

    cfg.run("crc", [&] {
        n = modify(n);
        ankerl::nanobench::doNotOptimizeAway(crc(n));
    });

    cfg.run("perfecthash64", [&] {
        n = modify(n);
        ankerl::nanobench::doNotOptimizeAway(perfecthash64(n));
    });

    cfg.run("rrmxmx2", [&] {
        n = modify(n);
        ankerl::nanobench::doNotOptimizeAway(rrmxmx2(n));
    });
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

double geomean(std::vector<double> const& data) {
    auto logSummarizer = [](double acc, double value) { return acc + std::log(value); };
    auto logSum = std::accumulate(data.begin(), data.end(), 0.0, logSummarizer);
    return std::exp(logSum / static_cast<double>(data.size()));
}

} // namespace

class EvalStrictAvalancheCriterion {
    static constexpr int BitsInType = 64;

public:
    explicit EvalStrictAvalancheCriterion(size_t bits)
        : mBinomialProbs(calculateBinomialDistributionProbabilities(BitsInType, 0.5))
        , mNumbers(generateUniqueLowEntrophyNumbers(bits)) {}

    template <typename Hasher>
    double operator()(Hasher const& hash) const noexcept {
        std::array<uint32_t, BitsInType + 1U> popCounts{};

        for (auto n : mNumbers) {
            // calc popcount with singly flipped bit
            auto h = hash(n);
            for (size_t flip = 0; flip < BitsInType; ++flip) {
                auto hashAfterBitflip = hash(n ^ (1U << flip));
                ++popCounts[popcount(h ^ hashAfterBitflip)];
            }
        }

        // iterate through the hash's infobit
        auto totalIters = static_cast<double>(mNumbers.size() * BitsInType);
        double chiPopcount = 0.0;
        double chiCount = 0.0;
        for (size_t i = 0; i < popCounts.size(); ++i) {
            if (mBinomialProbs[i] * totalIters > 5.0) {
                auto diff = mBinomialProbs[i] - static_cast<double>(popCounts[i]) / totalIters;
                chiPopcount += (diff * diff) / mBinomialProbs[i];
                ++chiCount;
            }
        }

        return chiPopcount / chiCount;
    }

private:
    std::vector<double> mBinomialProbs;
    std::vector<uint64_t> mNumbers;
};

class EvalBitFlipProbability {
    static constexpr int BitsInType = 64;

public:
    explicit EvalBitFlipProbability(size_t bits)
        : mNumbers(generateUniqueLowEntrophyNumbers(bits)) {}

    template <typename Hasher>
    double operator()(Hasher const& hash) const noexcept {
        std::array<size_t, BitsInType * BitsInType> countBitFlips{};

        for (auto n : mNumbers) {
            auto h = hash(n);
            for (size_t i = 0; i < BitsInType; ++i) {
                auto hashAfterBitflip = hash(n ^ (1U << i));
                auto flipped = h ^ hashAfterBitflip;
                for (size_t j = 0; j < BitsInType; ++j) {
                    // fully branch-less code, without any side effects in the inner loop. This is
                    // pretty important so this is fast. The compiler can convert this into a
                    // vpbroadcastq instruction, which makes this whole program ~20 times faster.
                    countBitFlips[i * BitsInType + j] += (flipped >> j) & 1U;
                }
            }
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

private:
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

TEST_CASE("strict_avalanche_criterion" * doctest::skip()) {
    using Hasher = Mul128XorMul;
    // using Hasher = Mul128Add;
    // using Hasher = Murmur;
    // using Hasher = MurmurMin;
    // using Hasher = Mul128Xor;
    // using Hasher = Aes;
    // using Hasher = Crc;
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
    EvalSpread evalSpread(10, 8, 0b10000000000000000);
    // EvalStrictAvalancheCriterion evalSac(10);
    EvalBitFlipProbability evalBitFlips(10);
    MarkovChainMonteCarlo<Hasher> mcmc(30);

    size_t iter = 0;
    while (true) {
        ++iter;
        // std::vector<double> costs{evalSpread(op), evalSac(op), evalBitFlips(op)};
        auto begin = std::chrono::high_resolution_clock::now();
        std::vector<double> costs{evalSpread(op), evalBitFlips(op)};
        auto end = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration<double>(end - begin).count();
        switch (mcmc.accept(op, costs)) {
        case Acceptance::rejected:
            break;

        case Acceptance::accepted:
            std::cout << mcmc << " // " << Hasher::name << " (" << elapsed << " sec)" << std::endl;
            break;

        case Acceptance::new_best:
            std::cout << mcmc << " // " << Hasher::name << " (" << elapsed << " sec) NEW BEST"
                      << std::endl;
            break;
        }
        op = mcmc.accepted();
        mutateBitFlipProbability(&op.mFactors, &rng, &rbool);
    }
}
