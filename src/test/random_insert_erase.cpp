#include <robin_hood.h>

#include "catch.hpp"
#include "sfc64.h"

#include <unordered_map>

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
	uint64_t combined_hash = 1;

	size_t numElements = 0;
	for (auto const& entry : map) {
		auto entry_hash = hash_combine(hash_value(entry.first), hash_value(entry.second));

		combined_hash ^= entry_hash;
		++numElements;
	}

	return hash_combine(combined_hash, numElements);
}

TEST_CASE("random insert & erase") {
	sfc64 rng(123);

	robin_hood::unordered_map<int, int> map;
	for (size_t i = 0; i < 10000; ++i) {
		map[rng(i)] = rng(i);
		map.erase(rng(i));
	}

	// number generated with std::unordered_map
	REQUIRE(hash(map) == UINT64_C(0x1215b1b2d32adfff));
}
