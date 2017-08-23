/// Modified, highly optimized Robin Hood Hashtable.
/// Algorithm from https://github.com/martinus/robin-hood-hashing/
///
/// @author martin.ankerl@gmail.com

#pragma once

#include <cstdint>
#include <cstring>
#include <functional>
#include <algorithm>
#include <memory>

// just with info byte
namespace RobinHoodInfobytePair {

namespace Style {

struct Fast {
    // bits: | 7 | 6   5   4   3   2   1   0|
    //       | ^ |         offset           |
    //         |
    //       full? 
    typedef uint8_t InfoType;
    static constexpr InfoType IS_BUCKET_TAKEN_MASK = 1 << 7;
    static constexpr std::size_t OVERFLOW_SIZE = 32;
    static constexpr std::size_t INITIAL_ELEMENTS = 32;
    typedef std::allocator<InfoType> AInfo;
    static constexpr float MAX_LOAD_FACTOR = 0.50f;
};

struct Default {
    // bits: | 7 | 6   5   4   3   2   1   0|
    //       | ^ |         offset           |
    //         |
    //       full? 
    typedef uint8_t InfoType;
    static constexpr InfoType IS_BUCKET_TAKEN_MASK = 1 << 7;
    static constexpr std::size_t OVERFLOW_SIZE = 32;
    static constexpr std::size_t INITIAL_ELEMENTS = 32;
    typedef std::allocator<InfoType> AInfo;
    static constexpr float MAX_LOAD_FACTOR = 0.90f;
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
    class T,
    class Hash = std::hash<Key>,
    class KeyEqual = std::equal_to<Key>,
    class Allocator = std::allocator<std::pair<Key, T>>,
    class Traits = Style::Default
>
class Map {
public:
    typedef Key key_type;
    typedef T mapped_type;
    typedef std::pair<Key, T> value_type;
    typedef std::size_t size_type;
    typedef Hash hasher;
    typedef KeyEqual key_equal;
    typedef Allocator allocator_type;
    typedef value_type* iterator;
    typedef const value_type* const_iterator;
    typedef Map<key_type, mapped_type, hasher, key_equal, allocator_type, Traits> Self;

    /// Creates an empty hash map.
    Map()
    : _hash()
    , _key_equal()
    , _max_load_factor(Traits::MAX_LOAD_FACTOR) {
        init_data(Traits::INITIAL_ELEMENTS);
    }

    /// Clears all data, without resizing.
    void clear() {
        for (size_t i = 0; i < _max_elements + Traits::OVERFLOW_SIZE; ++i) {
            if (_info[i] & Traits::IS_BUCKET_TAKEN_MASK) {
                _alloc_keyvals.destroy(_keyvals + i);
            }
        }
        std::memset(_info, 0, sizeof(typename Traits::InfoType) * (_max_elements + Traits::OVERFLOW_SIZE));
        _num_elements = 0;
    }

    /// Destroys the map and all it's contents.
    inline ~Map() {
        // clear also resets _info to 0, that's not really necessary.
        for (size_t i = 0; i < _max_elements + Traits::OVERFLOW_SIZE; ++i) {
            if (_info[i] & Traits::IS_BUCKET_TAKEN_MASK) {
                _alloc_keyvals.destroy(_keyvals + i);
            }
        }
        _alloc_keyvals.deallocate(_keyvals, _max_elements + Traits::OVERFLOW_SIZE);
        _alloc_info.deallocate(_info, _max_elements + Traits::OVERFLOW_SIZE);
    }

