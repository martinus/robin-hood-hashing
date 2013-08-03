#pragma once

#include <algorithm>
#include <unordered_map>
#include <cstdint>

// HopScotch algorithm. Based on
// http://mcg.cs.tau.ac.il/papers/disc2008-hopscotch.pdf
// https://github.com/harieshsathya/Hopscotch-Hashing/blob/master/hopscotch.hpp


template<class T, class H = DummyHash<size_t>, class A = std::allocator<T> >
class HopScotch {
private:
  enum CONSTANTS {
    HOP_SIZE = 8
  };

  //typedef unsigned int HopType;
  typedef std::uint8_t HopType;


public:
  HopScotch()
  : _allocator()
  {
    init_data(HOP_SIZE);
  }

  ~HopScotch() {
    for (size_t i=0; i<_size; ++i) {
      if (_keys[i] != (size_t)-1) {
        _allocator.destroy(_values + i);
      }
    }

    delete[] _keys;
    _allocator.deallocate(_values, _size);
  }

  // inline because it should be fast
  inline bool insert(size_t key, T val) {
    if (_size == _max_fullness) {
      increase_size();
    }

    const size_t sentinel = -1;
    size_t initial_idx = _hash(key) & _mask;

    // now, idx is the preferred position for this element. Search forward to find an empty place.
    // we use overflow area, so no modulo is required.
    size_t idx = initial_idx;
    while (idx < _max_size + HOP_SIZE && _keys[idx] != sentinel && _keys[idx] != key) {
      ++idx;
    }

    if (idx == _max_size + HOP_SIZE) {
      // retry insert
      increase_size();
      return insert(key, val);
    }

    if (_keys[idx] == key) {
      // found it! overwrite.
      _allocator.destroy(_values + idx);
      _allocator.construct(_values + idx, std::move(val));
      return false;
    }

    // we have found an empty spot. hop the hole back until we are at the right step.
    // idx is the empty spot.
    while (idx > initial_idx + HOP_SIZE - 1) {
      size_t search_start_idx = idx < HOP_SIZE ? 0 : idx - HOP_SIZE + 1;
      size_t i = search_start_idx;
      size_t h;

      while (i < idx && (h = _hash(_keys[i]) & _mask) < search_start_idx) {
        ++i;
      }
      if (i < idx) {
        // move it!
        _keys[idx] = std::move(_keys[i]);
        _allocator.construct(_values + idx, std::move(_values[i]));
        _allocator.destroy(_values + i);
        _hops[h] |= ((HopType)1 << (idx - h));

        // clear hop bit
        _hops[h] ^= ((HopType)1 << (i - h)); 
      } else {
        increase_size();
        return insert(key, val);
      }
      idx = i;
    }

    // now that we've moved everything, we can finally construct the element at
    // it's rightful place.
    _allocator.construct(_values + idx, std::move(val));
    _keys[idx] = std::move(key);
    _hops[initial_idx] |= ((HopType)1 << (idx - initial_idx));
    ++_size;
    return true;
  }

  T& find(size_t key, bool& success) {
    const size_t sentinel = -1;
    size_t idx = _hash(key) & _mask;
    HopType hops = _hops[idx];

    //for (size_t i=0; i<HOP_SIZE; ++i) {
    while (hops) {
      if (key == _keys[idx]) {
        success = true;
        return _values[idx];
      }
      hops >>= 1;
      ++idx;
    }

    success = false;
    return _values[0];
  }

  inline size_t size() const {
    return _size;
  }

  inline size_t max_size() const {
    return _max_size;
  }

private:
  // doubles size
  void increase_size() {
    size_t* old_keys = _keys;
    T* old_values = _values;
    size_t old_size = _max_size;
    init_data(_max_size*2);

    for (size_t i=0; i<old_size + HOP_SIZE; ++i) {
      if (old_keys[i] != (size_t)-1) {
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
    _keys = new size_t[_max_size + HOP_SIZE];
    _hops = new HopType[_max_size + HOP_SIZE];
    _values = _allocator.allocate(_max_size + HOP_SIZE);
    for (size_t i=0; i<_max_size + HOP_SIZE; ++i) {
      _keys[i] = (size_t)-1;
      _hops[i] = 0;
    }
    _max_fullness = _max_size*70/100;
  }

  size_t* _keys;
  HopType* _hops;
  T* _values;

  const H _hash;
  size_t _size;
  size_t _mask;
  size_t _max_size;
  size_t _max_fullness;

  A _allocator;
};
