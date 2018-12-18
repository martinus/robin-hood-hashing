#include "test_base.h"

#include <map>
#include <unordered_map>

static size_t sCountCtor = 0;
static size_t sCountDefaultCtor = 0;
static size_t sCountCopyCtor = 0;
static size_t sCountDtor = 0;
static size_t sCountEquals = 0;
static size_t sCountLess = 0;
static size_t sCountAssign = 0;
static size_t sCountSwaps = 0;
static size_t sCountGet = 0;
static size_t sCountConstGet = 0;
static size_t sCountHash = 0;
static size_t sCountMoveCtor = 0;
static size_t sCountMoveAssign = 0;

static bool sHasHeaderPrinted = false;

void resetStaticCounts() {
	sCountCtor = 0;
	sCountDefaultCtor = 0;
	sCountCopyCtor = 0;
	sCountDtor = 0;
	sCountEquals = 0;
	sCountLess = 0;
	sCountAssign = 0;
	sCountSwaps = 0;
	sCountGet = 0;
	sCountConstGet = 0;
	sCountHash = 0;
	sCountMoveCtor = 0;
	sCountMoveAssign = 0;
}

void printStaticHeaderOnce() {
	if (sHasHeaderPrinted) {
		return;
	}
	printf("    ctor   defctor  cpyctor     dtor   assign    swaps      get  cnstget     hash   equals     less   ctormv assignmv |   total\n");
	sHasHeaderPrinted = true;
}

void printStaticCounts(std::string const& title) {
	size_t total = sCountCtor + sCountDefaultCtor + sCountCopyCtor + sCountDtor + sCountEquals + sCountLess + sCountAssign + sCountSwaps + sCountGet +
				   sCountConstGet + sCountHash + sCountMoveCtor + sCountMoveAssign;

	printf("%9zu%9zu%9zu%9zu%9zu%9zu%9zu%9zu%9zu%9zu%9zu%9zu%9zu |%9zu %s\n", sCountCtor, sCountDefaultCtor, sCountCopyCtor, sCountDtor, sCountAssign,
		   sCountSwaps, sCountGet, sCountConstGet, sCountHash, sCountEquals, sCountLess, sCountMoveCtor, sCountMoveAssign, total, title.c_str());
}

class Counter {
public:
	Counter(const int& data)
		: mData(data) {
		++sCountCtor;
	}

	Counter()
		: mData() {
		++sCountDefaultCtor;
	}

	Counter(const Counter& o)
		: mData(o.mData) {
		++sCountCopyCtor;
	}

	Counter(Counter&& o)
		: mData(std::move(o.mData)) {
		++sCountMoveCtor;
	}

	~Counter() {
		++sCountDtor;
	}

	bool operator==(const Counter& o) const {
		++sCountEquals;
		return mData == o.mData;
	}

	bool operator<(const Counter& o) const {
		++sCountLess;
		return mData < o.mData;
	}

	Counter& operator=(const Counter& o) {
		++sCountAssign;
		mData = o.mData;
		return *this;
	}

	Counter& operator=(Counter&& o) {
		++sCountMoveAssign;
		mData = std::move(o.mData);
		return *this;
	}

	const int& get() const {
		++sCountConstGet;
		return mData;
	}

	int& get() {
		++sCountGet;
		return mData;
	}

	void swap(Counter& other) {
		++sCountSwaps;
		using std::swap;
		swap(mData, other.mData);
	}

private:
	int mData;
};

void swap(Counter& a, Counter& b) {
	a.swap(b);
}

namespace std {

template <>
class hash<Counter> {
public:
	size_t operator()(const Counter& c) const {
		++sCountHash;
		return hash<int>()(c.get());
	}
};

} // namespace std

template <typename K, typename V>
const char* name(std::unordered_map<K, V> const&) {
	return "std::unordered_map";
}

template <typename K, typename V>
const char* name(std::map<K, V> const&) {
	return "std::map";
}

template <typename K, typename V>
const char* name(robin_hood::flat_map<K, V> const&) {
	return "robin_hood::flat_map";
}

template <typename K, typename V>
const char* name(robin_hood::node_map<K, V> const&) {
	return "robin_hood::node_map";
}

TEMPLATE_TEST_CASE("counting random insert & erase", "[display]", (std::map<Counter, Counter>), (std::unordered_map<Counter, Counter>),
				   (robin_hood::flat_map<Counter, Counter>), (robin_hood::node_map<Counter, Counter>)) {

	printStaticHeaderOnce();

	resetStaticCounts();
	{
		Rng rng(321);
		TestType map;
		for (size_t i = 0; i < 10000; ++i) {
			map[rng(i)] = rng(i);
			map.erase(rng(i));
		}
	}

	REQUIRE(sCountDtor == sCountCtor + sCountDefaultCtor + sCountCopyCtor + sCountMoveCtor);
	printStaticCounts(std::string("10k random insert&erase for ") + name(TestType{}));
	resetStaticCounts();
}

TEMPLATE_TEST_CASE("counting ctor and dtor", "[display]", (std::map<Counter, Counter>), (std::unordered_map<Counter, Counter>),
				   (robin_hood::flat_map<Counter, Counter>), (robin_hood::node_map<Counter, Counter>)) {

	printStaticHeaderOnce();

	resetStaticCounts();
	{ TestType map; }
	REQUIRE(sCountDtor == sCountCtor + sCountDefaultCtor + sCountCopyCtor + sCountMoveCtor);
	printStaticCounts(std::string("ctor & dtor ") + name(TestType{}));
	resetStaticCounts();
}

TEMPLATE_TEST_CASE("counting ctor and dtor & single emplace", "[display]", (std::map<Counter, Counter>), (std::unordered_map<Counter, Counter>),
				   (robin_hood::flat_map<Counter, Counter>), (robin_hood::node_map<Counter, Counter>)) {

	printStaticHeaderOnce();

	resetStaticCounts();
	{
		TestType map;
		map.emplace(1, 2);
	}
	REQUIRE(sCountDtor == sCountCtor + sCountDefaultCtor + sCountCopyCtor + sCountMoveCtor);
	printStaticCounts(std::string("single emplace ") + name(TestType{}));
	resetStaticCounts();
}

TEMPLATE_TEST_CASE("100 emplace", "[display]", (std::map<Counter, Counter>), (std::unordered_map<Counter, Counter>),
				   (robin_hood::flat_map<Counter, Counter>), (robin_hood::node_map<Counter, Counter>)) {

	printStaticHeaderOnce();

	resetStaticCounts();
	{
		TestType map;
		for (int i = 0; i < 100; ++i) {
			map.emplace(i, i);
		}
	}
	REQUIRE(sCountDtor == sCountCtor + sCountDefaultCtor + sCountCopyCtor + sCountMoveCtor);
	printStaticCounts(std::string("100 emplace ") + name(TestType{}));
	resetStaticCounts();
}