#include "hashes.h"
#include "test_base.h"

#include <map>
#include <sstream>
#include <unordered_map>

TEMPLATE_TEST_CASE("hash std::string", "[!benchmark][hash]", (robin_hood::hash<std::string>),
                   (std::hash<std::string>)) {
    size_t h = 0;
    Rng rng(123);
    auto hasher = TestType{};

    for (auto s_runtime : {std::make_pair(7, 250058138u), std::make_pair(8, 310857313u),
                           std::make_pair(13, 225620314u), std::make_pair(100, 68319310u),
                           std::make_pair(1000, 6458275u)}) {
        std::string str(static_cast<size_t>(s_runtime.first), 'x');
        for (size_t i = 0; i < str.size(); ++i) {
            str[i] = rng.uniform<char>();
        }
        BENCHMARK("std::string length " + std::to_string(str.size())) {
            for (size_t i = 0; i < s_runtime.second; ++i) {
                // modify string to prevent optimization
                ++*reinterpret_cast<uint64_t*>(&str[0]);
                h += hasher(str);
            }
        }
    }
    // prevent optimization
    INFO(h);
}

TEST_CASE("hash integers", "[!benchmark][hash]") {
    Rng rng(123);
    // add a (neglectible) bit of randomization so the compiler can't optimize this away
    uint64_t const numIters = (2000000000 + rng(2)) * 31;
    size_t a = 0;
    BENCHMARK("robin_hood::hash") {
        robin_hood::hash<uint64_t> hasher;
        for (uint64_t i = 0; i < numIters; i += 31) {
            a += hasher(i);
        }
    }
    std::cout << a;

    a = 0;
    BENCHMARK("hash::FNV1a") {
        hash::FNV1a<uint64_t> hasher;
        for (uint64_t i = 0; i < numIters; i += 31) {
            a += hasher(i);
        }
    }
    std::cout << a;

    a = 0;
    BENCHMARK("std::hash") {
        std::hash<uint64_t> hasher;
        for (uint64_t i = 0; i < numIters; i += 31) {
            a += hasher(i);
        }
    }
    std::cout << a;
}

// dummy map for overhead calculation. Makes use of key so it can't be optimized away.
template <typename Key, typename Val>
class DummyMapForOverheadCalculation {
public:
    DummyMapForOverheadCalculation()
        : m_val{} {}

    Val& operator[](Key const& key) {
        return m_val[key & 1];
    }

    size_t erase(Key const& key) {
        auto r = m_val[key & 1];
        m_val[key & 1] = 0;
        return r;
    }

    void clear() {}

private:
    Val m_val[2];
};

TEMPLATE_TEST_CASE("insert & erase & clear", "[!benchmark][map]",
                   (robin_hood::unordered_flat_map<int, int, hash::Null<int>>),
                   (robin_hood::unordered_flat_map<int, int, hash::FNV1a<int>>),
                   (robin_hood::unordered_flat_map<int, int>),
                   (robin_hood::unordered_node_map<int, int>), (std::unordered_map<int, int>)) {
    Rng rng(123);

    BENCHMARK("Random insert erase") {
        uint64_t verifier = 0;
        TestType map;
        for (int n = 1; n < 20'000; ++n) {
            for (int i = 0; i < 20'000; ++i) {
                map[rng.uniform<int>(static_cast<uint64_t>(n))] = i;
                verifier += map.erase(rng.uniform<int>(static_cast<uint64_t>(n)));
            }
        }
        REQUIRE(verifier == 200050629);
    }
}

TEMPLATE_TEST_CASE("random insert erase", "[!benchmark][map]",
                   (robin_hood::unordered_flat_map<uint64_t, uint64_t>),
                   (robin_hood::unordered_flat_map<uint64_t, uint64_t, std::hash<uint64_t>>)) {
    Rng rng(123);

    size_t const max_n = 21000;

    size_t verifier = 0;
    BENCHMARK("Random insert erase") {
        TestType map;

        for (size_t n = 2; n < max_n; ++n) {
            for (size_t i = 0; i < max_n; ++i) {
                map[rng(n)] = i;
                verifier += map.erase(rng(n));
            }
        }
    }
    REQUIRE(verifier == 220534004);
}

