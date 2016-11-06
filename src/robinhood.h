#pragma once

#include <algorithm>
#include <unordered_map>
#include <cstdint>
#include <iostream>

#ifndef __has_cpp_attribute         // Optional of course.
  #define __has_cpp_attribute(x) 0  // Compatibility with non-clang compilers.
#endif

#if __has_cpp_attribute(clang::fallthrough)
#define FALLTHROUGH [[clang::fallthrough]]
#else
#define FALLTHROUGH
#endif

struct MurmurHash2 : public std::unary_function<size_t, std::string> {
  inline size_t operator()(const std::string& t) const {
    const size_t m = 0x5bd1e995;
    const int r = 24;
    size_t len = t.size();
    size_t h = len;

    size_t step_size = std::max(static_cast<size_t>(4), len/10);

    const unsigned char* data = reinterpret_cast<const unsigned char*>(t.c_str());
    while (len >= 4) {
      uint32_t k = *reinterpret_cast<const uint32_t*>(data);
      k *= m;
      k ^= k >> r;
      k *= m;

      h *= m;
      h ^= k;

      data += step_size;
      len -= step_size;
    }

    switch (len) {
    case 3:
      h ^= static_cast<size_t>(data[2]) << 16;
      FALLTHROUGH;
    case 2: 
      h ^= static_cast<size_t>(data[1]) << 8;
      FALLTHROUGH;
    case 1:
      h ^= data[0];
      h *= m;
    };

    // Do a few final mixes of the hash to ensure the last few
    // bytes are well-incorporated.

    h ^= h >> 13;
    h *= m;
    h ^= h >> 15;

    return h;
  }
};

struct Fnv : public std::unary_function<size_t, std::string> {
  inline size_t operator()(const std::string& t) const {
    size_t hash = 2166136261U;
    size_t first = 0;
    size_t last = t.size();
    size_t stride = 1 + last/10;
    for (; first < last; first += stride) {
      hash = (16777619U * hash) ^ static_cast<size_t>(t[first]);
    }
    return hash;
  }
};

template<class T>
struct DummyHash : public std::unary_function<T, T> {
  inline T operator()(const T& t) const {
    return t;
  }
};

template<class T>
struct MultiplyHash : public std::unary_function<T, T> {
  inline T operator()(const T& t) const {
    return 2654435769*t;
  }
};


struct InitSizet {
  InitSizet()
  : v(0)
  { }
  size_t v;
};



template<class T, class H = DummyHash<size_t>, class A = std::allocator<T> >
class RobinHoodHashMap {
public:
  RobinHoodHashMap()
  : _steps_to_count()
  , _moves()
  , _keys(0)
  , _values(0)
  , _hash()
  , _allocator()
  {
    init_data(8);
  }

  ~RobinHoodHashMap() {
    for (size_t i=0; i<_size; ++i) {
      if (_keys[i] != static_cast<size_t>(-1)) {
        _allocator.destroy(_values + i);
      }
    }

    delete[] _keys;
    _allocator.deallocate(_values, _size);
  }

  // inline because it should be fast
  inline bool insert(size_t key, T val) {
    const size_t sentinel = static_cast<size_t>(-1);
    size_t i = _hash(key) & _mask;

    size_t dist_key = 0;

    while (true) {
      if (_keys[i] == sentinel) {
        _keys[i] = std::move(key);
        // todo: is move ok here?
        _allocator.construct(_values + i, std::move(val));
        if (++_size == _max_fullness) {
          increase_size();
        }
        return true;

      } else if (_keys[i] == key) {
        _allocator.destroy(_values + i);
        _allocator.construct(_values + i, std::move(val));
        return false;

      } else {
        size_t dist_inplace = (i - _hash(_keys[i])) & _mask;

        if (dist_inplace < dist_key) {
          // this might be costly
          dist_key = dist_inplace;
          std::swap(key, _keys[i]);
          // todo allocator stuff here?
          std::swap(val, _values[i]);
        }
      }

      ++dist_key;
      ++i;
      i &= _mask;
    }
  }

  T& find(size_t key, bool& success) {
    static const size_t sentinel = static_cast<size_t>(-1);
    size_t i = _hash(key) & _mask;
    size_t dist_key = 0;

    //size_t steps = 0;
    while (true) {
      if (_keys[i] == sentinel || _keys[i] == key) {
        success = _keys[i] == key;
        return _values[i];
      }

      if (dist_key > ((i - _hash(_keys[i])) & _mask)) {
        success = false;
        return _values[i];
      }

      ++dist_key;
      ++i;
      i &= _mask;
    }
  }

  void print_steps() const {
    for (auto it = _steps_to_count.begin(); it != _steps_to_count.end(); ++it) {
      std::cout << it->first << ";" << it->second.v << std::endl;
    }
  }

  void print_moves() const {
    for (auto it = _moves.begin(); it != _moves.end(); ++it) {
      std::cout << it->first << ";" << it->second.v << std::endl;
    }
  }

  inline size_t size() const {
    return _size;
  }

  inline size_t max_size() const {
    return _max_size;
  }

private:
  std::unordered_map<size_t, InitSizet> _steps_to_count;
  std::unordered_map<size_t, InitSizet> _moves;

  // doubles size
  void increase_size() {
    size_t* old_keys = _keys;
    T* old_values = _values;
    size_t old_size = _max_size;
    init_data(_max_size*2);

    for (size_t i=0; i<old_size; ++i) {
      if (old_keys[i] != static_cast<size_t>(-1)) {
        insert(old_keys[i], old_values[i]);
        _allocator.destroy(old_values + i);
      }
    }
    delete[] old_keys;
    _allocator.deallocate(old_values, old_size);
  }


  void init_data(size_t new_size) {
    _size = 0;
    _max_size = new_size;
    _mask = _max_size - 1;
    _keys = new size_t[_max_size];
    _values = _allocator.allocate(_max_size);
    for (size_t i=0; i<_max_size; ++i) {
      _keys[i] = static_cast<size_t>(-1);
    }
    _max_fullness = _max_size*70/100;
  }

  size_t* _keys;
  T* _values;

  const H _hash;
  size_t _size;
  size_t _mask;
  size_t _max_size;
  size_t _max_fullness;

  A _allocator;
};
