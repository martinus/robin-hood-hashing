/// Modified, highly optimized Robin Hood Hashtable.
/// Algorithm from https://github.com/martinus/robin-hood-hashing/
///
/// @author martin.ankerl@gmail.com

// Here are some tricks that I want to use:
//
// * Keep a byte of distance around
// * highest bit of distance byte is 1 if bucket is taken, otherwise 0.
// * Remember minimum and maximum search distance so far? (minimum search can be used to speed up search.
// * keep median number of searches, and search outwards? maybe not useful.
// * Rehash if distance byte overflows? Rehash when maximum search distance is too large? /  maximum search distance reaches a maximum limit
// * keep overflow area to the right (how big should that be?

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

namespace RobinHood {

namespace Style {

struct Default {
    // bits: | 7 | 6   5   4   3 | 2   1   0 |
    //         ^ |    offset     | hash bits |
    //         |
    //       full?
    typedef std::uint8_t InfoType;
    enum is_bucket_taken_mask { IS_BUCKET_TAKEN_MASK = 128 };
    enum offset_mask { OFFSET_MASK = 120 };
    enum offset_shift { OFFSET_RSHIFT = 3 };
    enum overflow_area_size { OVERFLOW_AREA_SIZE = 16 + 1 };
};

// Default setting, with 32 bit hop. This is usually a good choice
// that scales very well and is quite fast.
struct Large {
    // bits: | 15| 14  13  12  11  10 | 9   8   7   6   5   4   3   2   1   0|
    //       | ^ |   offset           |  additional hash bits                |
    //         |
    //       full? 
    typedef std::uint16_t InfoType;
    enum is_bucket_taken_mask { IS_BUCKET_TAKEN_MASK = 32768 };
    enum offset_mask { OFFSET_MASK = 31744 };
    enum offset_shift { OFFSET_RSHIFT = 10 };
    enum overflow_area_size { OVERFLOW_AREA_SIZE = 32 + 1 };
};
}

/// This is a hash map implementation based on Robin Hood Hashing.
/// It is somewhat inspired by Hopscotch, and uses a (to the best of my
/// knowledge) novel bitset representation for each entry.
///
/// http://mcg.cs.tau.ac.il/papers/disc2008-hopscotch.pdf
/// https://github.com/harieshsathya/Hopscotch-Hashing/blob/master/hopscotch.cpp
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
    class AInfo = std::allocator<Traits::InfoType>
>
class Map {
public:
    typedef Val value_type;

    /// Creates an empty hash map.
    Map()
        : _level(0)
    {
        init_data(32);
    }

    /// Clears all data, without resizing.
    void clear() {
        // TODO
    }

    /// Destroys the map and all it's contents.
    ~Map() {
        // TODO
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
    inline bool insert_impl(Key&& key, Val&& val) {
        // TODO
        return true;
    }

    inline Val* find(const Key& key) {
        auto h = _hash(key);

        /// finds 
        // TODO
        return nullptr;
    }

    inline const Val* find(const Key& key) const {
        // TODO
        return nullptr;
    }

    inline size_t size() const {
        // TODO
        return 0;
    }

    inline size_t max_size() const {
        // TODO
        return 0;
    }

private:
    void init_data(size_t new_size) {
        _num_elements = 0;
        _max_elements = new_size;

        _info = _alloc_info.allocate(_max_elements + Traits::OVERFLOW_AREA_SIZE);
        _keys = _alloc_keys.allocate(_max_elements + Traits::OVERFLOW_AREA_SIZE, _info);
        _vals = _alloc_vals.allocate(_max_elements + Traits::OVERFLOW_AREA_SIZE, _keys);

        std::memset(_info, 0, sizeof(Traits::InfoType) * (_max_elements + Traits::OVERFLOW_AREA_SIZE));
    }

    Val* _vals;
    Key* _keys;
    typename Traits::InfoType* _info;

    const H _hash;
    size_t _level;
    size_t _num_elements;
    size_t _max_elements;

    AVals _alloc_vals;
    AKeys _alloc_keys;
    AInfo _alloc_info;
};

}
