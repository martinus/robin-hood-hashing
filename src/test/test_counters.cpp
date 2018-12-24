#include "test_base.h"

#include <bitset>
#include <iomanip>
#include <list>
#include <map>
#include <tuple>
#include <unordered_map>

class Counter {
public:
	struct Counts {
		size_t ctor{};
		size_t defaultCtor{};
		size_t copyCtor{};
		size_t dtor{};
		size_t equals{};
		size_t less{};
		size_t assign{};
		size_t swaps{};
		size_t get{};
		size_t constGet{};
		size_t hash{};
		size_t moveCtor{};
		size_t moveAssign{};

		void printHeader() {
			printf(
				"    ctor   defctor  cpyctor     dtor   assign    swaps      get  cnstget     hash   equals     less   ctormv assignmv |   total\n");
		}
		void reset() {
			ctor = 0;
			defaultCtor = 0;
			copyCtor = 0;
			dtor = 0;
			equals = 0;
			less = 0;
			assign = 0;
			swaps = 0;
			get = 0;
			constGet = 0;
			hash = 0;
			moveCtor = 0;
			moveAssign = 0;
		}

		void printCounts(std::string const& title) {
			size_t total = ctor + defaultCtor + copyCtor + dtor + equals + less + assign + swaps + get + constGet + hash + moveCtor + moveAssign;

			printf("%9zu%9zu%9zu%9zu%9zu%9zu%9zu%9zu%9zu%9zu%9zu%9zu%9zu |%9zu %s\n", ctor, defaultCtor, copyCtor, dtor, assign, swaps, get, constGet,
				   hash, equals, less, moveCtor, moveAssign, total, title.c_str());
		}
	};

	// required for operator[]
	Counter()
		: mData(0)
		, mCounts(nullptr) {}

	Counter(const int& data, Counts& counts)
		: mData(data)
		, mCounts(&counts) {
		if (mCounts) {
			++mCounts->ctor;
		}
	}

	Counter(const Counter& o)
		: mData(o.mData)
		, mCounts(o.mCounts) {
		if (mCounts) {
			++mCounts->copyCtor;
		}
	}

	Counter(Counter&& o)
		: mData(std::move(o.mData))
		, mCounts(std::move(o.mCounts)) {
		if (mCounts) {
			++mCounts->moveCtor;
		}
	}

	~Counter() {
		if (mCounts) {
			++mCounts->dtor;
		}
	}

	bool operator==(const Counter& o) const {
		if (mCounts) {
			++mCounts->equals;
		}
		return mData == o.mData;
	}

	bool operator<(const Counter& o) const {
		if (mCounts) {
			++mCounts->less;
		}
		return mData < o.mData;
	}

	Counter& operator=(const Counter& o) {
		mCounts = o.mCounts;
		if (mCounts) {
			++mCounts->assign;
		}
		mData = o.mData;
		return *this;
	}

	Counter& operator=(Counter&& o) {
		if (o.mCounts) {
			mCounts = std::move(o.mCounts);
		}
		mData = std::move(o.mData);
		if (mCounts) {
			++mCounts->moveAssign;
		}
		return *this;
	}

	const int& get() const {
		if (mCounts) {
			++mCounts->constGet;
		}
		return mData;
	}

	int& get() {
		if (mCounts) {
			++mCounts->get;
		}
		return mData;
	}

	void swap(Counter& other) {
		using std::swap;
		swap(mData, other.mData);
		swap(mCounts, other.mCounts);
		if (mCounts) {
			++mCounts->swaps;
		}
	}

	size_t hash() const {
		if (mCounts) {
			++mCounts->hash;
		}
		return std::hash<int>{}(mData);
	}

private:
	int mData;
	Counts* mCounts;
};

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

	Counter::Counts counts;
	counts.printHeader();
	{ TestType map; }
	counts.printCounts(std::string("ctor & dtor ") + name(TestType{}));
	REQUIRE(counts.dtor == counts.ctor + counts.defaultCtor + counts.copyCtor + counts.moveCtor);
	REQUIRE(counts.dtor == 0);
}

TEMPLATE_TEST_CASE("1 emplace", "[display]", (std::map<Counter, Counter>), (std::unordered_map<Counter, Counter>),
				   (robin_hood::flat_map<Counter, Counter>), (robin_hood::node_map<Counter, Counter>)) {

	Counter::Counts counts;
	{
		TestType map;
		map.emplace(std::piecewise_construct, std::forward_as_tuple(1, counts), std::forward_as_tuple(2, counts));
	}
	counts.printCounts(std::string("1 emplace ") + name(TestType{}));
	REQUIRE(counts.dtor == counts.ctor + counts.defaultCtor + counts.copyCtor + counts.moveCtor);
}

#define UNLIKELY(x) __builtin_expect((x), 0)

