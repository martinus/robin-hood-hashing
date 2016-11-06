#include <XoRoShiRo128Plus.h>

#include <hopscotch.h>
#include <HopScotchAdaptive.h>
#include <RobinHoodInfobitsHashbits.h>
#include <RobinHoodInfobyte.h>
#include <RobinHoodInfobyteJumpheuristic.h>
#include <RobinHoodInfobyteFastforward.h>

#include <robinhood.h>
#include <marsagliamwc99.h>
#include <XorShiftRng.h>
#include <XorShiftStar.h>
#include <Pcg32.h>

#include <string>
#include <iostream>
#include <unordered_set>
#include <unordered_map>

#include <random>
#include <chrono>
#include <fstream>

#include <google/dense_hash_map>
#include <RobinHoodInfobytePair.h>
#include <RobinHoodInfobytePairNoOverflow.h>
#include <MicroBenchmark.h>

#include <timer.h>

#include <chrono>
#include <thread>

#include <3rdparty/rigtorp/HashMap.h>
#include <3rdparty/sparsepp/sparsepp.h>
#include <3rdparty/tessil/hopscotch_map.h>
#include <3rdparty/sherwood_map/sherwood_map.hpp>

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
void set_high_priority() {
}
#endif

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

// test some hashing stuff
template<class T>
double bench_hashing(int& data) {
    size_t max_size_mask = (1 << 16) - 1;
    std::cout << max_size_mask << std::endl;
    Timer timer;
    T t;
    for (size_t i = 1; i < 1000000; ++i) {
        data += i*i + 7;
        t.insert(data & max_size_mask);
    }
    std::cout << t.size() << std::endl;
    return timer.elapsed();
}

class MyException : public std::exception {
public:
	MyException(const char* msg)
	: mMsg(msg) {
	}
	
private:
	const std::string mMsg;
};


#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define CHECK(x) if (!(x)) throw MyException(__FILE__ "(" TOSTRING(__LINE__) "): " #x);

