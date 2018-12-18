#include "test_base.h"

#include <unordered_map>
#include <unordered_set>

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

// calculates a hash of any iterable map. Order is irrelevant for the hash's result, as it simply xors the elements together.
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

// randomly modifies a map and then checks if an std::unordered_map gave the same result.
TEMPLATE_TEST_CASE("random insert & erase", "", FlatMap, NodeMap) {
	Rng rng(123);

	robin_hood::unordered_map<int, int> map;
	for (size_t i = 0; i < 10000; ++i) {
		map[rng(i)] = rng(i);
		map.erase(rng(i));
	}

	// number generated with std::unordered_map
	REQUIRE(hash(map) == UINT64_C(0x1215b1b2d32adfff));
}

class CtorDtorVerifier {
public:
	CtorDtorVerifier(int val)
		: mVal(val) {
		REQUIRE(mConstructedAddresses.insert(this).second);
		if (mDoPrintDebugInfo) {
			std::cout << this << " ctor(int) " << mConstructedAddresses.size() << std::endl;
		}
	}

	CtorDtorVerifier(size_t val)
		: mVal(static_cast<int>(val)) {
		REQUIRE(mConstructedAddresses.insert(this).second);
		if (mDoPrintDebugInfo) {
			std::cout << this << " ctor(size_t) " << mConstructedAddresses.size() << std::endl;
		}
	}

	CtorDtorVerifier()
		: mVal(-1) {
		REQUIRE(mConstructedAddresses.insert(this).second);
		if (mDoPrintDebugInfo) {
			std::cout << this << " ctor() " << mConstructedAddresses.size() << std::endl;
		}
	}

	CtorDtorVerifier(const CtorDtorVerifier& o)
		: mVal(o.mVal) {
		REQUIRE(mConstructedAddresses.insert(this).second);
		if (mDoPrintDebugInfo) {
			std::cout << this << " ctor(const CtorDtorVerifier& o) " << mConstructedAddresses.size() << std::endl;
		}
	}

	CtorDtorVerifier& operator=(CtorDtorVerifier&& o) {
		REQUIRE(1 == mConstructedAddresses.count(this));
		REQUIRE(1 == mConstructedAddresses.count(&o));
		mVal = std::move(o.mVal);
		return *this;
	}

	CtorDtorVerifier& operator=(const CtorDtorVerifier& o) {
		REQUIRE(1 == mConstructedAddresses.count(this));
		REQUIRE(1 == mConstructedAddresses.count(&o));
		mVal = o.mVal;
		return *this;
	}

	bool operator==(const CtorDtorVerifier& o) const {
		return mVal == o.mVal;
	}

	bool operator!=(const CtorDtorVerifier& o) {
		return mVal != o.mVal;
	}

	~CtorDtorVerifier() {
		REQUIRE(1 == mConstructedAddresses.erase(this));
		if (mDoPrintDebugInfo) {
			std::cout << this << " dtor " << mConstructedAddresses.size() << std::endl;
		}
	}

	bool eq(const CtorDtorVerifier& o) const {
		return mVal == o.mVal;
	}

	int val() const {
		return mVal;
	}

	static size_t mapSize() {
		return mConstructedAddresses.size();
	}

	static void printMap() {
		std::cout << "data in map:" << std::endl;
		for (std::unordered_set<CtorDtorVerifier const*>::const_iterator it = mConstructedAddresses.begin(), end = mConstructedAddresses.end();
			 it != end; ++it) {
			std::cout << "\t" << *it << std::endl;
		}
	}

	static bool contains(CtorDtorVerifier const* ptr) {
		return 1 == mConstructedAddresses.count(ptr);
	}

private:
	int mVal;
	static std::unordered_set<CtorDtorVerifier const*> mConstructedAddresses;
	static bool mDoPrintDebugInfo;
};

std::unordered_set<CtorDtorVerifier const*> CtorDtorVerifier::mConstructedAddresses;
bool CtorDtorVerifier::mDoPrintDebugInfo = false;

namespace std {
template <>
struct hash<CtorDtorVerifier> {
	std::size_t operator()(const CtorDtorVerifier& t) const {
		// hash is bad on purpose
		const size_t bitmaskWithoutLastBits = ~(size_t)5;
		return static_cast<size_t>(t.val()) & bitmaskWithoutLastBits;
	}
};

} // namespace std

using FlatMapVerifier = robin_hood::flat_map<CtorDtorVerifier, CtorDtorVerifier>;
using NodeMapVerifier = robin_hood::node_map<CtorDtorVerifier, CtorDtorVerifier>;

