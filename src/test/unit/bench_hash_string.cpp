#include <robin_hood.h>

#include <app/benchmark.h>
#include <app/doctest.h>
#include <app/fmt/hex.h>
#include <thirdparty/nanobench/nanobench.h>

#if 0
#    if defined(__GNUC__) && !defined(__clang__)
#        pragma GCC diagnostic push
#        pragma GCC diagnostic ignored "-Wcast-align"
#        pragma GCC diagnostic ignored "-Wold-style-cast"
#        pragma GCC diagnostic ignored "-Wundef"
#        pragma GCC diagnostic ignored "-Wsign-conversion"
#        pragma GCC diagnostic ignored "-Wunused-function"
#        define _MSC_VER 0
#        define MEOW_DUMP 0
#    endif

#    include <thirdparty/meow_hash_x64_aesni.h>

#    if defined(__GNUC__) && !defined(__clang__)
#        pragma GCC diagnostic pop
#    endif
#endif

#include <fstream>

namespace {

template <typename Op>
void bench(std::string name, Op&& op) {
    ankerl::nanobench::Rng rng(123);
    std::vector<uint8_t> blob(10000, 'x');
    for (auto& b : blob) {
        b = static_cast<uint8_t>(rng());
    }

    ankerl::nanobench::Bench bench;
    bench.output(nullptr);
    size_t len = 1;
    while (len <= blob.size()) {
        bench.complexityN(len).run(name, [&] {
            ankerl::nanobench::doNotOptimizeAway(op(blob.data(), len));
            blob[0] = static_cast<uint8_t>(blob[0] + UINT8_C(1));
            blob[len / 2] = static_cast<uint8_t>(blob[len / 2] + UINT8_C(2));
            blob[len - 1] = static_cast<uint8_t>(blob[len - 1] + UINT8_C(3));
        });
        len = std::max(len + 1, (len * 101U) / 100U);
    }
    std::ofstream fout(name + ".csv");

    bench.render(
        "\"" + name + "\"\n{{#result}}{{complexityN}}\t{{minimum(cpucycles)}}\n{{/result}}", fout);
}

} // namespace

TEST_CASE("bench_hash_bytes_robin" * doctest::test_suite("nanobench") * doctest::skip()) {
    bench("robin", [](void const* data, size_t len) { return robin_hood::hash_bytes(data, len); });
}

#if ROBIN_HOOD(CXX) >= ROBIN_HOOD(CXX17)

TEST_CASE("bench_hash_bytes_stdhash" * doctest::test_suite("nanobench") * doctest::skip()) {
    bench("stdhash", [](void const* data, size_t len) {
        auto sv = std::string_view{reinterpret_cast<char const*>(data), len};
        return std::hash<std::string_view>{}(sv);
    });
}

#endif

#if 0
TEST_CASE("bench_hash_bytes_xxhash" * doctest::test_suite("nanobench") * doctest::skip()) {
    bench("xxhash", [](void const* data, size_t len) { return XXH64(data, len, 0); });
}

TEST_CASE("bench_hash_bytes_wyhash" * doctest::test_suite("nanobench") * doctest::skip()) {
    bench("wyhash", [](void const* data, size_t len) { return wyhash(data, len, 0, _wyp); });
}

TEST_CASE("bench_hash_bytes_fallback" * doctest::test_suite("nanobench") * doctest::skip()) {
    bench("fallback", [](void const* data, size_t len) {
        return robin_hood::detail::fallback_hash_bytes(data, len);
    });
}

TEST_CASE("bench_hash_bytes_crc32c_hw" * doctest::test_suite("nanobench") * doctest::skip()) {
    bench("crc32c_hw", [](void const* data, size_t len) { return crc32c_hw(0, data, len); });
}

TEST_CASE("bench_hash_bytes_meow" * doctest::test_suite("nanobench") * doctest::skip()) {
    bench("meow", [](void const* data, size_t len) {
        return MeowHash(MeowDefaultSeed, len, const_cast<void*>(data));
    });
}

#endif

#if ROBIN_HOOD(HAS_CRC32)

TEST_CASE("hash_bytes_check_hash_no_collisions") {
    if (!robin_hood::detail::hasCrc32Support()) {
        // no CRC, so we don't use that collision test
    }

    robin_hood::unordered_flat_set<size_t> set;

    size_t size = 3000;

    size_t numCollisions = 0;
    size_t numTotal = 0;
    for (size_t len = 1; len < size; ++len) {
        std::vector<uint8_t> v(len, '\0');

        for (size_t i = 0; i < v.size(); ++i) {
            ++v[i];
            auto h = robin_hood::hash_bytes(v.data(), v.size());
            --v[i];

            ++numTotal;
            auto isCollision = !set.insert(h).second;
            if (isCollision) {
                ++numCollisions;
            }
#    if 0
            std::cout << "len=" << len << ", i=" << i << ", h=" << fmt::hex(h)
                      << (isCollision ? " BANG" : "") << std::endl;

#    endif
        }
    }

    REQUIRE(numCollisions == 0);
    REQUIRE(numTotal == (size * (size - 1)) / 2);
}

TEST_CASE("hash_bytes_check_no_collision_uint64_t") {
    if (!robin_hood::detail::hasCrc32Support()) {
        // no CRC, so we don't use that collision test
    }

    std::vector<uint64_t> data(100, 0);
    size_t numTrials = 100000;
    robin_hood::unordered_flat_set<uint32_t> set;
    for (size_t i = 0; i < numTrials; ++i) {
        auto h = robin_hood::hash_bytes(data.data(), data.size() * sizeof(uint64_t));
        auto lower32 = static_cast<uint32_t>(h);
        set.insert(lower32);
        ++data.back();
    }

    REQUIRE(set.size() == numTrials);
}

#endif