template<class H>
void test1_std(int times) {
    H rhhs;
    CHECK(rhhs.size() == 0);
    auto it = rhhs.insert(std::make_pair(32145, 123));
    CHECK(it.second);
    CHECK(it.first->first == 32145);
    CHECK(it.first->second == 123);
    CHECK(rhhs.size() == 1);

    for (int i = 0; i < times; ++i) {
        auto it = rhhs.insert(std::make_pair(i * 4, i));
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
    std::unordered_map<int, int> uo;
    XoRoShiRo128Plus rng;
    rng.seed(123);

    const int mod = times / 4;
    for (int i = 0; i < times; ++i) {
        int r = static_cast<int>(rng(mod));
        auto rhh_it = rhhs.insert(std::make_pair(r, r * 2));
        auto uo_it = uo.insert(std::make_pair(r, r * 2));
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

template<class H>
void test1(int times) {
    H rhhs;
    CHECK(rhhs.size() == 0);
    CHECK(rhhs.insert(32145, 123));
    CHECK(rhhs.size() == 1);

    for (int i = 0; i < times; ++i) {
        CHECK(rhhs.insert(i * 4, i));
        auto found = rhhs.find(i * 4);
        if (found == nullptr) {
            CHECK(found != nullptr);
        }
        CHECK(*found == i);
        if (rhhs.size() != 2 + static_cast<size_t>(i)) {
            CHECK(rhhs.size() == 2 + static_cast<size_t>(i));
        }
    }

    // check if everything can be found
    for (int i = 0; i < times; ++i) {
        auto found = rhhs.find(i * 4);
        CHECK(found != nullptr);
        CHECK(*found == i);
    }

    // check non-elements
    for (int i = 0; i < times; ++i) {
        auto found = rhhs.find((i + times) * 4);
        CHECK(found == nullptr);
    }

}


void test2() {
    RobinHoodHashMap<int> h;
    for (size_t i = 0; i < 11; ++i) {
        h.insert(i*i + 7, 123);
    }

    // query
    for (size_t i = 0; i < 11; ++i) {
        bool f;
        h.find(i*i + 7, f);
        CHECK(f);
    }
    for (size_t i = 12; i < 1000; ++i) {
        bool f;
        h.find(i*i + 7, f);
        CHECK(!f);
    }
}

void test3() {
    RobinHoodHashMap<int> h;
    for (size_t i = 0; i < 4; ++i) {
        h.insert(i * 8 + 14, 123);
        h.insert(i * 8 + 15, 123);
    }
    for (size_t i = 0; i < 4; ++i) {
    }
    for (size_t i = 0; i < 4; ++i) {
        bool f;
        h.find(i * 8 + 14, f);
        CHECK(f);
    }
}


void test4() {
    std::unordered_set<size_t> u;
    RobinHoodHashMap<int> r;

    XoRoShiRo128Plus rand;

    for (unsigned i = 0; i < 100000; ++i) {
        size_t val = static_cast<size_t>(rand(i + 1));
        CHECK(u.insert(val).second == r.insert(val, 123));
    }
}

std::string rand_str(XoRoShiRo128Plus& rand, const size_t num_letters) {
    std::string alphanum = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    std::string s;
    s.resize(num_letters);
    for (size_t i = 0; i < num_letters; ++i) {
        s[i] = alphanum[rand(alphanum.size())];
    }
    return s;
}



template<class H>
void bench_str(size_t insertions, size_t queries, size_t times) {
    XoRoShiRo128Plus rand(insertions * 5);
    const int seed = 23154;

    size_t key_length = 5;
    size_t val_length = 10;

    {
        HopScotch::Map<std::string, std::string, H, HopScotch::Style::Hop16> r;
        rand.seed(seed);
        size_t f = 0;
        Timer t;
        for (size_t its = 0; its < times; ++its) {
            for (size_t i = 0; i < insertions; ++i) {
                r.insert(rand_str(rand, key_length), rand_str(rand, val_length));
            }

            for (size_t i = 0; i < queries; ++i) {
                if (r.find(rand_str(rand, key_length)) != nullptr) {
                    ++f;
                }
            }
        }
        std::cout << t.elapsed();
        std::cout << " HopScotch::Map<std::string, std::string, H, HopScotch::Style::Fast> with move " << r.size() << " " << f << std::endl;
    }
    {
        HopScotch::Map<std::string, std::string, H, HopScotch::Style::Hop16> r;
        rand.seed(seed);
        size_t f = 0;
        Timer t;
        for (size_t its = 0; its < times; ++its) {
            for (size_t i = 0; i < insertions; ++i) {
                r.insert(rand_str(rand, key_length), rand_str(rand, val_length));
            }

            for (size_t i = 0; i < queries; ++i) {
                if (r.find(rand_str(rand, key_length)) != nullptr) {
                    ++f;
                }
            }
        }
        std::cout << t.elapsed();
        std::cout << " HopScotch::Map<std::string, std::string, H, HopScotch::Style::Hop16> with move " << r.size() << " " << f << std::endl;
    }
    {
        HopScotch::Map<std::string, std::string, H, HopScotch::Style::Hop16> r;
        rand.seed(seed);
        size_t f = 0;
        Timer t;
        for (size_t its = 0; its < times; ++its) {
            for (size_t i = 0; i < insertions; ++i) {
                r.insert(rand_str(rand, key_length), rand_str(rand, val_length));
            }

            for (size_t i = 0; i < queries; ++i) {
                if (r.find(rand_str(rand, key_length)) != nullptr) {
                    ++f;
                }
            }
        }
        std::cout << t.elapsed();
        std::cout << " HopScotch::Map<std::string, std::string, H, HopScotch::Style::Hop16> with move " << r.size() << " " << f << std::endl;
    }

    {
        std::unordered_map<std::string, std::string, H> r;
        rand.seed(seed);
        size_t f = 0;
        Timer t;
        for (size_t its = 0; its < times; ++its) {
            for (size_t i = 0; i < insertions; ++i) {
                r[rand_str(rand, key_length)] = rand_str(rand, val_length);
            }

            for (size_t i = 0; i < queries; ++i) {
                if (r.find(rand_str(rand, key_length)) != r.end()) {
                    ++f;
                }
            }
        }
        std::cout << t.elapsed();
        std::cout << " std::unordered_map<std::string, std::string, H> " << r.size() << " " << f << std::endl;
    }
}


template<class T, class H>
void bench1(size_t insertions, size_t queries, size_t times, T value) {

    XoRoShiRo128Plus rand(insertions * 5);
    const int seed = 23154;

    {
        HopScotch::Map<size_t, T, H, HopScotch::Style::Hop16> r;
        rand.seed(seed);
        size_t f = 0;
        Timer t;
        for (size_t its = 0; its < times; ++its) {
            for (size_t i = 0; i < insertions; ++i) {
                size_t x = rand();
                r.insert(x, std::move(value));
            }

            for (size_t i = 0; i < queries; ++i) {
                if (r.find(rand()) != nullptr) {
                    ++f;
                }
            }
        }
        std::cout << t.elapsed();
        std::cout << " HopScotch::Map<size_t, T, H, HopScotch::Style::Hop16> with move " << r.size() << " " << f << std::endl;
    }

    {
        HopScotch::Map<size_t, T, H, HopScotch::Style::Hop16> r;
        rand.seed(seed);
        size_t f = 0;
        Timer t;
        for (size_t its = 0; its < times; ++its) {
            for (size_t i = 0; i < insertions; ++i) {
                r.insert(rand(), value);
            }

            for (size_t i = 0; i < queries; ++i) {
                if (r.find(rand()) != nullptr) {
                    ++f;
                }
            }
        }
        std::cout << t.elapsed();
        std::cout << " HopScotch::Map<size_t, T, H, HopScotch::Style::Hop16> no move " << r.size() << " " << f << std::endl;
    }

    {
        HopScotch::Map<size_t, T, H, HopScotch::Style::Hop16> r;
        rand.seed(seed);
        size_t f = 0;
        Timer t;
        for (size_t its = 0; its < times; ++its) {
            for (size_t i = 0; i < insertions; ++i) {
                r.insert(rand(), value);
            }

            for (size_t i = 0; i < queries; ++i) {
                if (r.find(rand()) != nullptr) {
                    ++f;
                }
            }
        }
        std::cout << t.elapsed();
        std::cout << " HopScotch::Map<size_t, T, H, HopScotch::Style::Hop16> " << r.size() << " " << f << std::endl;
    }
    {
        HopScotch::Map<size_t, T, H, HopScotch::Style::Hop16> r;
        rand.seed(seed);
        size_t f = 0;
        Timer t;
        for (size_t its = 0; its < times; ++its) {
            for (size_t i = 0; i < insertions; ++i) {
                r.insert(rand(), value);
            }

            for (size_t i = 0; i < queries; ++i) {
                if (r.find(rand()) != nullptr) {
                    ++f;
                }
            }
        }
        std::cout << t.elapsed();
        std::cout << " HopScotch::Map<size_t, T, H, HopScotch::Style::Hop16> " << r.size() << " " << f << std::endl;
    }

    {
        RobinHoodHashMap<T, H> r;
        rand.seed(seed);
        size_t f = 0;
        Timer t;
        for (size_t its = 0; its < times; ++its) {
            for (size_t i = 0; i < insertions; ++i) {
                r.insert(rand(), value);
            }

            for (size_t i = 0; i < queries; ++i) {
                bool success;
                r.find(rand(), success);
                if (success) {
                    ++f;
                }
            }
        }
        std::cout << t.elapsed();
        std::cout << " RobinHoodHashMap<T, H> " << r.size() << " " << f << std::endl;
        r.print_steps();
    }

/*
    {
        hash_table<size_t, T, H> r;
        rand.seed(seed);
        size_t f = 0;
        Timer t;
        for (size_t its = 0; its < times; ++its) {
            for (size_t i = 0; i < insertions; ++i) {
                r.insert(rand(), value);
            }

            for (size_t i = 0; i < queries; ++i) {
                if (r.find(rand())) {
                    ++f;
                }
            }
        }
        std::cout << t.elapsed();
        std::cout << " hash_table<size_t, T, H> " << r.size() << " " << f << std::endl;
    }
*/
    {
        std::unordered_map<size_t, T, H> r;
        rand.seed(seed);
        size_t f = 0;
        Timer t;
        for (size_t its = 0; its < times; ++its) {
            for (size_t i = 0; i < insertions; ++i) {
                r[rand()] = value;
            }

            for (size_t i = 0; i < queries; ++i) {
                if (r.find(rand()) != r.end()) {
                    ++f;
                }
            }
        }
        std::cout << t.elapsed();
        std::cout << " std::unordered_map<size_t, T, H> " << r.size() << " " << f << std::endl;
    }
    std::cout << "bench done!\n\n";
}

template<class H>
void bh(size_t insertions, size_t queries, size_t times, const typename H::value_type& value, const char* msg) {

    XoRoShiRo128Plus rand(insertions * 5);
    const int seed = 23154;

    {
        H r;
        rand.seed(seed);
        size_t f = 0;
        Timer t;
        for (size_t its = 0; its < times; ++its) {
            for (size_t i = 0; i < insertions; ++i) {
                r.insert(rand(), value);
            }

            for (size_t i = 0; i < queries; ++i) {
                if (r.find(rand()) != nullptr) {
                    ++f;
                }
            }
        }
        std::cout << t.elapsed();
        std::cout << " " << msg << " " << r.size() << " " << f << std::endl;
    }
}

template<class T>
void test_map1(size_t times) {
    {
        Timer t;
        XoRoShiRo128Plus rand;
        rand.seed(321);
        HopScotch::Map<size_t, int, T, HopScotch::Style::Hop16> r;
        for (size_t i = 0; i < times; ++i) {
            r.insert(rand(i + 1), static_cast<int>(i));
        }
        std::cout << t.elapsed() << " HopScotch::Map<size_t, int, T, HopScotch::Style::Fast> " << r.size() << std::endl;
    }
    {
        Timer t;
        XoRoShiRo128Plus rand;
        rand.seed(321);
        HopScotch::Map<size_t, int, T, HopScotch::Style::Hop32> r;
        for (size_t i = 0; i < times; ++i) {
            r.insert(rand(i + 1), static_cast<int>(i));
        }
        std::cout << t.elapsed() << " HopScotch::Map<size_t, int, T, HopScotch::Style::Default> " << r.size() << std::endl;
    }
    {
        Timer t;
        XoRoShiRo128Plus rand;
        rand.seed(321);
        HopScotch::Map<size_t, int, T, HopScotch::Style::Hop64> r;
        for (size_t i = 0; i < times; ++i) {
            r.insert(rand(i + 1), static_cast<int>(i));
        }
        std::cout << t.elapsed() << " HopScotch::Map<size_t, int, T, HopScotch::Style::Compact> " << r.size() << std::endl;
    }
    /*
    {
        Timer t;
        XoRoShiRo128Plus rand;
        rand.seed(321);
        hash_table<size_t, int, T> ht;
        for (size_t i = 0; i < times; ++i) {
            ht.insert(rand(i + 1), static_cast<int>(i));
        }
        std::cout << t.elapsed() << " hash_table<size_t, int> " << ht.size() << std::endl;
    }
    */
    {
        Timer t;
        XoRoShiRo128Plus rand;
        rand.seed(321);
        RobinHoodHashMap<int, T> r;
        for (size_t i = 0; i < times; ++i) {
            r.insert(rand(i + 1), static_cast<int>(i));
        }
        std::cout << t.elapsed() << " RobinHoodHashMap<int> " << r.size() << std::endl;
        r.print_moves();
    }

    {
        Timer t;
        XoRoShiRo128Plus rand;
        rand.seed(321);
        std::unordered_map<size_t, int, T> u;
        for (size_t i = 0; i < times; ++i) {
            u[rand(i + 1)] = static_cast<int>(i);
        }
        std::cout << t.elapsed() << " std::unordered_map<size_t, int> " << u.size() << std::endl;
    }
    std::cout << "test_map done!\n" << std::endl;
}

void test_compare(size_t times) {
    XoRoShiRo128Plus rand;
    size_t seed = 142323;
    rand.seed(seed);

    RobinHoodInfobyte::Map<size_t, int> r;
    typedef std::unordered_map<size_t, int> StdMap;
    StdMap m;

    StdMap m2;
    m2[423] = 342;

    for (size_t i = 0; i < times; ++i) {
        size_t v = rand(i + 100);
        std::pair<StdMap::iterator, bool> p = m.insert(StdMap::value_type(v, static_cast<int>(i)));
        bool was_inserted = r.insert(v, static_cast<int>(i));

        if (m.size() != r.size() || was_inserted != p.second) {
            std::cout << i << ": " << v << " " << was_inserted << " " << p.second << std::endl;
        }

        v = rand(i + 100);
        bool is_there = (r.find(v) != nullptr);
        bool found_stdmap = m.find(v) != m.end();
        if (found_stdmap != is_there) {
            std::cout << i << ": " << v << " " << found_stdmap << " " << is_there << std::endl;
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


void test_count(size_t times) {
    XoRoShiRo128Plus rand;
    reset_x();
    {
        rand.seed(123);
        HopScotch::Map<X, X, HashX, HopScotch::Style::Hop16> hs;
        for (size_t i = 0; i < times; ++i) {
            hs.insert(static_cast<int>(rand()), static_cast<int>(i));
        }

        size_t f = 0;
        for (size_t i = 0; i < times * 10; ++i) {
            if (hs.find(static_cast<int>(rand())) != nullptr) {
                ++f;
            }
        }
        std::cout << f;
    }
    print_x("HopScotch");

    reset_x();
    {
        rand.seed(123);
        typedef std::unordered_map<X, X, HashX> StdMap;
        StdMap ms;
        for (size_t i = 0; i < times; ++i) {
            ms.insert(StdMap::value_type(static_cast<int>(rand()), static_cast<int>(i)));
        }
        size_t f = 0;
        for (size_t i = 0; i < times * 10; ++i) {
            if (ms.find(static_cast<int>(rand())) != ms.end()) {
                ++f;
            }
        }
        std::cout << f;
    }
    print_x("std::unordered_map");
    reset_x();
}


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
        , found(0) {
    }

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


template<class O>
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
            out << st.mem / (1024.0*1024);
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

template<class HS>
void bench_sequential_insert(HS& r, MicroBenchmark& mb, const std::string& title, size_t increase, size_t totalTimes, std::vector<std::vector<Stats>>& all_stats) {
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
            for (int v = times*upTo, e = times*upTo + upTo; v < e; ++v) {
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
    std::for_each(stats.begin(), stats.end(), [&sum](const Stats& s) {
        sum += s;
    });
    sum /= stats.size();

    std::cout
        << 1000000 * sum.elapsed_insert << "; "
        << 1000000 * sum.elapsed_find_existing << "; "
        << 1000000 * sum.elapsed_find_nonexisting << "; "
        << sum.mem / (1024.0 * 1024) << "; "
        << sum.found << std::endl;

    all_stats.push_back(stats);

    std::ofstream fout("out.txt");
    print(fout, all_stats);
}


template<class O>
void print_header(O& out, const std::vector<std::vector<Stats>>& s, const std::string& title) {
    out << std::endl << title << std::endl;
    for (size_t i = 0; i < s.size(); ++i) {
        out << ";" << s[i][0].title;
    }
    out << std::endl;
}


template<class H>
std::vector<std::vector<Stats>> bench_sequential_insert(size_t upTo, size_t times) {
    std::cout << "Title;1M inserts [sec];find 1M existing [sec];find 1M nonexisting [sec];memory usage [MB];foundcount" << std::endl;
    std::vector<std::vector<Stats>> all_stats;

    MicroBenchmark mb;
    //bench_sequential_insert(hopscotch_map<int, int, H>(), "tessil/hopscotch_map", upTo, times, all_stats);


    {
        RobinHoodInfobytePairNoOverflow::Map<int, int, H> m;
        m.max_load_factor(0.95f);
        bench_sequential_insert(m, mb, "RobinHoodInfobytePairNoOverflow 0.95", upTo, times, all_stats);
    }

    {
        RobinHoodInfobytePairNoOverflow::Map<int, int, H> m;
        m.max_load_factor(0.5f);
        bench_sequential_insert(m, mb, "RobinHoodInfobytePairNoOverflow 0.5", upTo, times, all_stats);
    }

    {
        RobinHoodInfobytePair::Map<int, int, H> m;
        m.max_load_factor(0.95f);
        bench_sequential_insert(m, mb, "RobinHoodInfobytePair 0.95", upTo, times, all_stats);
    }
    {
        RobinHoodInfobytePair::Map<int, int, H> m;
        m.max_load_factor(0.5f);
        bench_sequential_insert(m, mb, "RobinHoodInfobytePair 0.5", upTo, times, all_stats);
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
    /*
    bench_sequential_insert<RobinHoodInfobyteJumpheuristic::Map<int, int, H, std::equal_to<int>, RobinHoodInfobyteJumpheuristic::Style::Default>>("infobyte Jumpheuristic", upTo, times, searchtimes, all_stats);
    bench_sequential_insert<RobinHoodInfobyte::Map<int, int, H, std::equal_to<int>, RobinHoodInfobyte::Style::Default>>("Robin Hood Infobyte", upTo, times, searchtimes, all_stats);
    bench_sequential_insert<HopScotchAdaptive::Map<int, int, H, std::equal_to<int>, HopScotchAdaptive::Style::Default>>("HopScotchAdaptive Default", upTo, times, searchtimes, all_stats);

    bench_sequential_insert(spp::sparse_hash_map<int, int, H>(), "spp::spare_hash_map", upTo, times, searchtimes, all_stats);
    bench_sequential_insert(sherwood_map<int, int>(), "sherwood_map", upTo, times, searchtimes, all_stats);

    bench_sequential_insert<RobinHoodInfobitsHashbits::Map<int, int, H, RobinHoodInfobitsHashbits::Style::Default>>("info & hash & overflow check", upTo, times, searchtimes, all_stats);
    bench_sequential_insert<RobinHoodInfobyteFastforward::Map<int, int, H, RobinHoodInfobyteFastforward::Style::Default>>("info & fastforward", upTo, times, searchtimes, all_stats);
    bench_sequential_insert<HopScotch::Map<int, int, H, HopScotch::Style::Hop8>>("Hopscotch Hop8", upTo, times, searchtimes, all_stats);
    bench_sequential_insert<HopScotch::Map<int, int, H, HopScotch::Style::Hop16>>("Hopscotch Hop16", upTo, times, searchtimes, all_stats);
    bench_sequential_insert<HopScotch::Map<int, int, H, HopScotch::Style::Hop32>>("Hopscotch Hop32", upTo, times, searchtimes, all_stats);
    bench_sequential_insert<HopScotch::Map<int, int, H, HopScotch::Style::Hop64>>("Hopscotch Hop64", upTo, times, searchtimes, all_stats);

    bench_sequential_insert<HopScotchAdaptive::Map<int, int, H, std::equal_to<int>, HopScotchAdaptive::Style::Fast>>("HopScotchAdaptive Fast", upTo, times, searchtimes, all_stats);
    bench_sequential_insert<HopScotchAdaptive::Map<int, int, H, std::equal_to<int>, HopScotchAdaptive::Style::Compact>>("HopScotchAdaptive Compact", upTo, times, searchtimes, all_stats);
    */
    return all_stats;
}

template<class H>
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

    std::cout << std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count() / iters
        << " ns/iter for " << title << "(size=" << hm.size() << ")" << std::endl;
}

template<class Key, class Val>
class DummyMap {
public:
    DummyMap()
        : _key(0)
        , _val(0) {
    }

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

template<class Op>
double random_bench_std(const std::string& title, int times, Op o) {
    //std::ranlux48 mt; // 544.425
    //std::ranlux24 mt; // 142.268
    //std::ranlux48_base mt; // 14.9259
    //std::knuth_b mt; // 14.8667
    //std::minstd_rand0 mt; // 8.64232
    //std::minstd_rand mt; // 8.61236
    //std::mt19937_64 mt; // 7.42715
    //std::ranlux24_base mt; // 15.3052
    //std::mt19937 mt; // 6.54914
    //XorShiftRng mt; // 3.67276
    //MarsagliaMWC99 mt; // 3.4988
    XoRoShiRo128Plus mt;
    //std::uniform_int_distribution<int> ud(2, max_val);

    double min_ns = (std::numeric_limits<double>::max)();
    for (int i = 0; i < times; ++i) {
        mt.seed(123);
        auto start = std::chrono::high_resolution_clock::now();
        o(mt);
        auto stop = std::chrono::high_resolution_clock::now();
        auto duration = stop - start;
        double ns = static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count());
        min_ns = (std::min)(ns, min_ns);
    }
    return min_ns;
}


template<class T>
void doNotOptmizeAway(T&& dat) {
#ifdef _WIN32
    volatile auto x = &dat;
#else
    // see https://www.reddit.com/r/cpp/comments/52yg8b/donotoptimizeaway_for_pre_c11_compilers/
    asm volatile("" : : "g"(dat) : "memory");
#endif
}

template<class R>
void benchRng(const char *name) {
    //MicroBenchmark mb(5, 1.0);
    MicroBenchmark mb;
    R rng;
    auto n = rng();
    while (mb.keepRunning()) {
        n += rng();
    }
    doNotOptmizeAway(n);
    std::cout << (1 / ((mb.min)() * 1000*1000)) << " Million OPS for " << name << std::endl;
}


template<class M>
void benchRandomInsertAndDelete(const char* name, uint32_t mask, uint32_t numElements) {
    MicroBenchmark mb;
    XorShiftRng rng;

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

template<class M>
void benchRandomFind(const char* name, uint32_t mask, uint32_t numElements) {
    MicroBenchmark mb;
    XorShiftRng rng;

    rng.seed(321);
    M m;
    //m.max_load_factor(0.99f);
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

template<class M>
void benchRandomInsert(const char* name, uint32_t mask, uint32_t numElements) {
    MicroBenchmark mb;
    XorShiftRng rng;

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


template<class K, class V, class H = std::hash<int>>
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

void testRng() {
    MicroBenchmark mb;
    XoRoShiRo128Plus xorshift(123);
    double d = 0;
    while (mb.keepRunning()) {
        d += xorshift.rand01();
    }
    doNotOptmizeAway(d);
    std::cout << mb.min() << " XoRoShiRo128Plus rng(321)" << d << std::endl;

    MarsagliaMWC99 mwc99;
    mwc99.seed(123);
    while (mb.keepRunning()) {
        d += mwc99.rand01();
    }
    doNotOptmizeAway(d);
    std::cout << mb.min() << " MarsagliaMWC99 rng(321)" << d << std::endl;

    const size_t times = 10000000000;
    /*
    const size_t upto = 1000;
    std::vector<size_t> counts(upto, 0);
    for (size_t i = 0; i < times; ++i) {
        ++counts[rng(upto)];
    }
    std::sort(counts.begin(), counts.end());
    for (size_t i = 0; i < counts.size(); ++i) {
        std::cout << counts[i] << std::endl;
    }

    */
    double x = 0;
    double lower = 999;
    double upper = -999;
    for (size_t i = 0; i < times; ++i) {
        auto r = xorshift.rand01();
        if (r < lower || r > upper) {
            printf("lower=%.50g, upper=%.50g\n", lower, upper);
        }
        lower = std::min(lower, r);
        upper = std::max(upper, r);
        x += r;
    }
    std::cout << (x / times) << ", lower=" << lower << ", upper=" << upper << std::endl;
}

int main(int argc, char** argv) {
    set_high_priority();

    benchRng<XorShiftRng>("XorShiftRng");
    benchRng<XoRoShiRo128Plus>("XoRoShiRo128Plus");
    benchRng<XorShiftStar>("XorShiftStar");
    benchRng<MarsagliaMWC99>("MarsagliaMWC99");
    benchRng<Pcg32>("Pcg32");

    test1_std<RobinHoodInfobytePairNoOverflow::Map<int, int>>(100000);

    auto stats = bench_sequential_insert<std::hash<size_t>>(100 * 1000, 1000);
    print(std::cout, stats);
    std::ofstream fout("out.txt");
    print(fout, stats);

    for (int i = 0; i < 10; ++i) {
        uint32_t mask = (1 << (20 + i)) - 1;
        uint32_t numElements = 1000 * 1000;
        //uint32_t mask = -1;
        //uint32_t numElements = 16100;
        /*
        benchRandomInsertAndDelete<std::unordered_map<uint32_t, uint32_t>>("std::unordered_map", mask, numElements);
        benchRandomInsertAndDelete<GoogleMapWrapper<uint32_t, uint32_t>>("dense_hash_map", mask, numElements);
        benchRandomInsertAndDelete<RobinHoodInfobytePair::Map<uint32_t, uint32_t>>("RobinHoodInfobytePair::Map", mask, numElements);
        */

        benchRandomFind<RobinHoodInfobytePairNoOverflow::Map<uint32_t, uint32_t>>("RobinHoodInfobytePairNoOverflow::Map", mask, numElements);
        benchRandomFind<RobinHoodInfobytePair::Map<uint32_t, uint32_t>>("RobinHoodInfobytePair::Map", mask, numElements);
        //benchRandomFind<GoogleMapWrapper<uint32_t, uint32_t>>("dense_hash_map", mask, numElements);
        //benchRandomFind<std::unordered_map<uint32_t, uint32_t>>("std::unordered_map", mask, numElements);

        //uint32_t mask = -1;
        /*
        std::cout << numElements << std::endl;
        benchRandomInsert<GoogleMapWrapper<uint32_t, uint32_t>>("dense_hash_map", mask, numElements);
        benchRandomInsert<RobinHoodInfobytePair::Map<uint32_t, uint32_t>>("RobinHoodInfobytePair::Map", mask, numElements);
        benchRandomInsert<std::unordered_map<uint32_t, uint32_t>>("std::unordered_map", mask, numElements);
        */
        std::cout << std::endl;
    }

    uint32_t max_num = 131072;
    int iters = 10000000;
    while (max_num) {
        uint32_t mask = max_num - 1;
        std::cout << mask << std::endl;
        //random_bench_std(DummyMap<int, int>(), "Dummy", mask, iters);
        {
            google::dense_hash_map<int, int, std::hash<int>> googlemap;
            googlemap.set_empty_key(-1);
            googlemap.set_deleted_key(-2);
            googlemap.max_load_factor(0.9f);
            std::cout << random_bench_std("googlemap", 5, [&](XoRoShiRo128Plus& mt) {
                for (int i = 0; i < iters; ++i) {
                    googlemap[mt() & mask] = i;
                    googlemap.erase(mt() & mask);
                }
            }) << " " << googlemap.size() << std::endl;
        }
        /*
        random_bench_std(RobinHoodInfobytePair::Map<int, int, std::hash<int>>(), "RobinHoodInfobytePair", mask, iters);
        random_bench_std(RobinHoodInfobytePair::Map<int, int, MultiplyHash<int>>(), "RobinHoodInfobytePair MultiplyHash", mask, iters);
        random_bench_std(RobinHoodInfobytePair::Map<int, int, DummyHash<int>>(), "RobinHoodInfobytePair DummyHash", mask, iters);
        random_bench_std(RobinHoodInfobytePair::Map<int, int, std::hash<int>, std::equal_to<int>, std::allocator<std::pair<int, int>>, RobinHoodInfobytePair::Style::Fast>(), "RobinHoodInfobytePair Fast", mask, iters);
        random_bench_std(std::unordered_map<int, int, std::hash<int>>(), "unorderd_map", mask, iters);
        //random_bench_std(spp::sparse_hash_map<int, int, std::hash<int>>(), "spp::spare_hash_map", mask, iters);
        //random_bench_std(sherwood_map<int, int, std::hash<int>>(), "sherwood_map", mask, iters);
        max_num <<= 3;
        */
    }

    //random_bench_std(RobinHoodInfobyteJumpheuristic::Map<int, int>(), "RobinHoodInfobyteJumpheuristic", num_max, iters);
    //random_bench_std(HopScotchAdaptive::Map<int, int>(), "HopScotchAdaptive", num_max, iters);
    try {

        //test_compare(1000000);
        //random_bench<RobinHoodInfobitsHashbits::Map<int, int>>("RobinHoodInfobitsHashbits");
        //random_bench<RobinHoodInfobyteFastforward::Map<int, int>>("RobinHoodInfobyteFastforward");
        //random_bench<RobinHoodInfobyte::Map<int, int>>("RobinHoodInfobyte");
        //random_bench_std<rigtorp::HashMap<int, int>>("HashMap");
        //random_bench<HopScotch::Map<int, int>>("HopScotch");
        //random_bench_std<std::unordered_map<int, int>>("std::unordered_map");


        test1<RobinHoodInfobyteJumpheuristic::Map<int, int>>(100000);
        test1<HopScotchAdaptive::Map<int, int>>(100000);
        test1<RobinHoodInfobitsHashbits::Map<int, int>>(100000);
        test1<RobinHoodInfobyteFastforward::Map<int, int>>(100000);
        test1<RobinHoodInfobyte::Map<int, int>>(100000);
        test1<HopScotch::Map<int, int>>(100000);
        //test1<hopscotch_map<int, int>>(100000);
        std::cout << "test1 ok!" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
}
