#include "test_base.h"

#include <map>
#include <unordered_map>

TEMPLATE_TEST_CASE("benchmark random insert erase", "[!benchmark]", (robin_hood::flat_map<int, int>), (robin_hood::node_map<int, int>),
				   (std::unordered_map<int, int>), (std::map<int, int>)) {
	Rng rng(123);
	TestType map;

	static const size_t maxVal = 100000;

	BENCHMARK("Random insert erase") {
		for (int i = 0; i < 10'000'000; ++i) {
			map[rng(maxVal)] = i;
			map[rng(maxVal) << 8] = i;
			map[rng(maxVal) << 16] = i;
			map[rng(maxVal) << 24] = i;
			map[rng(maxVal) << 32] = i;
			map[rng(maxVal) << 40] = i;
			map[rng(maxVal) << 48] = i;
			map[rng(maxVal) << 56] = i;
			map.erase(rng(maxVal));
			map.erase(rng(maxVal) << 8);
			map.erase(rng(maxVal) << 16);
			map.erase(rng(maxVal) << 24);
			map.erase(rng(maxVal) << 32);
			map.erase(rng(maxVal) << 40);
			map.erase(rng(maxVal) << 48);
			map.erase(rng(maxVal) << 56);
		}
	}
	REQUIRE(map.size() == 132147);

	size_t sum_key = 0;
	size_t sum_val = 0;
	BENCHMARK("iterating") {
		for (int i = 0; i < 1000; ++i) {
			for (auto const& kv : map) {
				sum_key += kv.first;
				sum_val += kv.second;
			}
		}
	}
	REQUIRE(sum_key + sum_val == 0x778c26a1b4070);

	BENCHMARK("clear") {
		map.clear();
	}
}