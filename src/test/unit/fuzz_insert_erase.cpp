#include <robin_hood.h>

#include <app/doctest.h>
#include <app/fmt/hex.h>
#include <app/sfc64.h>

#include <iostream>
#include <unordered_map>

namespace std {

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
} // namespace std

TEST_CASE_TEMPLATE("fuzz insert erase" * doctest::test_suite("fuzz") * doctest::skip(), Map,
                   robin_hood::unordered_flat_map<int, int>,
                   robin_hood::unordered_node_map<int, int>) {
    size_t min_ops = 10000;
    uint64_t const n = 500;

    sfc64 rng(UINT64_C(0x16eb1b56f8dbc55a), UINT64_C(0x5822255f5bf83d8e),
              UINT64_C(0xa01d66fe68915228), UINT64_C(0x0000000000001a1d));
    while (true) {
        size_t op = 0;
        auto state = rng.state();
        try {
            std::unordered_map<int, int> suo;
            robin_hood::unordered_node_map<int, int> ruo;

            while (++op < min_ops) {
                auto const key = rng.uniform<int>(n);
                if (op & 1) {
                    suo.erase(key);
                    ruo.erase(key);
                } else {
                    suo[key] = key;
                    ruo[key] = key;
                }
                if (suo.size() != ruo.size()) {
                    MESSAGE("error after " << op << " ops, rng{" << state << "}");
                    min_ops = op;
                }
            }

        } catch (std::overflow_error const&) {
            MESSAGE("error after " << op << " ops, rng{" << state << "}");
            min_ops = op;
        }
    }
}
