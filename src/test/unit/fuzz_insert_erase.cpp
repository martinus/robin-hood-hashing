#include <robin_hood.h>

#include <app/doctest.h>
#include <app/fmt/hex.h>
#include <app/sfc64.h>

#include <iostream>
#include <sstream>
#include <unordered_map>

std::ostream& operator<<(std::ostream& os, decltype(sfc64{}.state()) const& state);

std::ostream& operator<<(std::ostream& os, decltype(sfc64{}.state()) const& state) {
    char const* prefix = "UINT64_C(0x";
    char const* postfix = ")";
    for (auto const& x : state) {
        os << prefix << fmt::hex(x) << postfix;
        prefix = ", UINT64_C(0x";
    }
    return os;
}

#if ROBIN_HOOD(HAS_EXCEPTIONS)

template <typename T, size_t S>
std::string to_string(std::array<T, S> const& data) {
    std::stringstream ss;
    auto prefix = "";
    for (auto const& x : data) {
        ss << prefix << x;
        prefix = ", ";
    }
    return ss.str();
}

TEST_CASE("fuzz insert erase" * doctest::test_suite("fuzz") * doctest::skip()) {
    sfc64 rng;
    size_t min_ops = 10000;

    size_t it = 0;

    // no endless loop to prevent warning
    size_t trials = (std::numeric_limits<size_t>::max)();
    while (0U != trials--) {
        auto state = rng.state();

        auto const n = rng.uniform<int>(1000) + 1;
        size_t op = 0;

        try {
            std::unordered_map<int, int> suo;
            robin_hood::unordered_node_map<int, int> ruo;

            while (++op < min_ops) {
                if (++it == 1000000) {
                    std::cout << ".";
                    std::cout.flush();
                    it = 0;
                }
                auto const key = rng.uniform<int>(n);
                if (0U != (op & 1U)) {
                    if (suo.erase(key) != ruo.erase(key)) {
                        MESSAGE("error after " << op << " ops, rng{" << to_string(state) << "}");
                        min_ops = op;
                    }
                } else {
                    if (suo.emplace(key, key).second != ruo.emplace(key, key).second) {
                        MESSAGE("error after " << op << " ops, rng{" << to_string(state) << "}");
                        min_ops = op;
                    }
                }
                if (suo.size() != ruo.size()) {
                    MESSAGE("error after " << op << " ops, rng{" << to_string(state) << "}");
                    min_ops = op;
                }
            }

        } catch (std::overflow_error const&) {
            MESSAGE("error after " << op << " ops, rng{" << to_string(state) << "}");
            min_ops = op;
        }
    }
}

#endif
