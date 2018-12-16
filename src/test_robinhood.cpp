#include <MicroBenchmark.h>
#include <robin_hood_map.h>
#include <sfc64.h>
#include <timer.h>

#include <chrono>
#include <fstream>
#include <iostream>
#include <random>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>

#include <3rdparty/rigtorp/HashMap.h>
#include <3rdparty/sherwood_map/sherwood_map.hpp>
#include <3rdparty/sparsepp/sparsepp.h>
#include <3rdparty/tessil/hopscotch_map.h>

// can't use 3rdparty here because of google's insistance of not using relative paths.
#include <google/dense_hash_map>

using DefaultRng = sfc64;

#ifdef _WIN32
#include <Windows.h>
#include <psapi.h>

void set_high_priority() {
	// see https://msdn.microsoft.com/en-us/library/windows/desktop/ms685100%28v=vs.85%29.aspx?f=255&MSPPError=-2147217396
	SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
}
#else
#include <regex>
void set_high_priority() {}
#endif

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

// test some hashing stuff
template <class T>
double bench_hashing(int& data) {
	size_t max_size_mask = (1 << 16) - 1;
	std::cout << max_size_mask << std::endl;
	Timer timer;
	T t;
	for (size_t i = 1; i < 1000000; ++i) {
		data += i * i + 7;
		t.insert(data & max_size_mask);
	}
	std::cout << t.size() << std::endl;
	return timer.elapsed();
}

