#include "test_base.h"

#include <map>
#include <unordered_map>

struct NullHash {
	size_t operator()(uint64_t val) const {
		return static_cast<size_t>(val);
	}
};

TEMPLATE_TEST_CASE("benchmark random insert erase", "[!benchmark]", (robin_hood::flat_map<uint64_t, uint64_t>),
				   (robin_hood::node_map<uint64_t, uint64_t>), (std::unordered_map<uint64_t, uint64_t>), (std::map<uint64_t, uint64_t>)) {
	Rng rng(123);
	TestType map;

	static const size_t maxVal = 2000;

	BENCHMARK("Random insert erase") {
		for (uint64_t i = 0; i < 100'000'000; ++i) {
			map[rng(maxVal)] = i;
			map.erase(rng(maxVal));
		}
	}

	uint64_t sum_key = 0;
	uint64_t sum_val = 0;
	BENCHMARK("iterating") {
		for (int i = 0; i < 1000; ++i) {
			for (auto const& kv : map) {
				sum_key += kv.first;
				sum_val += kv.second;
			}
		}
	}
	REQUIRE(map.size() == 993);
	REQUIRE(sum_key + sum_val == 0x5a5017eef9c8);
	BENCHMARK("clear") {
		map.clear();
	}
}

TEMPLATE_TEST_CASE("benchmark int", "[!benchmark]", (robin_hood::flat_map<int, int>), (robin_hood::node_map<int, int>),
				   (std::unordered_map<int, int>), (std::map<int, int>)) {
	Rng rng(123);
	TestType map;

	uint64_t verifier = 0;
	BENCHMARK("Random insert erase") {
		for (int n = 1; n < 10'000; ++n) {
			for (int i = 0; i < 10'000; ++i) {
				map[rng.uniform<int>((uint64_t)n)] = i;
				verifier += map.erase(rng.uniform<int>((uint64_t)n));
			}
		}
	}

	REQUIRE(verifier == 993);
	BENCHMARK("clear") {
		map.clear();
	}
}
