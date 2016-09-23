/// Adaptive HopScotch Algorithm
///
/// @author martin.ankerl@gmail.com

#pragma once

#include <cstdint>

// This is an adaptive hopscotch hash map. It automatically adapts the hop size based on the map's fullness.
// Here is the idea:
// Initially, it always starts with 1 byte hopinfo. Insertion and search will be very fast.
// When insertion was not possible any more and fullness is below a threshold (say 70% full), 
// hopinfo will be rebuild with 2byte info. This rebuilding will be quite fast.
// When insertion is again not possible any more, switch to 3 byte, then 4 byte (the maximum).
//
// To efficiently work that way with the hop information, I'm doing this:
//
// Say we currently use 1 byte hop info, I am using this bit representation:
//
//   |7 6 5 4 3 2 1 0|
//   | |--hop bits---|
//   |-|
//  full?
//
// so I am using the *highest* bit to indicate if the bucket is taken, and the lower 7 bits 
// for the hop bits. Bit 0 means offset 0, bit 1 offset 1, etc.
//
// The byte representation of all hop bits looks like this, for 16 bytes, and 1 element at that
// was hashed to idx 0, and 1 element that was hashed to idx 7:
// (81hex is 1000.0001 binary):
//   
//  |00 00 00 81 00 00 00 81 00 00 00 00 00 00 00 00|
//  | 0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15|
//
// hop info is extracted like this:
//
// std::uint32_t hops = _hopmask & *reinterpret_cast<std::uint32_t*>(_hopbytes + idx * _hopwidth);
// 
//        | _hopwidth | _hopoffset |  _hopmask  | _hopsize |
// -------+-----------+------------+------------+----------+
// 1 byte |         1 |          3 |       0x7F |        7 |
// 2 byte |         2 |          2 |     0x7FFF |       15 |
// 3 byte |         3 |          1 |   0x7FFFFF |       23 |
// 4 byte |         4 |          0 | 0x7FFFFFFF |       31 |
//
//
// allocation:
//
//   size_t num_hops = _max_size + Traits::HOP_SIZE;
//   // alloc_hop is a byte alloator
//   _hopbytes = _alloc_hop.allocate(4 + _hopwidth * (num_hops - 1));
//
// upgrading to 1 additional byte:
//
// * adds a 0 byte in between
// * ++_hopwidth.
// * _hopmask = (_hopmask << 8) | 0xFF;
// * _hopsize += 8;
//
// Maybe double hopwidth (1, 2, 4)?
namespace HopScotchAdaptive {

namespace Style {

// Very aggressive data structure. Use only for small number of elements,
// e.g. 10.000 or so. Use with a good hash. Enable debugging to see
// the fullness of the hashmap.

struct Default {
    typedef std::uint8_t HopType;
    static constexpr size_t RESIZE_PERCENTAGE = 200;
    static constexpr size_t MIN_HOPSIZE = 8 - 1;
    static constexpr size_t MAX_HOPSIZE = 32 - 1;
    static constexpr size_t ADD_RANGE = 496;
};

}

/// This is a hash map implementation, based on the hopscotch algorithm. For details see
/// http://mcg.cs.tau.ac.il/papers/disc2008-hopscotch.pdf
/// https://github.com/harieshsathya/Hopscotch-Hashing/blob/master/hopscotch.cpp
///
/// In many cases, the hashmap should be faster than std::unordered_map.
/// It is flexible with the use of different styles (fast, to compact).
///
/// When insertion fails, the hashmap resizes automatically.
template<
    class Key,
    class Val,
    class H = std::hash<Key>,
    class Traits = Style::Default,
    bool Debug = false,
    class AVal = std::allocator<Val>,
    class AKey = std::allocator<Key>,
    class AHop = std::allocator<Traits::HopType>
>
class Map {
public:
    typedef Val value_type;
    typedef Map<Key, Val, H, Traits, Debug, AVal, AKey, AHop> Self;

    /// Creates an empty hash map.
    Map() {
        init_data(32);
    }

    /// Clears all data, without resizing.
    void clear() {
        for (size_t i = 0; i < _max_size + _hopsize; ++i) {
            if (_hops[i] & 1) {
                _alloc_val.destroy(_vals + i);
                _alloc_key.destroy(_keys + i);
            }
            _hops[i] = (Traits::HopType)0;
        }
        _num_elements = 0;
    }

