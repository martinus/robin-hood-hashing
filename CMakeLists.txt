cmake_minimum_required(VERSION 3.5)
project(robin-hood-hashing LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(rh "")
add_subdirectory(cmake)
add_compile_flags_target(rh)
add_subdirectory(src)
