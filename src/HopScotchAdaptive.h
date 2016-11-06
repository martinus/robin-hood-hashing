/// Adaptive HopScotch Algorithm
///
/// @author martin.ankerl@gmail.com

#pragma once

#include <cstdint>
#include <cstring>
#include <functional>

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
//  |81 00 00 00 00 00 00 81 00 00 00 00 00 00 00 00|
//  | 0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15|
//
// hop info is extracted like this:
//
// uint32_t hops = _hopmask & *reinterpret_cast<uint32_t*>(_hopbytes + idx * _hopwidth);
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

struct Fast {
    typedef uint32_t HopType;
    static constexpr size_t RESIZE_PERCENTAGE = 200;
    static constexpr size_t MIN_HOPSIZE = 8 - 1;
    static constexpr size_t MAX_HOPSIZE = 32 - 1;
    static constexpr size_t ADD_RANGE = 256;
};

struct Default {
    typedef uint64_t HopType;
    static constexpr size_t RESIZE_PERCENTAGE = 200;
    static constexpr size_t MIN_HOPSIZE = 8 - 1;
    static constexpr size_t MAX_HOPSIZE = 64 - 1;
    static constexpr size_t ADD_RANGE = 512;
};


struct Compact {
    typedef uint64_t HopType;
    static constexpr size_t RESIZE_PERCENTAGE = 200;
    static constexpr size_t MIN_HOPSIZE = 8 - 1;
    static constexpr size_t MAX_HOPSIZE = 64 - 1;
    static constexpr size_t ADD_RANGE = 2048;
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
    class E = std::equal_to<Key>,
    class Traits = Style::Default,
    bool Debug = false,
    class AVal = std::allocator<Val>,
    class AKey = std::allocator<Key>,
    class AHop = std::allocator<uint8_t>
>
class Map {
public:
    typedef Val value_type;
    typedef Map<Key, Val, H, E, Traits, Debug, AVal, AKey, AHop> Self;

    /// Creates an empty hash map.
    Map()
    : _hash()
    , _key_equal() {
        init_data(32);
    }

    inline typename Traits::HopType& hop(size_t idx) {
        return *reinterpret_cast<typename Traits::HopType*>(_hopbytes + idx * _hopwidth);
    }

    inline const typename Traits::HopType& hop(size_t idx) const {
        return *reinterpret_cast<typename Traits::HopType*>(_hopbytes + idx * _hopwidth);
    }

    /// Clears all data, without resizing.
    void clear() {
        for (size_t i = 0; i < _max_size + _hopsize; ++i) {
            if (hop(i) & _is_bucket_taken_mask) {
                _alloc_val.destroy(_vals + i);
                _alloc_key.destroy(_keys + i);
            }
        }
        std::memset(_hopbytes, 0, _hopwidth * (_max_size + Traits::MAX_HOPSIZE - 1) + sizeof(typename Traits::HopType));
        _num_elements = 0;
    }

