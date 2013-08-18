#include <hopscotch.h>

#include <timer.h>
#include <robinhood.h>
#include <marsagliamwc99.h>
#include <rh_hash_table.hpp>

#include <string>
#include <iostream>
#include <unordered_set>
#include <unordered_map>



// test some hashing stuff
template<class T>
double bench_hashing(int& data) {
  size_t max_size_mask = (1<<16) - 1;
  std::cout << max_size_mask << std::endl;
  Timer timer;
  T t;
  for (size_t i=1; i<1000000; ++i) {
    data += i*i + 7;
    t.insert(data & max_size_mask);
  }
  std::cout << t.size() << std::endl;
  return timer.elapsed();
}


#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define CHECK(x) if (!(x)) throw std::exception(__FILE__ "(" TOSTRING(__LINE__) "): " #x);


void test1() {
  RobinHoodHashMap<int> rhhs;
  CHECK(rhhs.size() == 0);
  rhhs.insert(32145, 123);
  CHECK(rhhs.size() == 1);
  for (size_t i=0; i<10; ++i) {
    rhhs.insert(i*4, 423);
    CHECK(rhhs.size() == 2 + i);
  }
}


void test2() {
  RobinHoodHashMap<int> h;
  for (size_t i=0; i<11; ++i) {
    h.insert(i*i + 7, 123);
  }

  // query
  for (size_t i=0; i<11; ++i) {
    bool f;
    h.find(i*i+7, f);
    CHECK(f);
  }
  for (size_t i=12; i<1000; ++i) {
    bool f;
    h.find(i*i+7, f);
    CHECK(!f);
  }
}

void test3() {RobinHoodHashMap<int> h;
  for (size_t i=0; i<4; ++i) {
    h.insert(i * 8 + 14, 123);
    h.insert(i * 8 + 15, 123);
  }
  for (size_t i=0; i<4; ++i) {
  }
  for (size_t i=0; i<4; ++i) {
    bool f;
    h.find(i * 8 + 14, f);
    CHECK(f);
  }
}


void test4() {
  std::unordered_set<size_t> u;
  RobinHoodHashMap<int> r;

  MarsagliaMWC99 rand;

  for (size_t i=0; i<100000; ++i) {
    size_t val = rand(i+1);
    CHECK(u.insert(val).second == r.insert(val, 123));
  }
}

