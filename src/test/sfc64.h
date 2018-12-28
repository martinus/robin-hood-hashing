#pragma once

#include <array>
#include <cstdint>
#include <limits>
#include <random>
#include <utility>

// this is probably the fastest high quality 64bit random number generator that exists.
// Implements Small Fast Counting v4 RNG from PractRand.
class sfc64 {
public:
	using result_type = uint64_t;

	// no copy ctors so we don't accidentally get the same random again
	sfc64(sfc64 const&) = delete;
	sfc64& operator=(sfc64 const&) = delete;

	sfc64(sfc64&&) = default;
	sfc64& operator=(sfc64&&) = default;

	static constexpr uint64_t(min)() {
		return (std::numeric_limits<uint64_t>::min)();
	}
	static constexpr uint64_t(max)() {
		return (std::numeric_limits<uint64_t>::max)();
	}

	sfc64()
		: sfc64(UINT64_C(0x853c49e6748fea9b)) {}

	sfc64(uint64_t seed)
		: m_a(seed)
		, m_b(seed)
		, m_c(seed)
		, m_counter(1) {
		for (int i = 0; i < 12; ++i) {
			operator()();
		}
	}

	void seed() {
		seed(std::random_device{}());
	}

	void seed(uint64_t seed) {
		state(sfc64{seed}.state());
	}

	uint64_t operator()() noexcept {
		auto const tmp = m_a + m_b + m_counter++;
		m_a = m_b ^ (m_b >> right_shift);
		m_b = m_c + (m_c << left_shift);
		m_c = rotl(m_c, rotation) + tmp;
		return tmp;
	}

	template <typename T>
	T uniform(T i) {
		return static_cast<T>(operator()(i));
	}

	template <typename T>
	T uniform() {
		return static_cast<T>(operator()());
	}

	uint64_t multhi64(uint64_t const a, uint64_t const b) {
		uint64_t a_lo = (uint32_t)a;
		uint64_t a_hi = a >> 32;
		uint64_t b_lo = (uint32_t)b;
		uint64_t b_hi = b >> 32;

		uint64_t a_x_b_hi = a_hi * b_hi;
		uint64_t a_x_b_mid = a_hi * b_lo;
		uint64_t b_x_a_mid = b_hi * a_lo;
		uint64_t a_x_b_lo = a_lo * b_lo;

		uint64_t carry_bit = ((uint64_t)(uint32_t)a_x_b_mid + (uint64_t)(uint32_t)b_x_a_mid + (a_x_b_lo >> 32)) >> 32;

		uint64_t multhi = a_x_b_hi + (a_x_b_mid >> 32) + (b_x_a_mid >> 32) + carry_bit;

		return multhi;
	}

	// Uses the java method. A bit slower than 128bit magic from https://arxiv.org/pdf/1805.10941.pdf, but produces the exact same results in both
	// 32bit and 64 bit.
	uint64_t operator()(uint64_t boundExcluded) noexcept {
#ifdef __SIZEOF_INT128__
		return (uint64_t)(((unsigned __int128)operator()() * (unsigned __int128)boundExcluded) >> 64);
#else
		return multhi64(operator()(), boundExcluded);
#endif
		/*
		uint64_t x;
		uint64_t r;
		do {
			x = operator()();
			r = x % boundExcluded;
		} while (x - r > 0 - boundExcluded);
		return r;
		*/
	}

	std::array<uint64_t, 4> state() const {
		return {m_a, m_b, m_c, m_counter};
	}

	void state(std::array<uint64_t, 4> const& s) {
		m_a = s[0];
		m_b = s[1];
		m_c = s[2];
		m_counter = s[3];
	}

private:
	template <typename T>
	T rotl(T const x, int k) {
		return (x << k) | (x >> (8 * sizeof(T) - k));
	}

	static constexpr int rotation = 24;
	static constexpr int right_shift = 11;
	static constexpr int left_shift = 3;
	uint64_t m_a;
	uint64_t m_b;
	uint64_t m_c;
	uint64_t m_counter;
};
