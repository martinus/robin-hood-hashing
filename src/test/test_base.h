#pragma once

#include "catch.hpp"
#include "robin_hood.h"
#include "sfc64.h"

using FlatMap = robin_hood::flat_map<int, int>;
using NodeMap = robin_hood::node_map<int, int>;
using Rng = sfc64;