template <typename U = uint64_t>
class RandBool {
public:
	template <typename Rng>
	bool operator()(Rng& rng) {
		if (UNLIKELY(1 == m_rand)) {
			m_rand = std::uniform_int_distribution<U>{}(rng) | s_mask_left1;
		}
		bool const ret = m_rand & 1;
		m_rand >>= 1;
		return ret;
	}

private:
	static constexpr const U s_mask_left1 = U(1) << (sizeof(U) * 8 - 1);
	U m_rand = 1;
};

TEMPLATE_TEST_CASE("10k random insert & erase", "[display]", (std::map<Counter, Counter>), (std::unordered_map<Counter, Counter>),
				   (robin_hood::flat_map<Counter, Counter>), (robin_hood::node_map<Counter, Counter>)) {

	Counter::Counts counts;
	counts.printHeader();
	{
		Rng rng(321);
		TestType map;
		for (size_t i = 0; i < 10000; ++i) {
			for (size_t j = 0; j < 10; ++j) {
				map[Counter{rng(i), counts}] = Counter{i, counts};
				map.erase(Counter{rng(i), counts});

				map.emplace(std::piecewise_construct, std::forward_as_tuple(rng(i), counts), std::forward_as_tuple(i, counts));
				map.erase(Counter{rng(i), counts});

				map.insert(typename TestType::value_type{Counter{rng(i), counts}, Counter{i, counts}});
				map.erase(Counter{rng(i), counts});
			}
		}
	}

	counts.printCounts(std::string("10k random insert & erase - ") + name(TestType{}));
	REQUIRE(counts.dtor == counts.ctor + counts.defaultCtor + counts.copyCtor + counts.moveCtor);
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

struct hex {
	hex(int bits)
		: bits(bits) {}

	int const bits;
};

std::ostream& operator<<(std::ostream& os, hex const& h) {
	os << "0x" << std::setfill('0') << std::setw(h.bits * 2 / 8) << std::hex;
	return os;
}

void showHash(uint64_t val) {
	auto qm = robin_hood::hash_fast<uint64_t>{}(val);
	std::cout << hex(64) << val << " -> " << hex(64) << qm << " " << std::bitset<64>(qm) << std::endl;
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

template <typename T>
struct ConfigurableHash : public std::hash<T> {

	// 234679895032 masksum, 1.17938e+06 geomean for 0xbdcbaec81634e906 0xa309d159626eef52
	ConfigurableHash()
		: m_values{UINT64_C(0xe02b61472f2e2abf), UINT64_C(0x90c9f3e278ea1ac7)} {}

	ConfigurableHash(ConfigurableHash&& o) = default;
	ConfigurableHash(ConfigurableHash const& o) = default;

	ConfigurableHash& operator=(ConfigurableHash&& o) {
		m_values = std::move(o.m_values);
		return *this;
	}

	size_t operator()(T const& obj) const {
		using u128 = unsigned __int128;
		size_t h = std::hash<T>::operator()(obj);
		u128 factor = (u128(m_values[0]) << 64) | m_values[1];
		return (h * factor) >> 64;
	}

	std::array<uint64_t, 2> m_values;
	const std::array<uint64_t, 2> m_max_values = {UINT64_C(-1), UINT64_C(-1)};
};

template <size_t S>
void mutate(std::array<uint64_t, S>& vals, Rng& rng, RandBool<>& rbool) {
	do {
		if (rbool(rng)) {
			auto mask_bits = rng(24) + 1;
			uint64_t mask = (UINT64_C(1) << mask_bits) - 1;
			vals[rng(vals.size())] ^= mask << rng(64 - mask_bits);
		} else {
			vals[rng(vals.size())] = rng();
		}
	} while (rbool(rng));
}

template <typename A>
void eval(size_t const iters, A const& current_values, size_t& num_usecases, size_t& current_mask_sum, double& current_ops_sum) {
	using Map = robin_hood::flat_map<Counter, Counter, ConfigurableHash<Counter>>;
	try {
		Rng rng(iters * 0x135ff36020fe2455);
		// Rng rng(iters);

		Counter::Counts counts;

		size_t const num_iters = 50000;
		Map map;
		map.m_values = current_values;
		for (size_t i = 0; i < num_iters; ++i) {
			map[Counter{rng(i + 1), counts}] = Counter{rng(i + 1), counts};
			map.erase(Counter{rng(i + 1), counts});
			current_mask_sum += map.mask();
		}
		current_ops_sum += std::log(counts.swaps + counts.equals);
		++num_usecases;

		counts.reset();
		map = Map{};
		map.m_values = current_values;
		for (size_t i = 0; i < num_iters; ++i) {
			map.emplace(std::piecewise_construct, std::forward_as_tuple(rng(), counts), std::forward_as_tuple(i, counts));
			current_mask_sum += map.mask();
		}
		current_ops_sum += std::log(counts.swaps + counts.equals);
		++num_usecases;

		counts.reset();
		map = Map{};
		map.m_values = current_values;
		for (size_t i = 0; i < num_iters; ++i) {
			map.emplace(std::piecewise_construct, std::forward_as_tuple(rng(10000) << 32, counts), std::forward_as_tuple(i, counts));
			map.erase(Counter{rng(10000) << 32, counts});
			current_mask_sum += map.mask();
		}
		current_ops_sum += std::log(counts.swaps + counts.equals);
		++num_usecases;

		// shifted data
		counts.reset();
		map = Map{};
		static const size_t maxVal = 100000;
		for (int i = 0; i < num_iters / 8; ++i) {
			map[Counter{rng(maxVal), counts}] = Counter{i, counts};
			current_mask_sum += map.mask();
			map[Counter{rng(maxVal) << 8, counts}] = Counter{i, counts};
			current_mask_sum += map.mask();
			map[Counter{rng(maxVal) << 16, counts}] = Counter{i, counts};
			current_mask_sum += map.mask();
			map[Counter{rng(maxVal) << 24, counts}] = Counter{i, counts};
			current_mask_sum += map.mask();
			map[Counter{rng(maxVal) << 32, counts}] = Counter{i, counts};
			current_mask_sum += map.mask();
			map[Counter{rng(maxVal) << 40, counts}] = Counter{i, counts};
			current_mask_sum += map.mask();
			map[Counter{rng(maxVal) << 48, counts}] = Counter{i, counts};
			current_mask_sum += map.mask();
			map[Counter{rng(maxVal) << 56, counts}] = Counter{i, counts};
			current_mask_sum += map.mask();
			map.erase(Counter{rng(maxVal), counts});
			map.erase(Counter{rng(maxVal) << 8, counts});
			map.erase(Counter{rng(maxVal) << 16, counts});
			map.erase(Counter{rng(maxVal) << 24, counts});
			map.erase(Counter{rng(maxVal) << 32, counts});
			map.erase(Counter{rng(maxVal) << 40, counts});
			map.erase(Counter{rng(maxVal) << 48, counts});
			map.erase(Counter{rng(maxVal) << 56, counts});
		}
		current_ops_sum += std::log(counts.swaps + counts.equals);
		++num_usecases;

		// just sequential insertion
		counts.reset();
		map = Map{};
		map.m_values = current_values;
		for (size_t i = 0; i < num_iters; ++i) {
			map.emplace(std::piecewise_construct, std::forward_as_tuple(i, counts), std::forward_as_tuple(i, counts));
			current_mask_sum += map.mask();
		}
		current_ops_sum += std::log(counts.swaps + counts.equals);
		++num_usecases;

		// sequential shifted
		counts.reset();
		map = Map{};
		map.m_values = current_values;
		for (size_t i = 0; i < num_iters; ++i) {
			map.emplace(std::piecewise_construct, std::forward_as_tuple(i << 32, counts), std::forward_as_tuple(i, counts));
			current_mask_sum += map.mask();
		}
		current_ops_sum += std::log(counts.swaps + counts.equals);
		++num_usecases;
	} catch (std::overflow_error const&) {
		current_mask_sum += (std::numeric_limits<size_t>::max)() / 100;
		current_ops_sum += (std::numeric_limits<double>::max)() / 100;
		++num_usecases;
	}
}

TEST_CASE("quickmixoptimizer", "[!hide]") {
	Rng factorRng(std::random_device{}());
	RandBool<> rbool;

	using Map = robin_hood::flat_map<Counter, Counter, ConfigurableHash<Counter>>;
	Map startup_map;
	auto best_values = startup_map.m_values;
	for (size_t i = 0; i < best_values.size(); ++i) {
		best_values[i] = factorRng();
	}
	size_t best_mask_sum = (std::numeric_limits<size_t>::max)();
	double best_ops_sum = (std::numeric_limits<double>::max)();

	auto current_values = best_values;
	while (true) {

		size_t num_usecases = 0;
		size_t current_mask_sum = 0;
		double current_ops_sum = 0;
#pragma omp parallel for reduction(+ : num_usecases, current_mask_sum, current_ops_sum)
		for (size_t iters = 0; iters < 12 * 4; ++iters) {
			eval(iters, current_values, num_usecases, current_mask_sum, current_ops_sum);
		}
		std::cout << ".";
		std::cout.flush();

		// also assign when we are equally good, should lead to a bit more exploration
		if (std::tie(current_mask_sum, current_ops_sum) < std::tie(best_mask_sum, best_ops_sum)) {
			// if (3 * std::log(current_mask_sum) + std::log(current_ops_sum) < 3 * std::log(best_mask_sum) + std::log(best_ops_sum)) {
			std::cout << std::endl << std::dec << current_mask_sum << " masksum, " << std::exp(current_ops_sum / num_usecases) << " geomean for ";
			for (auto const x : current_values) {
				std::cout << hex(64) << x << " ";
			}
			std::cout << std::endl;
		}

		if (std::tie(current_mask_sum, current_ops_sum) <= std::tie(best_mask_sum, best_ops_sum)) {
			best_mask_sum = current_mask_sum;
			best_ops_sum = current_ops_sum;
			best_values = current_values;
		}

		// mutate *after* evaluation & setting best, so initial value is tried too
		current_values = best_values;
		mutate(current_values, factorRng, rbool);
	}
}
