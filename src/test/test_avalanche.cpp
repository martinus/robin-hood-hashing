#include "avalanche.h"
#include "test_base.h"

#include <array>
#include <fstream>
#include <iomanip>

TEST_CASE("avalanche hash", "[!hide]") {
    Rng rng(std::random_device{}());

    Avalanche a;
    a.eval(1000, [](uint64_t h) { return robin_hood::hash<uint64_t>{}(h); });
    a.save("robin_hood_hash_uint64_t.ppm");
}

#if defined(ROBIN_HOOD_UMULH)
TEST_CASE("avalanche optimizer", "[!hide]") {
    Rng rng(std::random_device{}());
    RandomBool rbool;

    std::array<uint64_t, 2> factors = {};
    auto best_factors = factors;
    size_t best_rms = (std::numeric_limits<size_t>::max)();

    while (true) {
        Avalanche a;
        a.eval(1000, [&factors](uint64_t h) {
            // https://github.com/ZilongTan/fast-hash/blob/master/fasthash.c
            // h ^= h >> 23;
            // h *= factors[0];
            // h ^= h >> 33;
            // h *= factors[1];

            h = ROBIN_HOOD_UMULH(h * factors[0], h * factors[1]);
            return static_cast<size_t>(h);
        });

        auto rms = a.rms();
        if (rms <= best_rms) {
            best_rms = rms;
            best_factors = factors;
            std::cout << std::endl;
            for (auto x : factors) {
                std::cout << hex(64) << x << " ";
            }
            std::cout << std::dec << rms << std::endl;

            a.save("avalanching_optimizer.ppm");
        }
        factors = best_factors;
        mutate(factors, rng, rbool);
    }
}
#endif
