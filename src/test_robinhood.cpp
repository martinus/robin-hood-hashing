#include <timer.h>
#include <robinhood.h>
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
  RobinHoodHashSet<> rhhs;
  CHECK(rhhs.size() == 0);
  rhhs.insert(32145);
  CHECK(rhhs.size() == 1);
  for (size_t i=0; i<10; ++i) {
    rhhs.insert(i*4);
    CHECK(rhhs.size() == 2 + i);
  }
}


void test2() {
  RobinHoodHashSet<> h;
  for (size_t i=0; i<11; ++i) {
    h.insert(i*i + 7);
  }

  // query
  for (size_t i=0; i<11; ++i) {
    CHECK(h.find(i*i+7));
  }
  for (size_t i=12; i<1000; ++i) {
    CHECK(!h.find(i*i+7));
  }
}

void test3() {
  RobinHoodHashSet<> h;
  for (size_t i=0; i<4; ++i) {
    h.insert(i * 8 + 14);
    h.insert(i * 8 + 15);
  }
  for (size_t i=0; i<4; ++i) {
  }
  for (size_t i=0; i<4; ++i) {
    CHECK(h.find(i * 8 + 14));
  }
}


void test4() {
  std::unordered_set<size_t> u;
  RobinHoodHashSet<> r;

  MarsagliaMWC99 rand;

  for (size_t i=0; i<100000; ++i) {
    size_t val = rand(i+1);
    CHECK(u.insert(val).second == r.insert(val));
  }
}


template<class T>
void bench1(size_t times, size_t max) {

  MarsagliaMWC99 rand;

  {
    RobinHoodHashMap<int, T> r;
    rand.seed(123);
    Timer t;
    for (size_t i=0; i<max; ++i) {
      r.insert(rand(), i);
    }
    std::cout << t.elapsed() << " ";
    t.restart();
    
    size_t f = 0;
    rand.seed(123);
    for (size_t i=0; i<times*2; ++i) {
      bool success;
      r.find(rand(), success);
      if (success) {
        ++f;
      }
    }
    std::cout << t.elapsed();
    std::cout << " RobinHoodHashMap<int> " << r.size() << " " << f << std::endl;
    r.print_steps();
  }

  {
    hash_table<size_t, int, T> r;
    rand.seed(123);
    Timer t;
    for (size_t i=0; i<max; ++i) {
      r.insert(rand(), i);
    }
    std::cout << t.elapsed() << " ";
    t.restart();
    
    size_t f = 0;
    rand.seed(123);
    for (size_t i=0; i<times*2; ++i) {
      if (r.find(rand())) {
        ++f;
      }
    }
    std::cout << t.elapsed();
    std::cout << " hash_table<size_t, int> " << r.size() << " " << f << std::endl;
  }

  {
    std::unordered_map<size_t, int, T> r;
    rand.seed(123);
    Timer t;
    for (size_t i=0; i<max; ++i) {
      r[rand()] = i;
    }
    std::cout << t.elapsed() << " ";
    t.restart();
    
    size_t f = 0;
    rand.seed(123);
    for (size_t i=0; i<times*2; ++i) {
      if (r.find(rand()) != r.end()) {
        ++f;
      }
    }
    std::cout << t.elapsed();
    std::cout << " std::unordered_map<size_t, int> " << r.size() << " " << f << std::endl;
  }

  {
    RobinHoodHashSet<T> r;
    rand.seed(123);
    Timer t;
    for (size_t i=0; i<max; ++i) {
      r.insert(rand());
    }
    std::cout << t.elapsed() << " ";
    t.restart();
    
    size_t f = 0;
    rand.seed(123);
    for (size_t i=0; i<times*2; ++i) {
      if (r.find(rand())) {
        ++f;
      }
    }
    std::cout << t.elapsed();
    std::cout << " RobinHoodHashSet<> " << r.size() << " " << f << std::endl;
    r.print_steps();
  }

  {
    std::unordered_set<size_t, T> r;
    rand.seed(123);
    Timer t;
    for (size_t i=0; i<max; ++i) {
      r.insert(rand());
    }
    std::cout << t.elapsed() << " ";
    t.restart();
    
    size_t f = 0;
    rand.seed(123);
    for (size_t i=0; i<times*2; ++i) {
      if (r.find(rand()) != r.end()) {
        ++f;
      }
    }
    std::cout << t.elapsed();
    std::cout << " std::unordered_set<size_t> " << r.size() << " " << f << std::endl;
  }
  std::cout << "bench done!\n\n";
}

void test5() {
  RobinHoodHashSet<> r;
  r.insert(7);
  r.insert(7+8);
  r.insert(0);
  r.insert(1);
  r.insert(0+8);
}

template<class T>
void test_map1(size_t times) {
  {
    Timer t;
    MarsagliaMWC99 rand;
    rand.seed(321);
    hash_table<size_t, int, T> ht;
    for (size_t i=0; i<times; ++i) {
      ht.insert(rand(i+1), i);
    }
    std::cout << t.elapsed() << " hash_table<size_t, int> " << ht.size() << std::endl;
    ht.print_moves();
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


int main(int argc, char** argv) {
  try {
    std::cout << "DummyHash bench" << std::endl;
    bench1<DummyHash<size_t> >(30000000, 1000000);

    std::cout << "std::hash bench" << std::endl;
    bench1<std::hash<size_t> >(30000000, 1000000);

    std::cout << "DummyHash" << std::endl;
    test_map1<DummyHash<size_t> >(500000);

    std::cout << "\nstd::hash" << std::endl;
    test_map1<std::hash<size_t> >(10000000);

    test1();
    test_rh();
    test5();
    test4();
    test3();
    test2();


  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }
}