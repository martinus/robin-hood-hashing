#ifndef __HOPSCOTCH_MAP_H__
#define __HOPSCOTCH_MAP_H__

#include <cinttypes>
#include <functional>
#include <cstddef>
#include <utility>
#include <type_traits>
#include <memory>
#include <limits>
#include <list>
#include <vector>
#include <cassert>
#include <iterator>
#include <initializer_list>
#include <stdexcept>
#include <algorithm>
#include <cstdint>
#include <climits>

namespace {
    /*
     * smallest_type_for_min_bits::type returns the smallest type that can fit MIN_BITS.
     */
    static const size_t SMALLEST_TYPE_MAX_BITS_SUPPORTED = 64;
    template<unsigned int MIN_BITS, typename Enable = void>
    class smallest_type_for_min_bits {
    };

    template<unsigned int MIN_BITS>
    class smallest_type_for_min_bits<MIN_BITS, typename std::enable_if<(MIN_BITS > 0) && (MIN_BITS <= 8)>::type> {
    public:
        using type = std::uint8_t;
    };

    template<unsigned int MIN_BITS>
    class smallest_type_for_min_bits<MIN_BITS, typename std::enable_if<(MIN_BITS > 8) && (MIN_BITS <= 16)>::type> {
    public:
        using type = std::uint16_t;
    };

    template<unsigned int MIN_BITS>
    class smallest_type_for_min_bits<MIN_BITS, typename std::enable_if<(MIN_BITS > 16) && (MIN_BITS <= 32)>::type> {
    public:
        using type = std::uint32_t;
    };

    template<unsigned int MIN_BITS>
    class smallest_type_for_min_bits<MIN_BITS, typename std::enable_if<(MIN_BITS > 32) && (MIN_BITS <= 64)>::type> {
    public:
        using type = std::uint64_t;
    };
}

/**
 * Implementation of a hash map using the hopscotch hashing algorithm.
 * 
 * The size of the neighborhood (NeighborhoodSize) must be > 0 and <= 62.
 * 
 * The Key must be copy-constructible. The value T must be either move-constructible, copy-constuctible or both.
 * 
 * Iterators invalidation:
 *  - clear, operator=: always invalidate the iterators.
 *  - insert, operator[]: invalidate the iterators if there is a rehash, or if a displacement is needed to resolve a collision (which mean that most of the time, insert will invalidate the iterators).
 *  - erase: iterator on the erased element is the only one which become invalid.
 */
template<class Key, 
         class T, 
         class Hash = std::hash<Key>,
         class KeyEqual = std::equal_to<Key>,
         //class Allocator = std::allocator<std::pair<const Key, T>>,
         unsigned int NeighborhoodSize = 62>
class hopscotch_map {
private:
    static const size_t NB_RESERVED_BITS_IN_NEIGHBORHOOD = 2; 
    static const size_t MAX_NEIGHBORHOOD_SIZE = SMALLEST_TYPE_MAX_BITS_SUPPORTED - NB_RESERVED_BITS_IN_NEIGHBORHOOD; 
    
    /*
     * NeighborhoodSize need to be between 0 and MAX_NEIGHBORHOOD_SIZE.
     * Static assert need a string litteral, we can't put the 62 in a variable. 
     * TODO way to avoid this?
     */
    static_assert(NeighborhoodSize > 0, "NeighborhoodSize should be > 0.");
    static_assert(NeighborhoodSize <= MAX_NEIGHBORHOOD_SIZE, "NeighborhoodSize should be <= 62.");
    
    
    using neighborhood_bitmap = typename smallest_type_for_min_bits<NeighborhoodSize + NB_RESERVED_BITS_IN_NEIGHBORHOOD>::type;
public:
    template<bool is_const = false>
    class hopscotch_iterator;
    
    using key_type = Key;
    using mapped_type = T;
    using value_type = std::pair<const Key, T>;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using hasher = Hash;
    using key_equal = KeyEqual;
    //using allocator_type = Allocator;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = value_type*;
    using const_pointer = const value_type*;
    using iterator = hopscotch_iterator<false>;
    using const_iterator = hopscotch_iterator<true>;
    
private:
    /*
     * Each bucket stores two elements:
     * - An aligned storage to store a value_type object with placement-new
     * - An unsigned integer of type neighborhood_bitmap used to tell us which buckets in the neighborhood of the current bucket
     *   contain a value with a hash belonging to the current bucket. 
     * 
     * For a bucket 'b' a bit 'i' (counting from 0 and from the least significant bit to the most significant) 
     * set to 1 means that the bucket 'b+i' contains a value with a hash belonging to bucket 'b'.
     * The bits used for that, start from the third least significant bit.
     * 
     * The least significant bit is set to 1 if there is a value in the bucket storage.
     * The second least significant bit is set to 1 if there is an overflow. More than NeighborhoodSize values give the same hash,
     * all overflow values are stored in the m_overflow_elements list of the map.
     */
    class hopscotch_bucket {
        using storage = typename std::aligned_storage<sizeof(value_type), alignof(value_type)>::type;
        
