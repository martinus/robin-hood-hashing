

boost::unordered_set<CtorDtorVerifier const*> CtorDtorVerifier::mConstructedAddresses;
bool CtorDtorVerifier::mDoPrintDebugInfo = false;

bool operator==(const CtorDtorVerifier& a, const CtorDtorVerifier& b) {
	return a.eq(b);
}

namespace util {
namespace RobinHood {

template <>
struct FastHash<CtorDtorVerifier> {
	std::size_t operator()(const CtorDtorVerifier& t) const {
		// hash is bad on purpose
		const size_t bitmaskWithoutLastBits = ~(size_t)5;
		return static_cast<size_t>(t.val()) & bitmaskWithoutLastBits;
	}
};

} // namespace RobinHood
} // namespace util

template <class T>
void benchmarkCtor(size_t fixedNumIters, const std::string& typeName) {
	util::MicroBenchmark mb;
	mb.fixedIterationsPerMeasurement(fixedNumIters);
	std::size_t optPrevention = 0;

	if (mb.isAutoTimed()) {
		std::string msg = std::string("ctor boost::unordered_map<int, ") + typeName + std::string(">");
		const char* title = msg.c_str();
		while (mb(title, optPrevention)) {
			boost::unordered_map<int, T, util::RobinHood::FastHash<int>> m;
			util::MicroBenchmark::doNotOptimize(m);
		}
	}

	optPrevention = 0;
	std::string msg = std::string("ctor util::RobinHood::Map<int, ") + typeName + std::string(">");
	const char* title = msg.c_str();
	while (mb(title)) {
		util::RobinHood::Map<int, T, util::RobinHood::FastHash<int>> m;
		util::MicroBenchmark::doNotOptimize(m);
	}
}

TEST_CASE(RobinHoodMapTest, PERFORMANCE_SLOW_benchmarkCtorInt) {
	benchmarkCtor<int>(20 * 1000 * 1000, "int");
}

TEST_CASE(RobinHoodMapTest, PERFORMANCE_SLOW_benchmarkCtorBigObject) {
	benchmarkCtor<BigObject>(20 * 1000 * 1000, "BigObject");
}

template <class M>
void benchCopy(util::MicroBenchmark& mb, const uint32_t param, const char* title) {
	// fill map with random data
	boost::mt19937 gen(123);
	M map;
	for (uint32_t i = 0; i < param; ++i) {
		map[gen() % param];
	}

	std::size_t optPrevention = 0;
	while (mb(title, optPrevention)) {
		M mapCpy = map;
		optPrevention += mapCpy.size();
	}
}

TEST_CASE(RobinHoodMapTest, PERFORMANCE_SLOW_benchmarkCopyInt) {
	uint32_t numElements = 5000;

	util::MicroBenchmark mb;
	mb.fixedIterationsPerMeasurement(100 * 1000);

	if (mb.isAutoTimed()) {
		benchCopy<boost::unordered_map<int, int, util::RobinHood::FastHash<int>>>(mb, numElements, "copy boost::unordered_map<int, int>");
	}
	benchCopy<util::RobinHood::Map<int, int>>(mb, numElements, "copy util::RobinHood<int, int>");
}

TEST_CASE(RobinHoodMapTest, PERFORMANCE_SLOW_benchmarkCopyBigObject) {
	uint32_t numElements = 5000;

	util::MicroBenchmark mb;
	mb.fixedIterationsPerMeasurement(500);
	if (mb.isAutoTimed()) {
		benchCopy<boost::unordered_map<int, BigObject, util::RobinHood::FastHash<int>>>(mb, numElements, "copy boost::unordered_map<int, BigObject>");
	}
	benchCopy<util::RobinHood::Map<int, BigObject>>(mb, numElements, "copy util::RobinHood<int, BigObject>");
}

template <class T>
void benchmarkCtorAndAddElements(size_t fixedNumIters, size_t numInserts, const std::string& typeName) {
	if (fixedNumIters == 0) {
		fixedNumIters = 1;
	}
	util::MicroBenchmark mb;
	mb.fixedIterationsPerMeasurement(fixedNumIters);
	mb.unitScaling("insert", numInserts);
	size_t pos = 0;

	if (mb.isAutoTimed()) {
		std::string msg = std::string("boost::unordered_map<int, ") + typeName + std::string(">");
		const char* title = msg.c_str();
		size_t noOpt = 0;
		while (mb(title, noOpt)) {
			boost::unordered_map<size_t, T, util::RobinHood::FastHash<size_t>> m;
			for (size_t i = 0; i < numInserts; ++i) {
				m[pos++];
			}
			noOpt += m.size();
		}
	}

	pos = 0;
	std::stringstream ss;
	ss << "util::RobinHood::Map<int, " << typeName << ">, " << numInserts << " elements";
	std::string str = ss.str();
	const char* title = str.c_str();

	size_t noOpt = 0;
	while (mb(title, noOpt)) {
		util::RobinHood::Map<size_t, T> m;
		for (size_t i = 0; i < numInserts; ++i) {
			m[pos++];
		}
		noOpt += m.size();
	}
}

class RobinHoodMapSize10Params : public ::testing::TestWithParam<int> {};

INSTANTIATE_TEST_CASE_P(RobinHoodInstance, RobinHoodMapSize10Params, ::testing::Values(1, 10, 100, 1000, 10000, 100000));

