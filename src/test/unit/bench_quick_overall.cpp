//#define ROBIN_HOOD_COUNT_ENABLED

#include <robin_hood.h>

#include <app/benchmark.h>
#include <app/doctest.h>
#include <app/geomean.h>
#include <app/sfc64.h>
#include <thirdparty/nanobench/nanobench.h>

#include <unordered_map>

TYPE_TO_STRING(robin_hood::unordered_flat_map<uint64_t, size_t>);
TYPE_TO_STRING(robin_hood::unordered_flat_map<std::string, size_t>);
TYPE_TO_STRING(robin_hood::unordered_node_map<uint64_t, size_t>);
TYPE_TO_STRING(robin_hood::unordered_node_map<std::string, size_t>);
TYPE_TO_STRING(std::unordered_map<uint64_t, size_t>);
TYPE_TO_STRING(std::unordered_map<std::string, size_t>);

namespace {

template <typename K>
inline K initKey();

template <>
inline uint64_t initKey<uint64_t>() {
    return {};
}

inline void randomizeKey(sfc64& rng, int n, uint64_t& key) {
    // we limit ourselfes to 32bit n
    auto limited = ((rng() >> 32U) * static_cast<uint64_t>(n)) >> 32U;
    key = limited;
}

template <>
inline std::string initKey<std::string>() {
    std::string str;
    str.resize(200);
    return str;
}
inline void randomizeKey(sfc64& rng, int n, std::string& key) {
    uint64_t k;
    randomizeKey(rng, n, k);
    std::memcpy(&key[0], &k, sizeof(k));
}

// Random insert & erase
template <typename Map>
ROBIN_HOOD(NOINLINE)
void benchRandomInsertErase(ankerl::nanobench::Config& cfg) {

    cfg.run("random insert erase", [&] {
        sfc64 rng(123);
        size_t verifier{};
        Map map;
        auto key = initKey<typename Map::key_type>();
        for (int n = 1; n < 20000; ++n) {
            for (int i = 0; i < 200; ++i) {
                randomizeKey(rng, n, key);
                map[key];
                randomizeKey(rng, n, key);
                verifier += map.erase(key);
            }
        }
        REQUIRE(verifier == 1996311U);
        REQUIRE(map.size() == 9966U);
    });
}

// iterate
template <typename Map>
ROBIN_HOOD(NOINLINE)
void benchIterate(ankerl::nanobench::Config& cfg) {
    size_t numElements = 5000;

    auto key = initKey<typename Map::key_type>();

    // insert
    cfg.run("iterate while adding then removing", [&] {
        sfc64 rng(555);
        Map map;
        size_t result = 0;
        for (size_t n = 0; n < numElements; ++n) {
            randomizeKey(rng, 1000000, key);
            map[key] = n;
            for (auto const& keyVal : map) {
                result += keyVal.second;
            }
        }

        rng.seed(555);
        do {
            randomizeKey(rng, 1000000, key);
            map.erase(key);
            for (auto const& keyVal : map) {
                result += keyVal.second;
            }
        } while (!map.empty());

        REQUIRE(result == 62343599601U);
    });
}

template <typename Map>
ROBIN_HOOD(NOINLINE)
void benchRandomFind(ankerl::nanobench::Config& cfg) {
    cfg.run("50% probability to find", [&] {
        sfc64 numberesInsertRng(222);
        sfc64 numbersSearchRng(222);

        sfc64 insertionRng(123);

        size_t checksum = 0;
        size_t found = 0;
        size_t notFound = 0;

        RandomBool rbool;

        Map map;
        auto key = initKey<typename Map::key_type>();
        for (size_t i = 0; i < 10000; ++i) {
            randomizeKey(numberesInsertRng, 1000000, key);
            if (insertionRng.uniform(100) < 50) {
                map[key] = i;
            }

            // search 1000 entries in the map
            for (size_t search = 0; search < 1000; ++search) {
                randomizeKey(numbersSearchRng, 1000000, key);
                auto it = map.find(key);
                if (it != map.end()) {
                    checksum += it->second;
                    ++found;
                } else {
                    ++notFound;
                }
                if (numbersSearchRng.counter() == numberesInsertRng.counter()) {
                    numbersSearchRng.seed(222);
                }
            }
        }

        REQUIRE(checksum == 12570603553U);
        REQUIRE(found == 4972187U);
        REQUIRE(notFound == 5027813U);
    });
}

template <typename Map>
ROBIN_HOOD(NOINLINE)
void benchAll(ankerl::nanobench::Config& cfg) {
    cfg.title("benchmarking " + type_string(Map{}));
    benchIterate<Map>(cfg);
    benchRandomInsertErase<Map>(cfg);
    benchRandomFind<Map>(cfg);
}

} // namespace

// A relatively quick benchmark that should get a relatively good single number of how good the map
// is. It calculates geometric mean of several benchmarks.
TEST_CASE("bench_quick_overall" * doctest::test_suite("bench") * doctest::skip()) {
    ankerl::nanobench::Config cfg;
    // benchAll<std::unordered_map<uint64_t, size_t>>(cfg);
    // benchAll<std::unordered_map<std::string, size_t>>(cfg);

    benchAll<robin_hood::unordered_flat_map<uint64_t, size_t>>(cfg);
    benchAll<robin_hood::unordered_flat_map<std::string, size_t>>(cfg);

    // benchAll<robin_hood::unordered_node_map<uint64_t, size_t>>(cfg);
    // benchAll<robin_hood::unordered_node_map<std::string, size_t>>(cfg);

    auto mean = geomean(cfg.results(), [](ankerl::nanobench::Result const& result) {
        return result.median().count();
    });

#ifdef ROBIN_HOOD_COUNT_ENABLED
    std::cout << robin_hood::counts() << std::endl;
#endif
    std::cout << mean << std::endl;
}
