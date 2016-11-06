/// Modified, highly optimized Robin Hood Hashtable.
/// Algorithm from https://github.com/martinus/robin-hood-hashing/
///
/// @author martin.ankerl@gmail.com

// Here are some tricks that I want to use:
//
// - todo - 
// * hopscotch: try to move as little as possible (not as much as possible, like its now. Search from right to left).
// http://bannalia.blogspot.co.at/2014/01/a-better-hash-table-clang.html
// https://probablydance.com/2014/05/03/i-wrote-a-fast-hash-table/
// * keep key, val, info interleaved?
// * steal from left and right? Maybe that leads to faster searches (less avalanche moves)
// * Remember minimum and maximum search distance so far? (minimum search can be used to speed up search.
// * keep median number of searches, and search outwards? maybe not useful.
// * instead of hashbits, store number of elements hashed to this index? Would that do any good?
// * adaptive hopscotch (with offsets). Max offset starts with e.g. 4. If we can't insert, increase max offset by 1 and try again.
//
// - done -
// * Keep a byte of distance around
// * highest bit of distance byte is 1 if bucket is taken, otherwise 0.
// * Rehash if distance byte overflows? Rehash when maximum search distance is too large? /  maximum search distance reaches a maximum limit
//
// - won't do - 
// * keep overflow area to the right (how big should that be?)


// say max dist should be 16 => 5 bits needed.
// max dist is 8: 4 bits needed (3210)
//  bit 3: true if bucket is taken.
//  bit 0-2: offset to original index

// use 1 byte as index: (76543210)
//  bit 7: true if bucket is taken.
//  bit 0-3: offset, value 0 to 16.
//  bit 4-6: what for? 8 values possible.

#pragma once

#include <cstdint>

// info bytes and a few hash bytes. Overflow check.
namespace RobinHoodInfobitsHashbits {

namespace Style {

struct Default {
    // bits: | 7 | 6   5   4   3 | 2   1   0 |
    //         ^ |    offset     | hash bits |
    //         |
    //       full?
    typedef std::uint8_t InfoType;
    static constexpr InfoType IS_BUCKET_TAKEN_MASK = 1<<7;
    static constexpr InfoType NUM_HASH_BITS = 3;
    static constexpr InfoType OFFSET_INC = 1 << NUM_HASH_BITS;
    static constexpr InfoType OFFSET_MASK = (IS_BUCKET_TAKEN_MASK - 1) ^ (OFFSET_INC - 1);
    static constexpr InfoType INITIAL_LEVEL = 5;
    static constexpr InfoType HASH_MASK = (1 << NUM_HASH_BITS) - 1;

    static constexpr InfoType MAX_OFFSET = IS_BUCKET_TAKEN_MASK >> NUM_HASH_BITS;

