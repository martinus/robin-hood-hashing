#ifndef FMT_HEX_H
#define FMT_HEX_H

#include <bitset>
#include <iostream>

namespace fmt {

template <typename T>
struct Bin {
    T value;
};

template <typename T>
Bin<T> bin(T value) {
    return Bin<T>{value};
}

template <typename T>
std::ostream& operator<<(std::ostream& os, Bin<T> const& b) {
    streamstate ss(os);
    os << std::bitset<sizeof(T) * 8>{b.value};
    return os;
}

} // namespace fmt

#endif
