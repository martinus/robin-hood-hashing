#include "test_base.h"

#include <bitset>
#include <iomanip>
#include <list>
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

TEMPLATE_TEST_CASE("map ctor & dtor", "[display]", (std::map<Counter, Counter>), (std::unordered_map<Counter, Counter>),
				   (robin_hood::flat_map<Counter, Counter>), (robin_hood::node_map<Counter, Counter>)) {

	printStaticHeaderOnce();

	resetStaticCounts();
	{ TestType map; }
	printStaticCounts(std::string("ctor & dtor ") + name(TestType{}));
	REQUIRE(sCountDtor == sCountCtor + sCountDefaultCtor + sCountCopyCtor + sCountMoveCtor);
	REQUIRE(sCountDtor == 0);
	resetStaticCounts();
}

TEMPLATE_TEST_CASE("1 emplace", "[display]", (std::map<Counter, Counter>), (std::unordered_map<Counter, Counter>),
				   (robin_hood::flat_map<Counter, Counter>), (robin_hood::node_map<Counter, Counter>)) {

	printStaticHeaderOnce();

	resetStaticCounts();
	{
		TestType map;
		map.emplace(1, 2);
	}
	printStaticCounts(std::string("1 emplace ") + name(TestType{}));
	REQUIRE(sCountDtor == sCountCtor + sCountDefaultCtor + sCountCopyCtor + sCountMoveCtor);
	resetStaticCounts();
}

TEMPLATE_TEST_CASE("10k random insert & erase", "[display]", (std::map<Counter, Counter>), (std::unordered_map<Counter, Counter>),
				   (robin_hood::flat_map<Counter, Counter>), (robin_hood::node_map<Counter, Counter>)) {

	printStaticHeaderOnce();

	resetStaticCounts();
	{
		Rng rng(321);
		TestType map;
		for (size_t i = 0; i < 10000; ++i) {
			for (size_t j = 0; j < 10; ++j) {
				map[rng(i)] = i;
				map.erase(rng(i));

				map.emplace(rng(i), i);
				map.erase(rng(i));

				map.insert(typename TestType::value_type{rng(i), i});
				map.erase(rng(i));
			}
		}
	}

	printStaticCounts(std::string("10k random insert & erase - ") + name(TestType{}));
	REQUIRE(sCountDtor == sCountCtor + sCountDefaultCtor + sCountCopyCtor + sCountMoveCtor);
	resetStaticCounts();
}

#define PRINT_SIZEOF(x, A, B) std::cout << sizeof(x<A, B>) << " bytes for " #x "<" #A ", " #B ">" << std::endl

struct BigObject {
	std::string str;
	std::vector<int> vec;
	std::shared_ptr<int> ptr;
	std::list<int> list;
};

namespace std {

template <>
class hash<BigObject> {
public:
	size_t operator()(BigObject const& o) const {
		return 0;
	}
};

} // namespace std

TEST_CASE("show datastructure sizes") {
	PRINT_SIZEOF(robin_hood::unordered_map, int, int);
	PRINT_SIZEOF(std::map, int, int);
	PRINT_SIZEOF(std::unordered_map, int, int);
	std::cout << std::endl;

	PRINT_SIZEOF(robin_hood::unordered_map, int, BigObject);
	PRINT_SIZEOF(std::map, int, BigObject);
	PRINT_SIZEOF(std::unordered_map, int, BigObject);
	std::cout << std::endl;

	PRINT_SIZEOF(robin_hood::unordered_map, BigObject, BigObject);
	PRINT_SIZEOF(std::map, BigObject, BigObject);
	PRINT_SIZEOF(std::unordered_map, BigObject, BigObject);
	std::cout << std::endl;

	PRINT_SIZEOF(robin_hood::unordered_map, BigObject, int);
	PRINT_SIZEOF(std::map, BigObject, int);
	PRINT_SIZEOF(std::unordered_map, BigObject, int);
}

void showHash(uint64_t val) {
	auto qm = robin_hood::detail::quickmix(val);
	std::cout << std::setfill('0') << std::setw(16) << std::hex << val << " -> " << std::setfill('0') << std::setw(16) << std::hex << qm << " "
			  << std::bitset<64>(qm) << std::endl;
}

TEST_CASE("show hash distribution") {
	std::cout << "input               output hex       output binary" << std::endl;
	for (uint64_t i = 0; i < 16; ++i) {
		showHash(i);
	}

	for (uint64_t i = 0; i < 5; ++i) {
		showHash(0x000023d700000063ULL + i * 0x100000000ULL);
	}

	for (uint64_t i = 0; i < 5; ++i) {
		showHash(i * 0x1000000000000000ULL);
	}

	for (uint64_t i = 1; i != 0; i *= 2) {
		showHash(i);
	}
}