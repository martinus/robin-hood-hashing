#include <hopscotch.h>
#include <HopScotchAdaptive.h>
#include <RobinHoodInfobitsHashbits.h>
#include <RobinHoodInfobyte.h>
#include <RobinHoodInfobyteFastforward.h>
#include <3rdparty/rigtorp/HashMap.h>
#include <3rdparty/sparsepp/sparsepp.h>

#include <timer.h>
#include <robinhood.h>
#include <marsagliamwc99.h>

#include <string>
#include <iostream>
#include <unordered_set>
#include <unordered_map>

#include <Windows.h>
#include <psapi.h>

#include <random>
#include <chrono>

#include <google/dense_hash_map>

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


#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define CHECK(x) if (!(x)) throw std::exception(__FILE__ "(" TOSTRING(__LINE__) "): " #x);


template<class H>
void test1(int times) {
    H rhhs;
    CHECK(rhhs.size() == 0);
    CHECK(rhhs.insert(32145, 123));
    CHECK(rhhs.size() == 1);

    for (int i = 0; i < times; ++i) {
        CHECK(rhhs.insert(i * 4, i));
        auto found = rhhs.find(i * 4);
        CHECK(found != nullptr);
        CHECK(*found == i);
        if (rhhs.size() != 2 + i) {
            CHECK(rhhs.size() == 2 + i);
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

    MarsagliaMWC99 rand;

    for (unsigned i = 0; i < 100000; ++i) {
        size_t val = static_cast<size_t>(rand(i + 1));
        CHECK(u.insert(val).second == r.insert(val, 123));
    }
}

template<class H>
void bench_str(size_t insertions, size_t queries, size_t times) {
    MarsagliaMWC99 rand(insertions * 5);
    const int seed = 23154;

    size_t key_length = 5;
    size_t val_length = 10;

    {
        HopScotch::Map<std::string, std::string, H, HopScotch::Style::Fast> r;
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

    MarsagliaMWC99 rand(insertions * 5);
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

    MarsagliaMWC99 rand(insertions * 5);
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
        MarsagliaMWC99 rand;
        rand.seed(321);
        HopScotch::Map<size_t, int, T, HopScotch::Style::Fast> r;
        for (size_t i = 0; i < times; ++i) {
            r.insert(rand(i + 1), static_cast<int>(i));
        }
        std::cout << t.elapsed() << " HopScotch::Map<size_t, int, T, HopScotch::Style::Fast> " << r.size() << std::endl;
    }
    {
        Timer t;
        MarsagliaMWC99 rand;
        rand.seed(321);
        HopScotch::Map<size_t, int, T, HopScotch::Style::Default> r;
        for (size_t i = 0; i < times; ++i) {
            r.insert(rand(i + 1), static_cast<int>(i));
        }
        std::cout << t.elapsed() << " HopScotch::Map<size_t, int, T, HopScotch::Style::Default> " << r.size() << std::endl;
    }
    {
        Timer t;
        MarsagliaMWC99 rand;
        rand.seed(321);
        HopScotch::Map<size_t, int, T, HopScotch::Style::Compact> r;
        for (size_t i = 0; i < times; ++i) {
            r.insert(rand(i + 1), static_cast<int>(i));
        }
        std::cout << t.elapsed() << " HopScotch::Map<size_t, int, T, HopScotch::Style::Compact> " << r.size() << std::endl;
    }
    {
        Timer t;
        MarsagliaMWC99 rand;
        rand.seed(321);
        hash_table<size_t, int, T> ht;
        for (size_t i = 0; i < times; ++i) {
            ht.insert(rand(i + 1), static_cast<int>(i));
        }
        std::cout << t.elapsed() << " hash_table<size_t, int> " << ht.size() << std::endl;
    }
    {
        Timer t;
        MarsagliaMWC99 rand;
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
        MarsagliaMWC99 rand;
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
    Timer t;
    MarsagliaMWC99 rand;
    size_t seed = 142323;
    rand.seed(seed);

    HopScotch::Map<size_t, int> r;
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

std::string rand_str(MarsagliaMWC99& rand, const size_t num_letters) {
    std::string alphanum = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    std::string s;
    s.resize(num_letters);
    for (size_t i = 0; i < num_letters; ++i) {
        s[i] = alphanum[rand(alphanum.size())];
    }
    return std::move(s);
}

struct HashX : public std::unary_function<size_t, X> {
    inline size_t operator()(const X& t) const {
        ++x_hash;
        return std::hash<int>()(t.x);
    }
};


void test_count(size_t times) {
    MarsagliaMWC99 rand(times * 5);
    reset_x();
    {
        rand.seed(123);
        HopScotch::Map<X, X, HashX, HopScotch::Style::Hop16> hs;
        for (size_t i = 0; i < times; ++i) {
            hs.insert(static_cast<int>(rand()), static_cast<int>(i));
        }

        size_t f = 0;
        for (size_t i = 0; i < times * 10; ++i) {
            if (hs.find(rand()) != nullptr) {
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
            std::pair<StdMap::iterator, bool> p = ms.insert(StdMap::value_type(static_cast<int>(rand()), static_cast<int>(i)));
        }
        size_t f = 0;
        for (size_t i = 0; i < times * 10; ++i) {
            if (ms.find(rand()) != ms.end()) {
                ++f;
            }
        }
        std::cout << f;
    }
    print_x("std::unordered_map");
    reset_x();
}


size_t get_mem() {
    PROCESS_MEMORY_COUNTERS objProcessMemoryInfo;
    if (GetProcessMemoryInfo(GetCurrentProcess(), &objProcessMemoryInfo, sizeof(objProcessMemoryInfo))) {
        return objProcessMemoryInfo.PagefileUsage;
    }
    return 0;
}

struct Stats {
    double elapsed_insert;
    double elapsed_find;
    size_t mem;
    size_t num;
    size_t found;
};


template<class HS>
void bench_sequential_insert(const std::string& title, size_t upTo, size_t times, std::vector<std::vector<Stats>>& all_stats) {
    std::vector<Stats> stats;
    Stats s;
    Timer t;
    size_t mem_before = get_mem();
    HS r;
    int i = 0;
    size_t found = 0;
    for (size_t ti = 0; ti < times; ++ti) {
        // insert
        t.restart();
        for (size_t up = 0; up < upTo; ++up) {
            r.insert(i, i);
            ++i;
        }
        s.elapsed_insert = t.elapsed();
        s.mem = get_mem() - mem_before;
        s.num = r.size();

        // query
        t.restart();
        for (int j = 0; j < 10; ++j) {
            for (int up = 0, e = static_cast<int>(upTo); up < e; ++up) {
                if (r.find(up)) {
                    ++found;
                }
            }
            for (int up = static_cast<int>(times*upTo), e = static_cast<int>(times*upTo + upTo); up < e; ++up) {
                if (r.find(up)) {
                    ++found;
                }
            }
        }
        s.elapsed_find = t.elapsed();
        s.found = found;
        stats.push_back(s);
    }

    Stats sum;
    sum.elapsed_find = 0;
    sum.elapsed_insert = 0;
    sum.found = 0;
    std::for_each(stats.begin(), stats.end(), [&sum](const Stats& s) {
        sum.elapsed_find += s.elapsed_find;
        sum.elapsed_insert += s.elapsed_insert;
        sum.found += s.found;
    });

    std::cout
        << sum.elapsed_insert << " sec insert, " 
        << sum.elapsed_find << " sec find for " << title 
        << " (" << sum.found << " found)" << std::endl;
    all_stats.push_back(stats);
}


void print(const std::vector<std::vector<Stats>>& s) {
    auto elems = s[0].size();
    std::cout << "time insert:" << std::endl;
    for (size_t e = 0; e < elems; ++e) {
        for (size_t i = 0; i < s.size(); ++i) {
            const auto& st = s[i][e];
            if (i > 0) {
                std::cout << ";";
            }
            std::cout << st.elapsed_insert;
        }
        std::cout << std::endl;
    }

    std::cout << std::endl << "time find:" << std::endl;
    for (size_t e = 0; e < elems; ++e) {
        for (size_t i = 0; i < s.size(); ++i) {
            const auto& st = s[i][e];
            if (i > 0) {
                std::cout << ";";
            }
            std::cout << st.elapsed_find;
        }
        std::cout << std::endl;
    }

    std::cout << std::endl << "mem:" << std::endl;
    for (size_t e = 0; e < elems; ++e) {
        for (size_t i = 0; i < s.size(); ++i) {
            const auto& st = s[i][e];
            if (i > 0) {
                std::cout << ";";
            }
            std::cout << st.mem;
        }
        std::cout << std::endl;
    }

    std::cout << std::endl << "num:" << std::endl;
    for (size_t e = 0; e < elems; ++e) {
        for (size_t i = 0; i < s.size(); ++i) {
            const auto& st = s[i][e];
            if (i > 0) {
                std::cout << ";";
            }
            std::cout << st.num;
        }
        std::cout << std::endl;
    }
}


template<class H>
std::vector<std::vector<Stats>> bench_sequential_insert(size_t upTo, size_t times) {
    std::vector<std::vector<Stats>> all_stats;
    //bench_sequential_insert<RobinHoodInfobyte::Map<int, int, H, RobinHoodInfobyte::Style::Default>>("infobyte", upTo, times, all_stats);
    //bench_sequential_insert<RobinHoodInfobitsHashbits::Map<int, int, H, RobinHoodInfobitsHashbits::Style::Default>>("info & hash & overflow check", upTo, times, all_stats);
    //bench_sequential_insert<RobinHoodInfobyteFastforward::Map<int, int, H, RobinHoodInfobyteFastforward::Style::Default>>("info & fastforward", upTo, times, all_stats);
    bench_sequential_insert<HopScotchAdaptive::Map<int, int, H>>("HopScotchAdaptive", upTo, times, all_stats);
    bench_sequential_insert<HopScotch::Map<int, int, H, HopScotch::Style::Hop8>>("Hopscotch Hop8", upTo, times, all_stats);
    bench_sequential_insert<HopScotch::Map<int, int, H, HopScotch::Style::Hop16>>("Hopscotch Hop16", upTo, times, all_stats);
    bench_sequential_insert<HopScotch::Map<int, int, H, HopScotch::Style::Hop32>>("Hopscotch Hop32", upTo, times, all_stats);
    bench_sequential_insert<HopScotch::Map<int, int, H, HopScotch::Style::Hop64>>("Hopscotch Hop64", upTo, times, all_stats);
    {
        std::vector<Stats> stats;
        Stats s;
        Timer t;
        size_t mem_before = get_mem();
        
        //spp::sparse_hash_map<int, int, H> r;
        //std::unordered_map<int, int, H> r;

        google::dense_hash_map<int, int, H> r;
        r.set_empty_key(-1);
        r.set_deleted_key(-2);

        int i = 0;
        size_t found = 0;
        for (size_t ti = 0; ti < times; ++ti) {
            // insert
            t.restart();
            for (size_t up = 0; up < upTo; ++up) {
                r[i] = i;
                //r.insert(i, i);
                ++i;
            }
            s.elapsed_insert = t.elapsed();
            s.mem = get_mem() - mem_before;
            s.num = r.size();

            // query
            t.restart();
            const auto endIt = r.end();
            for (int j = 0; j < 10; ++j) {
                for (int up = 0, e = static_cast<int>(upTo); up < e; ++up) {
                    if (endIt != r.find(up)) {
                        ++found;
                    }
                }
                for (int up = static_cast<int>(times*upTo), e = static_cast<int>(times*upTo + upTo); up < e; ++up) {
                    if (endIt != r.find(up)) {
                        ++found;
                    }
                }
            }
            s.elapsed_find = t.elapsed();
            s.found = found;
            stats.push_back(s);
        }

        Stats sum;
        sum.elapsed_find = 0;
        sum.elapsed_insert = 0;
        std::for_each(stats.begin(), stats.end(), [&sum](const Stats& s) {
            sum.elapsed_find += s.elapsed_find;
            sum.elapsed_insert += s.elapsed_insert;
        });

        std::cout << sum.elapsed_insert << " sec insert, " << sum.elapsed_find << " sec find for " << "std::unordered_map" << std::endl;
        all_stats.push_back(stats);
    }

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

template<class H>
void random_bench_std(const std::string& title) {
    constexpr size_t count = 1000000;
    constexpr size_t iters = 1000000000;

    H hm;
    std::mt19937 mt;
    std::uniform_int_distribution<int> ud(2, count);

    int val;
    for (size_t i = 0; i < count; ++i) {
        val = ud(mt);
        hm.emplace(val, val);
    }

    auto start = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < iters; ++i) {
        hm.erase(val);
        val = ud(mt);
        hm.emplace(val, val);
    }
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = stop - start;

    std::cout << std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count() / iters
        << " ns/iter for " << title << "(size=" << hm.size() << ")" << std::endl;
}

int main(int argc, char** argv) {
    try {
        //test_compare(1000000);
        //random_bench<RobinHoodInfobitsHashbits::Map<int, int>>("RobinHoodInfobitsHashbits");
        //random_bench<RobinHoodInfobyteFastforward::Map<int, int>>("RobinHoodInfobyteFastforward");
        //random_bench<RobinHoodInfobyte::Map<int, int>>("RobinHoodInfobyte");
        //random_bench_std<rigtorp::HashMap<int, int>>("HashMap");
        //random_bench<HopScotch::Map<int, int>>("HopScotch");
        //random_bench_std<std::unordered_map<int, int>>("std::unordered_map");

        test1<HopScotchAdaptive::Map<int, int>>(100000);
        test1<RobinHoodInfobitsHashbits::Map<int, int>>(100000);
        test1<RobinHoodInfobyteFastforward::Map<int, int>>(100000);
        test1<RobinHoodInfobyte::Map<int, int>>(100000);
        test1<HopScotch::Map<int, int>>(100000);
        std::cout << "test1 ok!" << std::endl;

        auto stats = bench_sequential_insert<std::hash<size_t>>(100 * 1000, 100);
        print(stats);
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
}