TEST_P(RobinHoodMapSize10Params, PERFORMANCE_SLOW_benchmarkCtorAndSingleElementAddedBigObject) {
	size_t size = static_cast<size_t>(GetParam());
	benchmarkCtorAndAddElements<BigObject>(800 * 1000 / size, size, "BigObject");
}

TEST_P(RobinHoodMapSize10Params, PERFORMANCE_SLOW_benchmarkCtorAndSingleElementAddedInt) {
	size_t size = static_cast<size_t>(GetParam());
	benchmarkCtorAndAddElements<BigObject>(1000 * 1000 / size, size, "int");
}

inline uint32_t limit(uint32_t r, uint32_t endExclusive) {
	uint64_t x = r;
	x *= endExclusive;
	return static_cast<uint32_t>(x >> 32);
}

void benchFind(bool isExisting, uint32_t numElements, size_t iters) {
	boost::mt19937 gen(123);

	util::MicroBenchmark mb;

	mb.fixedIterationsPerMeasurement(iters);

	if (mb.isAutoTimed()) {
		boost::unordered_map<int, int, util::RobinHood::FastHash<size_t>> uo;
		fill(uo, numElements, isExisting);
		std::size_t optPrevention = 0;
		while (mb("boost::unordered_map<size_t, size_t>::find", optPrevention)) {
			if (uo.end() != uo.find(limit(gen(), numElements))) {
				optPrevention++;
			}
		}
	}

	util::RobinHood::Map<int, int> rh;
	fill(rh, numElements, isExisting);
	std::size_t optPrevention = 0;
	while (mb("util::RobinHood::Map<size_t, size_t>::find", optPrevention)) {
		if (rh.end() != rh.find(limit(gen(), numElements))) {
			optPrevention++;
		}
	}
}

TEST_P(RobinHoodMapSize10Params, PERFORMANCE_SLOW_benchmarkFindExisting) {
	uint32_t size = static_cast<uint32_t>(GetParam());
	benchFind(true, size, 10 * 1000 * 1000);
}

TEST_P(RobinHoodMapSize10Params, PERFORMANCE_SLOW_benchmarkFindNonExisting) {
	uint32_t size = static_cast<uint32_t>(GetParam());
	benchFind(false, size, 10 * 1000 * 1000);
}

template <class Map>
void benchIterate(util::MicroBenchmark& mb, size_t numElements, const char* name) {
	boost::mt19937 gen;
	gen.seed(1234);

	// fill map
	Map m;
	for (size_t i = 0; i < numElements; ++i) {
		m[gen()] = gen();
	}

	size_t optPrevention = 0;
	while (mb(name, optPrevention)) {
		for (typename Map::const_iterator it = m.begin(), end = m.end(); it != end; ++it) {
			optPrevention += it->second;
		}
	}
}

TEST_CASE(RobinHoodMapTest, PERFORMANCE_SLOW_benchmarkIteration) {
	util::MicroBenchmark mb;
	mb.fixedIterationsPerMeasurement(2000);

	if (mb.isAutoTimed()) {
		benchIterate<boost::unordered_map<size_t, size_t>>(mb, 10000, "iterating boost::unordered_map");
	}

	benchIterate<util::RobinHood::Map<size_t, size_t>>(mb, 10000, "iterating util::RobinHood::Map");
}

template <class Map>
void benchIterateAlwaysEnd(util::MicroBenchmark& mb, size_t numElements, const char* name) {
	boost::mt19937 gen;
	gen.seed(1234);

	// fill map
	Map m;
	for (size_t i = 0; i < numElements; ++i) {
		m[gen()] = gen();
	}

	size_t optPrevention = 0;
	while (mb(name, optPrevention)) {
		for (typename Map::const_iterator it = m.begin(); it != m.end(); ++it) {
			optPrevention += it->second;
		}
	}
}

TEST_CASE(RobinHoodMapTest, PERFORMANCE_SLOW_benchmarkIterationAlwaysEnd) {
	util::MicroBenchmark mb;
	mb.fixedIterationsPerMeasurement(2000);

	if (mb.isAutoTimed()) {
		benchIterateAlwaysEnd<boost::unordered_map<size_t, size_t, util::RobinHood::FastHash<size_t>>>(mb, 10000, "iterating boost::unordered_map");
	}
	benchIterateAlwaysEnd<util::RobinHood::Map<size_t, size_t>>(mb, 10000, "iterating util::RobinHood::Map");
}

class RobinHoodMapTestRandomTest : public testing::TestWithParam<int> {};

template <class M>
void randomMapInsertDelete(size_t maxElements, util::MicroBenchmark& mb, const char* name) {
	boost::mt19937 gen;
	gen.seed(321);

	M m;
	while (mb(name)) {
		m[gen() % maxElements];
		m.erase(gen() % maxElements);
	}
	mb.noOpt(m.size());
}

template <class T>
void benchRandomInsertDelete(size_t fixedNumIters, const char* title, int s) {
	util::MicroBenchmark mb;

	std::stringstream ss;
	ss << ", " << s << " entries max for " << title << " (" << sizeof(T) << " bytes)";
	mb.clearBaseline();
	mb.fixedIterationsPerMeasurement(fixedNumIters);

	if (mb.isAutoTimed()) {
		randomMapInsertDelete<boost::unordered_map<int, T, util::RobinHood::FastHash<int>>>(s, mb, ("boost::unordered_map" + ss.str()).c_str());
		// randomMapInsertDelete<std::map<int, T> >(s, mb, ("std::map" + ss.str()).c_str());
		// randomMapInsertDelete<boost::container::flat_map<int, T> >(s, mb, ("boost::container::flat_map" + ss.str()).c_str());
	}

	randomMapInsertDelete<util::RobinHood::Map<int, T>>(s, mb, ("util::RobinHood::Map" + ss.str()).c_str());
}

