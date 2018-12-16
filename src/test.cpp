#include <robin_hood_map.h>
#include <unordered_map>

int bla() {
	std::unordered_map<int, int> uo;
	uo.insert(std::make_pair(1, 2));
	robin_hood::map<int, int> m;
	for (int i = 0; i < 100; ++i) {
		m.insert(std::make_pair(i, i));
	}
	return 0;
}