    public:
        hopscotch_bucket() noexcept : m_neighborhood_infos(0) {
            assert(is_empty());
        }
        
        hopscotch_bucket(const hopscotch_bucket & bucket) noexcept(std::is_nothrow_copy_constructible<value_type>::value) : 
                    m_neighborhood_infos(bucket.m_neighborhood_infos) 
        {
            if(!bucket.is_empty()) {
                ::new (static_cast<void *>(std::addressof(m_key_value))) value_type(bucket.get_key_value());
            }
        }
        
        hopscotch_bucket(hopscotch_bucket && bucket) noexcept(std::is_nothrow_move_constructible<value_type>::value) : 
                    m_neighborhood_infos(bucket.m_neighborhood_infos) 
        {
            if(!bucket.is_empty()) {
                ::new (static_cast<void *>(std::addressof(m_key_value))) value_type(std::move(bucket.get_key_value()));
            }
        }
        
        hopscotch_bucket & operator=(const hopscotch_bucket & bucket) noexcept(std::is_nothrow_copy_constructible<value_type>::value &&  
                                                                               std::is_nothrow_destructible<value_type>::value) 
        {
            if(this != &bucket) {
                if(!is_empty()) {
                    get_key_value().~value_type();
                }
                
                m_neighborhood_infos = bucket.m_neighborhood_infos;
                
                if(!bucket.is_empty()) {
                    ::new (static_cast<void *>(std::addressof(m_key_value))) value_type(bucket.get_key_value());
                }
            }
            
            return *this;
        }
        
        hopscotch_bucket & operator=(hopscotch_bucket && bucket) noexcept(std::is_nothrow_move_constructible<value_type>::value &&
                                                                          std::is_nothrow_destructible<value_type>::value) 
        {
            if(!is_empty()) {
                get_key_value().~value_type();
            }
            
            m_neighborhood_infos = bucket.m_neighborhood_infos;
            
            if(!bucket.is_empty()) {
                ::new (static_cast<void *>(std::addressof(m_key_value))) value_type(std::move(bucket.get_key_value()));
            }
            
            return *this;
        }
        
        ~hopscotch_bucket() noexcept(std::is_nothrow_destructible<value_type>::value) {
            if(!is_empty()) {
                get_key_value().~value_type();
            }
            
            m_neighborhood_infos = 0;
        }
        
        neighborhood_bitmap get_neighborhood_infos() const noexcept {
            return static_cast<neighborhood_bitmap>(m_neighborhood_infos >> NB_RESERVED_BITS_IN_NEIGHBORHOOD);
        }
        
        void set_overflow(bool has_overflow) noexcept {
            if(has_overflow) {
                m_neighborhood_infos = static_cast<neighborhood_bitmap>(m_neighborhood_infos | 2);
            }
            else {
                m_neighborhood_infos = static_cast<neighborhood_bitmap>(m_neighborhood_infos & ~2);
            }
        }
        
        bool has_overflow() const noexcept {
            return (m_neighborhood_infos & 2) != 0;
        }
        
        bool is_empty() const noexcept {
            return (m_neighborhood_infos & 1) == 0;
        }
        
        template<typename P>
        void set_key_value(P&& key_value) {
            if(!is_empty()) {
                get_key_value().~value_type();
                ::new (static_cast<void *>(std::addressof(m_key_value))) value_type(std::forward<P>(key_value));
            }
            else {
                ::new (static_cast<void *>(std::addressof(m_key_value))) value_type(std::forward<P>(key_value));
                set_is_empty(false);
            }
        }
        
        void remove_key_value() {
            if(!is_empty()) {
                get_key_value().~value_type();
                set_is_empty(true);
            }
        }
        
        void toggle_neighbor_presence(std::size_t ineighbor) noexcept {
            assert(ineighbor <= NeighborhoodSize);
            m_neighborhood_infos = static_cast<neighborhood_bitmap>(m_neighborhood_infos ^ (1ull << (ineighbor + NB_RESERVED_BITS_IN_NEIGHBORHOOD)));
        }
        
        bool check_neighbor_presence(std::size_t ineighbor) const noexcept {
            assert(ineighbor <= NeighborhoodSize);
            if(((m_neighborhood_infos >> (ineighbor + NB_RESERVED_BITS_IN_NEIGHBORHOOD)) & 1) == 1) {
                return true;
            }
            
            return false;
        }
        
        value_type & get_key_value() noexcept {
            assert(!is_empty());
            return *reinterpret_cast<value_type *>(std::addressof(m_key_value));
        }
        
