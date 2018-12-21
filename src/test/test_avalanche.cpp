#include "test_base.h"

#include <array>
#include <iomanip>

template <typename T>
T rotl(T const x, int k) {
	return (x << k) | (x >> (8 * sizeof(T) - k));
}

static uint64_t quickmix(uint64_t h) {
	/* original murmurhash3*/
	h ^= h >> 33;
	h *= 0xff51afd7ed558ccd;
	h ^= h >> 33;
	h *= 0xc4ceb9fe1a85ec53;
	h ^= h >> 33;
	return h;

	// http://zimbry.blogspot.com/2011/09/better-bit-mixing-improving-on.html
	/*
	h ^= h >> 31;
	h *= 0x7fb5d329728ea185;
	h ^= h >> 27;
	h *= 0x81dadef4bc2dd44d;
	h ^= h >> 33;
	return h;
	*/
}

TEST_CASE("avalanching test") {
	size_t num_flips = 10000000;

	Rng rng(123);

	std::array<int, 64 * 64> data = {};

	uint64_t input = 0;

	for (size_t i = 0; i < num_flips; ++i) {
		input = rng();
		uint64_t const output = quickmix(input);

		for (int bit = 0; bit < 64; ++bit) {
			auto const input_flipped = input ^ (UINT64_C(1) << bit);

			// generate output with flipped bit
			auto has_flipped = quickmix(input_flipped) ^ output;
			for (int i = 63; i >= 0; --i) {
				if (has_flipped & 1) {
					++data[bit * 64 + i];
				}
				has_flipped >>= 1;
			}
		}
	}

	for (size_t y = 0; y < 64; ++y) {
		for (size_t x = 0; x < 64; ++x) {
			std::cout << std::dec << std::setfill(' ') << std::setw(3) << (data[y * 64 + x] * 100 / num_flips);
		}
		std::cout << std::endl;
	}

	int const expected_flips = num_flips / 2;
	double sumtotal2 = 0;
	for (size_t y = 0; y < 64 * 64; ++y) {
		double e = data[y] - expected_flips;
		sumtotal2 += e * e;
	}
	auto err = std::sqrt(sumtotal2 / (64 * 64));
	std::cout << err << std::endl;
}