class RobinHoodMapSizeParams : public ::testing::TestWithParam<int> {};

TEST_P(RobinHoodMapSizeParams, PERFORMANCE_SLOW_benchmarkRandomInsertDeleteInt) {
	benchRandomInsertDelete<int>(2 * 1000 * 1000, "int", GetParam());
}

TEST_P(RobinHoodMapSizeParams, PERFORMANCE_SLOW_benchmarkRandomInsertDeleteBigObject) {
	benchRandomInsertDelete<BigObject>(1000 * 1000, "BigObject", GetParam());
}

INSTANTIATE_TEST_CASE_P(RobinHoodInstance, RobinHoodMapSizeParams,
						::testing::Values(0x1F * 100 / 50, 0x1FF * 100 / 70, 0x1FFFF * 100 / 51, 0x1FFFF * 100 / 70, 0x1FFFF * 100 / 80,
										  0x1FFFF * 100 / 90, 0x1FFFF * 100 / 95));

template <class Map>
void randomInsertDelete(const size_t numIters, size_t maxElement) {
	boost::mt19937 gen;
	gen.seed(static_cast<uint32_t>(12332));

	Map map;
	for (size_t i = 0; i < numIters; ++i) {
		map[gen() % maxElement] = i;
		map.erase(gen() % maxElement);
	}

	// non-move call
	typename Map::key_type key = 321;
	for (size_t i = 0; i < numIters; ++i) {
		map[key] = i;
	}
}

template <class T>
struct BigCounter : public Counter<T> {
	BigCounter(const T& data)
		: Counter<T>(data)
		, a(0)
		, b(0)
		, c(0)
		, d(0) {}

	BigCounter()
		: Counter<T>()
		, a(0)
		, b(0)
		, c(0)
		, d(0) {}

	BigCounter(const BigCounter& o)
		: Counter<T>(o)
		, a(0)
		, b(0)
		, c(0)
		, d(0) {}

	double a;
	double b;
	double c;
	double d;
};

template <class I>
struct PassThroughHash {
	inline size_t operator()(const I& i) const {
		return static_cast<size_t>(i);
	}
};

template <class H>
void showHashSamples(const std::string& str) {}

template <class I>
void benchHash(size_t fixedNumIters, const std::string& titleStr) {
	util::MicroBenchmark mb;
	mb.fixedIterationsPerMeasurement(fixedNumIters);

	if (mb.isAutoTimed()) {
		std::string msg = "PassThroughHash<" + titleStr + ">";
		const char* title = msg.c_str();

		PassThroughHash<I> ph;
		I counter = 0;
		size_t optPrevention = 0;
		while (mb(title, optPrevention)) {
			optPrevention += ph(counter++);
		}

		boost::hash<I> bh;
		optPrevention = 0;
		counter = 0;
		msg = "boost::hash<" + titleStr + ">";
		title = msg.c_str();
		while (mb(title, optPrevention)) {
			optPrevention += bh(counter++);
		}
	}

	util::RobinHood::FastHash<I> fh;
	size_t optPrevention = 0;
	I counter = 0;
	std::string msg = "util::RobinHood::FastHash<" + titleStr + ">";
	const char* title = msg.c_str();

	while (mb(title, optPrevention)) {
		optPrevention += fh(counter++);
	}
}

TEST_CASE(RobinHoodMapTest, PERFORMANCE_SLOW_hashUint32) {
	benchHash<apr_uint32_t>(100 * 1000 * 1000, "apr_uint32_t");
}

TEST_CASE(RobinHoodMapTest, PERFORMANCE_SLOW_hashUint64) {
	benchHash<apr_uint64_t>(100 * 1000 * 1000, "apr_uint64_t");
}

TEST_CASE(RobinHoodMapTest, PERFORMANCE_SLOW_hashString) {
	const std::string v = "This is a test string for hashing";
	util::MicroBenchmark mb;
	mb.fixedIterationsPerMeasurement(20 * 1000 * 1000);

	if (mb.isAutoTimed()) {
		boost::hash<std::string> bh;
		size_t optPrevention = 0;
		while (mb("boost::hash<std::string>", optPrevention)) {
			optPrevention += bh(v);
		}
	}

	util::RobinHood::FastHash<std::string> fh;
	size_t optPrevention = 0;
	while (mb("util::RobinHood::FastHash<std::string>", optPrevention)) {
		optPrevention += fh(v);
	}
}

TEST_CASE(RobinHoodMapTest, testCalcMaxLoadFactor128) {
	using util::RobinHood::calcMaxLoadFactor128;

	for (float f = 0.0f; f < 2.0f; f += 0.01f) {
		size_t r = calcMaxLoadFactor128(f);
		REQUIRE(static_cast<size_t>(0) <= r);
		REQUIRE(static_cast<size_t>(128) >= r);
	}

	REQUIRE(calcMaxLoadFactor128(0.0f) == static_cast<size_t>(0));
	REQUIRE(calcMaxLoadFactor128(2.0f) == static_cast<size_t>(128));
	REQUIRE(calcMaxLoadFactor128(0.5f) == static_cast<size_t>(128 / 2));
	REQUIRE(calcMaxLoadFactor128(0.8f) == static_cast<size_t>(102));
}

