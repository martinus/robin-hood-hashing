#pragma once

#include <algorithm>
#include <unordered_map>
#include <cstdint>

// HopScotch algorithm. Based on
// http://mcg.cs.tau.ac.il/papers/disc2008-hopscotch.pdf
// https://github.com/harieshsathya/Hopscotch-Hashing/blob/master/hopscotch.cpp

// TODO
// * get rid of sentinel: use one bit of the hop table to check if it's full.
//   if (hops & 1) {
//     // we are full!
//   } else {
//     // this is empty.
//   }
//   hops >>= 1; // after that, hops are defined as it was. Just with one less element at the end.
//
// * Use allocator for keys. Needs another allocator, because different type.
// * Make sure memory requirements stay OK (e.g. minimum fullness? max size?)
//   maybe automatically switch to HopScotchDefault if fast does not work any more?

struct HopScotchFast {
  typedef std::uint8_t HopType;
  enum Debug { DEBUG = 0 };
  enum resize_percentage { RESIZE_PERCENTAGE = 200 };
  enum hop_size { HOP_SIZE = 8-1 };
  enum add_range { ADD_RANGE = 512 };
  inline static size_t h(size_t v, size_t s, size_t mask) {
    return v & mask;
  }
};

struct HopScotchDefault {
  typedef std::uint32_t HopType;
  enum Debug { DEBUG = 0 };
  enum resize_percentage { RESIZE_PERCENTAGE = 200 };
  enum hop_size { HOP_SIZE = 32-1 };
  enum add_range { ADD_RANGE = 512 };
  inline static size_t h(size_t v, size_t s, size_t mask) {
    return v & mask;
  }
};

struct HopScotchCompact {
  typedef std::uint64_t HopType;
  enum Debug { DEBUG = 0 };
  enum resize_percentage { RESIZE_PERCENTAGE = 120 };
  enum hop_size { HOP_SIZE = 64-1 };
  enum add_range { ADD_RANGE = 1024 };
  inline static size_t h(size_t v, size_t s, size_t mask) {
    return v % s;
  }
};

template<
  class T, 
  class H = DummyHash<size_t>, 
  class Traits = HopScotchDefault,
  class A = std::allocator<T> >
class HopScotch {
public:
  typedef T value_type;

  HopScotch()
  : _allocator()
  {
    init_data(Traits::HOP_SIZE);
  }

  void clear() {
    for (size_t i=0; i<_max_size + Traits::HOP_SIZE; ++i) {
      if (_hops[i] & 1) {
        _allocator.destroy(_values + i);
      }
      _hops[i] = (HopType)0;
    }
    _size = 0;
  }

  ~HopScotch() {
    for (size_t i=0; i<_max_size + Traits::HOP_SIZE; ++i) {
      if (_hops[i] & 1) {
        _allocator.destroy(_values + i);
      }
    }

    delete[] _keys;
    delete[] _hops;
    _allocator.deallocate(_values, _size);
  }

