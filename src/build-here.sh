#!/usr/bin/env bash
set -e
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER="g++-8" -G Ninja $(dirname "$0")
ninja