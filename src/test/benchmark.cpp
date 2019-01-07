#include "test_base.h"

#include <map>
#include <unordered_map>

TEMPLATE_TEST_CASE("hash std::string", "[!benchmark]", (robin_hood::hash<std::string>), (std::hash<std::string>)) {
	size_t h = 0;
	Rng rng(123);
	auto hasher = TestType{};
	for (const int s : {8, 11, 100, 256}) {
		std::string str(static_cast<size_t>(s), 'x');
		for (size_t i = 0; i < str.size(); ++i) {
			str[i] = rng.uniform<char>();
		}
		BENCHMARK("std::string length " + std::to_string(str.size())) {
			for (size_t i = 0; i < 100000000u; ++i) {
				// modify string to prevent optimization
				*reinterpret_cast<uint64_t*>(&str[0]) = rng();
				h += hasher(str);
			}
		}
	}
	// prevent optimization
	INFO(h);
}

TEMPLATE_TEST_CASE("hash integers", "[!benchmark]", uint8_t, int8_t, uint16_t, int16_t, uint32_t, int32_t, uint64_t, int64_t) {
	size_t a = 0;
	size_t b = 0;
	size_t c = 0;
	size_t d = 0;

	Rng rng(123);
	BENCHMARK("robin_hood::hash") {
		robin_hood::hash<TestType> hasher;
		for (int i = 0; i < 1000000000; i += 4) {
			a += hasher(static_cast<TestType>(i));
			b += hasher(static_cast<TestType>(i + 1));
			c += hasher(static_cast<TestType>(i + 2));
			d += hasher(static_cast<TestType>(i + 3));
		}
	}
	INFO(a + b + c + d);

	BENCHMARK("std::hash") {
		std::hash<TestType> hasher;
		for (int i = 0; i < 1000000000; i += 4) {
			a += hasher(static_cast<TestType>(i));
			b += hasher(static_cast<TestType>(i + 1));
			c += hasher(static_cast<TestType>(i + 2));
			d += hasher(static_cast<TestType>(i + 3));
		}
	}
	INFO(a + b + c + d);
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

TEMPLATE_TEST_CASE("insert & erase & clear", "[!benchmark]", (robin_hood::flat_map<int, int>), (robin_hood::node_map<int, int>),
				   (std::unordered_map<int, int>)) {
	Rng rng(123);

	uint64_t verifier = 0;
	BENCHMARK("Random insert erase") {
		TestType map;
		for (int n = 1; n < 10'000; ++n) {
			for (int i = 0; i < 10'000; ++i) {
				map[rng.uniform<int>(static_cast<uint64_t>(n))] = i;
				verifier += map.erase(rng.uniform<int>(static_cast<uint64_t>(n)));
			}
		}
	}

	REQUIRE(verifier == 50024052);
}

// benchmark adapted from https://github.com/attractivechaos/udb2
// this implementation should have less overhead, because sfc64 and it's uniform() is extremely fast.
TEMPLATE_TEST_CASE("25% distinct", "[!benchmark]", (robin_hood::unordered_map<int, int>), (std::unordered_map<int, int>)) {
	Rng rng(123);

	int checksum = 0;
	size_t const upper = 50'000'000;
	size_t const lower = 10'000'000;
	size_t const num_steps = 5;
	size_t const step_width = (upper - lower) / num_steps;

	for (size_t n = lower; n <= upper; n += step_width) {
		size_t const max_rng = n / 4;
		BENCHMARK(std::to_string(n) + " ") {
			TestType map;
			for (size_t i = 0; i < n; ++i) {
				checksum += ++map[rng.uniform<int>(max_rng)];
			}
		}
	}

	REQUIRE(checksum == 539967125);
}

TEMPLATE_TEST_CASE("random find 50% existing", "[!benchmark]", (robin_hood::unordered_map<uint64_t, int>), (std::unordered_map<uint64_t, int>)) {
	Rng rng(777);

	TestType map;
	size_t found = 0;
	size_t not_found = 0;
	BENCHMARK("random find 50% existing") {
		for (size_t iters = 0; iters < 20; ++iters) {
			auto before_insert_rng_state = rng.state();
			for (int j = 0; j < 100'000; ++j) {
				map[rng()];
				// skip one rng number
				rng();
			}
			auto const final_rng = rng();

			// the search a few times
			for (size_t num_searches = 0; num_searches < 10; ++num_searches) {
				rng.state(before_insert_rng_state);
				uint64_t search_val = 0;
				do {
					search_val = rng();
					auto it = map.find(search_val);
					if (it != map.end()) {
						++found;
					} else {
						++not_found;
					}

				} while (search_val != final_rng);
			}
		}
	}

	REQUIRE(found == 20000000);
	REQUIRE(not_found == 20000200);
}