  // inline because it should be fast
  inline bool insert(size_t key, T val) {
    size_t initial_idx = Traits::h(_hash(key), _max_size, _mask);

    // now, idx is the preferred position for this element. Search forward to find an empty place.
    // we use overflow area, so no modulo is required.
    size_t idx = initial_idx;

    // find key & replace if found
    Traits::HopType hops = _hops[idx] >> 1;
    while (hops) {
      if ((hops & 1) && (_keys[idx] == key)) {
        // found the key! replace value
        _allocator.destroy(_values + idx);
        _allocator.construct(_values + idx, std::move(val));
        return false;
      }
      ++idx;
      hops >>= 1;
    }

    // key is not there, so find an empty spot
    idx = initial_idx;
    size_t e = std::min(_max_size + Traits::HOP_SIZE, initial_idx + Traits::ADD_RANGE);
    while ((idx < e) && (_hops[idx] & 1)) {
      ++idx;
    }

    // no insert possible? resize and retry.
    if (idx == e) {
      // retry insert
      increase_size();
      return insert(key, val);
    }

    // we have found an empty spot, but it might be far away. We have to move the hole to the front
    // until we are at the right step. idx is the empty spot.
    while (idx > initial_idx + Traits::HOP_SIZE - 1) {
      // h: where the hash wants to be
      // i: where it actually is
      // idx: where it can be moved
      size_t start_h = idx < Traits::HOP_SIZE ? 0 : idx - Traits::HOP_SIZE + 1;
      size_t h;
      size_t i = start_h - 1;
      do {
        ++i;
        // find i's h
        h = start_h;
        while (h <= i 
          && !(_hops[h] & ((Traits::HopType)1 << (i - h + 1))))
        {
          ++h;
        }
      } while (i < idx && h > i);

      // insertion failed? resize and try again.
      if (i >= idx) {
        increase_size();
        return insert(key, val);
      }

      // found a place! move hole to the front
      _keys[idx] = std::move(_keys[i]);
      _hops[idx] |= (Traits::HopType)1;

      _allocator.construct(_values + idx, std::move(_values[i]));
      _allocator.destroy(_values + i);
      _hops[h] |= ((Traits::HopType)1 << (idx - h + 1));

      // clear hop bit
      _hops[h] ^= ((Traits::HopType)1 << (i - h + 1)); 

      idx = i;
    }

    // now that we've moved everything, we can finally construct the element at
    // it's rightful place.
    _allocator.construct(_values + idx, std::move(val));
    _keys[idx] = std::move(key);
    _hops[idx] |= (Traits::HopType)1;

    _hops[initial_idx] |= ((Traits::HopType)1 << (idx - initial_idx + 1));
    ++_size;
    return true;
  }

  inline T& find(size_t key, bool& success) {
    size_t idx = Traits::h(_hash(key), _max_size, _mask);

    Traits::HopType hops = _hops[idx] >> 1;

    while (hops) {
      if ((hops & 1) && (key == _keys[idx])) {
        success = true;
        return _values[idx];
      }
      hops >>= 1;
      ++idx;
    }

    success = false;
    return _values[0];
  }

  inline const T& find(size_t key, bool& success) const {
    size_t idx = Traits::h(_hash(key), _max_size, _mask);

    Traits::HopType hops = _hops[idx] >> 1;
    while (hops) {
      if ((hops & 1) && (key == _keys[idx])) {
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
    if (Traits::DEBUG) {
      // calculate memory requirements
      std::cout << "resize: " << _max_size << "\t" << 1.0*_size / (_max_size + Traits::HOP_SIZE) << std::endl;
    }
    size_t* old_keys = _keys;
    T* old_values = _values;
    Traits::HopType* old_hops = _hops;

    size_t old_size = _max_size;
    init_data(_max_size * Traits::RESIZE_PERCENTAGE / 100);

    for (size_t i=0; i<old_size + Traits::HOP_SIZE; ++i) {
      if (old_hops[i] & 1) {
        insert(old_keys[i], old_values[i]);
        _allocator.destroy(old_values + i);
      }
    }

    delete[] old_keys;
    delete[] old_hops;
    _allocator.deallocate(old_values, old_size);
  }


  void init_data(size_t new_size) {
    _size = 0;
    _max_size = new_size;
    _mask = _max_size - 1;
    _keys = new size_t[_max_size + Traits::HOP_SIZE];
    _hops = new Traits::HopType[_max_size + Traits::HOP_SIZE];
    _values = _allocator.allocate(_max_size + Traits::HOP_SIZE);
    for (size_t i=0; i<_max_size + Traits::HOP_SIZE; ++i) {
      _hops[i] = (Traits::HopType)0;
    }

    if (Traits::DEBUG) {
      size_t keys = sizeof(size_t) * (_max_size + Traits::HOP_SIZE);
      size_t hops = sizeof(Traits::HopType) * (_max_size + Traits::HOP_SIZE);
      size_t values = sizeof(T) * (_max_size + Traits::HOP_SIZE);
      std::cout << (keys + hops + values) << " bytes (" << keys << " keys, " << hops << " hops, " << values << " values)" << std::endl;
    }


    //_max_fullness = (_max_size + HOP_SIZE) * 80/100;
    _max_fullness = _max_size + Traits::HOP_SIZE - 1;
  }

  size_t* _keys;
  typename Traits::HopType* _hops;
  T* _values;

  const H _hash;
  size_t _size;
  size_t _mask;
  size_t _max_size;
  size_t _max_fullness;

  A _allocator;
};
