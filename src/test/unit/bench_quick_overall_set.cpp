//#define ROBIN_HOOD_COUNT_ENABLED

#include <robin_hood.h>

#include <app/benchmark.h>
#include <app/doctest.h>
#include <app/geomean.h>
#include <app/sfc64.h>
#include <thirdparty/nanobench/nanobench.h>

#include <unordered_set>

TYPE_TO_STRING(robin_hood::unordered_flat_set<uint64_t>);
TYPE_TO_STRING(robin_hood::unordered_flat_set<std::string>);
TYPE_TO_STRING(robin_hood::unordered_node_set<uint64_t>);
TYPE_TO_STRING(robin_hood::unordered_node_set<std::string>);
TYPE_TO_STRING(std::unordered_set<uint64_t>);
TYPE_TO_STRING(std::unordered_set<std::string>);

namespace {

template <typename K>
inline K initKey();

template <>
inline uint64_t initKey<uint64_t>() {
    return {};
}
inline void randomizeKey(sfc64* rng, int n, uint64_t* key) {
    // we limit ourselfes to 32bit n
    auto limited = (((*rng)() >> 32U) * static_cast<uint64_t>(n)) >> 32U;
    *key = limited;
}
inline size_t getSizeT(uint64_t val) {
    return static_cast<size_t>(val);
}
inline size_t getSizeT(std::string const& str) {
    uint64_t x{};
    std::memcpy(&x, str.data(), sizeof(uint64_t));
    return static_cast<size_t>(x);
}

template <>
inline std::string initKey<std::string>() {
    std::string str;
    str.resize(200);
    return str;
}
inline void randomizeKey(sfc64* rng, int n, std::string* key) {
    uint64_t k{};
    randomizeKey(rng, n, &k);
    std::memcpy(&(*key)[0], &k, sizeof(k));
}

// Random insert & erase
template <typename Set>
void benchRandomInsertErase(ankerl::nanobench::Bench* bench) {
    bench->run(type_string(Set{}) + " random insert erase", [&] {
        sfc64 rng(123);
        size_t verifier{};
        Set set;
        auto key = initKey<typename Set::key_type>();
        for (int n = 1; n < 20000; ++n) {
            for (int i = 0; i < 200; ++i) {
                randomizeKey(&rng, n, &key);
                set.insert(key);
                randomizeKey(&rng, n, &key);
                verifier += set.erase(key);
            }
        }
        CHECK(verifier == 1996311U);
        CHECK(set.size() == 9966U);
    });
}

// iterate
template <typename Set>
void benchIterate(ankerl::nanobench::Bench* bench) {
    size_t numElements = 5000;

    auto key = initKey<typename Set::key_type>();

    // insert
    bench->run(type_string(Set{}) + " iterate while adding then removing", [&] {
        sfc64 rng(555);
        Set set;
        size_t result = 0;
        for (size_t n = 0; n < numElements; ++n) {
            randomizeKey(&rng, 1000000, &key);
            set.insert(key);
            for (auto const& keyVal : set) {
                result += getSizeT(keyVal);
            }
        }

        rng.seed(555);
        do {
            randomizeKey(&rng, 1000000, &key);
            set.erase(key);
            for (auto const& keyVal : set) {
                result += getSizeT(keyVal);
            }
        } while (!set.empty());

        CHECK(result == 12620652940000U);
    });
}

template <typename Set>
void benchRandomFind(ankerl::nanobench::Bench* bench) {
    bench->run(type_string(Set{}) + " 50% probability to find", [&] {
        sfc64 numberesInsertRng(222);
        sfc64 numbersSearchRng(222);

        sfc64 insertionRng(123);

        size_t checksum = 0;
        size_t found = 0;
        size_t notFound = 0;

        Set set;
        auto key = initKey<typename Set::key_type>();
        for (size_t i = 0; i < 10000; ++i) {
            randomizeKey(&numberesInsertRng, 1000000, &key);
            if (insertionRng.uniform(100) < 50) {
                set.insert(key);
            }

            // search 1000 entries in the set
            for (size_t search = 0; search < 1000; ++search) {
                randomizeKey(&numbersSearchRng, 1000000, &key);
                auto it = set.find(key);
                if (it != set.end()) {
                    checksum += getSizeT(*it);
                    ++found;
                } else {
                    ++notFound;
                }
                if (numbersSearchRng.counter() == numberesInsertRng.counter()) {
                    numbersSearchRng.seed(222);
                }
            }
        }

        CHECK(checksum == 2551566706382U);
        CHECK(found == 4972187U);
        CHECK(notFound == 5027813U);
    });
}

template <typename Set>
void benchAll(ankerl::nanobench::Bench* bench) {
    bench->title("benchmarking");
    benchIterate<Set>(bench);
    benchRandomInsertErase<Set>(bench);
    benchRandomFind<Set>(bench);
}

double geomean1(ankerl::nanobench::Bench const& bench) {
    return geomean(bench.results(), [](ankerl::nanobench::Result const& result) {
        return result.median(ankerl::nanobench::Result::Measure::elapsed);
    });
}

} // namespace

// A relatively quick benchmark that should get a relatively good single number of how good the set
// is. It calculates geometric mean of several benchmarks.
TEST_CASE("bench_quick_overall_set_flat" * doctest::test_suite("bench") * doctest::skip()) {
    ankerl::nanobench::Bench bench;
    benchAll<robin_hood::unordered_flat_set<uint64_t>>(&bench);
    benchAll<robin_hood::unordered_flat_set<std::string>>(&bench);
    benchAll<robin_hood::unordered_set<std::string>>(&bench);
    std::cout << geomean1(bench) << std::endl;

#ifdef ROBIN_HOOD_COUNT_ENABLED
    std::cout << robin_hood::counts() << std::endl;
#endif
}

TEST_CASE("bench_quick_overall_set_node" * doctest::test_suite("bench") * doctest::skip()) {
    ankerl::nanobench::Bench bench;
    benchAll<robin_hood::unordered_node_set<uint64_t>>(&bench);
    benchAll<robin_hood::unordered_node_set<std::string>>(&bench);
    std::cout << geomean1(bench) << std::endl;

#ifdef ROBIN_HOOD_COUNT_ENABLED
    std::cout << robin_hood::counts() << std::endl;
#endif
}

TEST_CASE("bench_quick_overall_set_std" * doctest::test_suite("bench") * doctest::skip()) {
    ankerl::nanobench::Bench bench;
    benchAll<std::unordered_set<uint64_t>>(&bench);
    benchAll<std::unordered_set<std::string>>(&bench);
    std::cout << geomean1(bench) << std::endl;
}

// 1773116 kb unordered_flat_set
// 2362932 kb unordered_node_set
// 4071776 kb std::unordered_set
TEST_CASE_TEMPLATE("memory_set_huge" * doctest::test_suite("bench") * doctest::skip(), Set,
                   robin_hood::unordered_flat_set<uint64_t>,
                   robin_hood::unordered_node_set<uint64_t>, std::unordered_set<uint64_t>) {
    Set set;
    for (uint64_t n = 0; n < 80000000; ++n) {
        set.insert(n);
    }
    std::cout << set.size() << std::endl;
}
