#pragma once

#include <algorithm>
#include <unordered_map>


template<class T>
struct DummyHash : public std::unary_function<T, T> {
  inline T operator()(const T& t) const {
    return t;
  }
};

struct InitSizet {
  InitSizet()
  : v(0)
  { }
  size_t v;
};



template<class T, class H = DummyHash<size_t> >
class RobinHoodHashMap {
public:
  RobinHoodHashMap()
  {
    init_data(8);
  }

  ~RobinHoodHashMap() {
    delete[] _keys;
    delete[] _values;
  }

  // inline because it should be fast
  inline bool insert(size_t key, T val) {
    const size_t sentinel = -1;
    size_t i = _hash(key) & _mask;

    size_t dist_key = 0;

    while (true) {
      if (_keys[i] == sentinel) {
        _keys[i] = std::move(key);
        _values[i] = std::move(val);
        if (++_size == _max_fullness) {
          increase_size();
        }
        return true;

      } else if (_keys[i] == key) {
        _values[i] = std::move(val);
        return false;

      } else {
        size_t dist_inplace = (i - _hash(_keys[i])) & _mask;

        if (dist_inplace < dist_key) {
          // this might be costly
          dist_key = dist_inplace;
          std::swap(key, _keys[i]);
          std::swap(val, _values[i]);
        }
      }

      ++dist_key;
      ++i;
      i &= _mask;
    }
  }

  T& find(size_t key, bool& success) {
    static const size_t sentinel = -1;
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
      if (old_keys[i] != (size_t)-1) {
        insert(old_keys[i], old_values[i]);
      }
    }
    delete[] old_keys;
    delete[] old_values;
  }


  void init_data(size_t new_size) {
    _size = 0;
    _max_size = new_size;
    _mask = _max_size - 1;
    _keys = new size_t[_max_size];
    _values = new T[_max_size];
    for (size_t i=0; i<_max_size; ++i) {
      _keys[i] = (size_t)-1;
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
};
