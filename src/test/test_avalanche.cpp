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

#if ROBIN_HOOD_HAS_UMULH
TEST_CASE("avalanche optimizer", "[!hide]") {
	Rng rng(std::random_device{}());
	RandomBool<> rbool;

	std::array<uint64_t, 2> factors = {0x8f51e0541f93532e, 0xd9f6699f11462b6d};
	auto best_factors = factors;
	double best_rms = (std::numeric_limits<double>::max)();

	while (true) {
		Avalanche a;
		a.eval(100, [&factors](uint64_t h) { return static_cast<size_t>(robin_hood::detail::umulh(factors[0], h * factors[1])); });

		auto rms = std::log(a.percentile()) + std::log(a.rms());
		if (rms <= best_rms) {
			best_rms = rms;
			best_factors = factors;
			std::cout << std::endl << hex(64) << factors[0] << " " << hex(64) << factors[1] << " " << std::dec << rms << std::endl;
			a.save("avalanching_optimizer.ppm");
		}
		factors = best_factors;
		mutate(factors, rng, rbool);
	}
}
#endif