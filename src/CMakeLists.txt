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

    test/unit/bench_copy_iterators.cpp
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
    test/unit/unit_clz.cpp
    test/unit/unit_count.cpp
    test/unit/unit_empty.cpp
    test/unit/include_only.h
    test/unit/include_only.cpp
    test/unit/unit_include_only.cpp
    test/unit/unit_initializerlist.cpp
    test/unit/unit_iterators_ctor.cpp
    test/unit/unit_iterators_empty.cpp
    test/unit/unit_iterators_insert.cpp
    test/unit/unit_iterators_stochastic.cpp
    test/unit/unit_load_factor.cpp
    test/unit/unit_not_copyable.cpp
    test/unit/unit_not_moveable.cpp
    test/unit/unit_pair_trivial.cpp
    test/unit/unit_reserve.cpp
    test/unit/unit_sizeof.cpp
    test/unit/unit_unique_ptr.cpp
)

target_include_directories( robinhood-test PRIVATE "include" "test")

macro(add_compiler_flags)
    foreach(flag ${ARGV})
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag}")
    endforeach()
endmacro()

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compiler_flags(-fdiagnostics-color)
    add_compiler_flags(-Werror)
    add_compiler_flags(-pedantic)
    add_compiler_flags(-pedantic-errors)
    add_compiler_flags(-fvisibility=hidden)
    add_compiler_flags(-fstrict-aliasing)
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    add_compiler_flags(-Wall)
    add_compiler_flags(-Wextra)
    add_compiler_flags(-fdiagnostics-show-option)
    add_compiler_flags(-Wconversion)
    add_compiler_flags(-Wold-style-cast)
    add_compiler_flags(-Wfloat-equal)
    add_compiler_flags(-Wlogical-op)
    add_compiler_flags(-Wundef)
    add_compiler_flags(-Wredundant-decls)
    add_compiler_flags(-Wshadow)
    add_compiler_flags(-Wstrict-overflow=5)
    add_compiler_flags(-Wwrite-strings)
    add_compiler_flags(-Wpointer-arith)
    add_compiler_flags(-Wcast-qual)
    add_compiler_flags(-Wformat=2)
    add_compiler_flags(-Wswitch-default)
    add_compiler_flags(-Wmissing-include-dirs)
    add_compiler_flags(-Wcast-align)
    add_compiler_flags(-Wswitch-enum)
    add_compiler_flags(-Wnon-virtual-dtor)
    add_compiler_flags(-Wctor-dtor-privacy)
    add_compiler_flags(-Wsign-conversion)
    add_compiler_flags(-Wdisabled-optimization)
    add_compiler_flags(-Weffc++)
    add_compiler_flags(-Winline)
    add_compiler_flags(-Winvalid-pch)
    add_compiler_flags(-Wmissing-declarations)
    add_compiler_flags(-Woverloaded-virtual)
    if(NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.6)
        add_compiler_flags(-Wnoexcept)
    endif()
    
    # no way to silence it in the expression decomposition macros: _Pragma() in macros doesn't work for the c++ front-end of g++
    # https://gcc.gnu.org/bugzilla/show_bug.cgi?id=55578
    # https://gcc.gnu.org/bugzilla/show_bug.cgi?id=69543
    # Also the warning is completely worthless nowadays - http://stackoverflow.com/questions/14016993
    #add_compiler_flags(-Waggregate-return)
    
    if(NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 5.0)
        add_compiler_flags(-Wdouble-promotion)
        add_compiler_flags(-Wtrampolines)
        add_compiler_flags(-Wzero-as-null-pointer-constant)
        add_compiler_flags(-Wuseless-cast)
        add_compiler_flags(-Wvector-operation-performance)
    endif()
    
    if(NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 6.0)
        add_compiler_flags(-Wshift-overflow=2)
        add_compiler_flags(-Wnull-dereference)
        add_compiler_flags(-Wduplicated-cond)
    endif()
    
    if(NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 7.0)
        add_compiler_flags(-Walloc-zero)
        add_compiler_flags(-Walloca)
        add_compiler_flags(-Wduplicated-branches)
    endif()
    
    if(NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 8.0)
        add_compiler_flags(-Wcast-align=strict)
    endif()
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compiler_flags(-Weverything)
    add_compiler_flags(-Wno-c++98-compat)
    add_compiler_flags(-Wno-c++98-compat-pedantic)
    add_compiler_flags(-Wno-c++98-compat-bind-to-temporary-copy)
    add_compiler_flags(-Wno-c++98-compat-local-type-template-args)
    add_compiler_flags(-Qunused-arguments -fcolor-diagnostics) # needed for ccache integration on travis
endif()

if(MSVC)
    add_compiler_flags(/std:c++latest) # for post c++14 updates in MSVC
    add_compiler_flags(/permissive-)   # force standard conformance - this is the better flag than /Za
    add_compiler_flags(/WX)
    add_compiler_flags(/Wall) # turns on warnings from levels 1 through 4 which are off by default - https://msdn.microsoft.com/en-us/library/23k5d385.aspx
    
    add_compiler_flags(
        /wd4514 # unreferenced inline function has been removed
        /wd4571 # SEH related
        /wd4710 # function not inlined
        /wd4711 # function 'x' selected for automatic inline expansion
        
        /wd4616 # invalid compiler warnings - https://msdn.microsoft.com/en-us/library/t7ab6xtd.aspx
        /wd4619 # invalid compiler warnings - https://msdn.microsoft.com/en-us/library/tacee08d.aspx
        
        #/wd4820 # padding in structs
        #/wd4625 # copy constructor was implicitly defined as deleted
        #/wd4626 # assignment operator was implicitly defined as deleted
        #/wd5027 # move assignment operator was implicitly defined as deleted
        #/wd5026 # move constructor was implicitly defined as deleted
        #/wd4623 # default constructor was implicitly defined as deleted
    )
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang|GNU" )
    #target_compile_options(robinhood-test PRIVATE -fdiagnostics-color -ggdb -g)
    
    # enable lots of warning checking
    #target_compile_options(robinhood-test PRIVATE -Werror -Wall -Wextra -Weffc++ -Wconversion -Wunreachable-code -Wuninitialized -Wshadow -Wfloat-equal -Wmissing-braces)

    # make sure OpenMP in the tests work
    #target_compile_options(robinhood-test PRIVATE -fopenmp)
    #target_link_libraries(robinhood-test PRIVATE -fopenmp)

    #target_compile_options(robinhood-test PRIVATE -fsanitize=undefined,float-divide-by-zero,float-cast-overflow)
    #target_link_libraries(robinhood-test PRIVATE -fsanitize=undefined,float-divide-by-zero,float-cast-overflow)
    
    #target_compile_options(robinhood-test PRIVATE -fsanitize=address)
    #target_link_libraries(robinhood-test PRIVATE -fsanitize=address)

    # warns in doctest.h :(
    #target_compile_options(robinhood-test PRIVATE -fsanitize=memory)
    #target_link_libraries(robinhood-test PRIVATE -fsanitize=memory)

endif()

