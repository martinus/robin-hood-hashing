#include "avalanche.h"
#include "hashes.h"
#include "test_base.h"

#include <bitset>
#include <list>
#include <map>
#include <tuple>
#include <unordered_map>

struct ConfigurableCounterHash {
    // 234679895032 masksum, 1.17938e+06 geomean for 0xbdcbaec81634e906 0xa309d159626eef52
    ConfigurableCounterHash()
        : m_values{{UINT64_C(0x5e1caf9535ce6811)}} {}

    ConfigurableCounterHash(ConfigurableCounterHash&& o) = default;
    ConfigurableCounterHash(ConfigurableCounterHash const& o) = default;

    ConfigurableCounterHash& operator=(ConfigurableCounterHash&& o) {
        m_values = std::move(o.m_values);
        return *this;
    }

    // 167079903232 masksum, 133853041 ops best: 0xfa2f2eef662c03e7
    // 167079903232 masksum, 133660376 ops best: 0x8127f0f4be8afcc9
    size_t operator()(size_t const& obj) const {
#if defined(ROBIN_HOOD_HAS_UMUL128)
        // 167079903232 masksum, 120428523 ops best: 0xde5fb9d2630458e9
        uint64_t h;
        uint64_t l = robin_hood::detail::umul128(obj, m_values[0], &h);
        auto result = h + l;
#elif ROBIN_HOOD(BITNESS) == 32
        uint64_t const r = obj * m_values[0];
        uint32_t h = static_cast<uint32_t>(r >> 32);
        uint32_t l = static_cast<uint32_t>(r);
        auto result = h + l;
#else
        // murmurhash 3 finalizer
        uint64_t h = obj;
        h ^= h >> 33;
        h *= 0xff51afd7ed558ccd;
        h ^= h >> 33;
        h *= 0xc4ceb9fe1a85ec53;
        h ^= h >> 33;
        auto h = static_cast<uint64_t>(m);
        auto l = static_cast<uint64_t>(m >> 64);
        auto result = h + l;
#endif
        return result >> m_shift;
    }

    size_t operator()(Counter const& c) const {
        return operator()(c.getForHash());
    }

    std::array<uint64_t, 1> m_values;
    int m_shift = 0;
};

template <typename A>
void eval(int const iters, A current_values, uint64_t& current_mask_sum,
          uint64_t& current_ops_sum) {
    using Map = robin_hood::unordered_flat_map<Counter, uint64_t, ConfigurableCounterHash,
                                               std::equal_to<Counter>, 95>;
    try {
        Rng rng(static_cast<uint64_t>(iters) * 0x135ff36020fe7455);
        /*
        Rng rng{0x405f2f9cff6e0d8f, 0x0f97ae53d08500ea, 0x91e06131913057c3, 13 + iters * 0};
        current_values[0] = UINT64_C(14708910760473355443);
        current_values[1] = UINT64_C(11794246526519903291);

        std::cout << rng << std::endl;
        for (auto x : current_values) {
                std::cout << x << " ";
        }
        std::cout << std::endl;
        */
        // RandomBool rbool;
        Counter::Counts counts;
        size_t const num_iters = 33000;

        {
            // this tends to be very slow because of lots of shifts
            Map map;
            map.m_values = current_values;
            map.m_shift = iters;

            for (size_t n = 2; n < 10000; n += (500 * 10000 / num_iters)) {
                for (size_t i = 0; i < 500; ++i) {
                    map.emplace(Counter{rng.uniform<size_t>(n * 10), counts}, i);
                    current_mask_sum += map.mask();
                    map.erase(Counter{rng.uniform<size_t>(n * 10), counts});
                }
            }
        }

        {
            Map map;
            map.m_values = current_values;
            map.m_shift = iters;

            for (size_t i = 0; i < num_iters; ++i) {
                map.emplace(Counter{rng.uniform<size_t>(i + 1), counts}, i);
                current_mask_sum += map.mask();
                map.erase(Counter{rng.uniform<size_t>(i + 1), counts});
            }
        }

        {
            Map map;
            map.m_values = current_values;
            map.m_shift = iters;

            for (size_t i = 0; i < num_iters; ++i) {
                map.emplace(std::piecewise_construct,
                            std::forward_as_tuple(rng.uniform<size_t>(), counts),
                            std::forward_as_tuple(i));
                current_mask_sum += map.mask();
            }
        }

        {
            Map map;
            map.m_values = current_values;
            map.m_shift = iters;

            for (size_t i = 0; i < num_iters; ++i) {
                map.emplace(std::piecewise_construct,
                            std::forward_as_tuple(
                                rng.uniform<size_t>(10000) << (ROBIN_HOOD(BITNESS) / 2), counts),
                            std::forward_as_tuple(i));
                current_mask_sum += map.mask();
                map.erase(Counter{rng.uniform<size_t>(10000) << (ROBIN_HOOD(BITNESS) / 2), counts});
            }
        }
        {
            Map map;
            map.m_values = current_values;
            map.m_shift = iters;

            static size_t const max_shift = ROBIN_HOOD(BITNESS) - 8;
            static size_t const min_shift = 1;
            static size_t const actual_iters = num_iters / (max_shift - min_shift);
            for (size_t i = 0; i < actual_iters; ++i) {
                for (size_t sh = 1; sh < max_shift; ++sh) {
                    map.emplace(Counter{rng.uniform<size_t>(100000) << sh, counts}, i);
                    current_mask_sum += map.mask();
                    map.erase(Counter{rng.uniform<size_t>(100000) << sh, counts});
                }
            }
        }
        {
            // just sequential insertion
            Map map;
            map.m_values = current_values;
            map.m_shift = iters;

            for (size_t i = 0; i < num_iters; ++i) {
                map.emplace(std::piecewise_construct, std::forward_as_tuple(i, counts),
                            std::forward_as_tuple(i));
                current_mask_sum += map.mask();
            }
        }

        {
            // sequential shifted
            Map map;
            map.m_values = current_values;
            map.m_shift = iters;

            for (size_t i = 0; i < num_iters; ++i) {
                map.emplace(std::piecewise_construct,
                            std::forward_as_tuple(i << ROBIN_HOOD(BITNESS) / 2, counts),
                            std::forward_as_tuple(i));
                current_mask_sum += map.mask();
            }
        }

        {
            Map map;
            map.m_values = current_values;
            map.m_shift = iters;

            for (size_t i = 1; i <= 100; ++i) {
                for (size_t j = 0; j < 1; ++j) {
                    map.emplace(std::piecewise_construct,
                                std::forward_as_tuple(rng.uniform<size_t>(), counts),
                                std::forward_as_tuple(i));
                    current_mask_sum += map.mask();
                }

                for (size_t j = 0; j < num_iters / 1000; ++j) {
                    map.count(Counter{rng.uniform<size_t>(), counts});
                }
            }
        }

        current_ops_sum += counts.moveAssign + counts.moveCtor + counts.equals + counts.hash;
    } catch (std::overflow_error const&) {
        current_mask_sum += (std::numeric_limits<uint64_t>::max)() / 1000;
        current_ops_sum += (std::numeric_limits<uint64_t>::max)() / 1000;
    }
}