TEST_CASE(RobinHoodMapTest, testCalcMaxNumElementsAllowed128) {
	using util::RobinHood::calcMaxNumElementsAllowed128;

	for (size_t maxElements = 1; maxElements > 0; maxElements *= 2) {
		for (unsigned char maxLoadFactor128 = 0; maxLoadFactor128 <= 128; ++maxLoadFactor128) {
			REQUIRE(maxLoadFactor128) == static_cast<size_t>(maxElements * (maxLoadFactor128 / 128.0)), calcMaxNumElementsAllowed128(maxElements);
		}
	}
}

TEST_CASE(RobinHoodMapTest, testCount) {
	util::RobinHood::Map<int, int> rh;
	boost::unordered_map<int, int> uo;
	REQUIRE(rh.count(123) == uo.count(123));
	REQUIRE(rh.count(0) == uo.count(0));
	rh[123];
	uo[123];
	REQUIRE(rh.count(123) == uo.count(123));
	REQUIRE(rh.count(0) == uo.count(0));
}

struct MyEquals : private boost::noncopyable {
	int& mState;

	MyEquals(int& state)
		: mState(state) {}

	MyEquals(const MyEquals& o)
		: mState(o.mState) {}

	bool operator()(const int& a, const int& b) const {
		std::cout << "equals with " << mState << std::endl;
		// modify state so we can test that this acutally works
		mState++;

		return a == b;
	}
};

TEST_CASE(RobinHoodMapTest, testCustomEquals) {
	int state = 0;
	MyEquals eq(state);
	typedef util::RobinHood::Map<int, int, util::RobinHood::FastHash<int>, MyEquals> MyMap;

	MyMap rh(0, util::RobinHood::FastHash<int>(), eq);

	// insert, no equals call
	rh[123] = 321;
	REQUIRE(state == 0);

	// lookup, should call equals once
	rh.find(123);
	REQUIRE(state == 1);

	// modify state ourselves
	state = 100;
	rh.find(123);
	REQUIRE(state == 101);

	// copy map, both should still use the same state
	MyMap rhCopy = rh;

	rhCopy.find(123);
	REQUIRE(state == 102);
	rh.find(123);
	REQUIRE(state == 103);

	// can't swap, deleted function.
}

TEST_CASE(RobinHoodMapTest, testCollisionSmall) {
	testCollisions<util::RobinHood::Map<CtorDtorVerifier, CtorDtorVerifier, DummyHash<CtorDtorVerifier>, std::equal_to<CtorDtorVerifier>, true>>();
}
TEST_CASE(RobinHoodMapTest, testCollisionBig) {
	testCollisions<util::RobinHood::Map<CtorDtorVerifier, CtorDtorVerifier, DummyHash<CtorDtorVerifier>, std::equal_to<CtorDtorVerifier>, false>>();
}

TEST_CASE(RobinHoodMapTest, testSmallTypes) {
	util::RobinHood::Map<uint8_t, uint8_t> map8;
	map8['x'] = 'a';
	REQUIRE(map8['x'] == 'a');
	REQUIRE(map8.size() == (size_t)1);

	util::RobinHood::Map<uint16_t, uint16_t> map16;
	map16[23] = 123;
	REQUIRE(map16[23] == (uint8_t)123);
	REQUIRE(map16.size() == (size_t)1);
}

template <class Map>
void eraseEven(Map& m) {
	typename Map::iterator it = m.begin();
	typename Map::iterator end = m.end();
	while (it != end) {
		// remove every second value (randomly)
		if (it->second % 2 == 0) {
			it = m.erase(it);
		} else {
			++it;
		}
	}
}

template <class A, class B>
void assertMapEqDirected(const A& a, const B& b) {
	REQUIRE(b.size() == a.size());
	for (typename A::const_iterator ia = a.begin(), aend = a.end(); ia != aend; ++ia) {
		typename B::const_iterator ib = b.find(ia->first);
		REQUIRE(ib != b.end());
		REQUIRE(ib->first == ia->first);
		REQUIRE(ib->second == ia->second);
	}
}

template <class A, class B>
void assertMapEq(const A& a, const B& b) {
	assertMapEqDirected(a, b);
	assertMapEqDirected(b, a);
}

TEST_CASE(RobinHoodMapTest, testErase) {
	typedef util::RobinHood::Map<uint32_t, uint32_t> Map;
	Map rh;
	boost::mt19937 gen;

	gen.seed(123);
	for (int i = 0; i < 10000; ++i) {
		rh[gen()] = gen();
	}
	std::cout << rh.size() << std::endl;
	eraseEven(rh);
	std::cout << rh.size() << std::endl;

	boost::unordered_map<uint32_t, uint32_t> uo;
	gen.seed(123);
	for (int i = 0; i < 10000; ++i) {
		uo[gen()] = gen();
	}
	std::cout << uo.size() << std::endl;
	eraseEven(uo);
	std::cout << uo.size() << std::endl;
	assertMapEq(rh, uo);
}

