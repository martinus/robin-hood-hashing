#include "test_base.h"

#include <map>
#include <unordered_map>

TEMPLATE_TEST_CASE("benchmark random insert erase", "[!benchmark]", (robin_hood::flat_map<uint64_t, int>), (robin_hood::node_map<uint64_t, int>),
				   (std::unordered_map<uint64_t, int>), (std::map<uint64_t, int>)) {
	Rng rng(123);
	TestType map;

	static const size_t maxVal = 100000;

	BENCHMARK("Random insert erase") {
		for (int i = 0; i < 50'000'000; ++i) {
			map[rng(maxVal)] = i;
			map.erase(rng(maxVal));
		}
	}
	REQUIRE(map.size() == 49789);

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
	REQUIRE(sum_key + sum_val == 0x8d8251bb58620);
	BENCHMARK("clear") {
		map.clear();
	}
}