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



template<class H = DummyHash<size_t> >
class RobinHoodHashSet {
public:
  RobinHoodHashSet() {
    init_data(8);
  }

  ~RobinHoodHashSet() {
    delete _data;
  }

  // inline because it should be fast
  inline bool insert(size_t key) {
    const size_t sentinel = -1;
    size_t i = _hash(key) & _mask;

    while (true) {
      if (_data[i] == sentinel) {
        _data[i] = key;
        if (++_size == _max_fullness) {
          increase_size();
        }
        return true;
      } else if (_data[i] == key) {
        return false;
      } else {
        size_t dist_inplace = (i - _hash(_data[i])) & _mask;
        size_t dist_key = (i - _hash(key)) & _mask;

        if (dist_key > dist_inplace) {
          std::swap(_data[i], key);
        }
      }
      ++i;
      i &= _mask;
    }
    _data[i] = key;

    return false;
  }

   bool find(size_t key) {
    const size_t sentinel = -1;
    const size_t initial = _hash(key) & _mask;
    size_t i = initial;

    //size_t steps = 0;
    while (true) {
      size_t d = _data[i];

      if (d == sentinel) {
        //++_steps_to_count[steps].v;
        return false;
      }

      if (d == key) {
        //++_steps_to_count[steps].v;
        return true;
      }

      if (((i - initial) & _mask) > ((i - _hash(d)) & _mask)) {
        //++_steps_to_count[steps].v;
        return false;
      }

      ++i;
      i &= _mask;
      //++steps;
    }
  }

  void print_steps() const {
    for (auto it = _steps_to_count.begin(); it != _steps_to_count.end(); ++it) {
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

  // doubles size
  void increase_size() {
    size_t* old_data = _data;
    size_t old_size = _max_size;
    init_data(_max_size*2);
    for (size_t i=0; i<old_size; ++i) {
      if (old_data[i] != -1) {
        insert(old_data[i]);
      }
    }
    delete old_data;
  }


  void init_data(size_t new_size) {
    _size = 0;
    _max_size = new_size;
    _mask = _max_size - 1;
    _data = new size_t[_max_size];
    for (size_t i=0; i<_max_size; ++i) {
      _data[i] = (size_t)-1;
    }
    _max_fullness = _max_size*70/100;
  }

  size_t* _data;

  const H _hash;
  size_t _size;
  size_t _mask;
  size_t _max_size;
  size_t _max_fullness;
};




template<class T, class H = DummyHash<size_t>, class A = std::allocator<T> >
class RobinHoodHashMap {
public:
  RobinHoodHashMap()
  : _allocator()
  {
    init_data(8);
  }

  ~RobinHoodHashMap() {
    delete _data;
    _allocator.destroy(_values);
  }

  // inline because it should be fast
  inline bool insert(size_t key, T val) {
    const size_t sentinel = -1;
    size_t i = _hash(key) & _mask;

    while (true) {
      if (_data[i] == sentinel) {
        _data[i] = key;
        _allocator.construct(&_values[i], val);
        if (++_size == _max_fullness) {
          increase_size();
        }
        return true;
      } else if (_data[i] == key) {
        // just replace
        _allocator.destroy(&_values[i]);
        _allocator.construct(&_values[i], val);
        return false;
      } else {
        size_t dist_inplace = (i - _hash(_data[i])) & _mask;
        size_t dist_key = (i - _hash(key)) & _mask;

        if (dist_key > dist_inplace) {
          std::swap(_data[i], key);
          std::swap(_values[i], val);
        }
      }
      ++i;
      i &= _mask;
    }
  }

  T& find(size_t key, bool& success) {
    const size_t sentinel = -1;
    const size_t initial = _hash(key) & _mask;
    size_t i = initial;

    //size_t steps = 0;
    while (true) {
      size_t d = _data[i];

      if (d == sentinel) {
        //++_steps_to_count[steps].v;
        success = false;
        return _values[0];
      }

      if (d == key) {
        //++_steps_to_count[steps].v;
        success = true;
        return _values[i];
      }

      if (((i - initial) & _mask) > ((i - _hash(d)) & _mask)) {
        //++_steps_to_count[steps].v;
        success = false;
        return _values[0];
      }

      ++i;
      i &= _mask;
      //++steps;
    }
  }

  void print_steps() const {
    for (auto it = _steps_to_count.begin(); it != _steps_to_count.end(); ++it) {
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

  // doubles size
  void increase_size() {
    size_t* old_data = _data;
    T* old_values = _values;
    size_t old_size = _max_size;
    init_data(_max_size*2);
    for (size_t i=0; i<old_size; ++i) {
      if (old_data[i] != -1) {
        insert(old_data[i], old_values[i]);
        _allocator.destroy(&old_values[i]);
      }
    }
    delete old_data;
    _allocator.deallocate(old_values, old_size);
  }


  void init_data(size_t new_size) {
    _size = 0;
    _max_size = new_size;
    _mask = _max_size - 1;
    _data = new size_t[_max_size];
    _values = _allocator.allocate(_max_size);
    for (size_t i=0; i<_max_size; ++i) {
      _data[i] = (size_t)-1;
    }
    _max_fullness = _max_size*70/100;
  }

  size_t* _data;
  A _allocator;
  T* _values;

  const H _hash;
  size_t _size;
  size_t _mask;
  size_t _max_size;
  size_t _max_fullness;
};
