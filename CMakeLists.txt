cmake_minimum_required(VERSION 3.5)
project(robin-hood-hashing LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


add_executable(rh "")
#set_target_properties(rh PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")
#set_target_properties(rh PROPERTIES COMPILE_FLAGS "-m64" LINK_FLAGS "-m64")

add_subdirectory(cmake)
add_compile_flags_target(rh)
add_subdirectory(src)