TEMPLATE_TEST_CASE("random insert & erase", "", FlatMapVerifier, NodeMapVerifier) {
	using Map = TestType;

	Map rhhs;
	REQUIRE(rhhs.size() == (size_t)0);
	std::pair<typename Map::iterator, bool> it = rhhs.insert(typename Map::value_type{32145, 123});
	REQUIRE(it.second);
	REQUIRE(it.first->first.val() == 32145);
	REQUIRE(it.first->second.val() == 123);
	REQUIRE(rhhs.size() == 1);

	const int times = 10000;
	for (int i = 0; i < times; ++i) {
		std::pair<typename Map::iterator, bool> it = rhhs.insert(typename Map::value_type(i * 4, i));

		REQUIRE(it.second);
		REQUIRE(it.first->first.val() == i * 4);
		REQUIRE(it.first->second.val() == i);

		typename Map::iterator found = rhhs.find(i * 4);
		REQUIRE(rhhs.end() != found);
		REQUIRE(found->second.val() == i);
		REQUIRE(rhhs.size() == (size_t)(2 + i));
	}

	// check if everything can be found
	for (int i = 0; i < times; ++i) {
		typename Map::iterator found = rhhs.find(i * 4);
		REQUIRE(rhhs.end() != found);
		REQUIRE(found->second.val() == i);
		REQUIRE(found->first.val() == i * 4);
	}

	// check non-elements
	for (int i = 0; i < times; ++i) {
		typename Map::iterator found = rhhs.find((i + times) * 4);
		REQUIRE(rhhs.end() == found);
	}

	// random test against std::unordered_map
	rhhs.clear();
	std::unordered_map<int, int> uo;

	Rng gen(123);

	for (int i = 0; i < times; ++i) {
		int r = gen(times / 4);
		std::pair<typename Map::iterator, bool> rhh_it = rhhs.insert(typename Map::value_type(r, r * 2));
		std::pair<std::unordered_map<int, int>::iterator, bool> uo_it = uo.insert(std::make_pair(r, r * 2));
		REQUIRE(rhh_it.second == uo_it.second);
		REQUIRE(rhh_it.first->first.val() == uo_it.first->first);
		REQUIRE(rhh_it.first->second.val() == uo_it.first->second);
		REQUIRE(rhhs.size() == uo.size());

		r = gen(times / 4);
		typename Map::iterator rhhsIt = rhhs.find(r);
		std::unordered_map<int, int>::iterator uoIt = uo.find(r);
		REQUIRE((rhhs.end() == rhhsIt) == (uo.end() == uoIt));
		if (rhhs.end() != rhhsIt) {
			REQUIRE(rhhsIt->first.val() == uoIt->first);
			REQUIRE(rhhsIt->second.val() == uoIt->second);
		}
	}

	uo.clear();
	rhhs.clear();
	for (int i = 0; i < times; ++i) {
		const int r = gen(times / 4);
		rhhs[r] = r * 2;
		uo[r] = r * 2;
		REQUIRE(rhhs.find(r)->second.val() == uo.find(r)->second);
		REQUIRE(rhhs.size() == uo.size());
	}

	std::size_t numChecks = 0;
	for (typename Map::const_iterator it = rhhs.begin(); it != rhhs.end(); ++it) {
		REQUIRE(uo.end() != uo.find(it->first.val()));
		++numChecks;
	}
	REQUIRE(rhhs.size() == numChecks);

	numChecks = 0;
	const Map& constRhhs = rhhs;
	for (const typename Map::value_type& vt : constRhhs) {
		REQUIRE(uo.end() != uo.find(vt.first.val()));
		++numChecks;
	}
	REQUIRE(rhhs.size() == numChecks);
}

template <class M>
void fill(M& map, size_t num, bool isExisting = true) {
	if (isExisting) {
		for (size_t i = 0; i < num; ++i) {
			map[static_cast<typename M::key_type>(i)];
		}
	} else {
		for (size_t i = 0; i < num; ++i) {
			map[static_cast<typename M::key_type>(i + num)];
		}
	}
}

TEMPLATE_TEST_CASE("assign, iterate, clear", "", FlatMapVerifier, NodeMapVerifier) {
	using Map = TestType;
	{
		Map m;
		fill(m, 1);
		for (typename Map::const_iterator it = m.begin(); it != m.end(); ++it) {
			REQUIRE(CtorDtorVerifier::contains(&it->first));
			REQUIRE(CtorDtorVerifier::contains(&it->second));
		}
		// dtor
	}
	if (0 != CtorDtorVerifier::mapSize()) {
		CtorDtorVerifier::printMap();
	}
	REQUIRE(CtorDtorVerifier::mapSize() == 0);
}

TEMPLATE_TEST_CASE("random insert & erase with Verifier", "", FlatMapVerifier, NodeMapVerifier) {
	Rng rng(123);

	TestType map;
	for (size_t i = 0; i < 10000; ++i) {
		map[rng(i)] = rng(i);
		map.erase(rng(i));
	}

	INFO("map size is " << CtorDtorVerifier::mapSize());
	REQUIRE(CtorDtorVerifier::mapSize() == 6620);
	REQUIRE(hash(map) == UINT64_C(0xa1931649d033a499));
	map.clear();

	REQUIRE(CtorDtorVerifier::mapSize() == 0);
	REQUIRE(hash(map) == UINT64_C(0x9e3779f8));
}
