#pragma once

#include <algorithm>
#include <cstdint>
#include <functional>
#include <utility>
#include <vector>

// This is WIP!

namespace ankerl {

template <class Key, class T, class Hash = std::hash<Key>, class Pred = std::equal_to<Key>>
class DenseHashMap {

    struct Bucket {
        static constexpr uint32_t Inc = 256;
        uint32_t info{}; // lower 8 bits are hash, upper 24 bits are offset to original bucket
        uint32_t idx{};  // index into m_values
    };

    using ValuesType = std::vector<std::pair<Key, T>>;

    using value_type = std::pair<Key, T>;
    using iterator = typename ValuesType::iterator;
    using const_iterator = typename ValuesType::const_iterator;
    ValuesType m_values;
    Bucket* m_buckets_start{};
    Bucket* m_buckets_end{};
    uint32_t m_shifts{};
    uint32_t m_size{};
    uint32_t m_num_elements{};

    [[nodiscard]] auto next(Bucket const* bucket) const -> Bucket const* {
        if (++bucket == m_buckets_end) {
            return m_buckets_start;
        }
        return bucket;
    }

    std::pair<uint32_t, Bucket const*> nextWhileLess(size_t hash) const {
        uint64_t h = static_cast<uint64_t>(hash) * UINT64_C(0x9E3779B97F4A7C15);

        // use lowest 8 bit for the info hash
        auto info = Bucket::Inc | (h & UINT64_C(0xFF));

        // use upper bits for the bucket index
        auto const* bucket = m_buckets_start + (h >> m_shifts);
        while (info < bucket->info) {
            ++info;
            bucket = next(bucket);
        }
        return {info, bucket};
    }

    void shiftUp(Bucket* start, Bucket* end) {
        if (end < start) {
            std::move_backward(m_buckets_start, end - 1, end);
            *m_buckets_start = *(m_buckets_end - 1);
            std::move_backward(start, m_buckets_end - 1, m_buckets_end);
        } else {
            std::move_backward(start, end - 1, end);
        }
    }

    template <typename K>
    std::pair<Key, T> const* find(K const& key) const {
        auto [info, bucket] = nextWhileLess(m_hash(key));

        while (info == bucket->info) {
            if (m_equals(key, m_values[bucket->idx].first)) {
                return &m_values[bucket->idx];
            }
            ++info;
            bucket = next(bucket);
        }
        return nullptr;
    }

    template <typename K, typename... Args>
    std::pair<iterator, bool> try_emplace(K&& key, Args&&... args) {
        auto [info, bucket] = nextWhileLess(m_hash(key));

        while (info == bucket->info) {
            if (m_equals(key, m_values[bucket->idx].first)) {
                // key found!
                return std::make_pair(m_values.begin() + bucket->idx, false);
            }

            ++info;
            bucket = next(bucket);
        }

        // key not found, so we are now exactly where we want to insert it
        auto* const insertion_bucket = bucket;

        // emplace the new value. If that throws an exception, no harm done; index is still in a
        // valid state
        m_values.emplace_back(std::piecewise_construct, std::forward_as_tuple(std::forward<K>(key)),
                              std::forward_as_tuple(std::forward<Args>(args)...));

        // place element and shift up until we find an empty spot
        auto tmp = Bucket{info, m_values.size() - 1};
        while (0 != bucket->info) {
            std::swap(tmp, *bucket);
            tmp.info += Bucket::Inc;
            bucket = next(bucket);
        }
        *bucket = tmp;

        ++m_num_elements;
        return std::make_pair(m_values.begin() + bucket->idx, true);
    }

    size_t erase(Key const& key) {
        auto [info, bucket] = nextWhileLess(m_hash(key));

        while (info == bucket->info && !m_equals(key, m_values[bucket->idx].first)) {
            ++info;
            bucket = next(bucket);
        }

        if (info != bucket->info) {
            // not found
            return 0;
        }

        // found it!
    }
};

} // namespace ankerl