cmake_minimum_required(VERSION 3.5)
project(robin-hood-hashing LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# configuration see .clang-tidy
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_CXX_CLANG_TIDY clang-tidy)
endif()

option(RH_sanitizer "Address sanitizer" OFF)
option(RH_coverage, "Enable coverage" OFF)

add_executable(rh "")

if (RH_sanitizer)
    message(STATUS "Building with Sanitizers")
    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        # see https://clang.llvm.org/docs/UndefinedBehaviorSanitizer.html#silencing-unsigned-integer-overflow
        # Compile with -g and -fno-omit-frame-pointer to get proper debug information in your binary
        target_compile_options(rh PRIVATE -g) 
        target_compile_options(rh PRIVATE -O2)
        target_compile_options(rh PRIVATE -fno-omit-frame-pointer)

        target_compile_options(rh PRIVATE -fsanitize=address)
        target_link_libraries(rh PRIVATE -fsanitize=address)

        target_compile_options(rh PRIVATE -fsanitize=undefined)
        target_link_libraries(rh PRIVATE -fsanitize=undefined)

        target_compile_options(rh PRIVATE -fsanitize=integer)
        target_compile_options(rh PRIVATE -fno-sanitize=unsigned-integer-overflow)
        target_link_libraries(rh PRIVATE -fsanitize=integer)
        target_link_libraries(rh PRIVATE -fno-sanitize=unsigned-integer-overflow)

        target_compile_options(rh PRIVATE -fsanitize=nullability)
        target_link_libraries(rh PRIVATE -fsanitize=nullability)
    endif()
    if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        # need to use gold linker, otherwise travis gets '/usr/bin/ld: --push-state: unknown option' error
        target_link_libraries(rh PRIVATE -fuse-ld=gold)

        target_compile_options(rh PRIVATE -g) 
        target_compile_options(rh PRIVATE -O2)
        target_compile_options(rh PRIVATE -fno-omit-frame-pointer)

        target_compile_options(rh PRIVATE -fsanitize=undefined,float-divide-by-zero,float-cast-overflow)
        target_link_libraries(rh PRIVATE -fsanitize=undefined,float-divide-by-zero,float-cast-overflow)

        target_compile_options(rh PRIVATE -fsanitize=pointer-compare,pointer-subtract)
        target_link_libraries(rh PRIVATE -fsanitize=pointer-compare,pointer-subtract)

        target_compile_options(rh PRIVATE -fsanitize=address)
        target_link_libraries(rh PRIVATE -fsanitize=address)
    endif()
endif()

if(RH_coverage AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    MESSAGE("coverage")
    # Add required flags (GCC & LLVM/Clang)
    target_compile_options(rh PRIVATE
        -O0        # no optimization
        -g         # generate debug info
        --coverage # sets all required flags
    )
    target_link_libraries(rh PRIVATE --coverage)

    # find the correct gcov version
    get_filename_component(COMPILER_PATH "${CMAKE_CXX_COMPILER}" PATH)
    string(REGEX MATCH "^[0-9]+" GCC_VERSION "${CMAKE_CXX_COMPILER_VERSION}")
    find_program(GCOV_BIN NAMES gcov-${GCC_VERSION} gcov HINTS ${COMPILER_PATH})

    # collect all source files from the chosen include dir
    file(GLOB_RECURSE SOURCE_FILES src/include/robin_hood.h)

    # COMMAND ${CMAKE_SOURCE_DIR}/test/thirdparty/imapdl/filterbr.py json.info.filtered > json.info.filtered.noexcept
    add_custom_target(lcov_html
        COMMAND lcov --directory . --capture --output-file json.info --rc lcov_branch_coverage=1
        COMMAND lcov -e json.info ${SOURCE_FILES} --output-file json.info.filtered --gcov-tool ${GCOV_BIN} --rc lcov_branch_coverage=1
        COMMAND genhtml --title "JSON for Modern C++" --legend --demangle-cpp --output-directory html --show-details --branch-coverage json.info.filtered
        COMMENT "Generating HTML report test/html/index.html"
    )

endif()

#set_target_properties(rh PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")
#set_target_properties(rh PROPERTIES COMPILE_FLAGS "-m64" LINK_FLAGS "-m64")

add_subdirectory(cmake)

add_compile_flags_target(rh)
add_subdirectory(src)

target_sources_local(rh PUBLIC .clang-tidy)