        const value_type & get_key_value() const noexcept {
            assert(!is_empty());
            return *reinterpret_cast<const value_type *>(std::addressof(m_key_value));
        }
        
        void swap_key_value_with_empty_bucket(hopscotch_bucket & empty_bucket) {
            assert(empty_bucket.is_empty());
            if(!is_empty()) {
                ::new (static_cast<void *>(std::addressof(empty_bucket.m_key_value))) value_type(std::move(get_key_value()));
                empty_bucket.set_is_empty(false);
                
                get_key_value().~value_type();
                set_is_empty(true);
            }
        }
        
        
    private:
        void set_is_empty(bool is_empty) noexcept {
            if(is_empty) {
                m_neighborhood_infos = static_cast<neighborhood_bitmap>(m_neighborhood_infos & ~1);
            }
            else {
                m_neighborhood_infos = static_cast<neighborhood_bitmap>(m_neighborhood_infos | 1);
            }
        }
        
    private:
        storage m_key_value;
        neighborhood_bitmap m_neighborhood_infos;
    };
    
    
public:    
    template<bool is_const>
    class hopscotch_iterator {
        friend class hopscotch_map;
    private:
        using iterator_bucket = typename std::conditional<is_const, 
                                                            typename std::vector<hopscotch_bucket>::const_iterator, 
                                                            typename std::vector<hopscotch_bucket>::iterator>::type;
        using iterator_overflow = typename std::conditional<is_const, 
                                                            typename std::list<typename hopscotch_map::value_type>::const_iterator, 
                                                            typename std::list<typename hopscotch_map::value_type>::iterator>::type;
    
        
        hopscotch_iterator(iterator_bucket buckets_iterator, iterator_bucket buckets_end_iterator, 
                           iterator_overflow overflow_iterator) noexcept : 
            m_buckets_iterator(buckets_iterator), m_buckets_end_iterator(buckets_end_iterator),
            m_overflow_iterator(overflow_iterator)
        {
        }
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = typename hopscotch_map::value_type;
        using difference_type = std::ptrdiff_t;
        using reference = typename std::conditional<is_const, const value_type&, value_type&>::type;
        using pointer = typename std::conditional<is_const, const value_type*, value_type*>::type;
        
        hopscotch_iterator() noexcept {
        }
        
        hopscotch_iterator(const hopscotch_iterator<false> & other) noexcept :
            m_buckets_iterator(other.m_buckets_iterator), m_buckets_end_iterator(other.m_buckets_end_iterator),
            m_overflow_iterator(other.m_overflow_iterator)
        {
        }
        
        reference operator*() const { 
            if(m_buckets_iterator != m_buckets_end_iterator) {
                return m_buckets_iterator->get_key_value();
            }
            
            return *m_overflow_iterator;
        }
        
        pointer operator->() const { 
            if(m_buckets_iterator != m_buckets_end_iterator) {
                return std::addressof(m_buckets_iterator->get_key_value()); 
            }
            
            return std::addressof(*m_overflow_iterator); 
        }
        
        hopscotch_iterator& operator++() {
            if(m_buckets_iterator == m_buckets_end_iterator) {
                ++m_overflow_iterator;
                return *this;
            }
            
            do {
                ++m_buckets_iterator;
            } while(m_buckets_iterator != m_buckets_end_iterator && m_buckets_iterator->is_empty());
            
            return *this; 
        }
        hopscotch_iterator operator++(int) {
            hopscotch_iterator tmp(*this);
            ++*this;
            
            return tmp;
        }
        
        friend bool operator==(const hopscotch_iterator& lhs, const hopscotch_iterator& rhs) { 
            return lhs.m_buckets_iterator == rhs.m_buckets_iterator && 
                   lhs.m_buckets_end_iterator == rhs.m_buckets_end_iterator &&
                   lhs.m_overflow_iterator == rhs.m_overflow_iterator; 
        }
        
        friend bool operator!=(const hopscotch_iterator& lhs, const hopscotch_iterator& rhs) { 
            return !(lhs == rhs); 
        }
    private:
        iterator_bucket m_buckets_iterator;
        iterator_bucket m_buckets_end_iterator;
        iterator_overflow m_overflow_iterator;
    };
    

    
public:
    /*
     * Constructors
     */
    hopscotch_map() : hopscotch_map(DEFAULT_INIT_BUCKETS_SIZE) {
    }
    
    explicit hopscotch_map(size_type bucket_count, 
                        const Hash& hash = Hash(),
                        const KeyEqual& equal = KeyEqual()
                        /*const Allocator& alloc = Allocator()*/) : hopscotch_map(bucket_count, hash, equal, DEFAULT_MAX_LOAD_FACTOR)
    {
    }
    
