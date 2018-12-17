#include <robin_hood.h>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "sfc64.h"

// final step from MurmurHash3
uint64_t fmix64(uint64_t k) {
	k ^= k >> 33;
	k *= 0xff51afd7ed558ccdULL;
	k ^= k >> 33;
	k *= 0xc4ceb9fe1a85ec53ULL;
	k ^= k >> 33;
	return k;
}

// from boost::hash_combine, with additional fmix64 of value
uint64_t hash_combine(uint64_t seed, uint64_t value) {
	return seed ^ (fmix64(value) + 0x9e3779b9 + (seed << 6) + (seed >> 2));
}

template <class T>
inline uint64_t hash_value(const T& value) {
	return std::hash<typename std::decay<decltype(value)>::type>{}(value);
}

// calculates a hash of any iterable map. Order is irrelevant.
template <typename M>
inline uint64_t hash(const M& map) {
	uint64_t h = 1;

	size_t numElements = 0;
	for (auto const& entry : map) {
		auto entry_hash = hash_combine(0, hash_value(entry.first));
		entry_hash = hash_combine(entry_hash, hash_value(entry.second));

		// make sure we never multiply by 0
		if (entry_hash == 0) {
			entry_hash = UINT64_C(0x853c49e6748fea9b);
		}

		h *= entry_hash;
		++numElements;
	}

	hash_combine(h, numElements);
	return h;
}
TEST_CASE("random insert & erase") {}

TEST_CASE("Api demonstration") {
	robin_hood::unordered_map<int, int> map;
	map[123] = 7;
	map[42] = 543;

	for (auto const& kv : map) {
		std::cout << kv.first << " -> " << kv.second << std::endl;
	}
}