class MyException : public std::exception {
public:
	MyException(const char* msg)
		: mMsg(msg) {}

private:
	const std::string mMsg;
};

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define CHECK(x)                                                                                                                                     \
	if (!(x))                                                                                                                                        \
		throw MyException(__FILE__ "(" TOSTRING(__LINE__) "): " #x);

template <class H>
void test1_std(int times) {
	H rhhs;
	CHECK(rhhs.size() == 0);
	auto it = rhhs.insert(typename H::value_type(32145, 123));
	CHECK(it.second);
	CHECK(it.first->first == 32145);
	CHECK(it.first->second == 123);
	CHECK(rhhs.size() == 1);

	for (int i = 0; i < times; ++i) {
		auto it = rhhs.insert(typename H::value_type(i * 4, i));
		CHECK(it.second);
		CHECK(it.first->first == i * 4);
		CHECK(it.first->second == i);
		auto found = rhhs.find(i * 4);
		if (found == rhhs.end()) {
			CHECK(found != rhhs.end());
		}
		CHECK(found->second == i);
		if (rhhs.size() != 2 + static_cast<size_t>(i)) {
			CHECK(rhhs.size() == 2 + static_cast<size_t>(i));
		}
	}

	// check if everything can be found
	for (int i = 0; i < times; ++i) {
		auto found = rhhs.find(i * 4);
		CHECK(found != rhhs.end());
		CHECK(found->second == i);
	}

	// check non-elements
	for (int i = 0; i < times; ++i) {
		auto found = rhhs.find((i + times) * 4);
		CHECK(found == rhhs.end());
	}

	// random test against std::unordered_map
	rhhs.clear();
	using StdMap = std::unordered_map<int, int>;
	StdMap uo;

	DefaultRng rng;
	rng.seed(123);

	const int mod = times / 4;
	for (int i = 0; i < times; ++i) {
		int r = static_cast<int>(rng(mod));
		auto rhh_it = rhhs.insert(typename H::value_type(r, r * 2));
		auto uo_it = uo.insert(StdMap::value_type(r, r * 2));
		CHECK(rhh_it.second == uo_it.second);
		CHECK(rhh_it.first->first == uo_it.first->first);
		CHECK(rhh_it.first->second == uo_it.first->second);
		CHECK(uo.size() == rhhs.size());
	}

	uo.clear();
	rhhs.clear();
	for (int i = 0; i < times; ++i) {
		int r = static_cast<int>(rng(mod));
		rhhs[r] = r * 2;
		uo[r] = r * 2;
		CHECK(uo.find(r)->second == rhhs.find(r)->second);
		CHECK(uo.size() == rhhs.size());
	}

	std::cout << "test1_std ok!" << std::endl;
}

template <class H>
void test1(int times) {
	H rhhs;
	CHECK(rhhs.size() == 0);
	CHECK(rhhs.insert(typename H::value_type(32145, 123)).second);
	CHECK(rhhs.size() == 1);

	for (int i = 0; i < times; ++i) {
		CHECK(rhhs.emplace(i * 4, i).second);
		auto it = rhhs.find(i * 4);
		CHECK(it != rhhs.end());
		CHECK(it->first == i);
		if (rhhs.size() != 2 + static_cast<size_t>(i)) {
			CHECK(rhhs.size() == 2 + static_cast<size_t>(i));
		}
	}

	// check if everything can be found
	for (int i = 0; i < times; ++i) {
		auto it = rhhs.find(i * 4);
		CHECK(it != rhhs.end());
		CHECK(it->first == i);
	}

	// check non-elements
	for (int i = 0; i < times; ++i) {
		auto it = rhhs.find((i + times) * 4);
		CHECK(it == rhhs.end());
	}
}

void test4() {
	std::unordered_map<size_t, size_t> u;
	robin_hood::map<size_t, size_t> r;

	DefaultRng rand;

	for (size_t i = 0; i < 100000; ++i) {
		size_t key = static_cast<size_t>(rand(i + 1));
		size_t val = static_cast<size_t>(rand());
		CHECK(u.emplace(key, val).first->second == r.emplace(key, val).first->second);
	}
}

std::string rand_str(DefaultRng& rand, const size_t num_letters) {
	std::string alphanum = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
	std::string s;
	s.resize(num_letters);
	for (size_t i = 0; i < num_letters; ++i) {
		s[i] = alphanum[rand(alphanum.size())];
	}
	return s;
}

void test_compare(size_t times) {
	DefaultRng rand;
	size_t seed = 142323;
	rand.seed(seed);

	using RhMap = robin_hood::map<size_t, int>;
	using StdMap = std::unordered_map<size_t, int>;
	RhMap r;
	StdMap m;

	StdMap m2;
	m2[423] = 342;

	for (size_t i = 0; i < times; ++i) {
		size_t v = rand(i + 100);
		auto pm = m.insert(StdMap::value_type(v, static_cast<int>(i)));
		auto pr = r.insert(RhMap::value_type(v, static_cast<int>(i)));

		if (m.size() != r.size() || pm.second != pr.second) {
			std::cout << i << ": " << v << " " << pm.second << " " << pr.second << std::endl;
		}

		v = rand(i + 100);
		bool found_m = m.find(v) != m.end();
		bool found_r = r.find(v) != r.end();
		if (found_m != found_r) {
			std::cout << i << ": " << v << " " << found_m << " " << found_r << std::endl;
		}

		v = rand(i + 100);
		if (m.erase(v) != r.erase(v)) {
			std::cout << "erase not equal!";
		}
		if (m.size() != r.size()) {
			std::cout << "sizes not equal after erase!" << std::endl;
		}
	}
	std::cout << "ok!" << std::endl;
}

static size_t x_ctor = 0;
static size_t x_dtor = 0;
static size_t x_eq_move = 0;
static size_t x_eq = 0;
static size_t x_mov = 0;
static size_t x_copyctor = 0;
static size_t x_operatoreq = 0;
static size_t x_intctor = 0;
static size_t x_hash = 0;

void print_x(std::string msg) {
	std::cout << msg << std::endl;
	std::cout << "  x_ctor " << x_ctor << std::endl;
	std::cout << "  x_dtor " << x_dtor << std::endl;
	std::cout << "  x_eq_move " << x_eq_move << std::endl;
	std::cout << "  x_eq " << x_eq << std::endl;
	std::cout << "  x_mov " << x_mov << std::endl;
	std::cout << "  x_copyctor " << x_copyctor << std::endl;
	std::cout << "  x_operatoreq " << x_operatoreq << std::endl;
	std::cout << "  x_intctor " << x_intctor << std::endl;
	std::cout << "  x_hash " << x_hash << std::endl;
	std::cout << std::endl;
}

void reset_x() {
	x_ctor = 0;
	x_dtor = 0;
	x_eq = 0;
	x_eq_move = 0;
	x_mov = 0;
	x_copyctor = 0;
	x_operatoreq = 0;
	x_intctor = 0;
	x_hash = 0;
}

class X {
public:
	X()
		: x(0) {
		++x_ctor;
	}

	X(X&& o)
		: x(o.x) {
		++x_mov;
	}

	X(const X& o)
		: x(o.x) {
		++x_copyctor;
	}

	X& operator=(X&& o) {
		x = o.x;
		++x_eq_move;
		return *this;
	}

	X& operator=(const X& o) {
		x = o.x;
		++x_eq;
		return *this;
	}

	~X() {
		++x_dtor;
	}

	bool operator==(const X& o) const {
		++x_operatoreq;
		return x == o.x;
	}

	X(int x_)
		: x(x_) {
		++x_intctor;
	}

public:
	int x;
};

struct HashX : public std::unary_function<size_t, X> {
	inline size_t operator()(const X& t) const {
		++x_hash;
		return std::hash<int>()(t.x);
	}
};

size_t get_mem() {
#ifdef _WIN32
	PROCESS_MEMORY_COUNTERS_EX info;
	info.cb = sizeof(info);
	if (GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&info, info.cb)) {
		return info.PrivateUsage;
	}
#else
	const static std::regex vmsizeRegex("VmSize:\\s*(\\d*) kB");

	std::ifstream file("/proc/self/status");
	std::smatch matchResult;
	std::string line;
	while (std::getline(file, line)) {
		std::regex_match(line, matchResult, vmsizeRegex);
		if (2 == matchResult.size()) {
			return std::stoll(matchResult[1].str()) * 1024;
		}
	}
#endif
	return 0;
}