template<class H>
void bench_str(size_t insertions, size_t queries, size_t times) {
  MarsagliaMWC99 rand(insertions*5);
  const int seed = 23154;

  size_t key_length = 5;
  size_t val_length = 10;

  {
    HopScotch<std::string, std::string, H, HopScotchFast> r;
    rand.seed(seed);
    size_t f = 0;
    Timer t;
    for (size_t its=0; its<times; ++its) {
      for (size_t i=0; i<insertions; ++i) {
        r.insert(rand_str(rand, key_length), rand_str(rand, val_length));
      }
    
      for (size_t i=0; i<queries; ++i) {
        if (r.find(rand_str(rand, key_length)) != nullptr) {
          ++f;
        }
      }
    }
    std::cout << t.elapsed();
    std::cout << " HopScotch<std::string, std::string, H, HopScotchFast> with move " << r.size() << " " << f << std::endl;
  }
  {
    HopScotch<std::string, std::string, H, HopScotchDefault> r;
    rand.seed(seed);
    size_t f = 0;
    Timer t;
    for (size_t its=0; its<times; ++its) {
      for (size_t i=0; i<insertions; ++i) {
        r.insert(rand_str(rand, key_length), rand_str(rand, val_length));
      }
    
      for (size_t i=0; i<queries; ++i) {
        if (r.find(rand_str(rand, key_length)) != nullptr) {
          ++f;
        }
      }
    }
    std::cout << t.elapsed();
    std::cout << " HopScotch<std::string, std::string, H, HopScotchDefault> with move " << r.size() << " " << f << std::endl;
  }
  {
    HopScotch<std::string, std::string, H, HopScotchCompact> r;
    rand.seed(seed);
    size_t f = 0;
    Timer t;
    for (size_t its=0; its<times; ++its) {
      for (size_t i=0; i<insertions; ++i) {
        r.insert(rand_str(rand, key_length), rand_str(rand, val_length));
      }
    
      for (size_t i=0; i<queries; ++i) {
        if (r.find(rand_str(rand, key_length)) != nullptr) {
          ++f;
        }
      }
    }
    std::cout << t.elapsed();
    std::cout << " HopScotch<std::string, std::string, H, HopScotchCompact> with move " << r.size() << " " << f << std::endl;
  }

  {
    std::unordered_map<std::string, std::string, H> r;
    rand.seed(seed);
    size_t f = 0;
    Timer t;
    for (size_t its=0; its<times; ++its) {
      for (size_t i=0; i<insertions; ++i) {
        r[rand_str(rand, key_length)] = rand_str(rand, val_length);
      }
    
      for (size_t i=0; i<queries; ++i) {
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

  MarsagliaMWC99 rand(insertions*5);
  const int seed = 23154;

  {
    HopScotch<size_t, T, H, HopScotchFast> r;
    rand.seed(seed);
    size_t f = 0;
    Timer t;
    for (size_t its=0; its<times; ++its) {
      for (size_t i=0; i<insertions; ++i) {
        size_t x = rand();
        r.insert(x, std::move(value));
      }      
    
      for (size_t i=0; i<queries; ++i) {
        if (r.find(rand()) != nullptr) {
          ++f;
        }
      }
    }
    std::cout << t.elapsed();
    std::cout << " HopScotch<size_t, T, H, HopScotchFast> with move " << r.size() << " " << f << std::endl;
  }

  {
    HopScotch<size_t, T, H, HopScotchFast> r;
    rand.seed(seed);
    size_t f = 0;
    Timer t;
    for (size_t its=0; its<times; ++its) {
      for (size_t i=0; i<insertions; ++i) {
        r.insert(rand(), value);
      }
    
      for (size_t i=0; i<queries; ++i) {
        if (r.find(rand()) != nullptr) {
          ++f;
        }
      }
    }
    std::cout << t.elapsed();
    std::cout << " HopScotch<size_t, T, H, HopScotchFast> no move " << r.size() << " " << f << std::endl;
  }

  {
    HopScotch<size_t, T, H, HopScotchDefault> r;
    rand.seed(seed);
    size_t f = 0;
    Timer t;
    for (size_t its=0; its<times; ++its) {
      for (size_t i=0; i<insertions; ++i) {
        r.insert(rand(), value);
      }
    
      for (size_t i=0; i<queries; ++i) {
        if (r.find(rand()) != nullptr) {
          ++f;
        }
      }
    }
    std::cout << t.elapsed();
    std::cout << " HopScotch<size_t, T, H, HopScotchDefault> " << r.size() << " " << f << std::endl;
  }
  {
    HopScotch<size_t, T, H, HopScotchCompact> r;
    rand.seed(seed);
    size_t f = 0;
    Timer t;
    for (size_t its=0; its<times; ++its) {
      for (size_t i=0; i<insertions; ++i) {
        r.insert(rand(), value);
      }
    
      for (size_t i=0; i<queries; ++i) {
        if (r.find(rand()) != nullptr) {
          ++f;
        }
      }
    }
    std::cout << t.elapsed();
    std::cout << " HopScotch<size_t, T, H, HopScotchCompact> " << r.size() << " " << f << std::endl;
  }

  {
    RobinHoodHashMap<T, H> r;
    rand.seed(seed);
    size_t f = 0;
    Timer t;
    for (size_t its=0; its<times; ++its) {
      for (size_t i=0; i<insertions; ++i) {
        r.insert(rand(), value);
      }
    
      for (size_t i=0; i<queries; ++i) {
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
    for (size_t its=0; its<times; ++its) {
      for (size_t i=0; i<insertions; ++i) {
        r.insert(rand(), value);
      }
    
      for (size_t i=0; i<queries; ++i) {
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
    for (size_t its=0; its<times; ++its) {
      for (size_t i=0; i<insertions; ++i) {
        r[rand()] = value;
      }
    
      for (size_t i=0; i<queries; ++i) {
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

  MarsagliaMWC99 rand(insertions*5);
  const int seed = 23154;

  {
    H r;
    rand.seed(seed);
    size_t f = 0;
    Timer t;
    for (size_t its=0; its<times; ++its) {
      for (size_t i=0; i<insertions; ++i) {
        r.insert(rand(), value);
      }
    
      for (size_t i=0; i<queries; ++i) {
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
    HopScotch<size_t, int, T, HopScotchFast> r;
    for (size_t i=0; i<times; ++i) {
      r.insert(rand(i+1), i);
    }
    std::cout << t.elapsed() << " HopScotch<size_t, int, T, HopScotchFast> " << r.size() << std::endl;
  }
  {
    Timer t;
    MarsagliaMWC99 rand;
    rand.seed(321);
    HopScotch<size_t, int, T, HopScotchDefault> r;
    for (size_t i=0; i<times; ++i) {
      r.insert(rand(i+1), i);
    }
    std::cout << t.elapsed() << " HopScotch<size_t, int, T, HopScotchDefault> " << r.size() << std::endl;
  }
  {
    Timer t;
    MarsagliaMWC99 rand;
    rand.seed(321);
    HopScotch<size_t, int, T, HopScotchCompact> r;
    for (size_t i=0; i<times; ++i) {
      r.insert(rand(i+1), i);
    }
    std::cout << t.elapsed() << " HopScotch<size_t, int, T, HopScotchCompact> " << r.size() << std::endl;
  }
  {
    Timer t;
    MarsagliaMWC99 rand;
    rand.seed(321);
    hash_table<size_t, int, T> ht;
    for (size_t i=0; i<times; ++i) {
      ht.insert(rand(i+1), i);
    }
    std::cout << t.elapsed() << " hash_table<size_t, int> " << ht.size() << std::endl;
  }
  {
    Timer t;
    MarsagliaMWC99 rand;
    rand.seed(321);
    RobinHoodHashMap<int, T> r;
    for (size_t i=0; i<times; ++i) {
      r.insert(rand(i+1), i);
    }
    std::cout << t.elapsed() << " RobinHoodHashMap<int> " << r.size() << std::endl;
    r.print_moves();
  }

  {
    Timer t;
    MarsagliaMWC99 rand;
    rand.seed(321);
    std::unordered_map<size_t, int, T> u;
    for (size_t i=0; i<times; ++i) {
      u[rand(i+1)] = i;
    }
    std::cout << t.elapsed() << " std::unordered_map<size_t, int> " << u.size() << std::endl;
  }
  std::cout << "test_map done!\n" << std::endl;
}

void test_rh() {
  hash_table<size_t, size_t> m;
}


struct D {
  D() {
    std::cout << "ctor" << std::endl;
  }
  ~D() {
    std::cout << "dtor" << std::endl;
  }
};

void test_del(int count) {
  D* d = new D[count];
  delete[] d;
}

template<class T>
void test_compare(size_t times) {
  Timer t;
  MarsagliaMWC99 rand;
  size_t seed = 142323;
  rand.seed(seed);
  
  HopScotch<size_t, int, T> r;
  typedef std::unordered_map<size_t, int, T> StdMap;
  StdMap m;

  StdMap m2;
  m2[423] = 342;

  for (size_t i=0; i<times; ++i) {
    size_t v = rand(i + 100);
    std::pair<StdMap::iterator, bool> p = m.insert(StdMap::value_type(v, i));
    bool was_inserted = r.insert(v, i);

    if (m.size() != r.size() || was_inserted != p.second) {
      std::cout << i << ": " << v << " " << was_inserted << " " << p.second << std::endl;
    }

    v = rand(i + 100);
    bool is_there = (r.find(v) != nullptr);
    bool found_stdmap = m.find(v) != m.end();
    if (found_stdmap != is_there) {
      std::cout << i << ": " << v << " " << found_stdmap << " " << is_there << std::endl;
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
  : x(0)
  {
    ++x_ctor;
  }

  X(X&& o)
  : x(o.x)
  {
    ++x_mov;
  }

  X(const X& o)
  : x(o.x)
  {
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

  ~X()
  {
    ++x_dtor;
  }

  bool operator==(const X& o) const {
    ++x_operatoreq;
    return x == o.x;
  }

  X(int x_)
  : x(x_)
  {
    ++x_intctor;
  }

public:
  int x;
};

struct HashX : public std::unary_function<size_t, X> {
  inline size_t operator()(const X& t) const {
    ++x_hash;
    return t.x;
    //return std::hash<int>()(t.x);
  }
};

std::string rand_str(MarsagliaMWC99& rand, const size_t num_letters) {
  std::string alphanum = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
  std::string s;
  s.resize(num_letters);
  for (size_t i=0; i<num_letters; ++i) {
    s[i] = alphanum[rand(alphanum.size())];
  }
  return std::move(s);
}

void test_count(size_t times) {
  MarsagliaMWC99 rand(times*3);
  reset_x();
  {
    rand.seed(123);
    HopScotch<X, X, HashX, HopScotchFast> hs;
    for (size_t i=0; i<times; ++i) {
      hs.insert(rand(), i);
    }

    size_t f = 0;
    for (size_t i=0; i<times*10; ++i) {
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
    for (size_t i=0; i<times; ++i) {
      std::pair<StdMap::iterator, bool> p = ms.insert(StdMap::value_type(rand(), i));
    }
    size_t f = 0;
    for (size_t i=0; i<times*10; ++i) {
      if (ms.find(rand()) != ms.end()) {
        ++f;
      }
    }
    std::cout << f;
  }
  print_x("std::unordered_map");
  reset_x();
}

void test_compare_str(size_t count) {
  typedef std::unordered_map<std::string, std::string> StdMap;
  StdMap ms;
  HopScotch<std::string, std::string, std::hash<std::string>, HopScotchFast> hs;

  MarsagliaMWC99 rand;
  rand.seed(123);
  size_t found_count = 0;
  for (size_t i=0; i<count; ++i) {
    std::string key = rand_str(rand, 40);
    std::string val = rand_str(rand, 0);

    std::pair<StdMap::iterator, bool> p = ms.insert(StdMap::value_type(key, val));
    bool was_inserted = hs.insert(key, val);

    if (ms.size() != hs.size() || p.second != was_inserted) {
      throw std::exception("string insert failure");
    }

    key = rand_str(rand, 40);
    bool found_stdmap = ms.find(key) != ms.end();
    const std::string* item = hs.find(key);
    if (item != nullptr) {
      ++found_count;
    }
    if (found_stdmap != (item != nullptr)) {
      std::cout << i << ": " << key << " " << found_stdmap << " " << (item != nullptr) << std::endl;
    }
  }

  std::cout << "string test: " << ms.size() << " " << hs.size() << " " << found_count << std::endl;
}


int main(int argc, char** argv) {
  std::unordered_map<X, X, HashX> m;
  m[32] = 123;

  size_t insertions = 200*1000;
  size_t queries = 100*1000*1000;
  size_t times = 1;


  try {
    //test_compare_str(1000000);

    test_count(244342);
    std::cout << "int, DummyHash" << std::endl;
    bench1<int, DummyHash<size_t> >(insertions, queries, times, 1231);

    test_compare<MultiplyHash<size_t> >(10000000);



    size_t i = 200*1000;
    size_t q = 100*1000*1000;
    size_t t = 1;
    bh<HopScotch<size_t, int, std::hash<size_t> > >(i, q, t, 1231, "HopScotch<size_t, int, std::hash<size_t> >");
    bh<HopScotch<size_t, int, std::hash<size_t>, HopScotchDefault> >(i, q, t, 1231, "HopScotch<size_t, int, std::hash<size_t>, HopScotchDefault>");
    bh<HopScotch<size_t, int, std::hash<size_t>, HopScotchCompact> >(i, q, t, 1231, "HopScotch<size_t, int, std::hash<size_t>, HopScotchCompact>");
    bh<HopScotch<size_t, int, DummyHash<size_t> > >(i, q, t, 1231, "HopScotch<size_t, int, DummyHash<size_t> >");
    bh<HopScotch<size_t, int, DummyHash<size_t>, HopScotchDefault> >(i, q, t, 1231, "HopScotch<size_t, int, DummyHash<size_t>, HopScotchDefault>");
    bh<HopScotch<size_t, int, DummyHash<size_t>, HopScotchCompact> >(i, q, t, 1231, "HopScotch<size_t, int, DummyHash<size_t>, HopScotchCompact>");
    bh<HopScotch<size_t, int, MultiplyHash<size_t> > >(i, q, t, 1231, "HopScotch<size_t, int, MultiplyHash<size_t> >");
    bh<HopScotch<size_t, int, MultiplyHash<size_t>, HopScotchDefault> >(i, q, t, 1231, "HopScotch<size_t, int, MultiplyHash<size_t>, HopScotchDefault>");
    bh<HopScotch<size_t, int, MultiplyHash<size_t>, HopScotchCompact> >(i, q, t, 1231, "HopScotch<size_t, int, MultiplyHash<size_t>, HopScotchCompact>");
    bh<HopScotch<size_t, std::string, DummyHash<size_t> > >(i, q, t, "fklajlejklahseh", "HopScotch<size_t, std::string, DummyHash<size_t> >");
    bh<HopScotch<size_t, std::string, DummyHash<size_t>, HopScotchDefault> >(i, q, t, "fklajlejklahseh", "HopScotch<size_t, std::string, DummyHash<size_t>, HopScotchDefault>");
    bh<HopScotch<size_t, std::string, DummyHash<size_t>, HopScotchCompact> >(i, q, t, "fklajlejklahseh", "HopScotch<size_t, std::string, DummyHash<size_t>, HopScotchCompact>");
    bh<HopScotch<size_t, std::string, std::hash<size_t> > >(i, q, t, "lfklkajasjefj", "HopScotch<size_t, std::string, std::hash<size_t> >");
    bh<HopScotch<size_t, std::string, std::hash<size_t>, HopScotchDefault> >(i, q, t, "lfklkajasjefj", "HopScotch<size_t, std::string, std::hash<size_t>, HopScotchDefault>");
    bh<HopScotch<size_t, std::string, std::hash<size_t>, HopScotchCompact> >(i, q, t, "lfklkajasjefj", "HopScotch<size_t, std::string, std::hash<size_t>, HopScotchCompact>");



    insertions = 2000*1000;
    queries = 1*1000*1000;
    times = 1;

    //bench_str<std::hash<std::string> >(insertions, queries, times);
    std::cout << ">>>>>>>>> String Benchmarks <<<<<<<<<<<<<" << std::endl;
    std::cout << "MurmurHash2" << std::endl;
    bench_str<MurmurHash2>(insertions, queries, times);
    std::cout << "Fnv" << std::endl;
    bench_str<Fnv>(insertions, queries, times);
    std::cout << "std::hash" << std::endl;
    bench_str<std::hash<std::string> >(insertions, queries, times);

    insertions = 200*1000;
    queries = 100*1000*1000;
    times = 1;

    std::cout << "\n>>>>>>>>> Benchmarking <<<<<<<<<<<<<" << std::endl;
    std::cout << "std::string, DummyHash" << std::endl;
    bench1<std::string, DummyHash<size_t> >(insertions, queries, times, "fklajlejklahseklsjd fjklals jlfasefjklasjlfejlasdjlfajlgd hashdgksadhas dhkhklsdahk sakhh");
    std::cout << "int, DummyHash" << std::endl;
    bench1<int, DummyHash<size_t> >(insertions, queries, times, 1231);
    std::cout << "int, std::hash" << std::endl;
    bench1<int, std::hash<size_t> >(insertions, queries, times, 1231);
    std::cout << "int, MultiplyHash" << std::endl;
    bench1<int, MultiplyHash<size_t> >(insertions, queries, times, 1231);
    std::cout << "std::string, std::hash" << std::endl;
    bench1<std::string, std::hash<size_t> >(insertions, queries, times, "lfklkajasjefj");

    std::cout << "test DummyHash" << std::endl;
    test_map1<DummyHash<size_t> >(500000);
    std::cout << "\nstd::hash" << std::endl;
    test_map1<std::hash<size_t> >(10000000);


    std::cout << std::endl << ">>>>>>>>> Tests <<<<<<<<<<<<<" << std::endl;




    test1();
    test_rh();
    test4();
    test3();
    test2();
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }
}