    inline mapped_type& operator[](value_type&& keyval) {
        if (_num_elements == _max_num_num_elements_allowed) {
            increase_size();
        }

        auto h = _hash(keyval.first);
        size_t idx = h & _mask;

        typename Traits::InfoType info = Traits::IS_BUCKET_TAKEN_MASK;
        while (info < _info[idx]) {
            ++idx;
            ++info;
        }

        // while we potentially have a match
        while (info == _info[idx]) {
            if (_key_equal(keyval.first, _keyvals[idx].first)) {
                // key already exists, do not insert.
                return _keyvals[idx].second;
            }
            ++idx;
            ++info;
        }

        // loop while we have not found an empty spot, and while no info overflow
        size_t insertion_idx;
        bool is_inserted = false;
        while (_info[idx] & Traits::IS_BUCKET_TAKEN_MASK && info) {
            if (info > _info[idx]) {
                // place element
                std::swap(keyval, _keyvals[idx]);
                std::swap(info, _info[idx]);
                if (!is_inserted) {
                    is_inserted = true;
                    insertion_idx = idx;
                }
            }
            ++idx;
            ++info;
        }

        if (idx == _max_elements + Traits::OVERFLOW_SIZE - 1 || 0 == info) {
            // Overflow! resize and try again.
            increase_size();
            return operator[](std::forward<value_type>(keyval));
        }

        // bucket is empty! put it there.
        _alloc_keyvals.construct(_keyvals + idx, std::move(keyval));
        _info[idx] = info;

        ++_num_elements;
        return _keyvals[is_inserted ? insertion_idx : idx].second;
    }

    inline mapped_type& operator[](const key_type& key) {
        return operator[](std::make_pair(key, mapped_type()));
    }

    inline std::pair<iterator, bool> insert(value_type&& keyval) {
        if (_num_elements == _max_num_num_elements_allowed) {
            increase_size();
        }

        auto h = _hash(keyval.first);
        size_t idx = h & _mask;

        typename Traits::InfoType info = Traits::IS_BUCKET_TAKEN_MASK;
        while (info < _info[idx]) {
            ++idx;
            ++info;
        }

        // while we potentially have a match
        while (info == _info[idx]) {
            if (_key_equal(keyval.first, _keyvals[idx].first)) {
                // key already exists, do not insert.
                return std::make_pair<iterator, bool>(_keyvals + idx, false);
            }
            ++idx;
            ++info;
        }

        // loop while we have not found an empty spot, and while no info overflow
        size_t insertion_idx;
        bool is_inserted = false;
        while (_info[idx] & Traits::IS_BUCKET_TAKEN_MASK && info) {
            if (info > _info[idx]) {
                // place element
                std::swap(keyval, _keyvals[idx]);
                std::swap(info, _info[idx]);
                if (!is_inserted) {
                    is_inserted = true;
                    insertion_idx = idx;
                }
            }
            ++idx;
            ++info;
        }

        if (idx == _max_elements + Traits::OVERFLOW_SIZE - 1 || 0 == info) {
            // Overflow! resize and try again.
            increase_size();
            return insert(std::move(keyval));
        }

        // bucket is empty! put it there.
        _alloc_keyvals.construct(_keyvals + idx, std::move(keyval));
        _info[idx] = info;

        ++_num_elements;
        return std::make_pair(_keyvals + (is_inserted ? insertion_idx : idx), true);
    }

    // inserts a keyval that is guaranteed to be new, e.g. when the hashmap is resized.
    inline void insert_unchecked(value_type&& keyval) {
        auto h = _hash(keyval.first);
        size_t idx = h & _mask;

        typename Traits::InfoType info = Traits::IS_BUCKET_TAKEN_MASK;
        while (info <= _info[idx]) {
            ++idx;
            ++info;
        }

        // loop while we have not found an empty spot, and while no info overflow
        while (_info[idx] & Traits::IS_BUCKET_TAKEN_MASK && info) {
            if (info > _info[idx]) {
                // place element
                std::swap(keyval, _keyvals[idx]);
                std::swap(info, _info[idx]);
            }
            ++idx;
            ++info;
        }

        if (idx == _max_elements + Traits::OVERFLOW_SIZE - 1 || 0 == info) {
            // Overflow! resize and try again.
            increase_size();
            insert(std::forward<value_type>(keyval));
            return;
        }

        // bucket is empty! put it there.
        _alloc_keyvals.construct(_keyvals + idx, std::move(keyval));
        _info[idx] = info;

        ++_num_elements;
    }

