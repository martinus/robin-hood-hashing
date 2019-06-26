#ifndef APP_CHECKSUM_H
#define APP_CHECKSUM_H

#include <app/CtorDtorVerifier.h>

#include <cstdint>

namespace checksum {

// final step from MurmurHash3
inline uint64_t mix(uint64_t k) {
    k ^= k >> 33;
    k *= 0xff51afd7ed558ccdULL;
    k ^= k >> 33;
    k *= 0xc4ceb9fe1a85ec53ULL;
    k ^= k >> 33;
    return k;
}

inline uint64_t mix(std::string const& data) {
    static constexpr uint64_t FNV_offset_basis = UINT64_C(14695981039346656037);
    static constexpr uint64_t FNV_prime = UINT64_C(1099511628211);

    uint64_t val = FNV_offset_basis;
    for (size_t i = 0; i < data.size(); ++i) {
        val ^= static_cast<uint64_t>(data[i]);
        val *= FNV_prime;
    }
    return val;
}

inline uint64_t mix(CtorDtorVerifier const& cdv) {
    return mix(cdv.val());
}

// from boost::hash_combine, with additional fmix64 of value
inline uint64_t combine(uint64_t seed, uint64_t value) {
    return seed ^ (value + 0x9e3779b9 + (seed << 6) + (seed >> 2));
}

// calculates a hash of any iterable map. Order is irrelevant for the hash's result, as it simply
// xors the elements together.
template <typename M>
uint64_t map(const M& map) {
    uint64_t combined_hash = 1;

    uint64_t numElements = 0;
    for (auto const& entry : map) {
        auto entry_hash = combine(mix(entry.first), mix(entry.second));

        combined_hash ^= entry_hash;
        ++numElements;
    }

    return combine(combined_hash, numElements);
}

// map of maps
template <typename MM>
uint64_t mapmap(const MM& mapmap) {
    uint64_t combined_hash = 1;

    uint64_t numElements = 0;
    for (auto const& entry : mapmap) {
        auto entry_hash = combine(mix(entry.first), map(entry.second));

        combined_hash ^= entry_hash;
        ++numElements;
    }

    return combine(combined_hash, numElements);
}

} // namespace checksum

#endif
