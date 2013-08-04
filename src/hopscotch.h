#pragma once

#include <algorithm>
#include <unordered_map>
#include <cstdint>

// HopScotch algorithm. Based on
// http://mcg.cs.tau.ac.il/papers/disc2008-hopscotch.pdf
// https://github.com/harieshsathya/Hopscotch-Hashing/blob/master/hopscotch.cpp


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
    const size_t sentinel = -1;
    if (_size == _max_fullness) {
      increase_size();
    }

    size_t initial_idx = _hash(key) & _mask;

    // now, idx is the preferred position for this element. Search forward to find an empty place.
    // we use overflow area, so no modulo is required.
    size_t idx = initial_idx;
    size_t e = initial_idx + HOP_SIZE;
    while (idx < e && _keys[idx] != sentinel && _keys[idx] != key) {
      ++idx;
    }
    if (idx == e) {
      // no need to check key from now on - this is definitely a new insertion.
      e = _max_size + HOP_SIZE;
      while (idx < e && _keys[idx] != sentinel) {
        ++idx;
      }
    } else {
      if (_keys[idx] == key) {
        // found it! overwrite.
        _allocator.destroy(_values + idx);
        _allocator.construct(_values + idx, std::move(val));
        return false;
      } else {
        _allocator.construct(_values + idx, std::move(val));
        _keys[idx] = std::move(key);
        _hops[initial_idx] |= ((HopType)1 << (idx - initial_idx));
        ++_size;
        return true;
      }
    }

    // no insert possible? resize and retry.
    if (idx == _max_size + HOP_SIZE) {
      // retry insert
      increase_size();
      return insert(key, val);
    }

    // we have found an empty spot, but it's far away. We have to move the hole to the front
    // until we are at the right step. idx is the empty spot.
    while (idx > initial_idx + HOP_SIZE - 1) {
      /*
      size_t h = idx < HOP_SIZE ? 0 : idx - HOP_SIZE + 1;

      // no need to rehash - use hop information!
      HopType hop_mask = (HopType)-1;

      size_t i = h;
      HopType hops = 0;
      while (hops == 0 && i < idx) {
        hop_mask >>= 1;
        i = h;
        hops = _hops[h++] & hop_mask;
        while (hops && !(hops & 1)) {
          hops >>= 1;
          ++i;
        }
      }
      */

      size_t h = idx < HOP_SIZE ? -1 : idx - HOP_SIZE;
      size_t i;
      // no need to rehash - use hop information!
      HopType hop_mask = (HopType)-1;
      HopType hops = 0;
      do {
        hop_mask >>= 1;
        hops = _hops[++h] & hop_mask;
        i = h;
        while (hops && !(hops & 1)) {
          hops >>= 1;
          ++i;
        }
      } while (hops == 0 && i < idx);

      // insertion failed? resize and try again.
      if (i >= idx) {
        increase_size();
        return insert(key, val);
      }

      // found a place! move hole to the front
      _keys[idx] = std::move(_keys[i]);
      _allocator.construct(_values + idx, std::move(_values[i]));
      _allocator.destroy(_values + i);
      _hops[h] |= ((HopType)1 << (idx - h));

      // clear hop bit
      _hops[h] ^= ((HopType)1 << (i - h)); 

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
    size_t idx = _hash(key) & _mask;
    HopType hops = _hops[idx];

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
    HopType* old_hops = _hops;

    size_t old_size = _max_size;
    init_data(_max_size * 2);

    HopType hops = 0;
    for (size_t i=0; i<old_size + HOP_SIZE; ++i) {
      hops |= old_hops[i];
      if (hops & 1) {
        insert(old_keys[i], old_values[i]);
        _allocator.destroy(old_values + i);
      }
      hops >>= 1;
    }

    delete[] old_keys;
    delete[] old_hops;
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
      _hops[i] = (HopType)0;
    }

    //_max_fullness = (_max_size + HOP_SIZE) * 80/100;
    _max_fullness = _max_size + HOP_SIZE - 1;
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
