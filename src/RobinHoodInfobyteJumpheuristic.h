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
#include <cstring>
#include <functional>

// just with info byte
namespace RobinHoodInfobyteJumpheuristic {

namespace Style {

struct Default {
    // bits: | 7 | 6   5   4   3   2   1   0|
    //       | ^ |         offset           |
    //         |
    //       full? 
    typedef std::uint8_t InfoType;
    static constexpr InfoType IS_BUCKET_TAKEN_MASK = 1 << 7;
    static constexpr InfoType OFFSET_MASK = IS_BUCKET_TAKEN_MASK - 1;
    static constexpr size_t OVERFLOW_SIZE = 32;
    static constexpr size_t INITIAL_LEVEL = 5;
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
    class E = std::equal_to<Key>,
    class Traits = Style::Default,
    bool Debug = false,
    class AVals = std::allocator<Val>,
    class AKeys = std::allocator<Key>,
    class AInfo = std::allocator<typename Traits::InfoType>
>
class Map {
public:
    typedef Val value_type;
    typedef Map<Key, Val, H, E, Traits, Debug, AVals, AKeys, AInfo> Self;

    /// Creates an empty hash map.
    Map()
    : _hash()
    , _key_equal() {
        init_data(Traits::INITIAL_LEVEL);
    }

    /// Clears all data, without resizing.
    void clear() {
        // TODO
    }

