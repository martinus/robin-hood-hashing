#include "test_base.h"

TEMPLATE_TEST_CASE("find with empty map", "", FlatMap, NodeMap) {
	using Map = TestType;
	Map m;

	REQUIRE(m.end() == m.find(123));
	REQUIRE(m.end() == m.begin());
	m[32];
	REQUIRE(m.end() != m.begin());
	REQUIRE(m.end() == m.find(123));
	REQUIRE(m.end() != m.find(32));

	m = Map();
	REQUIRE(m.end() == m.begin());
	REQUIRE(m.end() == m.find(123));
	REQUIRE(m.end() == m.find(32));

	Map m2(m);
	REQUIRE(m2.end() == m2.begin());
	REQUIRE(m2.end() == m2.find(123));
	REQUIRE(m2.end() == m2.find(32));
	m2[32];
	REQUIRE(m2.end() != m2.begin());
	REQUIRE(m2.end() == m2.find(123));
	REQUIRE(m2.end() != m2.find(32));

	Map mEmpty;
	Map m3(mEmpty);
	REQUIRE(m3.end() == m3.begin());
	REQUIRE(m3.end() == m3.find(123));
	REQUIRE(m3.end() == m3.find(32));
	m3[32];
	REQUIRE(m3.end() != m3.begin());
	REQUIRE(m3.end() == m3.find(123));
	REQUIRE(m3.end() != m3.find(32));
}