    /// Destroys the map and all it's contents.
    ~Map() {
        /*
        if (Debug) {
            std::cout << "dtor: " << _num_elements << " entries\t" << 1.0*_num_elements / (_max_size + Traits::MAX_HOPSIZE) << std::endl;
        }
        */
        for (size_t i = 0; i < _max_size + _hopsize; ++i) {
            if (hop(i) & _is_bucket_taken_mask) {
                _alloc_val.destroy(_vals + i);
                _alloc_key.destroy(_keys + i);
            }
        }

        _alloc_val.deallocate(_vals, _max_size + Traits::MAX_HOPSIZE);
        _alloc_key.deallocate(_keys, _max_size + Traits::MAX_HOPSIZE);
        _alloc_hop.deallocate(_hopbytes, _hopwidth * (_max_size + Traits::MAX_HOPSIZE - 1) + sizeof(typename Traits::HopType));
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
        typename Traits::HopType hops = hop(idx) & _hopmask;
        while (hops) {
            if ((hops & 1) && _key_equal(key, _keys[idx])) {
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
        size_t end_idx = initial_idx + Traits::ADD_RANGE;
        if (end_idx > _max_size + _hopsize) {
            end_idx = _max_size + _hopsize;
        }
        while ((idx < end_idx) && (hop(idx) & _is_bucket_taken_mask)) {
            // TODO insert swapping code here!
            ++idx;
        }

        // no insert possible? resize and retry.
        if (idx == end_idx) {
            // we've looked far but couldn't find an empty spot => we have to resize.
            increase_size();
            return insert(std::forward<Key>(key), std::forward<Val>(val));
        }

        // we've found an empty spot at idx! set it as taken, then swap elements back (hopscotch).
        hop(idx) |= _is_bucket_taken_mask;

        // We have found an empty spot, but it might be far away. We have to move the hole to the front
        // until we are at the right step. idx is the empty spot.

        // This tries to find a swappable element that is as far away as possible. To do that, it tries to
        // find out if the element furthest away can be moved, by finding the hop for this element.

        // loop until empty idx is close enough to place the element thats currently in key and val.
        while (idx > initial_idx + _hopsize - 1) {
            // position of hop info that's farthest away to potentially contain an element that can
            // be swapped to idx
            const size_t earliest_possible_hop_idx = idx < _hopsize ? 0 : idx - _hopsize + 1;

            // actual index for hop information that it is checked if it contains the element positioned
            // at swappable_candidate_idx
            size_t swappable_candidate_hopidx;

            // position of the bucket that might be swappable to idx
            size_t swappable_candidate_idx = earliest_possible_hop_idx - 1;
            do {
                ++swappable_candidate_idx;

                // find the hash place h for element i by looking through the hops
                swappable_candidate_hopidx = earliest_possible_hop_idx;
                typename Traits::HopType hop_mask = static_cast<typename Traits::HopType>(1) << (swappable_candidate_idx - earliest_possible_hop_idx);

                // check all the hopfields starting from earliest_possible_hop_idx, up to the swappable_candidate_idx if
                // one of them is the owner of swappable_candidate_idx
                while (swappable_candidate_hopidx <= swappable_candidate_idx && !(hop(swappable_candidate_hopidx) & hop_mask)) {
                    ++swappable_candidate_hopidx;
                    hop_mask >>= 1;
                }
            } while (swappable_candidate_idx < idx && swappable_candidate_hopidx > swappable_candidate_idx);

            // have we found an element with corresponding hopidx that's within the reach of idx?
            if (swappable_candidate_idx == idx) {
                // try to increase hopsize, maybe we can continue hopping with this.
                if (!increase_hopsize()) {
                    // could not increase hopsize :( Our only option left is to enlarge the hashmap.
                    // To do this, clean up current hashmap state by unsetting current idx' taken information,
                    // then increase size and try to insert again.
                    hop(idx) ^= _is_bucket_taken_mask;
                    increase_size();
                    return insert(std::forward<Key>(key), std::forward<Val>(val));
                }
            } else {
                // found a place! move hole to the front
                _alloc_key.construct(_keys + idx, std::move(_keys[swappable_candidate_idx]));
                _alloc_key.destroy(_keys + swappable_candidate_idx);
                _alloc_val.construct(_vals + idx, std::move(_vals[swappable_candidate_idx]));
                _alloc_val.destroy(_vals + swappable_candidate_idx);

                hop(swappable_candidate_hopidx) |= (static_cast<typename Traits::HopType>(1) << (idx - swappable_candidate_hopidx));
                hop(swappable_candidate_hopidx) ^= (static_cast<typename Traits::HopType>(1) << (swappable_candidate_idx - swappable_candidate_hopidx));

                idx = swappable_candidate_idx;
            }
        }

        // now that we've moved everything, we can finally construct the element at
        // it's rightful place.
        _alloc_val.construct(_vals + idx, std::forward<Val>(val));
        _alloc_key.construct(_keys + idx, std::forward<Key>(key));

        hop(idx) |= _is_bucket_taken_mask;
        hop(initial_idx) |= (static_cast<typename Traits::HopType>(1) << (idx - initial_idx));
        ++_num_elements;
        return true;
    }


    inline bool insert_v2(Key&& key, Val&& val) {
        const size_t initial_idx = _hash(key) & _mask;

        // now, idx is the preferred position for this element. Search forward to find an empty place.
        // we use overflow area, so no modulo is required.
        size_t idx = initial_idx;

        // find key & replace if found
        typename Traits::HopType hops = hop(idx) & _hopmask;
        while (hops) {
            if ((hops & 1) && _key_equal(key, _keys[idx])) {
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
        while ((idx < e) && (hop(idx) & _is_bucket_taken_mask)) {
            ++idx;
        }
        // no insert possible? resize and retry.
        if (idx == e) {
            // retry insert
            increase_size();
            return insert(std::forward<Key>(key), std::forward<Val>(val));
        }

        //++_counts[idx - initial_idx];

        // set the empty spot's hop bit. Now all taken bits are set like when insert would be successful.
        hop(idx) |= _is_bucket_taken_mask;

        // TODO can this be made faster?
        // we have found an empty spot, but it might be far away. We have to move the hole to the front
        // until we are at the right step. idx is the empty spot.

        // This tries to find a swappable element that is as far away as possible. To do that, it tries to
        // find out if the element furthest away can be moved, by finding the hop for this element.
        size_t h = idx < _hopsize ? 0 : idx - _hopsize + 1;
        while (idx > initial_idx + _hopsize - 1) {
            size_t initial_h = h;
            hops = hop(initial_h) & _hopmask;
            while (hops && h < idx) {
                if (hops & 1) {
                    // found something to move forward!
                    _alloc_key.construct(_keys + idx, std::move(_keys[h]));
                    _alloc_val.construct(_vals + idx, std::move(_vals[h]));
                    _alloc_key.destroy(_keys + h);
                    _alloc_val.destroy(_vals + h);

                    // update hop bit
                    hop(h) |= (static_cast<typename Traits::HopType>(1) << (idx - h));
                    // clear old hop bit
                    hop(h) ^= (static_cast<typename Traits::HopType>(1) << (h - initial_h));

                    hops = 0;
                    idx = h;
                    h = idx < _hopsize ? 0 : idx - _hopsize + 1;
                } else {
                    hops >>= 1;
                    ++h;
                }
            }
            if (h == idx) {
                // damn! insertion has failed.
                if (!increase_hopsize()) {
                    increase_size();
                }
                return insert(std::forward<Key>(key), std::forward<Val>(val));
            }
            ++h;
        }

        // now that we've moved everything, we can finally construct the element at
        // it's rightful place.
        _alloc_val.construct(_vals + idx, std::forward<Val>(val));
        _alloc_key.construct(_keys + idx, std::forward<Key>(key));

        hop(idx) |= _is_bucket_taken_mask;
        hop(initial_idx) |= (static_cast<typename Traits::HopType>(1) << (idx - initial_idx));
        ++_num_elements;
        return true;
    }

    inline Val* find(const Key& key) {
        return const_cast<Val*>(static_cast<const Self*>(this)->find(key));
    }

    inline const Val* find(const Key& key) const {
        auto idx = _hash(key) & _mask;

        typename Traits::HopType hops = hop(idx) & _hopmask;

        while (hops) {
            if ((hops & 1) && _key_equal(key, _keys[idx])) {
                return _vals + idx;
            }
            hops >>= 1;
            ++idx;
        }

        return nullptr;
    }

    /// Returns number of erased elements, 0 or 1.
    inline size_t erase(const Key& key) {
        const auto original_idx = _hash(key) & _mask;
        auto idx = original_idx;

        typename Traits::HopType hops = hop(idx) & _hopmask;
        while (hops) {
            if ((hops & 1) && _key_equal(key, _keys[idx])) {
                hop(original_idx) ^= 1 << (idx - original_idx);
                hop(idx) ^= 1;
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
    bool increase_hopsize() {
        if (_hopsize == Traits::MAX_HOPSIZE) {
            // can't increase hopsize any more
            return false;
        }

        const auto old_hopsize = _hopsize;
        const auto old_hopmask = _hopmask;
        const auto old_is_bucket_taken_mask = _is_bucket_taken_mask;
        const auto old_hopwidth = _hopwidth;
        const auto old_hopbytes = _hopbytes;

        // +1
        //_hopwidth *= 2;
        ++_hopwidth;

        /*
        if (Debug) {
            // calculate memory requirements
            std::cout << "hop resize to " << _hopwidth << ": " << _max_size << "\t" << 1.0*_num_elements / (_max_size + Traits::MAX_HOPSIZE) << std::endl;
        }
        */

        _hopsize = _hopwidth * 8 - 1;
        _is_bucket_taken_mask = static_cast<typename Traits::HopType>(1) << _hopsize;
        _hopmask = _is_bucket_taken_mask - 1;


        _hopbytes = _alloc_hop.allocate(_hopwidth * (_max_size + Traits::MAX_HOPSIZE - 1) + sizeof(typename Traits::HopType));
        std::memset(_hopbytes, 0, _hopwidth * (_max_size + Traits::MAX_HOPSIZE - 1) + sizeof(typename Traits::HopType));

        for (size_t i = 0; i < _max_size + old_hopsize; ++i) {
            const auto old_hop = *reinterpret_cast<typename Traits::HopType*>(old_hopbytes + i * old_hopwidth);
            auto& new_hop = *reinterpret_cast<typename Traits::HopType*>(_hopbytes + i * _hopwidth);
            if (old_hop & old_is_bucket_taken_mask) {
                new_hop |= _is_bucket_taken_mask;
            }
            new_hop |= (old_hop & old_hopmask);
        }

        _alloc_hop.deallocate(old_hopbytes, old_hopwidth * (_max_size + Traits::MAX_HOPSIZE - 1) + sizeof(typename Traits::HopType));

        // copy over old hopbytes
        return true;
    }

    // doubles size
    void increase_size() {
        /*
        if (Debug) {
            // calculate memory requirements
            std::cout << "resize: " << _max_size << "\t" << 1.0*_num_elements / (_max_size + Traits::MAX_HOPSIZE) << std::endl;
        }
        */
        Key* old_keys = _keys;
        Val* old_vals = _vals;
        auto* old_hopbytes = _hopbytes;
        auto old_hopwidth = _hopwidth;

        size_t old_size = _max_size;

        size_t new_size = _max_size * Traits::RESIZE_PERCENTAGE / 100;
        if (new_size < old_size) {
            // overflow! just double the size.
            new_size = old_size * 2;
            if (new_size < old_size) {
                // another overflow! break.
                // TODO do something smart here
                throw std::bad_alloc();
            }
        }
        const auto old_hopsize = _hopsize;
        const auto old_is_bucket_taken_mask = _is_bucket_taken_mask;
        init_data(new_size);

        for (size_t i = 0; i < old_size + old_hopsize; ++i) {
            // can't use hop(i) because _hopbytes is overwritten!            
            if (*reinterpret_cast<typename Traits::HopType*>(old_hopbytes + i * old_hopwidth) & old_is_bucket_taken_mask) {
                insert(std::move(old_keys[i]), std::move(old_vals[i]));
                _alloc_val.destroy(old_vals + i);
                _alloc_key.destroy(old_keys + i);
            }
        }

        _alloc_val.deallocate(old_vals, old_size + Traits::MAX_HOPSIZE);
        _alloc_key.deallocate(old_keys, old_size + Traits::MAX_HOPSIZE);
        _alloc_hop.deallocate(old_hopbytes, old_hopwidth * (old_size + Traits::MAX_HOPSIZE - 1) + sizeof(typename Traits::HopType));
    }


    void init_data(size_t new_size) {
        _num_elements = 0;
        _max_size = new_size;
        _mask = _max_size - 1;

        _hopsize = Traits::MIN_HOPSIZE;
        _hopmask = 0x7F;
        _is_bucket_taken_mask = 0x80;
        _hopwidth = 1;

        _hopbytes = _alloc_hop.allocate(_hopwidth * (_max_size + Traits::MAX_HOPSIZE - 1) + sizeof(typename Traits::HopType));
        _keys = _alloc_key.allocate(_max_size + Traits::MAX_HOPSIZE, _hopbytes);
        _vals = _alloc_val.allocate(_max_size + Traits::MAX_HOPSIZE, _keys);

        std::memset(_hopbytes, 0, _hopwidth * (_max_size + Traits::MAX_HOPSIZE - 1) + sizeof(typename Traits::HopType));

        /*
        if (Debug) {
            size_t keys = sizeof(size_t) * (_max_size + Traits::MAX_HOPSIZE);
            size_t hops = sizeof(typename Traits::HopType) * (_max_size + Traits::MAX_HOPSIZE);
            size_t values = sizeof(Val) * (_max_size + Traits::MAX_HOPSIZE);
            std::cout << (keys + hops + values) << " bytes (" << keys << " keys, " << hops << " hops, " << values << " values)" << std::endl;
        }
        */
    }

    Val* _vals;
    Key* _keys;
    uint8_t* _hopbytes;

    typename Traits::HopType _is_bucket_taken_mask;
    typename Traits::HopType _hopmask;
    size_t _hopwidth;

    const H _hash;
    const E _key_equal;
    size_t _num_elements;
    size_t _mask;
    size_t _max_size;
    size_t _hopsize;

    AVal _alloc_val;
    AKey _alloc_key;
    AHop _alloc_hop;
};

}