    template<class InputIt>
    hopscotch_map(InputIt first, InputIt last,
                size_type bucket_count = DEFAULT_INIT_BUCKETS_SIZE,
                const Hash& hash = Hash(),
                const KeyEqual& equal = KeyEqual()
                /*const Allocator& alloc = Allocator()*/) : hopscotch_map(bucket_count, hash, equal)
    {
        insert(first, last);
    }

    hopscotch_map(std::initializer_list<value_type> init,
                    size_type bucket_count = DEFAULT_INIT_BUCKETS_SIZE,
                    const Hash& hash = Hash(),
                    const KeyEqual& equal = KeyEqual()
                    /*const Allocator& alloc = Allocator() */) : hopscotch_map(init.begin(), init.end(), bucket_count, hash, equal)
    {
    }
    
    hopscotch_map& operator=(std::initializer_list<value_type> ilist) {
        *this = hopscotch_map(ilist);
        return *this;
    }
    
    
    /*
     * Iterators
     */
    iterator begin() noexcept {
        return iterator(get_first_non_empty_buckets_iterator(), m_buckets.end(), m_overflow_elements.begin());
    }
    
    const_iterator begin() const noexcept {
        return cbegin();
    }
    
    const_iterator cbegin() const noexcept {
        return const_iterator(get_first_non_empty_buckets_iterator(), m_buckets.cend(), m_overflow_elements.cbegin());
    }
    
    iterator end() noexcept {
        return iterator(m_buckets.end(), m_buckets.end(), m_overflow_elements.end());
    }
    
    const_iterator end() const noexcept {
        return cend();
    }
    
    const_iterator cend() const noexcept {
        return const_iterator(m_buckets.cend(), m_buckets.cend(), m_overflow_elements.cend());
    }
    
    
    /*
     * Capacity
     */
    bool empty() const noexcept {
        return m_nb_elements == 0;
    }
    
    size_type size() const noexcept {
        return m_nb_elements;
    }
    
    
    
    /*
     * Modifiers
     */
    void clear() noexcept {
        m_buckets.clear();
        m_overflow_elements.clear();
        m_nb_elements = 0;
        
        m_buckets.resize(DEFAULT_INIT_BUCKETS_SIZE);
    }
    
    std::pair<iterator, bool> insert(const value_type& value) {
        return insert_internal(value);
    }
    
    std::pair<iterator, bool> insert(value_type&& value) {
        return insert_internal(std::move(value));
    }
    
    template<class InputIt>
    void insert(InputIt first, InputIt last) {
        for(; first != last; ++first) {
            insert(*first);
        }
    }
    
    void insert(std::initializer_list<value_type> ilist) {
        insert(ilist.begin(), ilist.end());
    }
    
    template <class... Args>
    std::pair<iterator, bool> try_emplace(const key_type& k, Args&&... args) {
        return try_emplace_internal(k, std::forward<Args>(args)...);
    }
    
    template <class... Args>
    std::pair<iterator, bool> try_emplace(key_type&& k, Args&&... args) {
        return try_emplace_internal(std::move(k), std::forward<Args>(args)...);
    }
    
    iterator erase(const_iterator pos) {
        if(pos.m_buckets_iterator != pos.m_buckets_end_iterator) {
            return erase_from_bucket(pos);
        }
        else {
            return erase_from_overflow(pos);
        }
    }
    
    
    
    iterator erase(const_iterator first, const_iterator last) {
        if(first == last) {
            if(first.m_buckets_iterator != first.m_buckets_end_iterator) {
                // Get a non-const iterator
                auto it = m_buckets.begin() + std::distance(m_buckets.cbegin(), first.m_buckets_iterator);
                return iterator(it, m_buckets.end(), m_overflow_elements.begin());
            }
            else {
                // Get a non-const iterator
                auto it = m_overflow_elements.erase(first.m_overflow_iterator, first.m_overflow_iterator);
                return iterator(m_buckets.end(), m_buckets.end(), it);
            }
        }
        
        auto to_delete = erase(first);
        while(to_delete != last) {
            to_delete = erase(to_delete);
        }
        
        return to_delete;
    }
    
    size_type erase(const key_type& key) {
        auto it = find(key);
        if(it != end()) {
            erase(it);
            return 1;
        }
        else {
            return 0;
        }
    }
    
    
    
    /*
     * Lookup
     */
    T& at(const Key& key) {
        return const_cast<T&>(static_cast<const hopscotch_map*>(this)->at(key));
    }
    
    const T& at(const Key& key) const {
        auto it_find = find(key);
        if(it_find == cend()) {
            throw std::out_of_range("Couldn't find key.");
        }
        else {
            return it_find->second;
        }
    }
    
    T& operator[](const Key& key) {
        auto it_find = find(key);
        if(it_find == end()) {
            return insert(std::make_pair(key, T())).first->second;
        }
        else {
            return it_find->second;
        }
    }
    
