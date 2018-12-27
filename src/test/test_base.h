#pragma once

#include "catch.hpp"
#include "robin_hood.h"
#include "sfc64.h"

using FlatMap = robin_hood::flat_map<uint64_t, uint64_t>;
using NodeMap = robin_hood::node_map<uint64_t, uint64_t>;
using Rng = sfc64;