    /// Destroys the map and all it's contents.
    ~Map() {
        if (Debug) {
            std::cout << "dtor: " << _num_elements << " entries\t" << 1.0*_num_elements / (_max_size + Traits::MAX_HOPSIZE) << std::endl;
        }
        for (size_t i = 0; i < _max_size + _hopsize; ++i) {
            if (_hops[i] & 1) {
                _alloc_val.destroy(_vals + i);
                _alloc_key.destroy(_keys + i);
                _alloc_hop.destroy(_hops + i);
            }
        }

        _alloc_val.deallocate(_vals, _max_size + Traits::MAX_HOPSIZE);
        _alloc_key.deallocate(_keys, _max_size + Traits::MAX_HOPSIZE);
        _alloc_hop.deallocate(_hops, _max_size + Traits::MAX_HOPSIZE);
    }

    inline bool insert(const Key& key, Val&& val) {
        Key k(key);
        return insert(std::move(k), std::forward<Val>(val));
    }

    inline bool insert(Key&& key, const Val& val) {
        Val v(val);
        return insert(std::forward<Key>(key), std::move(v));
    }

    inline bool insert(const Key& key, const Val& val) {
        Key k(key);
        Val v(val);
        return insert(std::move(k), std::move(v));
    }

    inline bool insert(Key&& key, Val&& val) {
        const size_t initial_idx = _hash(key) & _mask;

        // now, idx is the preferred position for this element. Search forward to find an empty place.
        // we use overflow area, so no modulo is required.
        size_t idx = initial_idx;

        // find key & replace if found
        Traits::HopType hops = _hops[idx] >> 1;
        while (hops) {
            if ((hops & 1) && (_keys[idx] == key)) {
                // found the key! replace value
                _alloc_val.destroy(_vals + idx);
                _alloc_val.construct(_vals + idx, std::forward<Val>(val));
                return false;
            }
            ++idx;
            hops >>= 1;
        }

        // key is not there, find an empty spot
        idx = initial_idx;
        size_t e = initial_idx + Traits::ADD_RANGE;
        if (e > _max_size + _hopsize) {
            e = _max_size + _hopsize;
        }
        while ((idx < e) && (_hops[idx] & 1)) {
            ++idx;
        }

        // no insert possible? resize and retry.
        if (idx == e) {
            // retry insert
            increase_size();
            return insert(std::forward<Key>(key), std::forward<Val>(val));
        }

        //++_counts[idx - initial_idx];

        // set the empty spot's hop bit
        _hops[idx] |= (Traits::HopType)1;

        // TODO can this be made faster?
        // we have found an empty spot, but it might be far away. We have to move the hole to the front
        // until we are at the right step. idx is the empty spot.

        // This tries to find a swappable element that is as far away as possible. To do that, it tries to
        // find out if the element furthest away can be moved, by finding the hop for this element.
        while (idx > initial_idx + _hopsize - 1) {
            // h: where the hash wants to be
            // i: where it actually is
            // idx: where it can be moved
            size_t start_h = idx < _hopsize ? 0 : idx - _hopsize + 1;
            size_t h;
            size_t i = start_h - 1;
            do {
                ++i;
                // find the hash place h for element i by looking through the hops
                h = start_h;
                Traits::HopType hop_mask = (Traits::HopType)1 << (i - h + 1);
                while (h <= i && !(_hops[h] & hop_mask)) {
                    ++h;
                    hop_mask >>= 1;
                }
            } while (i < idx && h > i);

            // insertion failed? resize and try again.
            if (i >= idx) {
                // insertion failed, undo _hop[idx]
                _hops[idx] ^= (Traits::HopType)1;

                // retry with larger hopsize
                /*
                if (_hopsize < Traits::MAX_HOPSIZE) {
                    _hopsize += 8;
                } else {
                    increase_size();
                }
                */
                increase_size();
                return insert(std::forward<Key>(key), std::forward<Val>(val));
            }

            // found a place! move hole to the front
            _alloc_key.construct(_keys + idx, std::move(_keys[i]));
            _alloc_key.destroy(_keys + i);
            // no need to set _hops[idx] & 1

            _alloc_val.construct(_vals + idx, std::move(_vals[i]));
            _alloc_val.destroy(_vals + i);
            _hops[h] |= ((Traits::HopType)1 << (idx - h + 1));

            // clear hop bit
            _hops[h] ^= ((Traits::HopType)1 << (i - h + 1));

            idx = i;
        }

        // now that we've moved everything, we can finally construct the element at
        // it's rightful place.
        _alloc_val.construct(_vals + idx, std::forward<Val>(val));
        _alloc_key.construct(_keys + idx, std::forward<Key>(key));
        _hops[idx] |= (Traits::HopType)1;

        _hops[initial_idx] |= ((Traits::HopType)1 << (idx - initial_idx + 1));
        ++_num_elements;
        return true;
    }