#ifdef BOOST_HAS_RVALUE_REFS
TEST_CASE(RobinHoodMapTest, testMoves) {
	typedef util::RobinHood::Map<uint32_t, uint32_t> Map;
	boost::scoped_ptr<Map> rh(new Map());
	(*rh)[123] = 321;

	Map rh2(std::move(*rh));
	REQUIRE(rh2.size() == (size_t)1);
	REQUIRE(rh2.find(123) != rh2.end());
	REQUIRE(rh2.find(123)->second == 321);

	// destroy rh, everything should still be fine.
	rh.reset();
	REQUIRE(rh2.size() == (size_t)1);
	REQUIRE(rh2.find(123) != rh2.end());
	REQUIRE(rh2.find(123)->second == 321);
}

TEST_CASE(RobinHoodMapTest, testMoveAssignment) {
	typedef util::RobinHood::Map<uint32_t, uint32_t> Map;
	boost::scoped_ptr<Map> rh(new Map());
	(*rh)[123] = 321;

	Map rh2;
	rh2[444] = 555;

	rh2 = std::move(*rh);
	REQUIRE(rh2.size() == (size_t)1);
	REQUIRE(rh2.find(123) != rh2.end());
	REQUIRE(rh2.find(123)->second == 321);

	rh.reset();
	REQUIRE(rh2.size() == (size_t)1);
	REQUIRE(rh2.find(123) != rh2.end());
	REQUIRE(rh2.find(123)->second == 321);
}

template <class Map>
void benchMoveMap(util::MicroBenchmark& mb, const char* title) {
	Map m1;
	Map m2;
	for (int i = 0; i < 10000; ++i) {
		m1[i];
	}

	while (mb(title)) {
		m2 = std::move(m1);
		m1 = std::move(m2);
	}

	REQUIRE(10000 == m1.size());
}

TEST_CASE(RobinHoodMapTest, PERFORMANCE_SLOW_moveMap) {
	util::MicroBenchmark mb;
	mb.unitScaling("move", (size_t)2);

	mb.fixedIterationsPerMeasurement(20 * 1000 * 1000);
	if (mb.isAutoTimed()) {
		benchMoveMap<boost::unordered_map<int, BigObject>>(mb, "boost::unordered_map<int, BigObject>");
	}

	benchMoveMap<util::RobinHood::Map<int, BigObject>>(mb, "util::RobinHood::Map<int, BigObject>");
}
#endif

template <class M>
void assertEq(const M& a, const M& b) {
	REQUIRE(a == b);
	REQUIRE(b == a);
	REQUIRE(!(a != b));
	REQUIRE(!(b != a));
}

template <class M>
void assertNeq(const M& a, const M& b) {
	REQUIRE(a != b);
	REQUIRE(b != a);
	REQUIRE(!(a == b));
	REQUIRE(!(b == a));
}

TEST_CASE(RobinHoodMapTest, testCompare) {
	util::RobinHood::Map<std::string, int> a;
	util::RobinHood::Map<std::string, int> b;
	assertEq(a, b);

	a["123"] = 321;
	assertNeq(a, b);

	b["123"] = 321;
	assertEq(a, b);

	b["123"] = 320;
	assertNeq(a, b);

	b["123"] = 321;
	assertEq(a, b);

	b.clear();
	b["124"] = 321;
	assertNeq(a, b);

	a.clear();
	assertEq(a, util::RobinHood::Map<std::string, int>());

	a[""] = 0;
	assertNeq(a, b);
}

#include <boost/assign/list_of.hpp> // for 'map_list_of()'

TEST_CASE(RobinHoodMapTest, testAssignListOf) {
	util::RobinHood::Map<std::string, int> m = boost::assign::map_list_of("a", 1)("b", 2);
	REQUIRE(m.size() == (size_t)2);
}

TEST_CASE(RobinHoodMapTest, testFindOtherType) {
	util::RobinHood::Map<std::string, int, util::RobinHood::FastHash<>, util::RobinHood::EqualTo<>> m;
	m["asdf"] = 123;

	REQUIRE(m.end() != m.find(boost::string_view("asdf"), util::RobinHood::is_transparent_tag()));
	REQUIRE(m.end() != m.find("asdf", util::RobinHood::is_transparent_tag()));
	REQUIRE(m.end() == m.find(boost::string_view("asdx"), util::RobinHood::is_transparent_tag()));
}

#include <util/typed/String.h>
#include <util/typed/StringView.h>

// util::typed::String<util::typed::CCSID_UTF8>
// util::typed::StringView<util::typed::CCSID_UTF8>
template <class M, class Str, class View>
void benchStringQuery(M& m, util::MicroBenchmark& mb, const char* title, util::RobinHood::is_transparent_tag) {
	Str str("This is a dummy test string that is also the query. It's a bit long.");
	for (size_t i = 0; i < 127; ++i) {
		++str[5];
		m[str] = 123;
	}

	View sv(str);
	size_t optPrevention = 0;
	while (mb(title, optPrevention)) {
		if (m.end() != m.find(sv, util::RobinHood::is_transparent_tag())) {
			++optPrevention;
		}
		++str[5];
	}
}

template <class M, class Str, class View>
void benchStringQuery(M& m, util::MicroBenchmark& mb, const char* title) {
	Str str("This is a dummy test string that is also the query. It's a bit long.");
	for (size_t i = 0; i < 127; ++i) {
		++str[5];
		m[str] = 123;
	}

	View sv(str);
	size_t optPrevention = 0;
	while (mb(title, optPrevention)) {
		if (m.end() != m.find(sv)) {
			++optPrevention;
		}
		++str[5];
	}
}

