cmake_minimum_required(VERSION 3.5)
project(robin-hood-hashing LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# find ccache
find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
    SET_PROPERTY(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
    SET_PROPERTY(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
endif(CCACHE_FOUND)

add_executable(robinhood-test
    include/robin_hood.h
    test/thirdparty/doctest/doctest.h

    test/unit/main.cpp # first because its slowest

    test/app/fmt/hex.h
    test/app/fmt/bin.h
    test/app/fmt/streamstate.h
    test/app/fmt/streamstate.cpp
    test/app/benchmark.h
    test/app/benchmark.cpp
    test/app/counter_defaults.h
    test/app/Counter.h
    test/app/Counter.cpp
    test/app/doctest.h
    test/app/sfc64.h

    test/unit/bench_distinctness.cpp
    test/unit/bench_find_random.cpp
    test/unit/bench_hash_int.cpp
    test/unit/bench_hash_string.cpp
    test/unit/bench_iterate.cpp
    test/unit/bench_random_insert_erase.cpp
    test/unit/count_ctor_dtor.cpp
    test/unit/count_ctor_dtor_1.cpp
    test/unit/count_random_insert_erase.cpp
    test/unit/count_find_random.cpp
    test/unit/count_one_emplace.cpp
    test/unit/show_hash.cpp
    test/unit/unit_assignments.cpp
    test/unit/unit_assignment_combinations.cpp
    test/unit/unit_at.cpp
    test/unit/unit_count.cpp
    test/unit/unit_empty.cpp
    test/unit/include_only.h
    test/unit/include_only.cpp
    test/unit/unit_include_only.cpp
    test/unit/unit_initializerlist.cpp
    test/unit/unit_not_copyable.cpp
    test/unit/unit_not_moveable.cpp
    test/unit/unit_reserve.cpp
    test/unit/unit_sizeof.cpp
    test/unit/unit_unique_ptr.cpp
)

target_include_directories( robinhood-test PRIVATE "include" "test" "test/unit" "test/app")

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang|GNU" )
    target_compile_options(robinhood-test PRIVATE -fdiagnostics-color -ggdb -g)
    
    # enable lots of warning checking
    target_compile_options(robinhood-test PRIVATE -Werror -Wall -Wextra -Weffc++ -Wconversion -Wunreachable-code -Wuninitialized -Wshadow -Wfloat-equal -Wmissing-braces)

    # make sure OpenMP in the tests work
    target_compile_options(robinhood-test PRIVATE -fopenmp)
    target_link_libraries(robinhood-test PRIVATE -fopenmp)

    #target_compile_options(robinhood-test PRIVATE -fsanitize=undefined,float-divide-by-zero,float-cast-overflow)
    #target_link_libraries(robinhood-test PRIVATE -fsanitize=undefined,float-divide-by-zero,float-cast-overflow)
    
    #target_compile_options(robinhood-test PRIVATE -fsanitize=address)
    #target_link_libraries(robinhood-test PRIVATE -fsanitize=address)

    #target_compile_options(robinhood-test PRIVATE -fsanitize=memory)
    #target_link_libraries(robinhood-test PRIVATE -fsanitize=memory)

endif()
if ( CMAKE_CXX_COMPILER_ID MATCHES "MSVC" )
    target_compile_options( vector-test PRIVATE /W4 /w44265 /w44061 /w44062 )
endif()