    inline Val* find(const Key& key) {
        return const_cast<Val*>(static_cast<const Self*>(this)->find(key));
    }

    inline const Val* find(const Key& key) const {
        auto idx = _hash(key) & _mask;

        Traits::HopType hops = _hops[idx] >> 1;

        while (hops) {
            if ((hops & 1) && (key == _keys[idx])) {
                return _vals + idx;
            }
            hops >>= 1;
            ++idx;
        }

        return nullptr;
    }

    /// Returns number of erased elements, 0 or 1.
    inline size_t erase(const Key& key) {
        const auto original_idx = Traits::h(_hash(key), _max_size, _mask);
        auto idx = original_idx;

        Traits::HopType hops = _hops[idx] >> 1;
        while (hops) {
            if ((hops & 1) && (key == _keys[idx])) {
                _hops[original_idx] ^= 1 << (idx - original_idx + 1);
                _hops[idx] ^= 1;
                --_num_elements;
                return 1;
            }
            hops >>= 1;
            ++idx;
        }

        return 0;
    }

    inline size_t size() const {
        return _num_elements;
    }

    inline size_t max_size() const {
        return _max_size;
    }

private:
    // doubles size
    void increase_size() {
        if (Debug) {
            // calculate memory requirements
            std::cout << "resize: " << _max_size << "\t" << 1.0*_num_elements / (_max_size + Traits::MAX_HOPSIZE) << std::endl;
        }
        Key* old_keys = _keys;
        Val* old_vals = _vals;
        Traits::HopType* old_hops = _hops;

        size_t old_size = _max_size;

        size_t new_size = _max_size * Traits::RESIZE_PERCENTAGE / 100;
        if (new_size < old_size) {
            // overflow! just double the size.
            new_size = old_size * 2;
            if (new_size < old_size) {
                // another overflow! break.
                // TODO do something smart here
                throw std::exception("can't resize");
            }
        }
        auto old_hopsize = _hopsize;
        init_data(new_size);

        for (size_t i = 0; i < old_size + old_hopsize; ++i) {
            if (old_hops[i] & 1) {
                insert(std::move(old_keys[i]), std::move(old_vals[i]));
                _alloc_val.destroy(old_vals + i);
                _alloc_key.destroy(old_keys + i);
                _alloc_hop.destroy(old_hops + i);
            }
        }

        _alloc_val.deallocate(old_vals, old_size + Traits::MAX_HOPSIZE);
        _alloc_key.deallocate(old_keys, old_size + Traits::MAX_HOPSIZE);
        _alloc_hop.deallocate(old_hops, old_size + Traits::MAX_HOPSIZE);
    }


    void init_data(size_t new_size) {
        _num_elements = 0;
        _max_size = new_size;
        _mask = _max_size - 1;
        _hopsize = Traits::MIN_HOPSIZE;

        _hops = _alloc_hop.allocate(_max_size + Traits::MAX_HOPSIZE);
        _keys = _alloc_key.allocate(_max_size + Traits::MAX_HOPSIZE, _hops);
        _vals = _alloc_val.allocate(_max_size + Traits::MAX_HOPSIZE, _keys);

        for (size_t i = 0; i < _max_size + Traits::MAX_HOPSIZE; ++i) {
            _alloc_hop.construct(_hops + i, 0);
        }

        if (Debug) {
            size_t keys = sizeof(size_t) * (_max_size + Traits::MAX_HOPSIZE);
            size_t hops = sizeof(Traits::HopType) * (_max_size + Traits::MAX_HOPSIZE);
            size_t values = sizeof(Val) * (_max_size + Traits::MAX_HOPSIZE);
            std::cout << (keys + hops + values) << " bytes (" << keys << " keys, " << hops << " hops, " << values << " values)" << std::endl;
        }
    }

    Val* _vals;
    Key* _keys;
    typename Traits::HopType* _hops;

    const H _hash;
    size_t _num_elements;
    size_t _mask;
    size_t _max_size;
    size_t _hopsize;

    AVal _alloc_val;
    AKey _alloc_key;
    AHop _alloc_hop;
};

}
