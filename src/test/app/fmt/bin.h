#ifndef FMT_BIN_H
#define FMT_BIN_H

#include <fmt/streamstate.h>

#include <iomanip>
#include <iostream>

namespace fmt {

template <typename T>
struct Hex {
    T value;
};

template <typename T>
Hex<T> hex(T value) {
    return Hex<T>{value};
}

template <typename T>
std::ostream& operator<<(std::ostream& os, Hex<T> const& h) {
    streamstate ss(os);
    os << std::setfill('0') << std::setw(sizeof(T) * 2) << std::hex << h.value;
    return os;
}

} // namespace fmt

#endif
