#ifndef APP_HASH_FNV1A_H
#define APP_HASH_FNV1A_H

#include <cstddef>
#include <cstdint>

namespace hash {

// Visual Studio uses a FNV1a implementation.
template <typename T>
struct FNV1a {
    size_t operator()(T const& key) const {
#if SIZE_MAX == UINT32_MAX
        static constexpr size_t FNV_offset_basis = UINT32_C(2166136261);
        static constexpr size_t FNV_prime = UINT32_C(16777619);
#else
        static constexpr size_t FNV_offset_basis = UINT64_C(14695981039346656037);
        static constexpr size_t FNV_prime = UINT64_C(1099511628211);
#endif
        auto const data = reinterpret_cast<uint8_t const*>(&key);
        size_t val = FNV_offset_basis;
        for (size_t i = 0; i < sizeof(T); ++i) {
            val ^= static_cast<size_t>(data[i]);
            val *= FNV_prime;
        }
        return val;
    }
};

} // namespace hash

#endif