TEST_CASE(RobinHoodMapTest, PERFORMANCE_SLOW_testBoostStringView) {
	util::MicroBenchmark mb;
	mb.fixedIterationsPerMeasurement(1 * 1000 * 1000);
	{
		// does not compile:
		//	typedef boost::unordered_map<std::string, int> Map;
		//	typedef util::RobinHood::Map<std::string, int> Map;

		// but this does:
		typedef util::RobinHood::Map<std::string, int, util::RobinHood::FastHash<>, util::RobinHood::EqualTo<>> Map;
		Map map;
		benchStringQuery<Map, std::string, boost::string_view>(map, mb, "boost::string_view fast", util::RobinHood::is_transparent_tag());
	}
}

TEST_CASE(RobinHoodMapTest, PERFORMANCE_SLOW_testPerformanceStringSearchIsTransparentTag) {
	typedef util::typed::String<util::typed::CCSID_UTF8> Str;
	typedef util::typed::StringView<util::typed::CCSID_UTF8> View;

	util::MicroBenchmark mb;
	mb.fixedIterationsPerMeasurement(1 * 1000 * 1000);

	{
		typedef util::RobinHood::Map<Str, int> Map;
		Map m;
		benchStringQuery<Map, Str, View>(m, mb, "util::RobinHood::Map<Str, int>, int>", util::RobinHood::is_transparent_tag());
	}
}

TEST_CASE(RobinHoodMapTest, PERFORMANCE_SLOW_testPerformanceStringSearch) {
	typedef util::typed::String<util::typed::CCSID_UTF8> Str;
	typedef util::typed::StringView<util::typed::CCSID_UTF8> View;

	util::MicroBenchmark mb;
	mb.fixedIterationsPerMeasurement(1 * 1000 * 1000);

	{
		typedef util::RobinHood::Map<Str, int, util::RobinHood::FastHash<>, util::RobinHood::EqualTo<>> Map;
		Map m;
		benchStringQuery<Map, Str, View>(m, mb, "util::RobinHood::Map<Str, int, util::RobinHood::FastHash<>, util::RobinHood::EqualTo<> >",
										 util::RobinHood::is_transparent_tag());
	}
}

TEST_CASE(RobinHoodMapTest, testFindTyped) {
	// use FastHash<> and equal_to<> to enable the find() operation which does not copy.
	util::RobinHood::Map<util::typed::String<util::typed::CCSID_ASCII7>, int, util::RobinHood::FastHash<>, util::RobinHood::EqualTo<>> m;

	util::typed::String<util::typed::CCSID_ASCII7> str("asdf");
	m[str] = 123;

	util::typed::StringView<util::typed::CCSID_ASCII7> sv(str);
	REQUIRE(m.end() != m.find(sv));
}

