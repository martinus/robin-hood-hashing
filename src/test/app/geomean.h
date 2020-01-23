#ifndef APP_GEOMEAN_H
#define APP_GEOMEAN_H

#include <cmath>

template <typename It, typename Op>
double geomean(It begin, It end, Op op) {
    double sum = 0.0;
    size_t count = 0;
    while (begin != end) {
        sum += std::log(op(*begin));
        ++begin;
        ++count;
    }

    sum /= static_cast<double>(count);
    return std::exp(sum);
}

template <typename Container, typename Op>
double geomean(Container&& c, Op op) {
    return geomean(std::begin(c), std::end(c), std::move(op));
}

#endif