    size_type count(const Key& key) const {
        if(find(key) == end()) {
            return 0;
        }
        else {
            return 1;
        }
    }
    
    iterator find(const Key& key) {
        assert(!m_buckets.empty());
        const std::size_t ibucket_for_hash =  bucket_for_hash(m_hash(key));
        
        return find_internal(key, m_buckets.begin() + ibucket_for_hash);
    }
    
    const_iterator find(const Key& key) const {
        assert(!m_buckets.empty());
        const std::size_t ibucket_for_hash =  bucket_for_hash(m_hash(key));
        
        return find_internal(key, m_buckets.begin() + ibucket_for_hash);
    }
    
    /*
     * Bucket interface 
     */
    size_type bucket_count() const {
        return m_buckets.size(); 
    }
    
    
    /*
     *  Hash policy 
     */
    float load_factor() const {
        return (1.0f*m_nb_elements)/m_buckets.size();
    }
    
    float max_load_factor() const {
        return m_max_load_factor;
    }
    
    void max_load_factor(float ml) {
        m_max_load_factor = ml;
        m_load_threshold = m_buckets.size() * ml;
    }
    
    void rehash(size_type count) {
        rehash_internal(count);
    }
    
    
    /*
     * Observers
     */
    hasher hash_function() const {
        return m_hash;
    }
    
    key_equal key_eq() const {
        return m_key_equal;
    }
private:
    hopscotch_map(size_type bucket_count, 
                  const Hash& hash,
                  const KeyEqual& equal,
                  /*const Allocator& alloc*/
                  float max_load_factor) :  m_buckets(bucket_count), m_nb_elements(0), 
                                            m_max_load_factor(max_load_factor), 
                                            m_load_threshold((std::size_t) (m_buckets.size() * m_max_load_factor)),
                                            m_hash(hash), m_key_equal(equal)
    {
        // TODO round to nearsest power of 2, bucket_count is the minimal size in standard 
        if(!is_power_of_two(bucket_count)) {
            throw std::runtime_error("bucket_count must be a positive number and a power of 2.");
        }
    }
    
    
    std::size_t bucket_for_hash(std::size_t hash) const {
        return hash & (m_buckets.size() - 1);
    }
    
    template<typename U = value_type, typename std::enable_if<!std::is_nothrow_move_constructible<U>::value>::type* = nullptr>
    void rehash_internal(size_type count) {
        hopscotch_map tmp_map(count, m_hash, m_key_equal, m_max_load_factor);
        
        for(const auto & key_value : *this) {
            const std::size_t ibucket_for_hash = tmp_map.bucket_for_hash(tmp_map.m_hash(key_value.first));
            tmp_map.insert_internal(key_value, ibucket_for_hash);
        }
        
        std::swap(*this, tmp_map);
    }   
    
    template<typename U = value_type, typename std::enable_if<std::is_nothrow_move_constructible<U>::value>::type* = nullptr>
    void rehash_internal(size_type count) {
        /*
         * Little more complicate here. We want to avoid any exception so we can't just insert elements
         * in the map trough insert_internal. The method may throw even if value_type is nothrow_move_constructible
         * due to the m_overflow_elements.push_back in the method.
         * 
         * So we first move safely all the elements from m_buckets in the new map (we know they will not go into overflow). 
         * We then swap m_overflow_elements into the new map and we update all necessary overflow flags.
         * 
         * This way we don't do any memory allocation (outside the construction of m_buckets) and we avoid
         * any exception which may hinder the strong exception-safe guarantee.
         */
        hopscotch_map tmp_map(count, m_hash, m_key_equal, m_max_load_factor);
        
        for(hopscotch_bucket & bucket : m_buckets) {
            if(bucket.is_empty()) {
                continue;
            }
            
            const std::size_t ibucket_for_hash = tmp_map.bucket_for_hash(tmp_map.m_hash(bucket.get_key_value().first));
            tmp_map.insert_internal(std::move(bucket.get_key_value()), ibucket_for_hash);
        }
        
        assert(tmp_map.m_overflow_elements.empty());
        if(!m_overflow_elements.empty()) {
            tmp_map.m_overflow_elements.swap(m_overflow_elements);
            tmp_map.m_nb_elements += tmp_map.m_overflow_elements.size();
            
            for(const value_type & key_value : tmp_map.m_overflow_elements) {
                const std::size_t ibucket_for_hash = tmp_map.bucket_for_hash(tmp_map.m_hash(key_value.first));
                tmp_map.m_buckets[ibucket_for_hash].set_overflow(true);
            }
        }
        
        std::swap(*this, tmp_map);
    } 
    