struct Stats {
	double elapsed_insert;
	double elapsed_find_existing;
	double elapsed_find_nonexisting;
	size_t mem;
	size_t num;
	size_t found;
	std::string title;

	Stats()
		: elapsed_insert(0)
		, elapsed_find_existing(0)
		, elapsed_find_nonexisting(0)
		, mem(0)
		, num(0)
		, found(0) {}

	Stats& operator+=(const Stats& o) {
		elapsed_insert += o.elapsed_insert;
		elapsed_find_existing += o.elapsed_find_existing;
		elapsed_find_nonexisting += o.elapsed_find_nonexisting;
		mem += o.mem;
		num += o.num;
		found += o.found;
		return *this;
	}

	Stats& operator/=(size_t n) {
		elapsed_insert /= n;
		elapsed_find_existing /= n;
		elapsed_find_nonexisting /= n;
		mem /= n;
		num /= n;
		found /= n;
		return *this;
	}
};

template <class O>
void print(O& out, const std::vector<std::vector<Stats>>& s) {
	auto elems = s[0].size();
	std::vector<double> sums(s.size(), 0);
	print_header(out, s, "Cummulative insertion time");
	for (size_t e = 0; e < elems; ++e) {
		out << s[0][e].num << ";";
		for (size_t i = 0; i < s.size(); ++i) {
			const auto& st = s[i][e];
			if (i > 0) {
				out << ";";
			}
			sums[i] += st.elapsed_insert * s[0][0].num;
			out << sums[i];
		}
		out << std::endl;
	}

	print_header(out, s, "Time to find 1M existing elements");
	for (size_t e = 0; e < elems; ++e) {
		out << s[0][e].num << ";";
		for (size_t i = 0; i < s.size(); ++i) {
			const auto& st = s[i][e];
			if (i > 0) {
				out << ";";
			}
			out << st.elapsed_find_existing * 1000 * 1000;
		}
		out << std::endl;
	}

	print_header(out, s, "Time to find 1M nonexisting elements");
	for (size_t e = 0; e < elems; ++e) {
		out << s[0][e].num << ";";
		for (size_t i = 0; i < s.size(); ++i) {
			const auto& st = s[i][e];
			if (i > 0) {
				out << ";";
			}
			out << st.elapsed_find_nonexisting * 1000 * 1000;
		}
		out << std::endl;
	}

	print_header(out, s, "Memory usage in MB");
	for (size_t e = 0; e < elems; ++e) {
		out << s[0][e].num << ";";
		for (size_t i = 0; i < s.size(); ++i) {
			const auto& st = s[i][e];
			if (i > 0) {
				out << ";";
			}
			out << st.mem / (1024.0 * 1024);
		}
		out << std::endl;
	}

	for (size_t e = 0; e < elems; ++e) {
		for (size_t i = 1; i < s.size(); ++i) {
			if (s[i][e].num != s[0][e].num) {
				throw std::runtime_error("num not equal!");
			}
		}
	}
	out << "num checked." << std::endl;
}