#if defined(ROBIN_HOOD_UMULH)

bool ge(uint64_t mask_a, uint64_t ops_a, uint64_t mask_b, uint64_t ops_b) {
    // return std::log(mask_a) + std::log(ops_a) <= std::log(mask_b) + std::log(ops_b);
    return std::tie(mask_a, ops_a) <= std::tie(mask_b, ops_b);
}

TEST_CASE("quickmixoptimizer", "[!hide]") {
    Rng factorRng(std::random_device{}());
    RandomBool rbool;

    using Map = robin_hood::unordered_flat_map<Counter, Counter, ConfigurableCounterHash,
                                               std::equal_to<Counter>, 95>;
    Map startup_map;
    auto best_values = startup_map.m_values;
    auto global_best_values = best_values;

    std::cout << "initializing with random data" << std::endl;
    for (size_t i = 0; i < best_values.size(); ++i) {
        best_values[i] = factorRng();
    }

    uint64_t best_mask_sum = (std::numeric_limits<uint64_t>::max)();
    uint64_t best_ops_sum = (std::numeric_limits<uint64_t>::max)();

    uint64_t global_best_mask_sum = best_mask_sum;
    uint64_t global_best_ops_sum = best_ops_sum;

    auto current_values = best_values;
    int num_unsuccessful_tries = 0;
    while (true) {
        uint64_t current_mask_sum = 0;
        uint64_t current_ops_sum = 0;
#    pragma omp parallel for reduction(+ : current_mask_sum, current_ops_sum)
        for (int iters = 0; iters < 40; ++iters) {
            eval(iters, current_values, current_mask_sum, current_ops_sum);
        }
        // std::cout << ".";
        // std::cout.flush();

        ++num_unsuccessful_tries;

        // also assign when we are equally good, should lead to a bit more exploration
        if (ge(current_mask_sum, current_ops_sum, best_mask_sum, best_ops_sum) &&
            (current_values != best_values)) {

            best_mask_sum = current_mask_sum;
            best_ops_sum = current_ops_sum;
            best_values = current_values;

            if (ge(best_mask_sum, best_ops_sum, global_best_mask_sum, global_best_ops_sum) &&
                (global_best_values != best_values)) {

                global_best_mask_sum = best_mask_sum;
                global_best_ops_sum = best_ops_sum;
                global_best_values = best_values;

                Avalanche a;
                a.eval(5000, [&global_best_values](uint64_t h) {
                    ConfigurableCounterHash hasher;
                    hasher.m_values = global_best_values;
                    hasher.m_shift = 0;
                    return static_cast<size_t>(hasher(h));
                });
                a.save("quickmixoptimizer.ppm");
            }

            num_unsuccessful_tries = 0;

            std::cout << std::endl
                      << std::dec << global_best_mask_sum << " masksum, " << global_best_ops_sum
                      << " ops best: ";
            for (auto const x : global_best_values) {
                std::cout << hex(64) << x << " ";
            }

            std::cout << "  |  " << std::dec << best_mask_sum << " masksum, " << best_ops_sum
                      << " ops current: ";
            for (auto const x : best_values) {
                std::cout << hex(64) << x << " ";
            }

            std::cout << std::endl;
        }

        if (num_unsuccessful_tries == 800) {
            std::cout << "reinint after " << num_unsuccessful_tries << " tries" << std::endl;
            for (size_t i = 0; i < current_values.size(); ++i) {
                current_values[i] = factorRng();
            }
            best_values = current_values;
            best_mask_sum = (std::numeric_limits<uint64_t>::max)();
            best_ops_sum = (std::numeric_limits<uint64_t>::max)();
            num_unsuccessful_tries = 0;
        } else {
            // mutate *after* evaluation & setting best, so initial value is tried too
            current_values = best_values;
            mutate(current_values, factorRng, rbool);
        }
    }
}

#endif
