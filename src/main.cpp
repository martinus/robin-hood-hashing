#include <robin_hood_map.h>
#include <sfc64.h>

int main(int argc, char** argv) {
	robin_hood::map<int, int> m;
	m[123] = 321;
	m[42] = 3553;

	for (auto const& kv : m) {
		std::cout << kv.first << " -> " << kv.second << std::endl;
	}
}
