#include <robin_hood.h>

#include <app/checksum.h>
#include <app/doctest.h>

#include <string>

TYPE_TO_STRING(robin_hood::unordered_flat_map<std::string, uint64_t>);
TYPE_TO_STRING(robin_hood::unordered_node_map<std::string, uint64_t>);

TEST_CASE_TEMPLATE("reserve_and_assign", Map, robin_hood::unordered_flat_map<std::string, uint64_t>,
                   robin_hood::unordered_node_map<std::string, uint64_t>) {
    Map a = {
        {"button", {}},
        {"selectbox-tr", {}},
        {"slidertrack-t", {}},
        {"sliderarrowinc-hover", {}},
        {"text-l", {}},
        {"title-bar-l", {}},
        {"checkbox-checked-hover", {}},
        {"datagridexpand-active", {}},
        {"icon-waves", {}},
        {"sliderarrowdec-hover", {}},
        {"datagridexpand-collapsed", {}},
        {"sliderarrowinc-active", {}},
        {"radio-active", {}},
        {"radio-checked", {}},
        {"selectvalue-hover", {}},
        {"huditem-l", {}},
        {"datagridexpand-collapsed-active", {}},
        {"slidertrack-b", {}},
        {"selectarrow-hover", {}},
        {"window-r", {}},
        {"selectbox-tl", {}},
        {"icon-score", {}},
        {"datagridheader-r", {}},
        {"icon-game", {}},
        {"sliderbar-c", {}},
        {"window-c", {}},
        {"datagridexpand-hover", {}},
        {"button-hover", {}},
        {"icon-hiscore", {}},
        {"sliderbar-hover-t", {}},
        {"sliderbar-hover-c", {}},
        {"selectarrow-active", {}},
        {"window-tl", {}},
        {"checkbox-active", {}},
        {"sliderarrowdec-active", {}},
        {"sliderbar-active-b", {}},
        {"sliderarrowdec", {}},
        {"window-bl", {}},
        {"datagridheader-l", {}},
        {"sliderbar-t", {}},
        {"sliderbar-active-t", {}},
        {"text-c", {}},
        {"window-br", {}},
        {"huditem-c", {}},
        {"selectbox-l", {}},
        {"icon-flag", {}},
        {"sliderbar-hover-b", {}},
        {"icon-help", {}},
        {"selectvalue", {}},
        {"title-bar-r", {}},
        {"sliderbar-active-c", {}},
        {"huditem-r", {}},
        {"radio-checked-active", {}},
        {"selectbox-c", {}},
        {"selectbox-bl", {}},
        {"icon-invader", {}},
        {"checkbox-checked-active", {}},
        {"slidertrack-c", {}},
        {"sliderarrowinc", {}},
        {"checkbox", {}},
    };

    Map b;
    b = a;
    REQUIRE(b.find("button") != b.end()); // Passes OK.

    Map c;
    c.reserve(51);
    c = a;
    REQUIRE(checksum::map(a) == checksum::map(c));
    REQUIRE(c.find("button") != c.end()); // Fails.
}
