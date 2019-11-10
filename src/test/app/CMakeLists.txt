add_subdirectory(fmt)
add_subdirectory(hash)

target_sources_local(rh PRIVATE
    benchmark.cpp
    benchmark.h
    counter_defaults.h
    Counter.cpp
    Counter.h
    CtorDtorVerifier.h
    CtorDtorVerifier.cpp
    doctest.h
    nanobench.cpp
    PerformanceCounters.cpp
    PerformanceCounters.h
    randomseed.cpp
    randomseed.h
    sfc64.h
)
