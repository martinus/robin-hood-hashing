#include <app/fmt/mup.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <sstream>

// see http://fmtlib.net/latest/syntax.html

namespace {

std::string fmt(long double v, char symbol) {
    std::stringstream ss{};
    if (v >= 1 && v < 1000) {
        ss << std::fixed;
        if (v >= 100) {
            ss << std::setprecision(1);
        } else if (v >= 10) {
            ss << std::setprecision(2);
        } else {
            ss << std::setprecision(3);
        }
        ss << v;
        if (symbol != ' ') {
            ss << " " << symbol;
        }
    } else {
        ss << v;
    }
    return ss.str();
}

} // namespace

namespace martinus {

std::string mup(int value) {
    return mup("", static_cast<long double>(value));
}
std::string mup(long value) {
    return mup("", static_cast<long double>(value));
}
std::string mup(long long value) {
    return mup("", static_cast<long double>(value));
}
std::string mup(unsigned value) {
    return mup("", static_cast<long double>(value));
}
std::string mup(unsigned long value) {
    return mup("", static_cast<long double>(value));
}
std::string mup(unsigned long long value) {
    return mup("", static_cast<long double>(value));
}
std::string mup(float value) {
    return mup("", static_cast<long double>(value));
}
std::string mup(double value) {
    return mup("", static_cast<long double>(value));
}

std::string mup(char const* format, int value) {
    return mup(format, static_cast<long double>(value));
}
std::string mup(char const* format, long value) {
    return mup(format, static_cast<long double>(value));
}
std::string mup(char const* format, long long value) {
    return mup(format, static_cast<long double>(value));
}
std::string mup(char const* format, unsigned value) {
    return mup(format, static_cast<long double>(value));
}
std::string mup(char const* format, unsigned long value) {
    return mup(format, static_cast<long double>(value));
}
std::string mup(char const* format, unsigned long long value) {
    return mup(format, static_cast<long double>(value));
}
std::string mup(char const* format, float value) {
    return mup(format, static_cast<long double>(value));
}
std::string mup(char const* format, double value) {
    return mup(format, static_cast<long double>(value));
}

struct FP {
    std::string prefix;
};

std::string mup(char const* format, long double value) {
    (void)format;

    // one additional entry which is only used as a sentinel
    std::array<long double, 18> factors = {{1e-24L, 1e-21L, 1e-18L, 1e-15L, 1e-12L, 1e-9L, 1e-6L,
                                            1e-3L, 1e0L, 1e3L, 1e6L, 1e9L, 1e12L, 1e15L, 1e18L,
                                            1e21L, 1e24L, 1e27L}};
    char const* prefix = "yzafpnum kMGTPEZY";

    auto u = std::fabs(value);
    auto pos = std::upper_bound(factors.begin(), factors.end(), u);
    if (u < std::numeric_limits<long double>::lowest() || pos == factors.begin() ||
        pos == factors.end()) {
        pos = factors.begin() + 9;
    }

    if (pos != factors.begin()) {
        --pos;
    }

    auto dist = static_cast<size_t>(std::distance(factors.begin(), pos));

    // std::cout << __FILE__ << "(" << __LINE__ << "): " << value << " " << dist << " '" <<
    // factors[dist] << "' '" << prefix[dist] << "'" << std::endl;
    return fmt(value / factors[dist], prefix[dist]);
}

} // namespace martinus
