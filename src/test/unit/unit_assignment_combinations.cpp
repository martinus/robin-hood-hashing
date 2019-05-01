#include <robin_hood.h>

#include <app/doctest.h>

TYPE_TO_STRING(robin_hood::unordered_flat_map<uint64_t, uint64_t>);
TYPE_TO_STRING(robin_hood::unordered_node_map<uint64_t, uint64_t>);

TEST_CASE_TEMPLATE("assignment combinations", Map,
                   robin_hood::unordered_flat_map<uint64_t, uint64_t>,
                   robin_hood::unordered_node_map<uint64_t, uint64_t>) {
    {
        Map a;
        Map b;
        b = a;
    }
    {
        Map a;
        Map const& aConst = a;
        Map b;
        a[123] = 321;
        b = a;

        REQUIRE(a.find(123)->second == 321);
        REQUIRE(aConst.find(123)->second == 321);

        REQUIRE(b.find(123)->second == 321);
        a[123] = 111;
        REQUIRE(a.find(123)->second == 111);
        REQUIRE(aConst.find(123)->second == 111);
        REQUIRE(b.find(123)->second == 321);
        b[123] = 222;
        REQUIRE(a.find(123)->second == 111);
        REQUIRE(aConst.find(123)->second == 111);
        REQUIRE(b.find(123)->second == 222);
    }

    {
        Map a;
        Map b;
        a[123] = 321;
        a.clear();
        b = a;

        REQUIRE(a.size() == 0);
        REQUIRE(b.size() == 0);
    }

    {
        Map a;
        Map b;
        b[123] = 321;
        b = a;

        REQUIRE(a.size() == 0);
        REQUIRE(b.size() == 0);
    }
    {
        Map a;
        Map b;
        b[123] = 321;
        b.clear();
        b = a;

        REQUIRE(a.size() == 0);
        REQUIRE(b.size() == 0);
    }
    {
        Map a;
        a[1] = 2;
        Map b;
        b[3] = 4;
        b = a;

        REQUIRE(a.size() == 1);
        REQUIRE(b.size() == 1);
        REQUIRE(b.find(1)->second == 2);
        a[1] = 123;
        REQUIRE(a.size() == 1);
        REQUIRE(b.size() == 1);
        REQUIRE(b.find(1)->second == 2);
    }
    {
        Map a;
        a[1] = 2;
        a.clear();
        Map b;
        REQUIRE(a == b);
        b[3] = 4;
        REQUIRE(a != b);
        b = a;
        REQUIRE(a == b);
    }
    {
        Map a;
        a[1] = 2;
        Map b;
        REQUIRE(a != b);
        b[3] = 4;
        b.clear();
        REQUIRE(a != b);
        b = a;
        REQUIRE(a == b);
    }
    {
        Map a;
        a[1] = 2;
        a.clear();
        Map b;
        b[3] = 4;
        REQUIRE(a != b);
        b.clear();
        REQUIRE(a == b);
        b = a;
        REQUIRE(a == b);
    }
}
