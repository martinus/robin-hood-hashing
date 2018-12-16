robin-hood-hashing
==================

Hashtable based on Robin Hood Hashing. In general, this map is both faster and uses less memory than `std::unordered_map` implementations. It features:

## Installation & Usage

1. Add `robin_hood_map.h` to your C++ project.
1. Use `robin_hood::map` instead of `std::unordered_map`.

For most use cases, that's it! Enjoy less memory usage and more performance

## Benchmarks

All benchmarks are lies, but I usually see improvement in the order of

* 3 times faster insertion than `std::unordered_map`
* 2 times faster lookup than `std::unordered_map`
* 2 - 3 times less memory usage than `std::unordered_map`

Of course, your milage may vary. There are an infinite number of use cases, and while I've tested a wide range
of scenarios you have to do benchmarks with your own workload to be sure this is worth it.

## Competitors

There are lots of `std::unorderd_map` challengers, here are a few interesting ones that I had a look at:

* [google/absl::Hash](https://abseil.io/blog/20180927-swisstables) Brand new and from google, but a bit difficult to set up. Searches are extremely fast, about 30% faster than `robin_hood::map`. Insertion & deletion is about 40% slower though. clear() frees all memory, so reinserting will be as slow as initial insertion was. For integral types this seems to use exactly the same amount of memory as `robin_hood::map`, but for larger data structures (when `robin_hood::map` switches to it's node-based mode) memory usage blows up to 2.4 times as much. Use this if lookup performance is absolutely critical for you.

* [skarupke/flat_hash_map](https://github.com/skarupke/flat_hash_map/blob/master/flat_hash_map.hpp): Very fast, especially for integral types. Sometimes faster sometimes slower as `robin_hood::map`. Memory usage tends to be much higher for non-integral types.

* [google/dense_hash_map](http://goog-sparsehash.sourceforge.net/doc/dense_hash_map.html) Old and does not have a modern C++ interface.


## Blog Posts

* [Part 1: Hopscotch & Robin Hood Hashing](http://martin.ankerl.com/2016/09/15/very-fast-hashmap-in-c-part-1/)
* [Part 2: Implementation Variants](http://martin.ankerl.com/2016/09/21/very-fast-hashmap-in-c-part-2/)
* [Part 3: Benchmark Results](http://martin.ankerl.com/2016/09/21/very-fast-hashmap-in-c-part-3/)

by martinus
