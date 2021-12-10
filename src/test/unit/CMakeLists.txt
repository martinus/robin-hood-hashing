target_sources_local(rh PRIVATE
    main.cpp # first because its slowest

    # benchmarks
    bench_copy_iterators.cpp
    bench_distinctness.cpp
    bench_find_random.cpp
    bench_hash_int.cpp
    bench_hash_string.cpp
    bench_iterate.cpp
    bench_quick_overall_map.cpp
    bench_quick_overall_set.cpp
    bench_random_insert_erase.cpp
    bench_swap.cpp

    # count
    count_ctor_dtor.cpp
    count_ctor_dtor_1.cpp
    count_random_insert_erase.cpp
    count_find_random.cpp
    count_one_emplace.cpp

    # fuzzers
    fuzz_insert_erase.cpp

    # optimizations
    optimize_avalanche.cpp

    # show infos
    show_hash.cpp

    # unit tests
    include_only.cpp
    include_only.h
    unit_addOrFree.cpp
    unit_assertNotNull.cpp
    unit_assign_to_move.cpp
    unit_assignment_combinations.cpp
    unit_assignments.cpp
    unit_at.cpp
    unit_calcMaxNumElementsAllowed.cpp
    unit_calcsize.cpp
    unit_compact.cpp
    unit_copyassignment.cpp
    unit_count.cpp
    unit_diamond.cpp
    unit_empty.cpp
    unit_explicitctor.cpp
    unit_fallback_hash.cpp
    unit_hash_char_types.cpp
    unit_hash_smart_ptr.cpp
    unit_hash_string_view.cpp
    unit_heterogeneous.cpp
    unit_include_only.cpp
    unit_initializer_list_insert.cpp
    unit_initializerlist.cpp
    unit_insert_collision.cpp
    unit_insert_or_assign.cpp
    unit_insert.cpp
    unit_iterator_twice_bug.cpp
    unit_iterators_ctor.cpp
    unit_iterators_empty.cpp
    unit_iterators_erase.cpp
    unit_iterators_insert.cpp
    unit_iterators_postinc.cpp
    unit_iterators_stochastic.cpp
    unit_load_factor.cpp
    unit_maps_of_maps.cpp
    unit_memleak_reserve.cpp
    unit_multiple_apis.cpp
    unit_mup.cpp
    unit_no_intrinsics.cpp
    unit_not_copyable.cpp
    unit_not_moveable.cpp
    unit_overflow_collisions.cpp
    unit_overflow.cpp
    unit_overflow2.cpp
    unit_pair_operators.cpp
    unit_pair_trivial.cpp
    unit_playback.cpp
    unit_random_verifier.cpp
    unit_reserve_and_assign.cpp
    unit_reserve.cpp
    unit_rotr.cpp
    unit_scoped_free.cpp
    unit_sfc64_is_deterministic.cpp
    unit_sizeof.cpp
    unit_string.cpp
    unit_try_emplace.cpp
    unit_undefined_behavior_nekrolm.cpp
    unit_unique_ptr.cpp
    unit_unordered_set.cpp
    unit_vectorofmaps.cpp
    unit_xy.cpp
)
