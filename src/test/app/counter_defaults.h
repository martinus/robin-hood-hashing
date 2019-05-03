#ifndef COUNT_DEFAULTS_H
#define COUNT_DEFAULTS_H

#include <app/Counter.h>
#include <app/doctest.h>

#include <unordered_map>

using cnt = Counter::Obj;

TYPE_TO_STRING(robin_hood::unordered_flat_map<cnt, cnt, std::hash<cnt>>);
TYPE_TO_STRING(robin_hood::unordered_flat_map<cnt, cnt, robin_hood::hash<cnt>>);
TYPE_TO_STRING(robin_hood::unordered_node_map<cnt, cnt, std::hash<cnt>>);
TYPE_TO_STRING(robin_hood::unordered_node_map<cnt, cnt, robin_hood::hash<cnt>>);
TYPE_TO_STRING(std::unordered_map<cnt, cnt, std::hash<cnt>>);
TYPE_TO_STRING(std::unordered_map<cnt, cnt, robin_hood::hash<cnt>>);

#endif
