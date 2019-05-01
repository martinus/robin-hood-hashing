#ifndef APP_HASH_FNV1A_H
#define APP_HASH_FNV1A_H

#include <cstddef>
#include <cstdint>

namespace hash {

template <typename T>
struct Bad {
    size_t operator()(T const&) const {
        return 0;
    }
};

} // namespace hash

#endif
