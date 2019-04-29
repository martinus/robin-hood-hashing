cmake_minimum_required(VERSION 3.5)
project(robin-hood-hashing LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(robinhood-test
    include/robin_hood.h
    test/avalanche.h
    test/catch.hpp
    test/hashes.h
    test/hash_map.h
    test/sfc64.h
    test/test_base.h
    
    test/include_only.cpp
    test/test_empty.cpp
    test/test_avalanche.cpp
    test/test_iterators.cpp
    test/benchmark.cpp
    test/test_assignments.cpp
    test/test_pair.cpp
    test/test_demo.cpp
    test/test.cpp
    test/test_random_insert_erase.cpp
    test/test_counters.cpp    
    test/test_overflow.cpp
)
target_include_directories( robinhood-test PRIVATE "include" "test")

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang|GNU" )
    target_compile_options( robinhood-test PRIVATE -fdiagnostics-color -ggdb)
    
    # enable lots of warning checking
    target_compile_options(robinhood-test PRIVATE -Werror -Wall -Wextra -Weffc++ -Wconversion -Wunreachable-code -Wuninitialized -Wshadow -Wfloat-equal -Wmissing-braces)

    # make sure OpenMP in the tests work
    target_compile_options(robinhood-test PRIVATE -fopenmp)
    target_link_libraries(robinhood-test PRIVATE -fopenmp)

    #target_compile_options(robinhood-test PRIVATE -fsanitize=undefined,float-divide-by-zero,float-cast-overflow)
    #target_link_libraries(robinhood-test PRIVATE -fsanitize=undefined,float-divide-by-zero,float-cast-overflow)    

    target_compile_options(robinhood-test PRIVATE -fsanitize=address)
    target_link_libraries(robinhood-test PRIVATE -fsanitize=address)    
endif()
if ( CMAKE_CXX_COMPILER_ID MATCHES "MSVC" )
    target_compile_options( vector-test PRIVATE /W4 /w44265 /w44061 /w44062 )
endif()
