/// HopScotch Algorithm from https://github.com/martinus/robin-hood-hashing/
///
/// @author martin.ankerl@gmail.com

#pragma once

#include <cstdint>
#include <algorithm>

namespace HopScotch {

namespace Style {

// Very aggressive data structure. Use only for small number of elements,
// e.g. 10.000 or so. Use with a good hash. Enable debugging to see
// the fullness of the hashmap.

struct Hop8 {
    typedef std::uint8_t HopType;
    enum resize_percentage { RESIZE_PERCENTAGE = 200 };
    enum hop_size { HOP_SIZE = 8 - 1 };
    enum add_range { ADD_RANGE = 496 };
    inline static std::size_t h(std::size_t v, std::size_t, std::size_t mask) {
        return v & mask;
    }
};


// Default setting, with 32 bit hop. This is usually a good choice
// that scales very well and is quite fast.
struct Hop16 {
    typedef std::uint16_t HopType;
    enum resize_percentage { RESIZE_PERCENTAGE = 200 };
    enum hop_size { HOP_SIZE = 16 - 1 };
    enum add_range { ADD_RANGE = 496 };
    inline static std::size_t h(std::size_t v, std::size_t, std::size_t mask) {
        return v & mask;
    }
};


// Very compact hash map. Can have fullness of 90% or more.
// Has a 64 bit hop.
struct Hop32 {
    typedef std::uint32_t HopType;
    enum resize_percentage { RESIZE_PERCENTAGE = 200 };
    enum hop_size { HOP_SIZE = 32 - 1 };
    enum add_range { ADD_RANGE = 496 };
    inline static std::size_t h(std::size_t v, std::size_t, std::size_t mask) {
        return v & mask;
    }
};


// Very compact hash map. Can have fullness of 90% or more.
// Has a 64 bit hop.
struct Hop64 {
    typedef std::uint64_t HopType;
    enum resize_percentage { RESIZE_PERCENTAGE = 200 };
    enum hop_size { HOP_SIZE = 64 - 1 };
    enum add_range { ADD_RANGE = 496 };
    inline static std::size_t h(std::size_t v, std::size_t, std::size_t mask) {
        return v & mask;
    }
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
    class Traits = Style::Hop16,
    bool Debug = false,
    class AVal = std::allocator<Val>,
    class AKey = std::allocator<Key>,
    class AHop = std::allocator<typename Traits::HopType>
>
class Map {
public:
    typedef Val value_type;
    typedef Map<Key, Val, H, Traits, Debug, AVal, AKey, AHop> Self;

    /// Creates an empty hash map.
    Map()
    : _hash() {
        init_data(32);
    }

    /// Clears all data, without resizing.
    void clear() {
        for (std::size_t i = 0; i < _max_size + Traits::HOP_SIZE; ++i) {
            if (_hops[i] & 1) {
                _alloc_val.destroy(_vals + i);
                _alloc_key.destroy(_keys + i);
            }
            _hops[i] = static_cast<typename Traits::HopType>(0);
        }
        _num_elements = 0;
    }