template <class HS>
void bench_sequential_insert(HS& r, MicroBenchmark& mb, const std::string& title, size_t increase, size_t totalTimes,
							 std::vector<std::vector<Stats>>& all_stats) {
	std::cout << title << "; ";
	std::cout.flush();
	std::vector<Stats> stats;
	Stats s;
	s.title = title;
	Timer t;
	size_t mem_before = get_mem();
	const int upTo = static_cast<int>(increase);
	const int times = static_cast<int>(totalTimes);
	int i = 0;
	size_t found = 0;
	for (int ti = 0; ti < static_cast<int>(times); ++ti) {
		// insert
		t.restart();
		for (size_t up = 0; up < static_cast<size_t>(upTo); ++up) {
			r[i] = i;
			++i;
		}
		s.elapsed_insert = t.elapsed() / upTo;
		auto gm = get_mem();
		s.mem = gm - mem_before;
		if (gm < mem_before) {
			// overflow check
			s.mem = 0;
		}
		s.num = r.size();

		// query existing
		const auto endIt = r.end();
		const int inc = ti + 1;
		while (mb.keepRunning()) {
			for (int v = 0, e = upTo * inc; v < e; v += inc) {
				if (endIt != r.find(v)) {
					++found;
				}
			}
		}
		s.elapsed_find_existing = mb.min() / upTo;

		// query nonexisting
		while (mb.keepRunning()) {
			for (int v = times * upTo, e = times * upTo + upTo; v < e; ++v) {
				if (endIt != r.find(v)) {
					++found;
				}
			}
		}
		s.elapsed_find_nonexisting = mb.min() / upTo;
		s.found = found;
		stats.push_back(s);
	}

	Stats sum;
	std::for_each(stats.begin(), stats.end(), [&sum](const Stats& s) { sum += s; });
	sum /= stats.size();

	std::cout << 1000000 * sum.elapsed_insert << "; " << 1000000 * sum.elapsed_find_existing << "; " << 1000000 * sum.elapsed_find_nonexisting << "; "
			  << sum.mem / (1024.0 * 1024) << "; " << sum.found << std::endl;

	all_stats.push_back(stats);

	std::ofstream fout("out.txt");
	print(fout, all_stats);
}

template <class O>
void print_header(O& out, const std::vector<std::vector<Stats>>& s, const std::string& title) {
	out << std::endl << title << std::endl;
	for (size_t i = 0; i < s.size(); ++i) {
		out << ";" << s[i][0].title;
	}
	out << std::endl;
}

template <class H>
std::vector<std::vector<Stats>> bench_sequential_insert(size_t upTo, size_t times) {
	std::cout << "Title;1M inserts [sec];find 1M existing [sec];find 1M nonexisting [sec];memory usage [MB];foundcount" << std::endl;
	std::vector<std::vector<Stats>> all_stats;

	MicroBenchmark mb;

	{
		robin_hood::map<int, int, H> m;
		bench_sequential_insert(m, mb, "robin_hood::map", upTo, times, all_stats);
	}

	{
		google::dense_hash_map<int, int, H> googlemap;
		googlemap.set_empty_key(-1);
		googlemap.set_deleted_key(-2);
		googlemap.max_load_factor(0.95f);
		bench_sequential_insert(googlemap, mb, "google::dense_hash_map 0.95", upTo, times, all_stats);
	}

	{
		google::dense_hash_map<int, int, H> googlemap;
		googlemap.set_empty_key(-1);
		googlemap.set_deleted_key(-2);
		bench_sequential_insert(googlemap, mb, "google::dense_hash_map 0.5", upTo, times, all_stats);
	}

	{
		std::unordered_map<int, int, H> m;
		m.max_load_factor(0.95f);
		bench_sequential_insert(m, mb, "std::unordered_map 0.95", upTo, times, all_stats);
	}

	{
		std::unordered_map<int, int, H> m;
		m.max_load_factor(0.5f);
		bench_sequential_insert(m, mb, "std::unordered_map 0.5", upTo, times, all_stats);
	}

	return all_stats;
}

template <class H>
void random_bench(const std::string& title) {
	constexpr size_t count = 1000000;
	constexpr size_t iters = 1000000000;

	H hm;
	std::mt19937 mt;
	std::uniform_int_distribution<int> ud(2, count);

	int val;
	for (size_t i = 0; i < count; ++i) {
		val = ud(mt);
		hm.insert(val, val);
	}

	auto start = std::chrono::high_resolution_clock::now();
	for (size_t i = 0; i < iters; ++i) {
		hm.erase(val);
		val = ud(mt);
		hm.insert(val, val);
	}
	auto stop = std::chrono::high_resolution_clock::now();
	auto duration = stop - start;

	std::cout << std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count() / iters << " ns/iter for " << title << "(size=" << hm.size()
			  << ")" << std::endl;
}

template <class Key, class Val>
class DummyMap {
public:
	DummyMap()
		: _key(0)
		, _val(0) {}

	inline Val& operator[](const Key& k) {
		_key += k;
		return _val;
	}

	inline void erase(const Key& k) {
		_key += k;
	}

	inline size_t size() const {
		return static_cast<size_t>(_key);
	}

private:
	Key _key;
	Val _val;
};

template <class T>
void doNotOptmizeAway(T&& dat) {
#ifdef _WIN32
	volatile auto x = &dat;
#else
	// see https://www.reddit.com/r/cpp/comments/52yg8b/donotoptimizeaway_for_pre_c11_compilers/
	asm volatile("" : : "g"(dat) : "memory");
#endif
}

