# see https://cliutils.gitlab.io/modern-cmake/chapters/basics.html
cmake_minimum_required(VERSION 3.1...3.15)

if(${CMAKE_VERSION} VERSION_LESS 3.12)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
endif()


project(MyProject VERSION 1.0
                  DESCRIPTION "Very nice project"
                  LANGUAGES CXX)
add_subdirectory("${PROJECT_SOURCE_DIR}/../.." "extern/robin_hood")

add_executable(MyProject myproject.cpp)
target_link_libraries(MyProject robin_hood)
