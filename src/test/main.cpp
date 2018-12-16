#include <robin_hood.h>

int main(int argc, char** argv) {
	robin_hood::unordered_map<int, int> m;
	m[123] = 321;
	m[42] = 3553;

	for (auto const& kv : m) {
		std::cout << kv.first << " -> " << kv.second << std::endl;
	}
}
