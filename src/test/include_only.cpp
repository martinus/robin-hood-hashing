#include "robin_hood.h"

// this file makes sure robin_hood.h includes everything it needs

namespace {

static inline void foo() {
	robin_hood::unordered_map<int, int> map;
	map[123] = 43;
	map.clear();
	
	robin_hood::unordered_map<int, int> map2 = map;
}

}