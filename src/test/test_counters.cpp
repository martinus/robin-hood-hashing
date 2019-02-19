#include "avalanche.h"
#include "hashes.h"
#include "test_base.h"

#include <bitset>
#include <list>
#include <map>
#include <tuple>
#include <unordered_map>

// Counter for only swaps & equals. Used for optimizing.
// Can't use static counters here because I want to do it in parallel.
class Counter {
public:
    static size_t staticDefaultCtor;
    static size_t staticDtor;
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

        static void printHeader() {
            printf("     ctor  defctor  cpyctor     dtor   assign    swaps      get  cnstget     "
                   "hash   equals     less   ctormv assignmv |    total\n");
        }
        void reset() {
            ctor = 0;
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
            size_t total = ctor + staticDefaultCtor + copyCtor + (dtor + staticDtor) + equals +
                           less + assign + swaps + get + constGet + hash + moveCtor + moveAssign;

            printf("%9zu%9zu%9zu%9zu%9zu%9zu%9zu%9zu%9zu%9zu%9zu%9zu%9zu |%9zu %s\n", ctor,
                   staticDefaultCtor, copyCtor, dtor + staticDtor, assign, swaps, get, constGet,
                   hash, equals, less, moveCtor, moveAssign, total, title.c_str());
        }
    };

    // required for operator[]
    Counter()
        : mData(0)
        , mCounts(nullptr) {
        ++staticDefaultCtor;
    }

    Counter(const size_t& data, Counts& counts)
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
        } else {
            ++staticDtor;
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

    size_t const& get() const {
        if (mCounts) {
            ++mCounts->constGet;
        }
        return mData;
    }

    size_t& get() {
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

    size_t getForHash() const {
        if (mCounts) {
            ++mCounts->hash;
        }
        return mData;
    }

private:
    size_t mData;
    Counts* mCounts;
};

size_t Counter::staticDefaultCtor = 0;
size_t Counter::staticDtor = 0;

inline void swap(Counter& a, Counter& b) {
    a.swap(b);
}

namespace std {

template <>
struct hash<Counter> {
    size_t operator()(const Counter& c) const {
        return robin_hood::hash<uint64_t>{}(c.getForHash());
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
const char* name(robin_hood::unordered_flat_map<K, V> const&) {
    return "robin_hood::unordered_flat_map";
}

template <typename K, typename V>
const char* name(robin_hood::unordered_node_map<K, V> const&) {
    return "robin_hood::unordered_node_map";
}

TEST_CASE("prefix", "[display][counter]") {
    Counter::Counts::printHeader();
}

TEMPLATE_TEST_CASE("map ctor & dtor", "[display][counter]", (std::unordered_map<Counter, Counter>),
                   (robin_hood::unordered_flat_map<Counter, Counter>),
                   (robin_hood::unordered_node_map<Counter, Counter>)) {

    Counter::Counts counts;
    // counts.printHeader();
    { TestType map; }
    counts.printCounts(std::string("ctor & dtor ") + name(TestType{}));
    REQUIRE(counts.dtor == counts.ctor + counts.defaultCtor + counts.copyCtor + counts.moveCtor);
    REQUIRE(counts.dtor == 0);
}

TEMPLATE_TEST_CASE("1 emplace", "[display]", (std::unordered_map<Counter, Counter>),
                   (robin_hood::unordered_flat_map<Counter, Counter>),
                   (robin_hood::unordered_node_map<Counter, Counter>)) {
    Counter::Counts counts;
    {
        TestType map;
        map.emplace(std::piecewise_construct, std::forward_as_tuple(static_cast<size_t>(1), counts),
                    std::forward_as_tuple(static_cast<size_t>(2), counts));
    }
    counts.printCounts(std::string("1 emplace ") + name(TestType{}));
    REQUIRE(counts.dtor == counts.ctor + counts.defaultCtor + counts.copyCtor + counts.moveCtor);
}

TEMPLATE_TEST_CASE("10k random insert & erase", "[display][counter]",
                   (std::unordered_map<Counter, Counter>),
                   (robin_hood::unordered_flat_map<Counter, Counter>),
                   (robin_hood::unordered_node_map<Counter, Counter>)) {

    Counter::Counts counts;
    Counter::staticDefaultCtor = 0;
    Counter::staticDtor = 0;
    {
        Rng rng(321);
        TestType map;
        for (size_t i = 1; i < 10000; ++i) {
            for (size_t j = 0; j < 10; ++j) {
                map[Counter{rng.uniform<size_t>(i), counts}] = Counter{i, counts};
                map.erase(Counter{rng.uniform<size_t>(i), counts});

                map.emplace(std::piecewise_construct,
                            std::forward_as_tuple(rng.uniform<size_t>(i), counts),
                            std::forward_as_tuple(i, counts));
                map.erase(Counter{rng.uniform<size_t>(i), counts});

                map.insert(typename TestType::value_type{Counter{rng.uniform<size_t>(i), counts},
                                                         Counter{i, counts}});
                map.erase(Counter{rng.uniform<size_t>(i), counts});
            }
        }
    }

    counts.printCounts(std::string("10k random insert & erase - ") + name(TestType{}));
    REQUIRE(counts.dtor + Counter::staticDtor == Counter::staticDefaultCtor + counts.ctor +
                                                     counts.defaultCtor + counts.copyCtor +
                                                     counts.moveCtor);
}

TEMPLATE_TEST_CASE("500M random find nonexisting", "[display][counter]",
                   (std::unordered_map<Counter, size_t>),
                   (robin_hood::unordered_flat_map<Counter, size_t>),
                   (robin_hood::unordered_node_map<Counter, size_t>)) {

    size_t const num_iters = 10;
    size_t const insertion_factor = 1000;
    size_t const num_finds = 500'000;
    Rng rng(123);

    Counter::Counts counts;
    Counter::staticDefaultCtor = 0;
    Counter::staticDtor = 0;

    size_t num_found = 0;

    {
        TestType map;
        for (size_t iters = 1; iters <= num_iters; ++iters) {
            auto const max_insertion = insertion_factor * iters;

            for (size_t j = 0; j < max_insertion; ++j) {
                map.emplace(std::piecewise_construct,
                            std::forward_as_tuple(rng.uniform<size_t>(), counts),
                            std::forward_as_tuple(static_cast<size_t>(1)));
            }

            for (size_t n = 0; n < num_finds; ++n) {
                num_found += map.count(Counter{rng.uniform<size_t>(), counts});
            }
        }
    }

    counts.printCounts(std::string("500M random find nonexisting - ") + name(TestType{}));
    REQUIRE(counts.dtor + Counter::staticDtor == Counter::staticDefaultCtor + counts.ctor +
                                                     counts.defaultCtor + counts.copyCtor +
                                                     counts.moveCtor);
}

TEMPLATE_TEST_CASE("500M random find mostly existing", "[display][counter]",
                   (std::unordered_map<Counter, size_t>),
                   (robin_hood::unordered_flat_map<Counter, size_t>),
                   (robin_hood::unordered_node_map<Counter, size_t>)) {

    size_t const num_iters = 10;
    size_t const insertion_factor = 1000;
    size_t const num_finds = 500'000;
    Rng rng(123);

    Counter::Counts counts;
    Counter::staticDefaultCtor = 0;
    Counter::staticDtor = 0;

    size_t num_found = 0;

    {
        TestType map;
        for (size_t iters = 1; iters <= num_iters; ++iters) {
            auto const max_insertion = insertion_factor * iters;

            for (size_t j = 0; j < max_insertion; ++j) {
                map.emplace(std::piecewise_construct,
                            std::forward_as_tuple(rng.uniform<size_t>(max_insertion), counts),
                            std::forward_as_tuple(static_cast<size_t>(1)));
            }

            for (size_t n = 0; n < num_finds; ++n) {
                num_found += map.count(Counter{rng.uniform<size_t>(max_insertion), counts});
            }
        }
    }

    counts.printCounts(std::string("500M random find mostly existing - ") + name(TestType{}));
    REQUIRE(counts.dtor + Counter::staticDtor == Counter::staticDefaultCtor + counts.ctor +
                                                     counts.defaultCtor + counts.copyCtor +
                                                     counts.moveCtor);
}

TEMPLATE_TEST_CASE("10 insert erase", "[display][counter]",
                   (robin_hood::unordered_node_map<Counter, Counter>)) {
    for (size_t i = 23; i < 25; ++i) {
        Rng rng(12);
        Counter::Counts counts;
        Counter::staticDefaultCtor = 0;
        Counter::staticDtor = 0;
        {
            TestType map;
            for (size_t j = 0; j < 24; ++j) {
                map[Counter{rng.uniform<size_t>(i), counts}] = Counter{i, counts};
                map.erase(Counter{rng.uniform<size_t>(i), counts});
            }
        }

        REQUIRE(counts.dtor + Counter::staticDtor == Counter::staticDefaultCtor + counts.ctor +
                                                         counts.defaultCtor + counts.copyCtor +
                                                         counts.moveCtor);
    }
}

TEMPLATE_TEST_CASE("100k [] and erase", "[display][counter]",
                   (std::unordered_map<Counter, Counter>),
                   (robin_hood::unordered_flat_map<Counter, Counter>),
                   (robin_hood::unordered_node_map<Counter, Counter>)) {

    Counter::Counts counts;
    Counter::staticDefaultCtor = 0;
    Counter::staticDtor = 0;
    {
        static const size_t maxVal = 5000;
        Rng rng(123);
        TestType map;
        for (size_t i = 1; i < 100000; ++i) {
            map[Counter{rng.uniform<size_t>(maxVal), counts}] = Counter{i, counts};
            map.erase(Counter{rng.uniform<size_t>(maxVal), counts});
        }
    }

    counts.printCounts(std::string("100k [] and erase ") + name(TestType{}));
    REQUIRE(counts.dtor + Counter::staticDtor == Counter::staticDefaultCtor + counts.ctor +
                                                     counts.defaultCtor + counts.copyCtor +
                                                     counts.moveCtor);
}

TEMPLATE_TEST_CASE("100k emplace and erase", "[display][counter]",
                   (std::unordered_map<Counter, Counter>),
                   (robin_hood::unordered_flat_map<Counter, Counter>),
                   (robin_hood::unordered_node_map<Counter, Counter>)) {

    Counter::Counts counts;
    // Counter::Counts::printHeader();
    Counter::staticDefaultCtor = 0;
    Counter::staticDtor = 0;
    {
        static const size_t maxVal = 5000;
        Rng rng(123);
        TestType map;
        for (size_t i = 1; i < 100000; ++i) {
            map.emplace(std::piecewise_construct,
                        std::forward_as_tuple(rng.uniform<size_t>(i), counts),
                        std::forward_as_tuple(i, counts));
            map.erase(Counter{rng.uniform<size_t>(maxVal), counts});
        }
    }

    counts.printCounts(std::string("100k emplace and erase ") + name(TestType{}));
    REQUIRE(counts.dtor + Counter::staticDtor == Counter::staticDefaultCtor + counts.ctor +
                                                     counts.defaultCtor + counts.copyCtor +
                                                     counts.moveCtor);
}

TEMPLATE_TEST_CASE("eval stats", "[display][counter]", (std::unordered_map<Counter, Counter>),
                   (robin_hood::unordered_flat_map<Counter, Counter>),
                   (robin_hood::unordered_node_map<Counter, Counter>)) {
    using Map = TestType;
    Counter::Counts counts;
    Counter::staticDefaultCtor = 0;
    Counter::staticDtor = 0;

    Rng rng(123);
    size_t const num_iters = 100000;
    for (size_t bench_iters = 0; bench_iters < 1; ++bench_iters) {
        {
            // this tends to be very slow because of lots of shifts
            Map map;
            for (size_t n = 1; n < 10000; n += (500 * 10000 / num_iters)) {
                for (size_t i = 0; i < 500; ++i) {
                    map[Counter{rng.uniform<size_t>(n), counts}] = Counter{i, counts};
                    map.erase(Counter{rng.uniform<size_t>(n), counts});
                }
            }
        }

        {
            Map map;
            for (size_t i = 0; i < num_iters; ++i) {
                map[Counter{rng.uniform<size_t>(i + 1), counts}] =
                    Counter{rng.uniform<size_t>(i + 1), counts};
                map.erase(Counter{rng.uniform<size_t>(i + 1), counts});
            }
        }

        {
            Map map;
            for (size_t i = 0; i < num_iters; ++i) {
                map.emplace(std::piecewise_construct,
                            std::forward_as_tuple(rng.uniform<size_t>(), counts),
                            std::forward_as_tuple(i, counts));
            }
        }

        {
            Map map;
            for (size_t i = 0; i < num_iters; ++i) {
                map.emplace(std::piecewise_construct,
                            std::forward_as_tuple(
                                rng.uniform<size_t>(10000) << (ROBIN_HOOD_BITNESS / 2), counts),
                            std::forward_as_tuple(i, counts));
                map.erase(Counter{rng.uniform<size_t>(10000) << (ROBIN_HOOD_BITNESS / 2), counts});
            }
        }

        {
            Map map;
            static const size_t maxVal = 100000 / (ROBIN_HOOD_BITNESS - 8);
            for (size_t i = 0; i < num_iters / 8; ++i) {
                for (size_t sh = 0; sh < (ROBIN_HOOD_BITNESS - 8); ++sh) {
                    map[Counter{rng.uniform<size_t>(maxVal) << sh, counts}] = Counter{i, counts};
                    map.erase(Counter{rng.uniform<size_t>(maxVal) << sh, counts});
                }
            }
        }

        {
            // just sequential insertion
            Map map;
            for (size_t i = 0; i < num_iters; ++i) {
                map.emplace(std::piecewise_construct, std::forward_as_tuple(i, counts),
                            std::forward_as_tuple(i, counts));
            }
        }

        {
            // sequential shifted
            Map map;
            for (size_t i = 0; i < num_iters; ++i) {
                map.emplace(std::piecewise_construct,
                            std::forward_as_tuple(i << ROBIN_HOOD_BITNESS / 2, counts),
                            std::forward_as_tuple(i, counts));
            }
        }
    }

    counts.printCounts(std::string("eval stats ") + name(TestType{}));
    REQUIRE(counts.dtor + Counter::staticDtor == Counter::staticDefaultCtor + counts.ctor +
                                                     counts.defaultCtor + counts.copyCtor +
                                                     counts.moveCtor);
}

struct BigObject {
    std::string str;
    std::vector<int> vec;
    std::shared_ptr<int> ptr;
    std::list<int> list;
};

namespace std {

template <>
struct hash<BigObject> {
    size_t operator()(BigObject const&) const {
        return 0;
    }
};

} // namespace std

#define PRINT_SIZEOF(x, A, B)                                                                  \
    std::cout << sizeof(x<A, A>) << " " << sizeof(x<A, B>) << " " << sizeof(x<B, A>) << " "    \
              << sizeof(x<B, B>)                                                               \
              << " bytes for " #x "<" #A ", " #A ">, <" #A ", " #B ">, <" #B ", " #A ">, <" #B \
                 ", " #B ">"                                                                   \
              << std::endl

TEST_CASE("show datastructure sizes", "[display]") {
    PRINT_SIZEOF(std::unordered_map, int, BigObject);
    PRINT_SIZEOF(robin_hood::unordered_map, int, BigObject);
    PRINT_SIZEOF(std::map, int, BigObject);
}

void showHash(size_t val) {
    auto sh = std::hash<size_t>{}(val);
    auto fh = hash::FNV1a<size_t>{}(val);
    auto rh = robin_hood::hash<size_t>{}(val);
    std::cout << hex(ROBIN_HOOD_BITNESS) << val << " ->  " << hex(ROBIN_HOOD_BITNESS) << sh << "   "
              << hex(ROBIN_HOOD_BITNESS) << fh << "    " << hex(ROBIN_HOOD_BITNESS) << rh << "   "
              << std::bitset<ROBIN_HOOD_BITNESS>{rh} << std::endl;
}

TEST_CASE("show hash distribution", "[display]") {
    std::cout << "input                  std::hash            hash::FNV1a           "
                 "robin_hood::hash     robin_hood::hash binary"
              << std::endl;
    for (size_t i = 0; i < 100; ++i) {
        showHash(i);
    }

    for (size_t i = 0; i < 10; ++i) {
        size_t s = ((0x23d7 + i) << (ROBIN_HOOD_BITNESS / 2)) + static_cast<size_t>(63);
        showHash(s);
    }

    for (size_t i = 1; i < 8; ++i) {
        showHash(i * (static_cast<size_t>(1) << (ROBIN_HOOD_BITNESS - 4)));
    }

    for (size_t i = 1; i != 0; i *= 2) {
        showHash(i);
        showHash(i + 1);
    }
}

struct ConfigurableCounterHash {
    // 234679895032 masksum, 1.17938e+06 geomean for 0xbdcbaec81634e906 0xa309d159626eef52
    ConfigurableCounterHash()
        : m_values{{UINT64_C(0x5e1caf9535ce6811)}} {}

    ConfigurableCounterHash(ConfigurableCounterHash&& o) = default;
    ConfigurableCounterHash(ConfigurableCounterHash const& o) = default;

    ConfigurableCounterHash& operator=(ConfigurableCounterHash&& o) {
        m_values = std::move(o.m_values);
        return *this;
    }

    // 167079903232 masksum, 133853041 ops best: 0xfa2f2eef662c03e7
    // 167079903232 masksum, 133660376 ops best: 0x8127f0f4be8afcc9
    size_t operator()(size_t const& obj) const {
#if defined(ROBIN_HOOD_HAS_UMUL128)
        // 167079903232 masksum, 120428523 ops best: 0xde5fb9d2630458e9
        uint64_t h;
        uint64_t l = robin_hood::detail::umul128(obj, m_values[0], &h);
        auto result = h + l;
#elif ROBIN_HOOD_BITNESS == 32
        uint64_t const r = obj * m_values[0];
        uint32_t h = static_cast<uint32_t>(r >> 32);
        uint32_t l = static_cast<uint32_t>(r);
        auto result = h + l;
#else
        // murmurhash 3 finalizer
        uint64_t h = obj;
        h ^= h >> 33;
        h *= 0xff51afd7ed558ccd;
        h ^= h >> 33;
        h *= 0xc4ceb9fe1a85ec53;
        h ^= h >> 33;
        auto h = static_cast<uint64_t>(m);
        auto l = static_cast<uint64_t>(m >> 64);
        auto result = h + l;
#endif
        return result >> m_shift;
    }

    size_t operator()(Counter const& c) const {
        return operator()(c.getForHash());
    }

    std::array<uint64_t, 1> m_values;
    int m_shift = 0;
};

template <typename A>
void eval(int const iters, A current_values, uint64_t& current_mask_sum,
          uint64_t& current_ops_sum) {
    using Map = robin_hood::unordered_flat_map<Counter, uint64_t, ConfigurableCounterHash,
                                               std::equal_to<Counter>, 95>;
    try {
        Rng rng(static_cast<uint64_t>(iters) * 0x135ff36020fe7455);
        /*
        Rng rng{0x405f2f9cff6e0d8f, 0x0f97ae53d08500ea, 0x91e06131913057c3, 13 + iters * 0};
        current_values[0] = UINT64_C(14708910760473355443);
        current_values[1] = UINT64_C(11794246526519903291);

        std::cout << rng << std::endl;
        for (auto x : current_values) {
                std::cout << x << " ";
        }
        std::cout << std::endl;
        */
        // RandomBool rbool;
        Counter::Counts counts;
        size_t const num_iters = 33000;

        {
            // this tends to be very slow because of lots of shifts
            Map map;
            map.m_values = current_values;
            map.m_shift = iters;

            for (size_t n = 2; n < 10000; n += (500 * 10000 / num_iters)) {
                for (size_t i = 0; i < 500; ++i) {
                    map.emplace(Counter{rng.uniform<size_t>(n * 10), counts}, i);
                    current_mask_sum += map.mask();
                    map.erase(Counter{rng.uniform<size_t>(n * 10), counts});
                }
            }
        }

        {
            Map map;
            map.m_values = current_values;
            map.m_shift = iters;

            for (size_t i = 0; i < num_iters; ++i) {
                map.emplace(Counter{rng.uniform<size_t>(i + 1), counts}, i);
                current_mask_sum += map.mask();
                map.erase(Counter{rng.uniform<size_t>(i + 1), counts});
            }
        }

        {
            Map map;
            map.m_values = current_values;
            map.m_shift = iters;

            for (size_t i = 0; i < num_iters; ++i) {
                map.emplace(std::piecewise_construct,
                            std::forward_as_tuple(rng.uniform<size_t>(), counts),
                            std::forward_as_tuple(i));
                current_mask_sum += map.mask();
            }
        }

        {
            Map map;
            map.m_values = current_values;
            map.m_shift = iters;

            for (size_t i = 0; i < num_iters; ++i) {
                map.emplace(std::piecewise_construct,
                            std::forward_as_tuple(
                                rng.uniform<size_t>(10000) << (ROBIN_HOOD_BITNESS / 2), counts),
                            std::forward_as_tuple(i));
                current_mask_sum += map.mask();
                map.erase(Counter{rng.uniform<size_t>(10000) << (ROBIN_HOOD_BITNESS / 2), counts});
            }
        }
        {
            Map map;
            map.m_values = current_values;
            map.m_shift = iters;

            static size_t const max_shift = ROBIN_HOOD_BITNESS - 8;
            static size_t const min_shift = 1;
            static size_t const actual_iters = num_iters / (max_shift - min_shift);
            for (size_t i = 0; i < actual_iters; ++i) {
                for (size_t sh = 1; sh < max_shift; ++sh) {
                    map.emplace(Counter{rng.uniform<size_t>(100000) << sh, counts}, i);
                    current_mask_sum += map.mask();
                    map.erase(Counter{rng.uniform<size_t>(100000) << sh, counts});
                }
            }
        }
        {
            // just sequential insertion
            Map map;
            map.m_values = current_values;
            map.m_shift = iters;

            for (size_t i = 0; i < num_iters; ++i) {
                map.emplace(std::piecewise_construct, std::forward_as_tuple(i, counts),
                            std::forward_as_tuple(i));
                current_mask_sum += map.mask();
            }
        }

        {
            // sequential shifted
            Map map;
            map.m_values = current_values;
            map.m_shift = iters;

            for (size_t i = 0; i < num_iters; ++i) {
                map.emplace(std::piecewise_construct,
                            std::forward_as_tuple(i << ROBIN_HOOD_BITNESS / 2, counts),
                            std::forward_as_tuple(i));
                current_mask_sum += map.mask();
            }
        }

        {
            Map map;
            map.m_values = current_values;
            map.m_shift = iters;

            for (size_t i = 1; i <= 100; ++i) {
                for (size_t j = 0; j < 1; ++j) {
                    map.emplace(std::piecewise_construct,
                                std::forward_as_tuple(rng.uniform<size_t>(), counts),
                                std::forward_as_tuple(i));
                    current_mask_sum += map.mask();
                }

                for (size_t j = 0; j < num_iters / 1000; ++j) {
                    map.count(Counter{rng.uniform<size_t>(), counts});
                }
            }
        }

        current_ops_sum += counts.moveAssign + counts.moveCtor + counts.equals + counts.hash;
    } catch (std::overflow_error const&) {
        current_mask_sum += (std::numeric_limits<uint64_t>::max)() / 1000;
        current_ops_sum += (std::numeric_limits<uint64_t>::max)() / 1000;
    }
}

#if defined(ROBIN_HOOD_UMULH)

bool ge(uint64_t mask_a, uint64_t ops_a, uint64_t mask_b, uint64_t ops_b) {
    // return std::log(mask_a) + std::log(ops_a) <= std::log(mask_b) + std::log(ops_b);
    return std::tie(mask_a, ops_a) <= std::tie(mask_b, ops_b);
}

TEST_CASE("quickmixoptimizer", "[!hide]") {
    Rng factorRng(std::random_device{}());
    RandomBool rbool;

    using Map = robin_hood::unordered_flat_map<Counter, Counter, ConfigurableCounterHash,
                                               std::equal_to<Counter>, 95>;
    Map startup_map;
    auto best_values = startup_map.m_values;
    auto global_best_values = best_values;

    std::cout << "initializing with random data" << std::endl;
    for (size_t i = 0; i < best_values.size(); ++i) {
        best_values[i] = factorRng();
    }

    uint64_t best_mask_sum = (std::numeric_limits<uint64_t>::max)();
    uint64_t best_ops_sum = (std::numeric_limits<uint64_t>::max)();

    uint64_t global_best_mask_sum = best_mask_sum;
    uint64_t global_best_ops_sum = best_ops_sum;

    auto current_values = best_values;
    int num_unsuccessful_tries = 0;
    while (true) {
        uint64_t current_mask_sum = 0;
        uint64_t current_ops_sum = 0;
#    pragma omp parallel for reduction(+ : current_mask_sum, current_ops_sum)
        for (int iters = 0; iters < 40; ++iters) {
            eval(iters, current_values, current_mask_sum, current_ops_sum);
        }
        // std::cout << ".";
        // std::cout.flush();

        ++num_unsuccessful_tries;

        // also assign when we are equally good, should lead to a bit more exploration
        if (ge(current_mask_sum, current_ops_sum, best_mask_sum, best_ops_sum) &&
            (current_values != best_values)) {

            best_mask_sum = current_mask_sum;
            best_ops_sum = current_ops_sum;
            best_values = current_values;

            if (ge(best_mask_sum, best_ops_sum, global_best_mask_sum, global_best_ops_sum) &&
                (global_best_values != best_values)) {

                global_best_mask_sum = best_mask_sum;
                global_best_ops_sum = best_ops_sum;
                global_best_values = best_values;

                Avalanche a;
                a.eval(5000, [&global_best_values](uint64_t h) {
                    ConfigurableCounterHash hasher;
                    hasher.m_values = global_best_values;
                    hasher.m_shift = 0;
                    return static_cast<size_t>(hasher(h));
                });
                a.save("quickmixoptimizer.ppm");
            }

            num_unsuccessful_tries = 0;

            std::cout << std::endl
                      << std::dec << global_best_mask_sum << " masksum, " << global_best_ops_sum
                      << " ops best: ";
            for (auto const x : global_best_values) {
                std::cout << hex(64) << x << " ";
            }

            std::cout << "  |  " << std::dec << best_mask_sum << " masksum, " << best_ops_sum
                      << " ops current: ";
            for (auto const x : best_values) {
                std::cout << hex(64) << x << " ";
            }

            std::cout << std::endl;
        }

        if (num_unsuccessful_tries == 800) {
            std::cout << "reinint after " << num_unsuccessful_tries << " tries" << std::endl;
            for (size_t i = 0; i < current_values.size(); ++i) {
                current_values[i] = factorRng();
            }
            best_values = current_values;
            best_mask_sum = (std::numeric_limits<uint64_t>::max)();
            best_ops_sum = (std::numeric_limits<uint64_t>::max)();
            num_unsuccessful_tries = 0;
        } else {
            // mutate *after* evaluation & setting best, so initial value is tried too
            current_values = best_values;
            mutate(current_values, factorRng, rbool);
        }
    }
}

#endif
