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
    std::stringstream ss;
    if (v >= 1 && v < 1000) {
        if (v >= 100) {
            ss << std::setprecision(1);
        } else if (v >= 10) {
            ss << std::setprecision(2);
        } else {
            ss << std::setprecision(3);
        }
        ss << std::fixed << v << " " << symbol;
    } else {
        ss << v << " ";
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
    double factor;
    char prefix;
};

std::string mup(char const* format, long double value) {
    (void)format;

    std::array<long double, 18> factors = {
        1e-24l, 1e-21l, 1e-18l, 1e-15l, 1e-12l, 1e-9l, 1e-6l, 1e-3l, 1e0l,
        1e3l,   1e6l,   1e9l,   1e12l,  1e15l,  1e18l, 1e21l, 1e24l,
        1e27l // one additional entry which is only used as a sentinel
    };
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
    return fmt(value / factors[dist], prefix[dist]);
}

} // namespace martinus
