#include <timer.h>
#include <robinhood.h>
#include <hopscotch.h>
#include <marsagliamwc99.h>
#include <rh_hash_table.hpp>

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

void test3() {
  RobinHoodHashMap<int> h;
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


template<class T, class H>
void bench1(size_t insertions, size_t queries, const T& value) {

  MarsagliaMWC99 rand(1000000);
  const int seed = 23154;

  {
    HopScotch<T, H> r;
    rand.seed(seed);
    Timer t;
    for (size_t i=0; i<insertions; ++i) {
      r.insert(i, value);
    }
    std::cout << t.elapsed() << " ";
    t.restart();
    
    size_t f = 0;
    rand.seed(seed+1);
    for (size_t i=0; i<queries; ++i) {
      bool success;
      r.find(rand(), success);
      if (success) {
        ++f;
      }
    }
    std::cout << t.elapsed();
    std::cout << " HopScotch<T, H> " << r.size() << " " << f << std::endl;
  }

  {
    RobinHoodHashMap<T, H> r;
    rand.seed(seed);
    Timer t;
    for (size_t i=0; i<insertions; ++i) {
      r.insert(i, value);
    }
    std::cout << t.elapsed() << " ";
    t.restart();
    
    size_t f = 0;
    rand.seed(seed+1);
    for (size_t i=0; i<queries; ++i) {
      bool success;
      r.find(rand(), success);
      if (success) {
        ++f;
      }
    }
    std::cout << t.elapsed();
    std::cout << " RobinHoodHashMap<T, H> " << r.size() << " " << f << std::endl;
    r.print_steps();
  }

  {
    hash_table<size_t, T, H> r;
    rand.seed(seed);
    Timer t;
    for (size_t i=0; i<insertions; ++i) {
      r.insert(i, value);
    }
    std::cout << t.elapsed() << " ";
    t.restart();
    
    size_t f = 0;
    rand.seed(seed+1);
    for (size_t i=0; i<queries; ++i) {
      if (r.find(rand())) {
        ++f;
      }
    }
    std::cout << t.elapsed();
    std::cout << " hash_table<size_t, T, H> " << r.size() << " " << f << std::endl;
  }

  {
    std::unordered_map<size_t, T, H> r;
    rand.seed(seed);
    Timer t;
    for (size_t i=0; i<insertions; ++i) {
      r[i] = value;
    }
    std::cout << t.elapsed() << " ";
    t.restart();
    
    size_t f = 0;
    rand.seed(seed+1);
    for (size_t i=0; i<queries; ++i) {
      if (r.find(rand()) != r.end()) {
        ++f;
      }
    }
    std::cout << t.elapsed();
    std::cout << " std::unordered_map<size_t, T, H> " << r.size() << " " << f << std::endl;
  }
  std::cout << "bench done!\n\n";
}

template<class T>
void test_map1(size_t times) {
  {
    Timer t;
    MarsagliaMWC99 rand;
    rand.seed(321);
    HopScotch<int, T> r;
    double slowest = 0;
    for (size_t i=0; i<times; ++i) {
      r.insert(rand(i+1), i);
    }
    std::cout << t.elapsed() << " HopScotch<int> " << r.size() << std::endl;
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
    double slowest = 0;
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
  size_t max_i = std::numeric_limits<size_t>::max();
  rand.seed(seed);
  
  HopScotch<int, T> r;
  typedef std::unordered_map<size_t, int, T> StdMap;
  StdMap m;

  for (size_t i=0; i<times; ++i) {
    if (i == 32) {
      std::cout << "xx" << std::endl;
    }
    size_t v = rand(i + 100);
    bool was_inserted = r.insert(v, i);
    std::pair<StdMap::iterator, bool> p = m.insert(StdMap::value_type(v, i));

    if (was_inserted != p.second) {
      std::cout << i << ": " << v << " " << was_inserted << " " << p.second << std::endl;
    }

    bool is_there;
    v = rand(i + 100);
    r.find(v, is_there);
    bool found_stdmap = m.find(v) != m.end();
    if (found_stdmap != is_there) {
      std::cout << i << ": " << v << " " << found_stdmap << " " << is_there << std::endl;
    }
  }
  std::cout << "ok!" << std::endl;
}

int main(int argc, char** argv) {
  try {
    //test_compare<MultiplyHash<size_t> >(10000000);


    std::cout << ">>>>>>>>> Benchmarking <<<<<<<<<<<<<" << std::endl;
    size_t insertions = 200000;
    size_t queries = 100000000;
    std::cout << "int, std::hash" << std::endl;
    bench1<int, std::hash<size_t> >(insertions, queries, 1231);
    std::cout << "int, DummyHash" << std::endl;
    bench1<int, DummyHash<size_t> >(insertions, queries, 1231);
    std::cout << "int, MultiplyHash" << std::endl;
    bench1<int, MultiplyHash<size_t> >(insertions, queries, 1231);

    std::cout << "std::string, DummyHash" << std::endl;
    bench1<std::string, DummyHash<size_t> >(insertions, queries, "fklajlejklahseh");

    std::cout << "std::string, std::hash" << std::endl;
    bench1<std::string, std::hash<size_t> >(insertions, queries, "lfklkajasjefj");


    std::cout << "int, std::hash" << std::endl;
    bench1<int, std::hash<size_t> >(insertions, queries, 1231);


    std::cout << "test DummyHash" << std::endl;
    test_map1<DummyHash<size_t> >(500000);

    std::cout << "\nstd::hash" << std::endl;
    test_map1<std::hash<size_t> >(10000000);



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