    /// Destroys the map and all it's contents.
    ~Map() {
        for (size_t i = 0; i < _max_elements + Traits::OVERFLOW_SIZE; ++i) {
            if (_info[i] & Traits::IS_BUCKET_TAKEN_MASK) {
                _alloc_vals.destroy(_vals + i);
                _alloc_keys.destroy(_keys + i);
            }
        }

        _alloc_vals.deallocate(_vals, _max_elements + Traits::OVERFLOW_SIZE);
        _alloc_keys.deallocate(_keys, _max_elements + Traits::OVERFLOW_SIZE);
        _alloc_info.deallocate(_info, _max_elements + Traits::OVERFLOW_SIZE);
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

    /*
    golden_ratio = (1.0 + Math::sqrt(5)) / 2

    h = 0
    10.times do
    h += golden_ratio
    h %= 1
    puts "0x#{(2**32 * h).to_i.to_s(16)}"
    end
    */
    inline void update_jump_heuristic_offset() {
        // take a few samples, using golden ratio 0.618033988749895
        size_t sum = 0;
        sum += _info[0x9e3779b9 & _mask] & Traits::OFFSET_MASK;
        sum += _info[0x3c6ef372 & _mask] & Traits::OFFSET_MASK;
        sum += _info[0xdaa66d2c & _mask] & Traits::OFFSET_MASK;
        sum += _info[0x78dde6e5 & _mask] & Traits::OFFSET_MASK;
        sum += _info[0x1715609f & _mask] & Traits::OFFSET_MASK;

        sum += _info[0xb54cda58 & _mask] & Traits::OFFSET_MASK;
        sum += _info[0x53845412 & _mask] & Traits::OFFSET_MASK;
        sum += _info[0xf1bbcdcb & _mask] & Traits::OFFSET_MASK;
        sum += _info[0x8ff34785 & _mask] & Traits::OFFSET_MASK;
        sum += _info[0x2e2ac13e & _mask] & Traits::OFFSET_MASK;

        _jump_heuristic_offset = static_cast<typename Traits::InfoType>(sum / 10);

        /*
        sum += _info[]
        if (_num_elements & 0xfff) {
            if (_level > 23) {
                _jump_heuristic_offset = static_cast<Traits::InfoType>((_level - 23) * _max_elements / (static_cast<size_t>(5) * (_max_elements - _num_elements)));
            } else {
                _jump_heuristic_offset = 0;
            }
        }
        */
    }

    inline bool insert(Key&& key, Val&& val) {
        if (_num_elements == _max_num_num_elements_allowed) {
            increase_size();
        }

        const size_t original_idx = _hash(key) & _mask;

        size_t idx = original_idx + _jump_heuristic_offset;
        typename Traits::InfoType info = Traits::IS_BUCKET_TAKEN_MASK + _jump_heuristic_offset;

        while (idx > original_idx && info >= _info[idx]) {
            --idx;
            --info;
        }

        while (info < _info[idx]) {
            ++idx;
            ++info;
        }

        // while we potentially have a match
        while (info == _info[idx]) {
            if (_key_equal(key, _keys[idx])) {
                _alloc_vals.destroy(_vals + idx);
                _alloc_vals.construct(_vals + idx, std::forward<Val>(val));
                // no new insert
                return false;
            }
            ++idx;
            ++info;
        }

        // loop while we have not found an empty spot, and while no info overflow
        while (_info[idx] & Traits::IS_BUCKET_TAKEN_MASK && info) {
            if (info > _info[idx]) {
                // place element
                std::swap(key, _keys[idx]);
                std::swap(val, _vals[idx]);
                std::swap(info, _info[idx]);
            }
            ++idx;
            ++info;
        }

        if (idx == _max_elements + Traits::OVERFLOW_SIZE || 0 == info) {
            // Overflow! resize and try again.
            increase_size();
            return insert(std::forward<Key>(key), std::forward<Val>(val));
        }

        // bucket is empty! put it there.
        _alloc_keys.construct(_keys + idx, std::forward<Key>(key));
        _alloc_vals.construct(_vals + idx, std::forward<Val>(val));
        _info[idx] = info;

        ++_num_elements;
        if (!(_num_elements & 0xfff)) {
            update_jump_heuristic_offset();
        }
        return true;
    }

    Val* find(const Key& key) {
        return const_cast<Val*>(static_cast<const Self*>(this)->find(key));
    }

    const Val* find(const Key& key) const {
        const size_t original_idx = _hash(key) & _mask;

        size_t idx = original_idx + _jump_heuristic_offset;
        typename Traits::InfoType info = Traits::IS_BUCKET_TAKEN_MASK + _jump_heuristic_offset;

        while (idx > original_idx && info >= _info[idx]) {
            --idx;
            --info;
        }
        
        while (info < _info[idx]) {
            ++idx;
            ++info;
        }

        // check while info matches with the source idx
        while (info == _info[idx]) {
            if (_key_equal(key, _keys[idx])) {
                return _vals + idx;
            }
            ++idx;
            ++info;
        }

        // nothing found!
        return nullptr;
    }

    size_t erase(const Key& key) {
        size_t idx = _hash(key) & _mask;

        auto info = Traits::IS_BUCKET_TAKEN_MASK;
        while (info < _info[idx]) {
            ++idx;
            ++info;
        }

        // check while info matches with the source idx
        while (info == _info[idx]) {
            if (_key_equal(key, _keys[idx])) {
                // found it! perform backward shift deletion: shift elements to the left
                // until we find one that is either empty or has zero offset.
                //
                // Note: no need to check for last element, this acts as a sentinel
                while (_info[idx + 1] > Traits::IS_BUCKET_TAKEN_MASK) {
                    _info[idx] = _info[idx + 1] - 1;
                    _keys[idx] = std::move(_keys[idx + 1]);
                    _vals[idx] = std::move(_vals[idx + 1]);
                    ++idx;
                }
                if (_info[idx]) {
                    _info[idx] = 0;
                    _alloc_vals.destroy(_vals + idx);
                    _alloc_keys.destroy(_keys + idx);
                }

                --_num_elements;
                if (!(_num_elements & 0xfff)) {
                    update_jump_heuristic_offset();
                }
                return 1;
            }
            ++idx;
            ++info;
        }

        // nothing found to delete
        return 0;
    }

    inline size_t size() const {
        return _num_elements;
    }

    inline size_t max_size() const {
        return _max_elements;
    }

private:
    void init_data(size_t level) {
        _level = level;
        _jump_heuristic_offset = 0;
        _max_elements = static_cast<size_t>(1) << _level;
        _num_elements = 0;
        _mask = _max_elements - 1;

        // max * (1 - 1/20) = max * 0.95
        _max_num_num_elements_allowed = _max_elements - std::max(static_cast<size_t>(1), _max_elements / 20);

        _info = _alloc_info.allocate(_max_elements + Traits::OVERFLOW_SIZE);
        _keys = _alloc_keys.allocate(_max_elements + Traits::OVERFLOW_SIZE, _info);
        _vals = _alloc_vals.allocate(_max_elements + Traits::OVERFLOW_SIZE, _keys);

        std::memset(_info, 0, sizeof(typename Traits::InfoType) * (_max_elements + Traits::OVERFLOW_SIZE));
    }

    void increase_size() {
        //std::cout << (100.0*_num_elements / _max_elements) << "% full, resizing" << std::endl;
        auto* old_keys = _keys;
        auto* old_vals = _vals;
        auto* old_info = _info;

        auto old_max_elements = _max_elements;
        auto old_level = _level;
        init_data(old_level + 1);

        int num_ins = 0;
        for (size_t i = 0; i < old_max_elements + Traits::OVERFLOW_SIZE; ++i) {
            if (old_info[i] & Traits::IS_BUCKET_TAKEN_MASK) {
                ++num_ins;
                insert(std::move(old_keys[i]), std::move(old_vals[i]));
                _alloc_vals.destroy(old_vals + i);
                _alloc_keys.destroy(old_keys + i);
            }
        }

        _alloc_vals.deallocate(old_vals, old_max_elements + Traits::OVERFLOW_SIZE);
        _alloc_keys.deallocate(old_keys, old_max_elements + Traits::OVERFLOW_SIZE);
        _alloc_info.deallocate(old_info, old_max_elements + Traits::OVERFLOW_SIZE);
    }

    Val* _vals;
    Key* _keys;
    typename Traits::InfoType* _info;

    const H _hash;
    const E _key_equal;

    size_t _num_elements;
    size_t _max_elements;
    size_t _level;
    size_t _mask;
    size_t _max_num_num_elements_allowed;

    typename Traits::InfoType _jump_heuristic_offset;

    AVals _alloc_vals;
    AKeys _alloc_keys;
    AInfo _alloc_info;
};

}
