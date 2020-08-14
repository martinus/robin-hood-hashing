//#define ROBIN_HOOD_COUNT_ENABLED

#include <robin_hood.h>

#include <app/benchmark.h>
#include <app/doctest.h>
#include <app/geomean.h>
#include <thirdparty/nanobench/nanobench.h>

#include <unordered_map>
#include <unordered_set>

TYPE_TO_STRING(robin_hood::unordered_flat_map<uint64_t, size_t>);
TYPE_TO_STRING(robin_hood::unordered_flat_map<uint32_t, size_t>);
TYPE_TO_STRING(robin_hood::unordered_flat_map<std::string, size_t>);
TYPE_TO_STRING(robin_hood::unordered_node_map<uint64_t, size_t>);
TYPE_TO_STRING(robin_hood::unordered_node_map<uint32_t, size_t>);
TYPE_TO_STRING(robin_hood::unordered_node_map<std::string, size_t>);
TYPE_TO_STRING(std::unordered_map<uint64_t, size_t>);
TYPE_TO_STRING(std::unordered_map<uint32_t, size_t>);
TYPE_TO_STRING(std::unordered_map<std::string, size_t>);

namespace {

template <typename K>
inline K initKey() {
    return {};
}

template <typename T>
inline void randomizeKey(ankerl::nanobench::Rng* rng, int n, T* key) {
    // we limit ourselfes to 32bit n
    auto limited = (((*rng)() >> 32U) * static_cast<uint64_t>(n)) >> 32U;
    *key = static_cast<T>(limited);
}

template <>
inline std::string initKey<std::string>() {
    std::string str;
    str.resize(200);
    return str;
}
inline void randomizeKey(ankerl::nanobench::Rng* rng, int n, std::string* key) {
    uint64_t k{};
    randomizeKey(rng, n, &k);
    std::memcpy(&(*key)[0], &k, sizeof(k));
}

// Random insert & erase
template <typename Map>
void benchRandomInsertErase(ankerl::nanobench::Bench* bench) {
    bench->run(type_string(Map{}) + " random insert erase", [&] {
        ankerl::nanobench::Rng rng(123);
        size_t verifier{};
        Map map;
        auto key = initKey<typename Map::key_type>();
        for (int n = 1; n < 20000; ++n) {
            for (int i = 0; i < 200; ++i) {
                randomizeKey(&rng, n, &key);
                map[key];
                randomizeKey(&rng, n, &key);
                verifier += map.erase(key);
            }
        }
        CHECK(verifier == 1994641U);
        CHECK(map.size() == 9987U);
    });
}

// iterate
template <typename Map>
void benchIterate(ankerl::nanobench::Bench* bench) {
    size_t numElements = 5000;

    auto key = initKey<typename Map::key_type>();

    // insert
    bench->run(type_string(Map{}) + " iterate while adding then removing", [&] {
        ankerl::nanobench::Rng rng(555);
        Map map;
        size_t result = 0;
        for (size_t n = 0; n < numElements; ++n) {
            randomizeKey(&rng, 1000000, &key);
            map[key] = n;
            for (auto const& keyVal : map) {
                result += keyVal.second;
            }
        }

        rng = ankerl::nanobench::Rng(555);
        do {
            randomizeKey(&rng, 1000000, &key);
            map.erase(key);
            for (auto const& keyVal : map) {
                result += keyVal.second;
            }
        } while (!map.empty());

        CHECK(result == 62282755409U);
    });
}

// 111.903 222
// 112.023 123123
template <typename Map>
void benchRandomFind(ankerl::nanobench::Bench* bench) {

    bench->run(type_string(Map{}) + " 50% probability to find", [&] {
        uint64_t const seed = 123123;
        ankerl::nanobench::Rng numbersInsertRng(seed);
        size_t numbersInsertRngCalls = 0;

        ankerl::nanobench::Rng numbersSearchRng(seed);
        size_t numbersSearchRngCalls = 0;

        ankerl::nanobench::Rng insertionRng(123);

        size_t checksum = 0;
        size_t found = 0;
        size_t notFound = 0;

        Map map;
        auto key = initKey<typename Map::key_type>();
        for (size_t i = 0; i < 100000; ++i) {
            randomizeKey(&numbersInsertRng, 1000000, &key);
            ++numbersInsertRngCalls;

            if (insertionRng() & 1U) {
                map[key] = i;
            }

            // search 100 entries in the map
            for (size_t search = 0; search < 100; ++search) {
                randomizeKey(&numbersSearchRng, 1000000, &key);
                ++numbersSearchRngCalls;

                auto it = map.find(key);
                if (it != map.end()) {
                    checksum += it->second;
                    ++found;
                } else {
                    ++notFound;
                }
                if (numbersInsertRngCalls == numbersSearchRngCalls) {
                    numbersSearchRng = ankerl::nanobench::Rng(seed);
                    numbersSearchRngCalls = 0;
                }
            }
        }
        ankerl::nanobench::doNotOptimizeAway(checksum);
        ankerl::nanobench::doNotOptimizeAway(found);
        ankerl::nanobench::doNotOptimizeAway(notFound);
    });
}

template <typename Map>
void benchAll(ankerl::nanobench::Bench* bench) {
    bench->title("benchmarking");
    benchIterate<Map>(bench);
    benchRandomInsertErase<Map>(bench);
    benchRandomFind<Map>(bench);
}

double geomean1(ankerl::nanobench::Bench const& bench) {
    return geomean(bench.results(), [](ankerl::nanobench::Result const& result) {
        return result.median(ankerl::nanobench::Result::Measure::elapsed);
    });
}

} // namespace

