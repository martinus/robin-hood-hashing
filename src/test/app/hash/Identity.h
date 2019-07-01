#ifndef APP_HASH_IDENTITY_H
#define APP_HASH_IDENTITY_H

#include <cstddef>
#include <cstdint>

namespace hash {

// Linux uses a noop hash
template <typename T>
struct Identity {
    size_t operator()(T const& key) const {
        return static_cast<size_t>(key);
    }
};

template <typename T>
struct Identity<T*> {
    size_t operator()(T* key) const {
        return reinterpret_cast<size_t>(key);
    }
};

} // namespace hash

#endif