    /*
     * Find in m_overflow_elements an element for which te bucket it initially belong to equals original_bucket_for_hash.
     * Return m_overflow_elements.end() if none.
     */
    typename std::list<value_type>::iterator find_in_overflow_from_bucket(typename std::list<value_type>::iterator search_start, 
                                                                          std::size_t original_bucket_for_hash) 
    {
        for(auto it = search_start; it != m_overflow_elements.end(); ++it) {
            const std::size_t bucket_for_overflow_hash = bucket_for_hash(m_hash(it->first));
            if(bucket_for_overflow_hash == original_bucket_for_hash) {
                return it;
            }
        }
        
        return m_overflow_elements.end();
    }
    
    // iterator is in overflow list
    iterator erase_from_overflow(const_iterator pos) {
        assert(pos.m_overflow_iterator != m_overflow_elements.cend());
        
        const key_type& key = pos->first;
        const std::size_t ibucket_for_hash = bucket_for_hash(m_hash(key));
        
        auto it = m_overflow_elements.erase(pos.m_overflow_iterator);
        m_nb_elements--;
        
        // Check if we can remove the overflow flag
        assert(m_buckets[ibucket_for_hash].has_overflow());
        if(find_in_overflow_from_bucket(m_overflow_elements.begin(), ibucket_for_hash) == m_overflow_elements.end()) {
            m_buckets[ibucket_for_hash].set_overflow(false);
        }
        
        return iterator(m_buckets.end(), m_buckets.end(), it);
    }
    
    // iterator is in bucket
    iterator erase_from_bucket(const_iterator pos) {
        assert(pos.m_buckets_iterator != pos.m_buckets_end_iterator);
        
        const key_type& key = pos->first;
        const std::size_t ibucket_for_hash = bucket_for_hash(m_hash(key));
        const std::size_t ibucket_for_key = std::distance(m_buckets.cbegin(), pos.m_buckets_iterator);
        
        m_buckets[ibucket_for_key].remove_key_value();
        m_buckets[ibucket_for_hash].toggle_neighbor_presence(ibucket_for_key - ibucket_for_hash);
        m_nb_elements--;
    
        // Get next non-empty bucket iterator
        auto it_next = m_buckets.begin() + ibucket_for_key + 1;
        while(it_next != m_buckets.end() && it_next->is_empty()) {
            ++it_next;
        }
        
        return iterator(it_next, m_buckets.end(), m_overflow_elements.begin()); 
    }
    
        
    template <typename P, class... Args>
    std::pair<iterator, bool> try_emplace_internal(P&& key, Args&&... args_value) {
        const std::size_t ibucket_for_hash = bucket_for_hash(m_hash(key));
        
        // Check if already presents
        auto it_find = find_internal(key, m_buckets.begin() + ibucket_for_hash);
        if(it_find != end()) {
            return std::make_pair(it_find, false);
        }
        
        // TODO We copy the key two times, once here and then later to copy it in bucket. Optimize.
        return insert_internal(value_type(std::piecewise_construct, 
                                          std::forward_as_tuple(std::forward<P>(key)), 
                                          std::forward_as_tuple(std::forward<Args>(args_value)...)), 
                               ibucket_for_hash);
    }
    
    template<typename P>
    std::pair<iterator, bool> insert_internal(P&& key_value) {
        const std::size_t ibucket_for_hash = bucket_for_hash(m_hash(key_value.first));
        
        // Check if already presents
        auto it_find = find_internal(key_value.first, m_buckets.begin() + ibucket_for_hash);
        if(it_find != end()) {
            return std::make_pair(it_find, false);
        }
        
        
        return insert_internal(std::forward<P>(key_value), ibucket_for_hash);
    }
    
    template<typename P>
    std::pair<iterator, bool> insert_internal(P&& key_value, std::size_t ibucket_for_hash) {
        assert(!m_buckets.empty());
        
        if((m_nb_elements + 1) > m_load_threshold) {
            rehash(m_buckets.size() * REHASH_SIZE_MULTIPLICATION_FACTOR);
            ibucket_for_hash = bucket_for_hash(m_hash(key_value.first));
        }
        
        std::size_t ibucket_empty = find_empty_bucket(ibucket_for_hash);
        if(ibucket_empty < m_buckets.size()) {
            do {
                // Empty bucket is in range of NeighborhoodSize, use it
                if(ibucket_empty - ibucket_for_hash < NeighborhoodSize) {
                    auto it = insert_in_bucket(std::forward<P>(key_value), ibucket_empty, ibucket_for_hash);
                    return std::make_pair(iterator(it, m_buckets.end(), m_overflow_elements.begin()), true);
                }
            }
            // else, try to swap values to get a closer empty bucket
            while(swap_empty_bucket_closer(ibucket_empty));
            
            
            // A rehash will not change the neighborhood, put the value in overflow list
            if(!will_neighborhood_change_on_rehash(ibucket_for_hash)) {
                m_overflow_elements.push_back(std::forward<P>(key_value));
                m_buckets[ibucket_for_hash].set_overflow(true);
                m_nb_elements++;
                
                return std::make_pair(iterator(m_buckets.end(), m_buckets.end(), std::prev(m_overflow_elements.end())), true);
            }
            
            
        }
    
        rehash(m_buckets.size() * REHASH_SIZE_MULTIPLICATION_FACTOR);
        
        ibucket_for_hash = bucket_for_hash(m_hash(key_value.first));
        return insert_internal(std::forward<P>(key_value), ibucket_for_hash);
    }    
    
