add_subdirectory(app)
add_subdirectory(thirdparty)
add_subdirectory(unit)
target_include_directories(rh PRIVATE ${CMAKE_CURRENT_LIST_DIR})


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