    // calculate hash bits based on h:
    // level 5:  32 elements: info |= (h >> level) & 7
    // level 1:  64 elements: info |= (h >> level) & 7
    // level 2: 128 elements: info |= (h >> level) & 7
    // level 3: 256 elements: info |= (h >> level) & 7
    // level 4: 512 elements: info |= (h >> level) & 7;

};

// Default setting, with 32 bit hop. This is usually a good choice
// that scales very well and is quite fast.
struct Large {
    // bits: | 15| 14  13  12  11  10 | 9   8   7   6   5   4   3   2   1   0|
    //       | ^ |   offset           |  additional hash bits                |
    //         |
    //       full? 
    typedef std::uint16_t InfoType;
    static constexpr InfoType IS_BUCKET_TAKEN_MASK = static_cast<InfoType>(1) << 15;
    static constexpr InfoType NUM_HASH_BITS = 11;
    static constexpr InfoType OFFSET_INC = 1 << NUM_HASH_BITS;
    static constexpr InfoType OFFSET_MASK = (IS_BUCKET_TAKEN_MASK - 1) ^ (OFFSET_INC - 1);
    static constexpr InfoType INITIAL_LEVEL = 5;
    static constexpr InfoType HASH_MASK = (1 << NUM_HASH_BITS) - 1;
    static constexpr InfoType MAX_OFFSET = IS_BUCKET_TAKEN_MASK >> NUM_HASH_BITS;

};

// Default setting, with 32 bit hop. This is usually a good choice
// that scales very well and is quite fast.
struct Big {
    // bits: | 15| 14  13  12  11  10 | 9   8   7   6   5   4   3   2   1   0|
    //       | ^ |   offset           |  additional hash bits                |
    //         |
    //       full? 
    typedef std::uint32_t InfoType;
    static constexpr InfoType IS_BUCKET_TAKEN_MASK = static_cast<InfoType>(1) << 31;
    static constexpr InfoType NUM_HASH_BITS = 27;
    static constexpr InfoType OFFSET_INC = 1 << NUM_HASH_BITS;
    static constexpr InfoType OFFSET_MASK = (IS_BUCKET_TAKEN_MASK - 1) ^ (OFFSET_INC - 1);
    static constexpr InfoType INITIAL_LEVEL = 5;
    static constexpr InfoType HASH_MASK = (1 << NUM_HASH_BITS) - 1;
    static constexpr InfoType MAX_OFFSET = IS_BUCKET_TAKEN_MASK >> NUM_HASH_BITS;
};


// Default setting, with 32 bit hop. This is usually a good choice
// that scales very well and is quite fast.
struct Huge {
    // bits: | 15| 14  13  12  11  10 | 9   8   7   6   5   4   3   2   1   0|
    //       | ^ |   offset           |  additional hash bits                |
    //         |
    //       full? 
    typedef std::uint64_t InfoType;
    static constexpr InfoType IS_BUCKET_TAKEN_MASK = static_cast<InfoType>(1) << 63;
    static constexpr InfoType NUM_HASH_BITS = 59;
    static constexpr InfoType OFFSET_INC = static_cast<InfoType>(1) << NUM_HASH_BITS;
    static constexpr InfoType OFFSET_MASK = (IS_BUCKET_TAKEN_MASK - 1) ^ (OFFSET_INC - 1);
    static constexpr InfoType INITIAL_LEVEL = 5;
    static constexpr InfoType HASH_MASK = (static_cast<InfoType>(1) << NUM_HASH_BITS) - 1;
    static constexpr InfoType MAX_OFFSET = IS_BUCKET_TAKEN_MASK >> NUM_HASH_BITS;
};
}

/// This is a hash map implementation based on Robin Hood Hashing.
/// It is somewhat inspired by Hopscotch, and uses a bitset
/// representation for each entry.
///
///
/// The goal is to create a hashmap that is 
/// * more memory efficient than std::unordered_map
/// * faster for insert, find, delete.
template<
    class Key,
    class Val,
    class H = std::hash<Key>,
    class Traits = Style::Default,
    bool Debug = false,
    class AVals = std::allocator<Val>,
    class AKeys = std::allocator<Key>,
    class AInfo = std::allocator<typename Traits::InfoType>
>
class Map {
public:
    typedef Val value_type;
    typedef Map<Key, Val, H, Traits, Debug, AVals, AKeys, AInfo> Self;

    /// Creates an empty hash map.
    Map()
        : _hash()
        , _level(Traits::INITIAL_LEVEL)
    {
        init_data();
    }

    /// Clears all data, without resizing.
    void clear() {
        // TODO
    }

    /// Destroys the map and all it's contents.
    ~Map() {
        for (size_t i = 0; i < _max_elements + Traits::MAX_OFFSET + 1; ++i) {
            if (_info[i] & Traits::IS_BUCKET_TAKEN_MASK) {
                _alloc_vals.destroy(_vals + i);
                _alloc_keys.destroy(_keys + i);
            }
        }

        _alloc_vals.deallocate(_vals, _max_elements + Traits::MAX_OFFSET + 1);
        _alloc_keys.deallocate(_keys, _max_elements + Traits::MAX_OFFSET + 1);
        _alloc_info.deallocate(_info, _max_elements + Traits::MAX_OFFSET + 1);
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
        if (_num_elements == _max_num_num_elements_allowed) {
            increase_size();
        }

        auto h = _hash(key);

        // create info field: IS_BUCKET_TAKEN_MASK, offset is 0, with hash info.
        typename Traits::InfoType info = Traits::IS_BUCKET_TAKEN_MASK | ((h >> _level_shift) & Traits::HASH_MASK);

        // calculate array position
        size_t idx = h & _mask;

        // search forward, until we find an entry "richer" than us
        // (closer to it's original position).

        // while we are richer than what's already there
        while (info < _info[idx]) {
            ++idx;
            //idx &= _mask;
            info += Traits::OFFSET_INC;
            if (!(info & Traits::IS_BUCKET_TAKEN_MASK)) {
                increase_size();
                return insert(std::forward<Key>(key), std::forward<Val>(val));
            }
        }

        // while we potentially have a match
        while (info == _info[idx]) {
            if (_keys[idx] == key) {
                // same! replace value, then bail out.
                _alloc_vals.destroy(_vals + idx);
                _alloc_vals.construct(_vals + idx, std::forward<Val>(val));
                return false;
            }
            ++idx;
            //idx &= _mask;
            info += Traits::OFFSET_INC;
            if (!(info & Traits::IS_BUCKET_TAKEN_MASK)) {
                increase_size();
                return insert(std::forward<Key>(key), std::forward<Val>(val));
            }
        }

        while (_info[idx] & Traits::IS_BUCKET_TAKEN_MASK) {
            // bucket taken! Steal it, then continue the loop.
            // we also steal it when offset is the same but hashbits are lower.
            if (info > _info[idx]) {
                // only swap when necessary
                std::swap(key, _keys[idx]);
                std::swap(val, _vals[idx]);
                std::swap(info, _info[idx]);
            }
            ++idx;
            //idx &= _mask;
            info += Traits::OFFSET_INC;
            if (!(info & Traits::IS_BUCKET_TAKEN_MASK)) {
                increase_size();
                return insert(std::forward<Key>(key), std::forward<Val>(val));
            }
        }

        // bucket is empty! Place it, then bail out.
        _alloc_keys.construct(_keys + idx, std::forward<Key>(key));
        _alloc_vals.construct(_vals + idx, std::forward<Val>(val));
        _info[idx] = info;
        ++_num_elements;
        return true;
    }

