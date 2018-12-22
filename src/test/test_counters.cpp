#include "test_base.h"

#include <bitset>
#include <iomanip>
#include <list>
#include <map>
#include <unordered_map>

static bool sHasHeaderPrinted = false;

class Counter {
	static size_t sCountCtor;
	static size_t sCountDefaultCtor;
	static size_t sCountCopyCtor;
	static size_t sCountDtor;
	static size_t sCountEquals;
	static size_t sCountLess;
	static size_t sCountAssign;
	static size_t sCountSwaps;
	static size_t sCountGet;
	static size_t sCountConstGet;
	static size_t sCountHash;
	static size_t sCountMoveCtor;
	static size_t sCountMoveAssign;

public:
	static void resetStaticCounts() {
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

	static void printStaticHeaderOnce() {
		if (sHasHeaderPrinted) {
			return;
		}
		printf("    ctor   defctor  cpyctor     dtor   assign    swaps      get  cnstget     hash   equals     less   ctormv assignmv |   total\n");
		sHasHeaderPrinted = true;
	}

	static void printStaticCounts(std::string const& title) {
		size_t total = sCountCtor + sCountDefaultCtor + sCountCopyCtor + sCountDtor + sCountEquals + sCountLess + sCountAssign + sCountSwaps +
					   sCountGet + sCountConstGet + sCountHash + sCountMoveCtor + sCountMoveAssign;

		printf("%9zu%9zu%9zu%9zu%9zu%9zu%9zu%9zu%9zu%9zu%9zu%9zu%9zu |%9zu %s\n", sCountCtor, sCountDefaultCtor, sCountCopyCtor, sCountDtor,
			   sCountAssign, sCountSwaps, sCountGet, sCountConstGet, sCountHash, sCountEquals, sCountLess, sCountMoveCtor, sCountMoveAssign, total,
			   title.c_str());
	}

	static size_t countCtor() {
		return sCountCtor + sCountDefaultCtor + sCountCopyCtor + sCountMoveCtor;
	}

	static size_t countDtor() {
		return sCountDtor;
	}

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

	size_t hash() const {
		++sCountHash;
		return std::hash<int>{}(mData);
	}

private:
	int mData;
};

size_t Counter::sCountCtor{};
size_t Counter::sCountDefaultCtor{};
size_t Counter::sCountCopyCtor{};
size_t Counter::sCountDtor{};
size_t Counter::sCountEquals{};
size_t Counter::sCountLess{};
size_t Counter::sCountAssign{};
size_t Counter::sCountSwaps{};
size_t Counter::sCountGet{};
size_t Counter::sCountConstGet{};
size_t Counter::sCountHash{};
size_t Counter::sCountMoveCtor{};
size_t Counter::sCountMoveAssign{};

void swap(Counter& a, Counter& b) {
	a.swap(b);
}

namespace std {

template <>
class hash<Counter> {
public:
	size_t operator()(const Counter& c) const {
		return c.hash();
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

	Counter::printStaticHeaderOnce();

	Counter::resetStaticCounts();
	{ TestType map; }
	Counter::printStaticCounts(std::string("ctor & dtor ") + name(TestType{}));
	REQUIRE(Counter::countDtor() == Counter::countCtor());
	REQUIRE(Counter::countDtor() == 0);
	Counter::resetStaticCounts();
}

TEMPLATE_TEST_CASE("1 emplace", "[display]", (std::map<Counter, Counter>), (std::unordered_map<Counter, Counter>),
				   (robin_hood::flat_map<Counter, Counter>), (robin_hood::node_map<Counter, Counter>)) {

	Counter::printStaticHeaderOnce();

	Counter::resetStaticCounts();
	{
		TestType map;
		map.emplace(1, 2);
	}
	Counter::printStaticCounts(std::string("1 emplace ") + name(TestType{}));
	REQUIRE(Counter::countDtor() == Counter::countCtor());
	Counter::resetStaticCounts();
}

TEMPLATE_TEST_CASE("10k random insert & erase", "[display]", (std::map<Counter, Counter>), (std::unordered_map<Counter, Counter>),
				   (robin_hood::flat_map<Counter, Counter>), (robin_hood::node_map<Counter, Counter>)) {

	Counter::printStaticHeaderOnce();

	Counter::resetStaticCounts();
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

	Counter::printStaticCounts(std::string("10k random insert & erase - ") + name(TestType{}));
	REQUIRE(Counter::countDtor() == Counter::countCtor());
	Counter::resetStaticCounts();
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