/*


#include "Windows.h"
#include <psapi.h>

#include "util/AprThread.h"

#include <boost/bind.hpp>

class PeriodicMemoryStats : private boost::noncopyable {
public:
	struct Stats {
		apr_time_t timestamp;
		size_t memPrivateUsage;
	};

	PeriodicMemoryStats(double intervalSeconds)
		: mIntervalMicros(static_cast<uint64_t>(intervalSeconds * 1000 * 1000))
		, mDoContinue(true)
		, mThread(util::thread::runAsync(boost::bind(&PeriodicMemoryStats::runner, this, _1), (void*)0)) {}

	~PeriodicMemoryStats() {
		stop();
	}

	void stop() {
		mDoContinue = false;
		if (mThread) {
			mThread->join();
		}
	}

	void event(const char* title) {
		Stats s;
		s.timestamp = apr_time_now();
		s.memPrivateUsage = getMem();
		mEvents.push_back(std::pair<Stats, std::string>(s, title));
	}

	friend std::ostream& operator<<(std::ostream& stream, const PeriodicMemoryStats& pms);

private:
	apr_status_t runner(void*) {
		apr_time_t nextStop = apr_time_now();

		Stats s;
		while (mDoContinue) {
			nextStop += mIntervalMicros;

			s.timestamp = apr_time_now();
			s.memPrivateUsage = getMem();
			mPeriodic.push_back(s);

			if (nextStop < s.timestamp) {
				// we can't keep up!
				nextStop = s.timestamp;
			}
			util::thread::sleep_for_usec(nextStop - s.timestamp);
		}

		// add one last measurement
		s.timestamp = apr_time_now();
		s.memPrivateUsage = getMem();
		mPeriodic.push_back(s);

		return APR_SUCCESS;
	}

	size_t getMem() {
		PROCESS_MEMORY_COUNTERS_EX info;
		info.cb = sizeof(info);
		if (GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&info, info.cb)) {
			return info.PrivateUsage;
		}
		return 0;
	}

	std::vector<Stats> mPeriodic;
	std::vector<std::pair<Stats, std::string>> mEvents;

	uint64_t mIntervalMicros;
	bool mDoContinue;

	boost::scoped_ptr<util::AprThread<void*>> mThread;
};

inline std::ostream& operator<<(std::ostream& stream, const PeriodicMemoryStats& pms) {
	apr_time_t begin = pms.mPeriodic[0].timestamp;
	if (!pms.mEvents.empty()) {
		begin = (std::min)(begin, pms.mEvents[0].first.timestamp);
		for (size_t i = 0; i < pms.mEvents.size(); ++i) {
			const PeriodicMemoryStats::Stats& s = pms.mEvents[i].first;
			stream << (s.timestamp - begin) / 1000000.0 << "; " << s.memPrivateUsage << "; " << pms.mEvents[i].second << std::endl;
		}
		stream << std::endl;
	}

	for (size_t i = 0; i < pms.mPeriodic.size(); ++i) {
		const PeriodicMemoryStats::Stats& s = pms.mPeriodic[i];
		stream << (s.timestamp - begin) / 1000000.0 << "; " << s.memPrivateUsage << std::endl;
	}
	return stream;
}

#include "khash.h"

//#define my_size_t_hash(key) (khint32_t)(util::RobinHood::internal_hasher::fmix64(key))
// KHASH_INIT(map_size_t_size_t, khint64_t, size_t, 1, my_size_t_hash, kh_int64_hash_equal)
KHASH_MAP_INIT_INT64(map_size_t_size_t, size_t);
KHASH_MAP_INIT_INT64(map_size_t_BigObject, BigObject);

class KHashSizeT {
public:
	KHashSizeT()
		: mMap(kh_init(map_size_t_size_t)) {}

	~KHashSizeT() {
		kh_destroy(map_size_t_size_t, mMap);
	}

	size_t& operator[](const size_t& key) {
		int absent;
		auto k = kh_put(map_size_t_size_t, mMap, key, &absent);
		return kh_value(mMap, k);
	}

	size_t count(size_t s) const {
		if (kh_end(mMap) == kh_get(map_size_t_size_t, mMap, s)) {
			return 0;
		}
		return 1;
	}

	size_t size() const {
		return kh_size(mMap);
	}

	void clear() {
		kh_clear(map_size_t_size_t, mMap);
	}

private:
	khash_t(map_size_t_size_t) * mMap;
};

class KHashBigObject {
public:
	KHashBigObject()
		: mMap(kh_init(map_size_t_BigObject)) {}

	~KHashBigObject() {
		kh_destroy(map_size_t_BigObject, mMap);
	}

	BigObject& operator[](const size_t& key) {
		int absent;
		auto k = kh_put(map_size_t_BigObject, mMap, key, &absent);
		return kh_value(mMap, k);
	}

	size_t count(size_t s) const {
		if (kh_end(mMap) == kh_get(map_size_t_BigObject, mMap, s)) {
			return 0;
		}
		return 1;
	}

	size_t size() const {
		return kh_size(mMap);
	}

	void clear() {
		kh_clear(map_size_t_BigObject, mMap);
	}

private:
	khash_t(map_size_t_BigObject) * mMap;
};


template <class Map>
void benchRamUsage(const size_t numIters) {

	boost::mt19937 gen;
	gen.seed(123);

	size_t c = 0;
	PeriodicMemoryStats stats(0.02);
	{
		Map map;
		for (size_t i = 0; i < numIters; ++i) {
			map[gen()];
		}
		c += map.size();
		stats.event("inserted");

		map.clear();
		stats.event("cleared");

		for (size_t i = 0; i < numIters; ++i) {
			map[gen()];
		}
		c += map.size();
		stats.event("inserted");
	}
	stats.event("destructed");
	stats.stop();

	std::cout << stats << std::endl;
	std::cout << c << std::endl;
}

template <class Map>
void benchCreateClearDestroy(size_t numIters) {
	boost::mt19937 gen;
	gen.seed(123);

	size_t c = 0;
	{
		Map map;
		for (size_t i = 0; i < numIters; ++i) {
			map[gen()];
		}
		c += map.size();
		map.clear();
		for (size_t i = 0; i < numIters; ++i) {
			map[gen()];
		}
		c += map.size();
	}
	std::cout << c << std::endl;
}

TEST_CASE(RobinHoodMapTest, DISABLED_PERFORMANCE_SLOW_benchCreateClearRobinHoodBig) {
	benchRamUsage<util::RobinHood::Map<size_t, BigObject>>(10000000);
}
TEST_CASE(RobinHoodMapTest, DISABLED_PERFORMANCE_SLOW_benchCreateClearStdUnorderedMapBig) {
	benchRamUsage<std::unordered_map<size_t, BigObject>>(10000000);
}
TEST_CASE(RobinHoodMapTest, DISABLED_PERFORMANCE_SLOW_benchCreateClearBoostUnorderedMapBig) {
	benchRamUsage<boost::unordered_map<size_t, BigObject>>(10000000);
}
TEST_CASE(RobinHoodMapTest, DISABLED_PERFORMANCE_SLOW_benchCreateClearStdMapBig) {
	benchRamUsage<std::map<size_t, BigObject>>(10000000);
}

TEST_CASE(RobinHoodMapTest, DISABLED_PERFORMANCE_SLOW_benchCreateClearRobinHoodSizeT) {
	benchRamUsage<util::RobinHood::Map<size_t, size_t>>(10000000);
}
TEST_CASE(RobinHoodMapTest, DISABLED_PERFORMANCE_SLOW_benchCreateClearStdUnorderedMapSizeT) {
	benchRamUsage<std::unordered_map<size_t, size_t>>(10000000);
}
TEST_CASE(RobinHoodMapTest, DISABLED_PERFORMANCE_SLOW_benchCreateClearBoostUnorderedMapSizeT) {
	benchRamUsage<boost::unordered_map<size_t, size_t>>(10000000);
}
TEST_CASE(RobinHoodMapTest, DISABLED_PERFORMANCE_SLOW_benchCreateClearStdMapSizeT) {
	benchRamUsage<std::map<size_t, size_t>>(10000000);
}

TEST_CASE(RobinHoodMapTest, DISABLED_testRamUsage) {
	// typedef util::RobinHood::Map<size_t, size_t> Map;
	// typedef util::RobinHood::Map<size_t, size_t, util::RobinHood::FastHash<size_t>, std::equal_to<size_t>, false> Map;
	// typedef util::RobinHood::Map<size_t, size_t, boost::hash<size_t>, std::equal_to<size_t>, false> Map;
	// typedef util::RobinHood::Map<size_t, size_t, boost::hash<size_t>, std::equal_to<size_t>, true> Map;
	// typedef std::map<size_t, size_t> Map;
	// typedef boost::unordered_map<size_t, size_t, util::RobinHood::FastHash<size_t> > Map;
	// typedef boost::unordered_map<size_t, size_t> Map;
	// typedef KHashBigObject Map;

	// typedef util::RobinHood::Map<size_t, BigObject> Map;
	// typedef util::RobinHood::IndirectMap<size_t, BigObject> Map;
	// typedef boost::unordered_map<size_t, BigObject> Map;
	// typedef std::unordered_map<size_t, size_t, util::RobinHood::FastHash<size_t> > Map;

	typedef util::RobinHood::IndirectMap<size_t, size_t> Map;
	benchRamUsage<Map>(20000000);
}
*/