template <class R>
void benchRng(const char* name) {
	// MicroBenchmark mb(5, 1.0);
	MicroBenchmark mb;
	R rng;
	auto n = rng();
	while (mb.keepRunning()) {
		n += rng();
	}
	doNotOptmizeAway(n);
	std::cout << (1 / ((mb.min)() * 1000 * 1000)) << " Million OPS for " << name << std::endl;
}

template <class M>
void benchRandomInsertAndDelete(const char* name, uint32_t mask, uint32_t numElements) {
	MicroBenchmark mb;
	DefaultRng rng;

	size_t s = 0;
	while (mb.keepRunning()) {
		rng.seed(123);
		M m;
		for (uint32_t i = 0; i < numElements; ++i) {
			m[rng() & mask] = i;
			m.erase(rng() & mask);
		}
		s = m.size();
	}
	doNotOptmizeAway(s);
	std::cout << (mb.min)() << " " << name << " " << s << std::endl;
}

template <class M>
void benchRandomFind(const char* name, uint32_t mask, uint32_t numElements) {
	MicroBenchmark mb;
	DefaultRng rng;

	rng.seed(321);
	M m;
	// m.max_load_factor(0.99f);
	for (uint32_t i = 0; i < numElements; ++i) {
		m[rng() & mask] = i;
	}

	size_t s = 0;
	while (mb.keepRunning()) {
		rng.seed(321);
		for (uint32_t i = 0, e = numElements; i < e; ++i) {
			if (m.find(rng() & mask) != m.end()) {
				++s;
			}
		}
	}
	doNotOptmizeAway(s);
	std::cout << (mb.min)() << " " << name << " benchRandomFind (existing) load=" << m.load_factor() << " size=" << m.size() << std::endl;

	while (mb.keepRunning()) {
		rng.seed(999);
		for (uint32_t i = 0, e = numElements; i < e; ++i) {
			if (m.find(rng() & mask) != m.end()) {
				++s;
			}
		}
	}
	doNotOptmizeAway(s);
	std::cout << (mb.min)() << " " << name << " benchRandomFind (nonexisting) load=" << m.load_factor() << " size=" << m.size() << std::endl;
}

template <class M>
void benchRandomInsert(const char* name, uint32_t mask, uint32_t numElements) {
	MicroBenchmark mb;
	DefaultRng rng;

	float s = 0;
	while (mb.keepRunning()) {
		rng.seed(123);
		M m;
		m.max_load_factor(0.99f);
		for (uint32_t i = 0; i < numElements; ++i) {
			m[rng() & mask] = i;
		}
		s = m.load_factor();
	}
	doNotOptmizeAway(s);
	std::cout << (mb.min)() << " " << name << " " << s << std::endl;
}

template <class K, class V, class H = std::hash<int>>
class GoogleMapWrapper {
private:
	typedef google::dense_hash_map<K, V, H> Map;
	Map mGm;

public:
	inline GoogleMapWrapper() {
		mGm.set_empty_key(-1);
		mGm.set_deleted_key(-2);
	}

	inline typename Map::data_type& operator[](const typename Map::key_type& key) {
		return mGm[key];
	}

	inline typename Map::size_type erase(const typename Map::key_type& key) {
		return mGm.erase(key);
	}

	inline typename Map::size_type size() const {
		return mGm.size();
	}

	inline typename Map::const_iterator find(const typename Map::key_type& key) const {
		return mGm.find(key);
	}

	inline typename Map::const_iterator end() const {
		return mGm.end();
	}

	inline void max_load_factor(float newLoadFactor) {
		mGm.max_load_factor(newLoadFactor);
	}

	inline size_t max_size() const {
		return mGm.max_size();
	}

	inline float load_factor() const {
		return mGm.load_factor();
	}
};

int main(int argc, char** argv) {
	set_high_priority();

	test1_std<robin_hood::map<int, int>>(100000);

	auto stats = bench_sequential_insert<std::hash<size_t>>(100 * 1000, 1000);
	print(std::cout, stats);
	std::ofstream fout("out.txt");
	print(fout, stats);

	for (int i = 0; i < 10; ++i) {
		uint32_t mask = (1 << (20 + i)) - 1;
		uint32_t numElements = 1000 * 1000;

		benchRandomFind<robin_hood::map<uint32_t, uint32_t>>("robin_hood::map", mask, numElements);

		std::cout << std::endl;
	}

	try {
		test1<robin_hood::map<int, int>>(100000);
		std::cout << "test1 ok!" << std::endl;
	} catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return 1;
	}
}