// benchmark adapted from https://github.com/attractivechaos/udb2
// this implementation should have less overhead, because sfc64 and it's uniform() is extremely
// fast.
TEMPLATE_TEST_CASE("distinctness", "[!benchmark][map]",
                   (robin_hood::unordered_map<int, int, hash::Null<int>>),
                   (robin_hood::unordered_map<int, int, hash::FNV1a<int>>),
                   (robin_hood::unordered_map<int, int, robin_hood::hash<int>>)) {
    static size_t const upper = 50'000'000;
    static size_t const lower = 10'000'000;
    static size_t const num_steps = 5;
    static size_t const step_width = (upper - lower) / num_steps;

    Rng rng(123);

    int checksum = 0;
    BENCHMARK("  5% distinct") {
        for (size_t n = lower; n <= upper; n += step_width) {
            size_t const max_rng = n / 20;
            TestType map;
            for (size_t i = 0; i < n; ++i) {
                checksum += ++map[rng.uniform<int>(max_rng)];
            }
        }
    }
    REQUIRE(checksum == 1979940949);

    checksum = 0;
    BENCHMARK(" 25% distinct") {
        for (size_t n = lower; n <= upper; n += step_width) {
            size_t const max_rng = n / 4;
            TestType map;
            for (size_t i = 0; i < n; ++i) {
                checksum += ++map[rng.uniform<int>(max_rng)];
            }
        }
    }
    REQUIRE(checksum == 539988321);

    checksum = 0;
    BENCHMARK(" 50% distinct") {
        for (size_t n = lower; n <= upper; n += step_width) {
            size_t const max_rng = n / 2;
            TestType map;
            for (size_t i = 0; i < n; ++i) {
                checksum += ++map[rng.uniform<int>(max_rng)];
            }
        }
    }
    REQUIRE(checksum == 360007397);

    checksum = 0;
    BENCHMARK("100% distinct") {
        for (size_t n = lower; n <= upper; n += step_width) {
            TestType map;
            for (size_t i = 0; i < n; ++i) {
                checksum += ++map[rng.uniform<int>()];
            }
        }
    }
    REQUIRE(checksum == 180759494);
}

