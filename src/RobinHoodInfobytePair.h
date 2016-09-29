/// Modified, highly optimized Robin Hood Hashtable.
/// Algorithm from https://github.com/martinus/robin-hood-hashing/
///
/// @author martin.ankerl@gmail.com

#pragma once

#include <cstdint>
#include <cstring>
#include <functional>
#include <algorithm>

// just with info byte
namespace RobinHoodInfobytePair {

namespace Style {

struct Default {
    // bits: | 7 | 6   5   4   3   2   1   0|
    //       | ^ |         offset           |
    //         |
    //       full? 
    typedef uint8_t InfoType;
    static constexpr InfoType IS_BUCKET_TAKEN_MASK = 1 << 7;
    static constexpr std::size_t OVERFLOW_SIZE = 32;
    static constexpr std::size_t INITIAL_ELEMENTS = 32;

    typedef std::allocator<typename InfoType> AInfo;
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
    class AKeyVals = std::allocator<std::pair<Key, Val>>,
    class Traits = Style::Default
>
class Map {
public:
    typedef Key key_type;
    typedef Val mapped_type;
    typedef Val value_type;
    typedef Map<Key, Val, H, E, AKeyVals, Traits> Self;

    /// Creates an empty hash map.
    Map()
    : _hash()
    , _key_equal() {
        init_data(Traits::INITIAL_ELEMENTS);
    }

    /// Clears all data, without resizing.
    void clear() {
        for (size_t i = 0; i < _max_elements + Traits::OVERFLOW_SIZE; ++i) {
            if (_info[i] & Traits::IS_BUCKET_TAKEN_MASK) {
                _alloc_vals.destroy(_vals + i);
                _alloc_keys.destroy(_keys + i);
            }
        }
        std::memset(_info, 0, sizeof(typename Traits::InfoType) * (_max_elements + Traits::OVERFLOW_SIZE));
        _num_elements = 0;
    }

    /// Destroys the map and all it's contents.
    ~Map() {
        // clear also resets _info to 0, that's not really necessary.
        for (size_t i = 0; i < _max_elements + Traits::OVERFLOW_SIZE; ++i) {
            if (_info[i] & Traits::IS_BUCKET_TAKEN_MASK) {
                _alloc_keyvals.destroy(_keyvals + i);
            }
        }
        _alloc_keyvals.deallocate(_keyvals, _max_elements + Traits::OVERFLOW_SIZE);
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

    inline bool insert(Key&& key, Val&& val) {
        if (_num_elements == _max_num_num_elements_allowed) {
            increase_size();
        }

        auto h = _hash(key);
        size_t idx = h & _mask;

        typename Traits::InfoType info = Traits::IS_BUCKET_TAKEN_MASK;
        while (info < _info[idx]) {
            ++idx;
            ++info;
        }

        // while we potentially have a match
        while (info == _info[idx]) {
            if (_key_equal(key, _keyvals[idx].first)) {
                // key already exists, do not insert.
                return false;
            }
            ++idx;
            ++info;
        }

        // loop while we have not found an empty spot, and while no info overflow
        auto keyval = std::make_pair(key, val);
        while (_info[idx] & Traits::IS_BUCKET_TAKEN_MASK && info) {
            if (info > _info[idx]) {
                // place element
                std::swap(keyval, _keyvals[idx]);
                std::swap(info, _info[idx]);
            }
            ++idx;
            ++info;
        }

        if (idx == _max_elements + Traits::OVERFLOW_SIZE || 0 == info) {
            // Overflow! resize and try again.
            increase_size();
            return insert(std::move(keyval.first), std::move(keyval.second));
            //return insert(std::forward<Key>(key), std::forward<Val>(val));
        }

        // bucket is empty! put it there.
        _alloc_keyvals.construct(_keyvals + idx, std::move(keyval));
        _info[idx] = info;

        ++_num_elements;
        return true;
    }

    Val* find(const Key& key) {
        return const_cast<Val*>(static_cast<const Self*>(this)->find(key));
    }

    const Val* find(const Key& key) const {
        size_t idx = _hash(key) & _mask;

        auto info = Traits::IS_BUCKET_TAKEN_MASK;
        while (info < _info[idx]) {
            ++idx;
            ++info;
        }

        // check while info matches with the source idx
        while (info == _info[idx]) {
            if (_key_equal(key, _keyvals[idx].first)) {
                return &_keyvals[idx].second;
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
            if (_key_equal(key, _keyvals[idx].first)) {
                // found it! perform backward shift deletion: shift elements to the left
                // until we find one that is either empty or has zero offset.
                //
                // Note: no need to check for last element, this acts as a sentinel
                while (_info[idx + 1] > Traits::IS_BUCKET_TAKEN_MASK) {
                    _info[idx] = _info[idx + 1] - 1;
                    _keyvals[idx] = std::move(_keyvals[idx + 1]);
                    ++idx;
                }
                if (_info[idx]) {
                    _info[idx] = 0;
                    _alloc_keyvals.destroy(_keyvals + idx);
                }

                --_num_elements;
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
    void init_data(size_t max_elements) {
        _max_elements = max_elements;
        _num_elements = 0;
        _mask = _max_elements - 1;

        // max * (1 - 1/20) = max * 0.95
        _max_num_num_elements_allowed = _max_elements - std::max((size_t)1, _max_elements / 20);

        _info = _alloc_info.allocate(_max_elements + Traits::OVERFLOW_SIZE);
        _keyvals = _alloc_keyvals.allocate(_max_elements + Traits::OVERFLOW_SIZE, _info);

        std::memset(_info, 0, sizeof(typename Traits::InfoType) * (_max_elements + Traits::OVERFLOW_SIZE));
    }

    void increase_size() {
        //std::cout << (100.0*_num_elements / _max_elements) << "% full, resizing" << std::endl;
        auto* old_keyvals = _keyvals;
        auto* old_info = _info;

        auto old_max_elements = _max_elements;

        init_data(old_max_elements * 2);

        int num_ins = 0;
        for (size_t i = 0; i < old_max_elements + Traits::OVERFLOW_SIZE; ++i) {
            if (old_info[i] & Traits::IS_BUCKET_TAKEN_MASK) {
                ++num_ins;
                insert(std::move(old_keyvals[i].first), std::move(old_keyvals[i].second));
                _alloc_keyvals.destroy(old_keyvals + i);
            }
        }

        _alloc_keyvals.deallocate(old_keyvals, old_max_elements + Traits::OVERFLOW_SIZE);
        _alloc_info.deallocate(old_info, old_max_elements + Traits::OVERFLOW_SIZE);
    }

    std::pair<Key, Val>* _keyvals;
    typename Traits::InfoType* _info;

    const H _hash;
    const E _key_equal;

    size_t _num_elements;
    size_t _max_elements;
    size_t _mask;
    size_t _max_num_num_elements_allowed;

    typename AKeyVals _alloc_keyvals;
    typename Traits::AInfo _alloc_info;
};

}
