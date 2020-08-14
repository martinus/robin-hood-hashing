cmake_minimum_required(VERSION 3.5)
project(robin-hood-hashing LANGUAGES CXX)

# generate compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# determine whether this is a standalone project or included by other projects
set(RH_STANDALONE_PROJECT OFF)
if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(RH_STANDALONE_PROJECT ON)
endif ()

if (RH_STANDALONE_PROJECT)
    option(RH_sanitizer "Address sanitizer" OFF)
    option(RH_coverage "Enable coverage" OFF)
    set(RH_cxx_standard "14" CACHE STRING "C++ standard, e.g. 11, 14, 17")

    set(CMAKE_CXX_STANDARD ${RH_cxx_standard})
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)

    # configuration see .clang-tidy
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        find_program(CLANG_TIDY_PROGRAM NAMES clang-tidy)
        if(CLANG_TIDY_PROGRAM)
            set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_PROGRAM}")
        endif()
    endif()

    find_program(CCACHE_PROGRAM ccache)
    if(CCACHE_PROGRAM)
        SET(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
    endif()

    add_executable(rh "")

    if (RH_no_exceptions)
        target_compile_options(rh PRIVATE -fno-exceptions)
        target_link_libraries(rh PRIVATE -fno-exceptions)
        add_definitions(-DDOCTEST_CONFIG_NO_EXCEPTIONS_BUT_WITH_ALL_ASSERTS)
    endif()

    if (RH_sanitizer)
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
        MESSAGE(STATUS "Coverage enable")

        # Add required flags (GCC & LLVM/Clang)
        target_compile_options(rh PRIVATE -fprofile-arcs -ftest-coverage)
        target_link_libraries(rh PRIVATE -fprofile-arcs -ftest-coverage)

        # find the correct gcov version
        #get_filename_component(COMPILER_PATH "${CMAKE_CXX_COMPILER}" PATH)
        #string(REGEX MATCH "^[0-9]+" GCC_VERSION "${CMAKE_CXX_COMPILER_VERSION}")
        #find_program(GCOV_BIN NAMES gcov-${GCC_VERSION} gcov HINTS ${COMPILER_PATH})

        # collect all source files from the chosen include dir
        file(GLOB_RECURSE SOURCE_FILES src/include/robin_hood.h)

        # COMMAND ${CMAKE_SOURCE_DIR}/test/thirdparty/imapdl/filterbr.py json.info.filtered > json.info.filtered.noexcept
        add_custom_target(lcov
            COMMAND lcov --directory . --capture --output-file coverage.info # parse coverage data
            COMMAND lcov -e coverage.info ${SOURCE_FILES} --output-file coverage.info # filter
            COMMAND coveralls-lcov coverage.info  # upload results
        )

    endif()

    #set_target_properties(rh PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")
    #set_target_properties(rh PROPERTIES COMPILE_FLAGS "-m64" LINK_FLAGS "-m64")

    add_subdirectory(cmake)

    add_compile_flags_target(rh)
    add_subdirectory(src)

    target_sources_local(rh PUBLIC .clang-tidy)
else()
    # creates a library robin_hood which is an interface (header files only)
    add_library(robin_hood INTERFACE)
    add_library(robin_hood::robin_hood ALIAS robin_hood)

    target_include_directories(robin_hood INTERFACE
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/include>
        $<INSTALL_INTERFACE:src/include>
    )
endif()