TEMPLATE_TEST_CASE("random find", "[!benchmark][map]",
                   (robin_hood::unordered_flat_map<size_t, size_t>),
                   (robin_hood::unordered_node_map<size_t, size_t>),
                   (std::unordered_map<size_t, size_t>)) {
    size_t const num_iters = 30;
    size_t const insertion_factor = 10'000;
    size_t const num_finds = 50'000'000;
    Rng rng(123);

    size_t num_found = 0;
    BENCHMARK("random find mostly existing") {
        TestType map;
        for (size_t iters = 1; iters <= num_iters; ++iters) {
            auto const max_insertion = insertion_factor * iters;

            for (size_t j = 0; j < max_insertion; ++j) {
                map.emplace(rng.uniform<size_t>(max_insertion), j);
            }

            for (size_t n = 0; n < num_finds; ++n) {
                num_found += map.count(rng.uniform<size_t>(max_insertion));
            }
        }
    }
    REQUIRE(num_found == 1'397'003'591);

    num_found = 0;
    BENCHMARK("random find nonexisting") {
        TestType map;
        for (size_t iters = 1; iters <= num_iters; ++iters) {
            auto const max_insertion = insertion_factor * iters;

            for (size_t j = 0; j < max_insertion; ++j) {
                map.emplace(rng.uniform<size_t>(), j);
            }

            for (size_t n = 0; n < num_finds; ++n) {
                num_found += map.count(rng.uniform<size_t>());
            }
        }
    }
    REQUIRE(num_found == 0);
}

TEMPLATE_TEST_CASE("random find 50%", "[!benchmark][map]",
                   (robin_hood::unordered_flat_map<size_t, size_t>),
                   (robin_hood::unordered_node_map<size_t, size_t>),
                   (std::unordered_map<size_t, size_t>)) {
    Rng rng(123);
    RandomBool rbool;

    size_t num_found = 0;
    BENCHMARK("random find") {
        TestType map;
        for (size_t i = 1; i <= 1000000; ++i) {
            if (rbool(rng)) {
                map.emplace(i, i);
            }

            for (size_t j = 0; j < 1000; ++j) {
                num_found += map.count(rng.uniform<size_t>(i));
            }
        }
    }
    REQUIRE(num_found == 500787924);
}

template <typename Map, size_t NumRandom>
static void randomFind() {
    size_t constexpr NumTotal = 4;
    size_t constexpr NumSequential = NumTotal - NumRandom;

    size_t constexpr NumInserts = 1000000;
    size_t constexpr NumFindsPerIter = 1000 * NumTotal;

    size_t constexpr Percent = (NumSequential * 100 / NumTotal);
    size_t constexpr NotSequentialFactor = 31;

    std::stringstream ss;
    ss << std::setw(3) << Percent << "% find success";

    size_t num_found = 0;

    Rng rng(123);

    std::array<bool, NumTotal> insertRandom = {{false}};
    for (size_t i = 0; i < NumRandom; ++i) {
        insertRandom[i] = true;
    }

    BENCHMARK(ss.str()) {
        Map map;
        size_t i = 0;
        do {
            // insert NumTotal entries: some random, some sequential.
            std::shuffle(insertRandom.begin(), insertRandom.end(), rng);
            for (bool isRandomToInsert : insertRandom) {
                if (isRandomToInsert) {
                    // [1..30], [32..61], ...
                    map.emplace(static_cast<size_t>(NotSequentialFactor * i +
                                                    rng(NotSequentialFactor - 1) + 1),
                                i);
                } else {
                    // 0, 31, 62, ...
                    map.emplace(NotSequentialFactor * i, i);
                }
                ++i;
            }

            for (size_t j = 0; j < NumFindsPerIter; ++j) {
                num_found += map.count(static_cast<size_t>(NotSequentialFactor * rng(i)));
            }
        } while (i < NumInserts);
    }
    // 100%, 75%, 50%, 25%, 0%
    size_t results[] = {1000000000, 750004934, 500038550, 249992050, 0};
    REQUIRE(num_found == results[NumRandom]);
}

TEMPLATE_TEST_CASE("rfind", "[!benchmark][map]",
                   (robin_hood::unordered_flat_map<size_t, size_t, robin_hood::hash<size_t>>),
                   (robin_hood::unordered_flat_map<size_t, size_t, hash::Null<size_t>>),
                   (robin_hood::unordered_flat_map<size_t, size_t, hash::FNV1a<size_t>>)) {
    randomFind<TestType, 0>();
    randomFind<TestType, 1>();
    randomFind<TestType, 2>();
    randomFind<TestType, 3>();
    randomFind<TestType, 4>();
}

TEMPLATE_TEST_CASE("iterate", "[!benchmark][map]",
                   (robin_hood::unordered_flat_map<uint64_t, size_t>),
                   (robin_hood::unordered_node_map<uint64_t, size_t>),
                   (std::unordered_map<uint64_t, size_t>)) {
    size_t totals = 50000;
    uint64_t rng_seed = UINT64_C(123);
    Rng rng{rng_seed};
    uint64_t checksum = 0;

    BENCHMARK("iterate") {
        TestType map;
        for (size_t n = 0; n < totals; ++n) {
            // insert a single element
            map[rng()];

            // then iterate everything
            uint64_t iter_sum = 0;
            for (auto const& keyVal : map) {
                iter_sum += keyVal.first;
            }
            checksum += iter_sum * n;
        }

        // now remove element after element until the map is empty
        rng.seed(rng_seed);

        for (size_t n = 0; n < totals; ++n) {
            map.erase(rng());

            // then iterate everything
            uint64_t iter_sum = 0;
            for (auto const& keyVal : map) {
                iter_sum += keyVal.first;
            }
            checksum += iter_sum * n;
        }
    }

    REQUIRE(checksum == UINT64_C(0x82ab73e8f2678680));
}