#ifndef TEST_BASE_H
#define TEST_BASE_H

#include "catch.hpp"
#include "robin_hood.h"
#include "sfc64.h"

#include <iomanip>
#include <iostream>

using FlatMap = robin_hood::unordered_flat_map<uint64_t, uint64_t>;
using NodeMap = robin_hood::unordered_node_map<uint64_t, uint64_t>;
using Rng = sfc64;

struct hex {
    explicit hex(int b)
        : bits(b) {}

    int const bits;
};

inline std::ostream& operator<<(std::ostream& os, hex const& h) {
    os << "0x" << std::setfill('0') << std::setw(h.bits * 2 / 8) << std::hex;
    return os;
}

// Mutates input
template <size_t S>
void mutate(std::array<uint64_t, S>& vals, Rng& rng, RandomBool& rbool) {
    do {
        uint64_t mask = 0;
        do {
            mask |= UINT64_C(1) << (rng(62) + 1);
        } while (rng(3));
        vals[rng.uniform<size_t>(vals.size())] ^= mask;

        // force 1 to lowest byte
    } while (rbool(rng));
    for (auto& v : vals) {
        v |= UINT64_C(1);
        v |= (UINT64_C(1) << 63);
    }
#if 0
	do {
		if (rbool(rng)) {
			auto mask_bits = rng(24) + 1;
			uint64_t mask = rng((UINT64_C(1) << mask_bits) - 1) + 1;
			// uint64_t mask = (UINT64_C(1) << mask_bits) - 1;
			vals[rng.uniform<size_t>(vals.size())] ^= mask << rng(64 - mask_bits);
		} else {
			vals[rng.uniform<size_t>(vals.size())] = rng();
		}
	} while (rbool(rng));
#endif
}

#endif
