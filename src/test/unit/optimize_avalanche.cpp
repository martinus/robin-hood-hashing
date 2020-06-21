#include <robin_hood.h>

#include <app/avalanche.h>
#include <app/doctest.h>
#include <app/fmt/hex.h>
#include <app/sfc64.h>

#include <array>
#include <fstream>
#include <iomanip>
#include <iostream>

TEST_CASE("avalanche image generation" * doctest::test_suite("show") * doctest::skip()) {
    sfc64 rng;

    Avalanche a;
    a.eval(1000, [](uint64_t h) { return robin_hood::hash<uint64_t>{}(h); });
    a.save("robin_hood_hash_uint64_t.ppm");
}

#if ROBIN_HOOD(BITNESS) == 64
#    if 0

namespace {

// Mutates input
template <size_t S>
void mutate(std::array<uint64_t, S>* vals, sfc64* rng, RandomBool* rbool) {
    do {
        uint64_t mask = 0;
        do {
            mask |= UINT64_C(1) << ((*rng)(62) + 1);
        } while ((*rng)(3));
        (*vals)[rng->uniform<size_t>(vals->size())] ^= mask;

        // force 1 to lowest byte
    } while ((*rbool)(*rng));
    for (auto& v : *vals) {
        v |= UINT64_C(1);
        v |= (UINT64_C(1) << 63U);
    }
}

} // namespace

TEST_CASE("avalanche optimizer" * doctest::test_suite("optimize") * doctest::skip()) {
    sfc64 rng;
    RandomBool rbool;

    auto factors = std::array<uint64_t, 2>();

    auto best_factors = factors;
    size_t best_rms = (std::numeric_limits<size_t>::max)();

    size_t looper = (std::numeric_limits<size_t>::max)();
    while (0U != looper--) {
        Avalanche a;
        a.eval(1000, [&factors](uint64_t h) {
            // https://github.com/ZilongTan/fast-hash/blob/master/fasthash.c
            // h ^= h >> 23;
            // h *= factors[0];
            // h ^= h >> 33;
            // h *= factors[1];

            uint64_t high;
            auto low = robin_hood::detail::umul128(h, factors[0], &high);
            return high + low;
        });

        auto rms = a.rms();
        if (rms <= best_rms) {
            best_rms = rms;
            best_factors = factors;
            for (auto x : factors) {
                std::cout << "0x" << fmt::hex(x) << " ";
            }
            std::cout << rms << std::endl;
            a.save("avalanching_optimizer.ppm");
        }
        factors = best_factors;
        mutate(&factors, &rng, &rbool);
    }
}
#    endif
#endif
