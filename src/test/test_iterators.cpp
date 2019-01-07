#include "test_base.h"

TEMPLATE_TEST_CASE("iterators", "", FlatMap, NodeMap) {
	for (size_t i = 0; i < 10; ++i) {
		TestType m;
		REQUIRE(m.begin() == m.end());

		REQUIRE(m.end() == m.find(132));

		m[1];
		REQUIRE(m.begin() != m.end());
		REQUIRE(++m.begin() == m.end());
		m.clear();

		REQUIRE(m.begin() == m.end());
	}
}