    /// Destroys the map and all it's contents.
    ~Map() {
		/*
        if (Debug) {
            std::cout << "dtor: " << _num_elements << " entries\t" << 1.0*_num_elements / (_max_size + Traits::HOP_SIZE) << std::endl;
        }
        */
        
        for (std::size_t i = 0; i < _max_size + Traits::HOP_SIZE; ++i) {
            if (_hops[i] & 1) {
                _alloc_val.destroy(_vals + i);
                _alloc_key.destroy(_keys + i);
                _alloc_hop.destroy(_hops + i);
            }
        }

        _alloc_val.deallocate(_vals, _max_size + Traits::HOP_SIZE);
        _alloc_key.deallocate(_keys, _max_size + Traits::HOP_SIZE);
        _alloc_hop.deallocate(_hops, _max_size + Traits::HOP_SIZE);
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
        std::size_t initial_idx = Traits::h(_hash(key), _max_size, _mask);

        // now, idx is the preferred position for this element. Search forward to find an empty place.
        // we use overflow area, so no modulo is required.
        auto idx = initial_idx;

        // find key & replace if found
        typename Traits::HopType hops = _hops[idx] >> 1;
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
        std::size_t e = initial_idx + Traits::ADD_RANGE;
        if (e > _max_size + Traits::HOP_SIZE) {
            e = _max_size + Traits::HOP_SIZE;
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
        _hops[idx] |= static_cast<typename Traits::HopType>(1);

        // TODO can this be made faster?
        // we have found an empty spot, but it might be far away. We have to move the hole to the front
        // until we are at the right step. idx is the empty spot.

        // This tries to find a swappable element that is as far away as possible. To do that, it tries to
        // find out if the element furthest away can be moved, by finding the hop for this element.
        while (idx > initial_idx + Traits::HOP_SIZE - 1) {
            // h: where the hash wants to be
            // i: where it actually is
            // idx: where it can be moved
            std::size_t start_h = idx < Traits::HOP_SIZE ? 0 : idx - Traits::HOP_SIZE + 1;
            std::size_t h;
            std::size_t i = start_h - 1;
            do {
                ++i;
                // find the hash place h for element i by looking through the hops
                h = start_h;
                typename Traits::HopType hop_mask = static_cast<typename Traits::HopType>(1) << (i - h + 1);
                while (h <= i && !(_hops[h] & hop_mask)) {
                    ++h;
                    hop_mask >>= 1;
                }
            } while (i < idx && h > i);

            // insertion failed? resize and try again.
            if (i >= idx) {
                // insertion failed, undo _hop[idx]
                _hops[idx] ^= static_cast<typename Traits::HopType>(1);
                increase_size();
                return insert(std::forward<Key>(key), std::forward<Val>(val));
            }

            // found a place! move hole to the front
            _alloc_key.construct(_keys + idx, std::move(_keys[i]));
            _alloc_key.destroy(_keys + i);
            // no need to set _hops[idx] & 1

            _alloc_val.construct(_vals + idx, std::move(_vals[i]));
            _alloc_val.destroy(_vals + i);
            _hops[h] |= (static_cast<typename Traits::HopType>(1) << (idx - h + 1));

            // clear hop bit
            _hops[h] ^= (static_cast<typename Traits::HopType>(1) << (i - h + 1));

            idx = i;
        }

        // now that we've moved everything, we can finally construct the element at
        // it's rightful place.
        _alloc_val.construct(_vals + idx, std::forward<Val>(val));
        _alloc_key.construct(_keys + idx, std::forward<Key>(key));
        _hops[idx] |= static_cast<typename Traits::HopType>(1);

        _hops[initial_idx] |= (static_cast<typename Traits::HopType>(1) << (idx - initial_idx + 1));
        ++_num_elements;
        return true;
    }

    inline Val* find(const Key& key) {
        return const_cast<Val*>(static_cast<const Self*>(this)->find(key));
    }

    inline const Val* find(const Key& key) const {
        auto idx = Traits::h(_hash(key), _max_size, _mask);

        typename Traits::HopType hops = _hops[idx] >> 1;

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
    inline std::size_t erase(const Key& key) {
        const auto original_idx = Traits::h(_hash(key), _max_size, _mask);
        auto idx = original_idx;

        typename Traits::HopType hops = _hops[idx] >> 1;
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

    inline std::size_t size() const {
        return _num_elements;
    }

    inline std::size_t max_size() const {
        return _max_size;
    }

private:
    // doubles size
    void increase_size() {
		/*
        if (Debug) {
            // calculate memory requirements
            std::cout << "resize: " << _max_size << "\t" << 1.0*_num_elements / (_max_size + Traits::HOP_SIZE) << std::endl;
        }
        */
        Key* old_keys = _keys;
        Val* old_vals = _vals;
        typename Traits::HopType* old_hops = _hops;

        std::size_t old_size = _max_size;

        std::size_t new_size = _max_size * Traits::RESIZE_PERCENTAGE / 100;
        if (new_size < old_size) {
            // overflow! just double the size.
            new_size = old_size * 2;
            if (new_size < old_size) {
                // another overflow! break.
                // TODO do something smart here
                throw std::bad_alloc();
            }
        }
        init_data(new_size);

        for (std::size_t i = 0; i < old_size + Traits::HOP_SIZE; ++i) {
            if (old_hops[i] & 1) {
                insert(std::move(old_keys[i]), std::move(old_vals[i]));
                _alloc_val.destroy(old_vals + i);
                _alloc_key.destroy(old_keys + i);
                _alloc_hop.destroy(old_hops + i);
            }
        }

        _alloc_val.deallocate(old_vals, old_size + Traits::HOP_SIZE);
        _alloc_key.deallocate(old_keys, old_size + Traits::HOP_SIZE);
        _alloc_hop.deallocate(old_hops, old_size + Traits::HOP_SIZE);
    }


    void init_data(std::size_t new_size) {
        _num_elements = 0;
        _max_size = new_size;
        _mask = _max_size - 1;

        _hops = _alloc_hop.allocate(_max_size + Traits::HOP_SIZE);
        _keys = _alloc_key.allocate(_max_size + Traits::HOP_SIZE, _hops);
        _vals = _alloc_val.allocate(_max_size + Traits::HOP_SIZE, _keys);

        for (std::size_t i = 0; i < _max_size + Traits::HOP_SIZE; ++i) {
            _alloc_hop.construct(_hops + i, 0);
        }

		/*
        if (Debug) {
            std::size_t keys = sizeof(std::size_t) * (_max_size + Traits::HOP_SIZE);
            std::size_t hops = sizeof(typename Traits::HopType) * (_max_size + Traits::HOP_SIZE);
            std::size_t values = sizeof(Val) * (_max_size + Traits::HOP_SIZE);
            std::cout << (keys + hops + values) << " bytes (" << keys << " keys, " << hops << " hops, " << values << " values)" << std::endl;
        }
        */
    }

    Val* _vals;
    Key* _keys;
    typename Traits::HopType* _hops;

    const H _hash;
    std::size_t _num_elements;
    std::size_t _mask;
    std::size_t _max_size;

    AVal _alloc_val;
    AKey _alloc_key;
    AHop _alloc_hop;
};

}