// A relatively quick benchmark that should get a relatively good single number of how good the map
// is. It calculates geometric mean of several benchmarks.
TEST_CASE("bench_quick_overall_map_flat" * doctest::test_suite("bench") * doctest::skip()) {
    ankerl::nanobench::Bench bench;
    benchAll<robin_hood::unordered_flat_map<uint64_t, size_t>>(&bench);
    benchAll<robin_hood::unordered_flat_map<std::string, size_t>>(&bench);
    std::cout << geomean1(bench) << std::endl;

#ifdef ROBIN_HOOD_COUNT_ENABLED
    std::cout << robin_hood::counts() << std::endl;
#endif
}

TEST_CASE("bench_quick_overall_map_node" * doctest::test_suite("bench") * doctest::skip()) {
    ankerl::nanobench::Bench bench;
    benchAll<robin_hood::unordered_node_map<uint64_t, size_t>>(&bench);
    benchAll<robin_hood::unordered_node_map<std::string, size_t>>(&bench);
    std::cout << geomean1(bench) << std::endl;

#ifdef ROBIN_HOOD_COUNT_ENABLED
    std::cout << robin_hood::counts() << std::endl;
#endif
}

TEST_CASE("bench_quick_overall_map_std" * doctest::test_suite("bench") * doctest::skip()) {
    ankerl::nanobench::Bench bench;
    benchAll<std::unordered_map<uint64_t, size_t>>(&bench);
    benchAll<std::unordered_map<std::string, size_t>>(&bench);
    std::cout << geomean1(bench) << std::endl;
}

// 3345972 kb unordered_flat_map
// 2616020 kb unordered_node_map
// 4071892 kb std::unordered_map
TEST_CASE_TEMPLATE("memory_map_huge" * doctest::test_suite("bench") * doctest::skip(), Map,
                   robin_hood::unordered_flat_map<uint64_t, size_t>,
                   robin_hood::unordered_node_map<uint64_t, size_t>,
                   std::unordered_map<uint64_t, size_t>) {
    Map map;
    for (uint64_t n = 0; n < 80000000; ++n) {
        map[n];
    }
    std::cout << map.size() << std::endl;
}

template <typename Set>
void benchConsecutiveInsert(char const* name) {
    auto before = std::chrono::high_resolution_clock::now();
    Set s{};
    for (uint64_t x = 0; x < 100000000; ++x) {
        s.insert(x);
    }
    auto after = std::chrono::high_resolution_clock::now();
    std::cout << '\t' << std::chrono::duration<double>(after - before).count()
              << "s, size=" << s.size() << " for " << name << std::endl;
}

template <typename Set>
void benchRandomInsert(char const* name) {
    ankerl::nanobench::Rng rng(23);
    auto before = std::chrono::high_resolution_clock::now();
    Set s{};
    for (uint64_t x = 0; x < 100000000; ++x) {
        s.insert(rng());
    }
    auto after = std::chrono::high_resolution_clock::now();
    std::cout << '\t' << std::chrono::duration<double>(after - before).count()
              << "s, size=" << s.size() << " for " << name << std::endl;
}

template <typename Set>
void benchShiftedInsert(char const* name) {
    auto before = std::chrono::high_resolution_clock::now();
    Set s{};
    for (uint64_t x = 0; x < 100000000; ++x) {
        s.insert(x << 4U);
    }
    auto after = std::chrono::high_resolution_clock::now();
    std::cout << '\t' << std::chrono::duration<double>(after - before).count()
              << "s, size=" << s.size() << " for " << name << std::endl;
}

TEST_CASE("bench_insert_huge" * doctest::test_suite("bench") * doctest::skip()) {
    std::cout << "insert consecutive numbers" << std::endl;
    benchConsecutiveInsert<robin_hood::unordered_flat_set<uint64_t>>(
        "robin_hood::unordered_flat_set<uint64_t>");
    benchConsecutiveInsert<robin_hood::unordered_node_set<uint64_t>>(
        "robin_hood::unordered_node_set<uint64_t>");
    benchConsecutiveInsert<std::unordered_set<uint64_t>>("std::unordered_set<uint64_t>");

    std::cout << "insert random numbers" << std::endl;
    benchRandomInsert<robin_hood::unordered_flat_set<uint64_t>>(
        "robin_hood::unordered_flat_set<uint64_t>");
    benchRandomInsert<robin_hood::unordered_node_set<uint64_t>>(
        "robin_hood::unordered_node_set<uint64_t>");
    benchRandomInsert<std::unordered_set<uint64_t>>("std::unordered_set<uint64_t>");

    std::cout << "insert consecutive, shifted left 4 bits" << std::endl;
    benchShiftedInsert<robin_hood::unordered_flat_set<uint64_t>>(
        "robin_hood::unordered_flat_set<uint64_t>");
    benchShiftedInsert<robin_hood::unordered_node_set<uint64_t>>(
        "robin_hood::unordered_node_set<uint64_t>");
    benchShiftedInsert<std::unordered_set<uint64_t>>("std::unordered_set<uint64_t>");
}