    Val* find(const Key& key) {
        return const_cast<Val*>(static_cast<const Self*>(this)->find(key));
    }

    const Val* find(const Key& key) const {
        auto h = _hash(key);

        // create info field: offset is bucket is taken, offset is 0, with hash info.
        typename Traits::InfoType info = Traits::IS_BUCKET_TAKEN_MASK | ((h >> _level_shift) & Traits::HASH_MASK);

        // calculate array position
        size_t idx = h & _mask;

        // find info field
        while (info < _info[idx]) {
            ++idx;
            info += Traits::OFFSET_INC;
        }

        // check while it seems we have the correct element
        while (info == _info[idx]) {
            if (key == _keys[idx]) {
                return _vals + idx;
            }
            ++idx;
            info += Traits::OFFSET_INC;
        }

        return nullptr;
    }

    inline size_t size() const {
        return _num_elements;
    }

    inline size_t max_size() const {
        return _max_elements;
    }

private:
    void init_data() {
        _num_elements = 0;
        _max_elements = static_cast<size_t>(1) << _level;
        _mask = _max_elements - 1;

        // max * (1 - 1/50) = max * 0.98
        // we allow a maximum fullness of 98%.
        _max_num_num_elements_allowed = _max_elements - std::max(static_cast<size_t>(1), _max_elements / 20);
     
        // e.g. for initial_level 5, rshift 3: 
        // level  5: (( 5 - 5) / 3) * 3 + 5 = 5
        // level  6: (( 6 - 5) / 3) * 3 + 5 = 5
        // level  7: (( 7 - 5) / 3) * 3 + 5 = 5
        // level  8: (( 8 - 5) / 3) * 3 + 5 = 8
        // level  9: (( 9 - 5) / 3) * 3 + 5 = 8
        // level 10: ((10 - 5) / 3) * 3 + 5 = 8
        // level 11: ((11 - 5) / 3) * 3 + 5 = 11
        // level 12: ((12 - 5) / 3) * 3 + 5 = 11
        // level 13: ((13 - 5) / 3) * 3 + 5 = 11
        _level_shift = ((_level - Traits::INITIAL_LEVEL) / Traits::NUM_HASH_BITS) * Traits::NUM_HASH_BITS + Traits::INITIAL_LEVEL;

        _info = _alloc_info.allocate(_max_elements + Traits::MAX_OFFSET + 1);
        _keys = _alloc_keys.allocate(_max_elements + Traits::MAX_OFFSET + 1, _info);
        _vals = _alloc_vals.allocate(_max_elements + Traits::MAX_OFFSET + 1, _keys);

        std::memset(_info, 0, sizeof(typename Traits::InfoType) * (_max_elements + Traits::MAX_OFFSET + 1));
    }

    void increase_size() {
        //std::cout << (100.0*_num_elements / _max_elements) << "% full, resizing" << std::endl;
        Key* old_keys = _keys;
        Val* old_vals = _vals;
        typename Traits::InfoType* old_info = _info;

        size_t old_max_elements = _max_elements;

        ++_level;
        init_data();

        int num_ins = 0;
        for (size_t i = 0; i < old_max_elements + Traits::MAX_OFFSET + 1; ++i) {
            if (old_info[i] & Traits::IS_BUCKET_TAKEN_MASK) {
                ++num_ins;
                // TODO reuse hash value! We already have it!
                insert(std::move(old_keys[i]), std::move(old_vals[i]));
                _alloc_vals.destroy(old_vals + i);
                _alloc_keys.destroy(old_keys + i);
            }
        }

        _alloc_vals.deallocate(old_vals, old_max_elements + Traits::MAX_OFFSET + 1);
        _alloc_keys.deallocate(old_keys, old_max_elements + Traits::MAX_OFFSET + 1);
        _alloc_info.deallocate(old_info, old_max_elements + Traits::MAX_OFFSET + 1);
    }

    Val* _vals;
    Key* _keys;
    typename Traits::InfoType* _info;

    const H _hash;

    // level 0: 32 elements (5 bits of hash)
    size_t _level;
    size_t _level_shift;
    size_t _num_elements;
    size_t _max_elements;
    size_t _mask;
    size_t _find_count;
    size_t _max_num_num_elements_allowed;

    AVals _alloc_vals;
    AKeys _alloc_keys;
    AInfo _alloc_info;
};

}
