#include "hashes.h"
#include "test_base.h"

#include <map>
#include <sstream>
#include <unordered_map>

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