TEST_CASE(RobinHoodMapTest, testEmplace) {
	util::RobinHood::Map<std::string, std::string> map;
	auto itAndInserted = map.emplace("key", "val");
	REQUIRE(itAndInserted.second);
	REQUIRE(itAndInserted.first != map.end());
	REQUIRE(itAndInserted.first->first == "key");
	REQUIRE(itAndInserted.first->second == "val");

	// insert fails, returns iterator to previously inserted element
	itAndInserted = map.emplace("key", "val2");
	REQUIRE(!itAndInserted.second);
	REQUIRE(itAndInserted.first != map.end());
	REQUIRE(itAndInserted.first->first == "key");
	REQUIRE(itAndInserted.first->second == "val");
}

TEST_CASE(RobinHoodMapTest, testStringMoved) {
	util::RobinHood::Map<std::string, std::string> map;
	map["foo"] = "bar";
}

TEST_CASE(RobinHoodMapTest, testAssignIterat) {
	const std::string key = "key";
	util::RobinHood::Map<std::string, std::string> map;
	map[key] = "sadf";
	auto it = map.end();
	it = map.end();

	auto cit = map.cend();
	cit = map.cend();

	cit = it;
}
/*
template <class M>
void testInitializerList() {
	M m = {{123, "Hello"}, {321, "Hugo"}, {123, "Goodbye"}, {}};
	REQUIRE(m.size() == (size_t)3);
	REQUIRE(m[123] == "Hello");
	REQUIRE(m[321] == "Hugo");
	REQUIRE(m[0] == "");
	REQUIRE(m.size() == (size_t)3);
}

TEST_CASE(RobinHoodMapTest, testInitializerList) {
	testInitializerList<std::map<int, std::string>>();
	testInitializerList<util::RobinHood::Map<int, std::string>>();
}
*/
template <class Map>
void testBrackets() {
	resetStaticCounts();
	printStaticHeader();
	{
		Map map;
		printStaticCounts("empty");
		resetStaticCounts();
		for (size_t i = 0; i < 5; ++i) {
			map[321] = 123;
			printStaticCounts("map[321] = 123");
			resetStaticCounts();
		}
	}
	printStaticCounts("dtor");
	resetStaticCounts();
}

TEST_CASE(RobinHoodMapTest, testBracketsRobinHoodDirect) {
	testBrackets<util::RobinHood::DirectMap<Counter<int>, int>>();
}
TEST_CASE(RobinHoodMapTest, testBracketsRobinHoodIndirect) {
	testBrackets<util::RobinHood::IndirectMap<Counter<int>, int>>();
}
TEST_CASE(RobinHoodMapTest, testBracketsUnorderedMap) {
	testBrackets<std::unordered_map<Counter<int>, int>>();
}
TEST_CASE(RobinHoodMapTest, testBracketsMap) {
	testBrackets<std::map<Counter<int>, int>>();
}

template <class Map>
void benchStringQuery(util::MicroBenchmark& mb, const char* title) {
	Map m;
	std::string const str = "this is my query. It is a very nice query, but unfortunately quite long, so when it is copied this will be slow.";
	std::vector<std::string> strings;
	for (size_t i = 0; i < 100; ++i) {
		strings.push_back(str + std::to_string(i));
	}

	while (mb(title)) {
		for (size_t i = 0; i < strings.size(); ++i) {
			++m[strings[i]];
		}
	}
	mb.noOpt(m.size());
}

TEST_CASE(RobinHoodMapTest, PERFORMANCE_SLOW_accessExistingWithConstRef) {
	util::MicroBenchmark mb;
	mb.fixedIterationsPerMeasurement(20'000);
	mb.unitScaling("operator[]", 100ull);
	if (mb.isAutoTimed()) {
		benchStringQuery<std::unordered_map<std::string, size_t>>(mb, "std::unordered_map");
		benchStringQuery<boost::unordered_map<std::string, size_t>>(mb, "boost::unordered_map");
	}
	benchStringQuery<util::RobinHood::DirectMap<std::string, size_t>>(mb, "util::RobinHood::DirectMap");
	benchStringQuery<util::RobinHood::IndirectMap<std::string, size_t>>(mb, "util::RobinHood::IndirectMap");

	mb.plot();
}