    /*
     * Return true if a rehash will change the position of a key-value in the neighborhood of ibucket_neighborhood_check.
     * In this case a rehash is needed instead of puting the value in overflow list.
     */
    bool will_neighborhood_change_on_rehash(size_t ibucket_neighborhood_check) const {
        for(size_t ibucket = ibucket_neighborhood_check; 
            ibucket < m_buckets.size() && (ibucket - ibucket_neighborhood_check) < NeighborhoodSize; 
            ++ibucket)
        {
            assert(!m_buckets[ibucket].is_empty());
            
            const value_type & key_value = m_buckets[ibucket].get_key_value();
            const size_t hash = m_hash(key_value.first);
            
            if((hash & (m_buckets.size() - 1)) != (hash & (m_buckets.size() * REHASH_SIZE_MULTIPLICATION_FACTOR - 1))) {
                return true;
            }
        }
        
        return false;
    }
    
    /*
     * Return the index of an empty bucket in m_buckets.
     * If none, the returned index equals m_buckets.size()
     */
    std::size_t find_empty_bucket(std::size_t ibucket_start) const {
        for(; ibucket_start < m_buckets.size(); ibucket_start++) {
            if(m_buckets[ibucket_start].is_empty()) {
                break;
            }
        }
        
        return ibucket_start;
    }
    
    /*
     * Insert key_value in ibucket_empty for key_value originally belonging to ibucket_for_hash
     * 
     * Return bucket iterator to ibucket_empty
     */
    template<typename P>
    typename std::vector<hopscotch_bucket>::iterator insert_in_bucket(P && key_value, std::size_t ibucket_empty, std::size_t ibucket_for_hash) {        
        assert(ibucket_empty >= ibucket_for_hash );
        assert(m_buckets[ibucket_empty].is_empty());
        m_buckets[ibucket_empty].set_key_value(std::forward<P>(key_value));
        
        assert(!m_buckets[ibucket_for_hash].is_empty());
        m_buckets[ibucket_for_hash].toggle_neighbor_presence(ibucket_empty - ibucket_for_hash);
        m_nb_elements++;
        
        return m_buckets.begin() + ibucket_empty;
    }
    
    /*
     * Try to swap the bucket ibucket_empty_in_out with a bucket preceding it while keeping the neighborhood conditions correct.
     * 
     * If a swap was possible, the position of ibucket_empty_in_out will be closer to 0 and true will re returned.
     */
    bool swap_empty_bucket_closer(std::size_t & ibucket_empty_in_out) {
        assert(ibucket_empty_in_out >= NeighborhoodSize);
        const std::size_t neighborhood_start = ibucket_empty_in_out - NeighborhoodSize + 1;
        
        for(std::size_t to_check = neighborhood_start; to_check < ibucket_empty_in_out; to_check++) {
            neighborhood_bitmap neighborhood_infos = m_buckets[to_check].get_neighborhood_infos();
            std::size_t to_swap = to_check;
            
            while(neighborhood_infos != 0 && to_swap < ibucket_empty_in_out) {
                if((neighborhood_infos & 1) == 1) {
                    assert(m_buckets[ibucket_empty_in_out].is_empty());
                    assert(!m_buckets[to_swap].is_empty());
                    
                    m_buckets[to_swap].swap_key_value_with_empty_bucket(m_buckets[ibucket_empty_in_out]);
                    
                    assert(!m_buckets[to_check].check_neighbor_presence(ibucket_empty_in_out - to_check));
                    assert(m_buckets[to_check].check_neighbor_presence(to_swap - to_check));
                    
                    m_buckets[to_check].toggle_neighbor_presence(ibucket_empty_in_out - to_check);
                    m_buckets[to_check].toggle_neighbor_presence(to_swap - to_check);
                    
                    
                    ibucket_empty_in_out = to_swap;
                    
                    return true;
                }
                
                to_swap++;
                neighborhood_infos = static_cast<neighborhood_bitmap>(neighborhood_infos >> 1);
            }
        }
        
        return false;
    }
    
    typename std::vector<hopscotch_bucket>::iterator get_first_non_empty_buckets_iterator() {
        return m_buckets.begin() + std::distance(m_buckets.cbegin(), static_cast<const hopscotch_map*>(this)->get_first_non_empty_buckets_iterator()); 
    }
    
