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
  class Key,
  class Val, 
  class H = std::hash<Key>, 
  class Traits = HopScotchDefault,
  class AVal = std::allocator<Val>,
  class AKey = std::allocator<Key>
>
class HopScotch {
public:
  typedef Val value_type;

  HopScotch()
  {
    init_data(Traits::HOP_SIZE + 1);
  }

  void clear() {
    for (size_t i=0; i<_max_size + Traits::HOP_SIZE; ++i) {
      if (_hops[i] & 1) {
        _alloc_val.destroy(_values + i);
        _alloc_key.destroy(_keys + i);
      }
      _hops[i] = (HopType)0;
    }
    _size = 0;
  }

  ~HopScotch() {
    for (size_t i=0; i<_max_size + Traits::HOP_SIZE; ++i) {
      if (_hops[i] & 1) {
        _alloc_val.destroy(_values + i);
        _alloc_key.destroy(_keys + i);
      }
    }

    delete[] _hops;
    _alloc_val.deallocate(_values, _size);
    _alloc_key.deallocate(_keys, _size);
  }

  inline bool insert(const Key& key, Val&& val) {
    return insert_impl(std::move(Key(key)), std::forward<Val>(val));
  }

  inline bool insert(Key&& key, const Val& val) {
    return insert_impl(std::forward<Key>(key), std::move(Val(val)));
  }

  inline bool insert(const Key& key, const Val& val) {
    return insert_impl(std::move(Key(key)), std::move(Val(val)));
  }

  inline bool insert(Key&& key, Val&& val) {
    return insert_impl(std::forward<Key>(key), std::forward<Val>(val));
  }

  // inline because it should be fast
  // todo use && for key as well
  inline bool insert_impl(Key&& key, Val&& val) {
    size_t initial_idx = Traits::h(_hash(key), _max_size, _mask);

    // now, idx is the preferred position for this element. Search forward to find an empty place.
    // we use overflow area, so no modulo is required.
    size_t idx = initial_idx;

    // find key & replace if found
    Traits::HopType hops = _hops[idx] >> 1;
    while (hops) {
      if ((hops & 1) && (_keys[idx] == key)) {
        // found the key! replace value
        _alloc_val.destroy(_values + idx);
        _alloc_val.construct(_values + idx, std::move(val));
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
      return insert(std::forward<Key>(key), std::forward<Val>(val));
    }

    // set the empty spot's hop bit
    _hops[idx] |= (Traits::HopType)1;

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
        // insertion failed, undo _hop[idx]
        _hops[idx] &= (Traits::HopType)-2;
        increase_size();
        return insert(std::forward<Key>(key), std::forward<Val>(val));
      }

      // found a place! move hole to the front
      _alloc_key.construct(_keys + idx, std::move(_keys[i]));
      _alloc_key.destroy(_keys + i);
      // no need to set _hops[idx] & 1

      _alloc_val.construct(_values + idx, std::move(_values[i]));
      _alloc_val.destroy(_values + i);
      _hops[h] |= ((Traits::HopType)1 << (idx - h + 1));

      // clear hop bit
      _hops[h] ^= ((Traits::HopType)1 << (i - h + 1)); 

      idx = i;
    }

    // now that we've moved everything, we can finally construct the element at
    // it's rightful place.
    _alloc_val.construct(_values + idx, std::move(val));
    _alloc_key.construct(_keys + idx, std::move(key));
    _hops[idx] |= (Traits::HopType)1;

    _hops[initial_idx] |= ((Traits::HopType)1 << (idx - initial_idx + 1));
    ++_size;
    return true;
  }

  inline Val& find(const Key& key, bool& success) {
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

  inline const Val& find(const Key& key, bool& success) const {
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
    Key* old_keys = _keys;
    Val* old_values = _values;
    Traits::HopType* old_hops = _hops;

    size_t old_size = _max_size;
    init_data(_max_size * Traits::RESIZE_PERCENTAGE / 100);

    for (size_t i=0; i<old_size + Traits::HOP_SIZE; ++i) {
      if (old_hops[i] & 1) {
        insert(std::move(old_keys[i]), std::move(old_values[i]));
        _alloc_val.destroy(old_values + i);
        _alloc_key.destroy(old_keys + i);
      }
    }

    delete[] old_hops;
    _alloc_val.deallocate(old_values, old_size);
    _alloc_key.deallocate(old_keys, old_size);
  }


  void init_data(size_t new_size) {
    _size = 0;
    _max_size = new_size;
    _mask = _max_size - 1;
    _keys = _alloc_key.allocate(_max_size + Traits::HOP_SIZE);
    _values = _alloc_val.allocate(_max_size + Traits::HOP_SIZE, _keys);
    _hops = new Traits::HopType[_max_size + Traits::HOP_SIZE];
    for (size_t i=0; i<_max_size + Traits::HOP_SIZE; ++i) {
      _hops[i] = (Traits::HopType)0;
    }

    if (Traits::DEBUG) {
      size_t keys = sizeof(size_t) * (_max_size + Traits::HOP_SIZE);
      size_t hops = sizeof(Traits::HopType) * (_max_size + Traits::HOP_SIZE);
      size_t values = sizeof(Val) * (_max_size + Traits::HOP_SIZE);
      std::cout << (keys + hops + values) << " bytes (" << keys << " keys, " << hops << " hops, " << values << " values)" << std::endl;
    }


    //_max_fullness = (_max_size + HOP_SIZE) * 80/100;
    _max_fullness = _max_size + Traits::HOP_SIZE - 1;
  }

  Key* _keys;
  typename Traits::HopType* _hops;
  Val* _values;

  const H _hash;
  size_t _size;
  size_t _mask;
  size_t _max_size;
  size_t _max_fullness;

  AVal _alloc_val;
  AKey _alloc_key;
};