    inline std::pair<iterator, bool> insert(const value_type& value) {
        value_type v(value);
        return insert(std::move(v));
    }

    inline iterator find(const key_type& key) {
        return const_cast<iterator>(static_cast<const Self*>(this)->find(key));
    }

    inline const_iterator end() const {
        return nullptr;
    }

    inline const_iterator find(const key_type& key) const {
        /*
        const size_t original_idx = _hash(key) & _mask;

        auto info = _info[original_idx];
        size_t idx = original_idx + info - Traits::IS_BUCKET_TAKEN_MASK;
        if (idx >= _max_elements + Traits::OVERFLOW_SIZE) {
            info = Traits::IS_BUCKET_TAKEN_MASK;
            idx = original_idx;
        }
        while (idx > original_idx && info >= _info[idx]) {
            --idx;
            --info;
        }
        */
        size_t idx = _hash(key) & _mask;
        auto info = Traits::IS_BUCKET_TAKEN_MASK;
        while (info < _info[idx]) {
            ++idx;
            ++info;
        }

        // check while info matches with the source idx
        while (info == _info[idx]) {
            if (_key_equal(key, _keyvals[idx].first)) {
                return _keyvals + idx;
            }
            ++idx;
            ++info;
        }

        // nothing found!
        return end();
    }

    inline size_t erase(const key_type& key) {
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

    inline size_type size() const {
        return _num_elements;
    }

    inline size_type max_size() const {
        return (size_type)-1;
    }

    inline bool empty() const {
        return 0 == _num_elements;
    }

    inline float max_load_factor() const {
        return _max_load_factor;
    }

    inline void max_load_factor(float ml) {
        _max_load_factor = ml;
        update_max_num_elements_allowed();
    }

    inline float load_factor() const {
        return _num_elements / static_cast<float>(_max_elements + Traits::OVERFLOW_SIZE);
    }

private:
    inline void update_max_num_elements_allowed() {
        _max_num_num_elements_allowed = std::min(
            _max_elements - 2,
            static_cast<size_t>(_max_load_factor * _max_elements));
    }

    inline void init_data(size_t max_elements) {
        _max_elements = max_elements;
        _num_elements = 0;
        _mask = _max_elements - 1;


        // max * (1 - 1/20) = max * 0.95
        update_max_num_elements_allowed();

        _keyvals = _alloc_keyvals.allocate(_max_elements + Traits::OVERFLOW_SIZE);
        _info = _alloc_info.allocate(_max_elements + Traits::OVERFLOW_SIZE, _keyvals);

        std::memset(_info, 0, sizeof(typename Traits::InfoType) * (_max_elements + Traits::OVERFLOW_SIZE));
    }

    inline void increase_size() {
        //std::cout << (100.0*_num_elements / _max_elements) << "% full, resizing" << std::endl;
        auto* old_keyvals = _keyvals;
        auto* old_info = _info;

        auto old_max_elements = _max_elements;

        init_data(old_max_elements * 2);

        int num_ins = 0;
        for (size_t i = 0; i < old_max_elements + Traits::OVERFLOW_SIZE; ++i) {
            if (old_info[i] & Traits::IS_BUCKET_TAKEN_MASK) {
                ++num_ins;
                insert_unchecked(std::move(old_keyvals[i]));
                _alloc_keyvals.destroy(old_keyvals + i);
            }
        }

        _alloc_keyvals.deallocate(old_keyvals, old_max_elements + Traits::OVERFLOW_SIZE);
        _alloc_info.deallocate(old_info, old_max_elements + Traits::OVERFLOW_SIZE);
    }

    std::pair<key_type, mapped_type>* _keyvals;
    typename Traits::InfoType* _info;

    const hasher _hash;
    const key_equal _key_equal;

    size_t _num_elements;
    size_t _max_elements;
    size_t _mask;
    size_t _max_num_num_elements_allowed;

    float _max_load_factor;

    Allocator _alloc_keyvals;
    typename Traits::AInfo _alloc_info;
};

}