    typename std::vector<hopscotch_bucket>::const_iterator get_first_non_empty_buckets_iterator() const {
        auto begin = m_buckets.cbegin();
        while(begin != m_buckets.cend() && begin->is_empty()) {
            ++begin;
        }
        
        return begin;
    }
    
    iterator find_internal(const Key& key, typename std::vector<hopscotch_bucket>::iterator it_bucket) {
        auto it = find_in_buckets(key, it_bucket);
        if(it != m_buckets.cend()) {
            return iterator(it, m_buckets.end(), m_overflow_elements.begin());
        }
        
        if(!it_bucket->has_overflow()) {
            return end();
        }
        
        return iterator(m_buckets.end(), m_buckets.end(), std::find_if(m_overflow_elements.begin(), m_overflow_elements.end(), 
                                                                        [&](const value_type & value) { 
                                                                            return m_key_equal(key, value.first); 
                                                                        }));
    }
    
    const_iterator find_internal(const Key& key, typename std::vector<hopscotch_bucket>::const_iterator it_bucket) const {
        auto it = find_in_buckets(key, it_bucket);
        if(it != m_buckets.cend()) {
            return const_iterator(it, m_buckets.cend(), m_overflow_elements.cbegin());
        }
        
        if(!it_bucket->has_overflow()) {
            return cend();
        }

        
        return const_iterator(m_buckets.cend(), m_buckets.cend(), std::find_if(m_overflow_elements.cbegin(), m_overflow_elements.cend(), 
                                                                                [&](const value_type & value) { 
                                                                                    return m_key_equal(key, value.first); 
                                                                                }));        
    }
    
    typename std::vector<hopscotch_bucket>::iterator find_in_buckets(const Key& key, typename std::vector<hopscotch_bucket>::iterator it_bucket) {      
        return m_buckets.begin() + std::distance(m_buckets.cbegin(), static_cast<const hopscotch_map*>(this)->find_in_buckets(key, it_bucket)); 
    }

    
    typename std::vector<hopscotch_bucket>::const_iterator find_in_buckets(const Key& key, typename std::vector<hopscotch_bucket>::const_iterator it_bucket) const {      
        // TODO Try to optimize the function. 
        // I tried to use ffs and  __builtin_ffs functions but I could not reduce the time the function takes with -march=native
        
        neighborhood_bitmap neighborhood_infos = it_bucket->get_neighborhood_infos();
        while(neighborhood_infos != 0) {
            if((neighborhood_infos & 1) == 1) {
                if(m_key_equal(it_bucket->get_key_value().first, key)) {
                    return it_bucket;
                }
            }
            
            ++it_bucket;
            neighborhood_infos = static_cast<neighborhood_bitmap>(neighborhood_infos >> 1);
        }
        
        return m_buckets.end();
    }
    
    static constexpr bool is_power_of_two(size_t value) {
        return value != 0 && (value & (value - 1)) == 0;
    }
    
private:    
    static const std::size_t DEFAULT_INIT_BUCKETS_SIZE = 16;
    static const std::size_t REHASH_SIZE_MULTIPLICATION_FACTOR = 2;
    static constexpr float DEFAULT_MAX_LOAD_FACTOR = 0.9f;

    
    /*
     * m_buckets.size() should be a power of 2. We can then use "hash & (m_buckets.size - 1)" 
     * to get the bucket for a hash
     */
    static_assert(is_power_of_two(DEFAULT_INIT_BUCKETS_SIZE), "DEFAULT_INIT_BUCKETS_SIZE should be a power of 2.");
    std::vector<hopscotch_bucket> m_buckets;
    std::list<value_type> m_overflow_elements;
    
    std::size_t m_nb_elements;
    
    
    float m_max_load_factor;
    std::size_t m_load_threshold;
    
    hasher m_hash;
    key_equal m_key_equal;
};

template<class Key, class T, class Hash, class KeyEqual, unsigned int NeighborhoodSize>
inline bool operator==(const hopscotch_map<Key, T, Hash, KeyEqual, NeighborhoodSize>& lhs, 
                       const hopscotch_map<Key, T, Hash, KeyEqual, NeighborhoodSize>& rhs)
{
    if(lhs.size() != rhs.size()) {
        return false;
    }
    
    for(const auto & element_lhs : lhs) {
        const auto it_element_rhs = rhs.find(element_lhs.first);
        if(it_element_rhs == rhs.cend() || element_lhs.second != it_element_rhs->second) {
            return false;
        }
    }
    
    return true;
}


template<class Key, class T, class Hash, class KeyEqual, unsigned int NeighborhoodSize>
inline bool operator!=(const hopscotch_map<Key, T, Hash, KeyEqual, NeighborhoodSize>& lhs, 
                       const hopscotch_map<Key, T, Hash, KeyEqual, NeighborhoodSize>& rhs)
{
    return !operator==(lhs, rhs);
}

#endif
