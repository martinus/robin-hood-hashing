target_sources_local(rh PRIVATE
    main.cpp # first because its slowest

    # benchmarks
    bench_copy_iterators.cpp
    bench_distinctness.cpp
    bench_find_random.cpp
    bench_hash_int.cpp
    bench_hash_string.cpp
    bench_iterate.cpp
    bench_random_insert_erase.cpp

    # count
    count_ctor_dtor.cpp
    count_ctor_dtor_1.cpp
    count_random_insert_erase.cpp
    count_find_random.cpp
    count_one_emplace.cpp

    # fuzzers
    fuzz_insert_erase.cpp

    # show infos
    show_hash.cpp

    # unit tests
    unit_assignments.cpp
    unit_assignment_combinations.cpp
    unit_at.cpp
    unit_clz.cpp
    unit_count.cpp
    unit_empty.cpp
    include_only.h
    include_only.cpp
    unit_include_only.cpp
    unit_initializerlist.cpp
    unit_iterators_ctor.cpp
    unit_iterators_empty.cpp
    unit_iterators_insert.cpp
    unit_iterators_stochastic.cpp
    unit_load_factor.cpp
    unit_not_copyable.cpp
    unit_not_moveable.cpp
    unit_overflow.cpp
    unit_pair_trivial.cpp
    unit_reserve.cpp
    unit_sizeof.cpp
    unit_unique_ptr.cpp
)