robin-hood-hashing
==================

Hashtable based on Robin Hood Hashing. In general, this map is both faster and uses less memory than
`std::unordered_map` implementations.

## Installation & Usage

1. Add `robin_hood.h` to your C++ project.
1. Use `robin_hood::unordered_map` instead of `std::unordered_map`.

That's it! Enjoy less memory usage and more performance.

## Benchmarks

All benchmarks are lies, but I usually see improvement in the order of

* 3 times faster insertion than `std::unordered_map`
* 2 times faster lookup than `std::unordered_map`
* 2 - 3 times less memory usage than `std::unordered_map`

Of course, your milage may vary. There are an infinite number of use cases, and while I've tested a wide range
of scenarios you have to do benchmarks with your own workload to be sure this is worth it.

## Good, Bad, Ugly

### The Good

In most cases, you can simply replace `std::unordered_map` with `robin_hood::unordered_map` and enjoy a substantial speedup and less memory. robin-hood map is well optimized for both native types and complex types: It supports two different memory layouts: `flat_map` for fast direct access, and `node_map` which is node based, enjoying stable references while being much more memory efficient than `std::unordered_map`. 

The map defaults to using `robin_hood::hash` which uses a very fast well tuned hash for native types, and uses defaults to `std::hash` for everything else.

### The Bad

The performance of any map depends on how good the hash function is. But even worse for `robin_hood`, for a really bad hash the performance will not only degrade, the map will simply fail with an exception if not even doubling its size helps. So choose your hash well. (Note, that some `std::unordered_map` implementations fail as well with bad hashes).

If the map gets very full but just below it's resizing limit, continuously inserting and removing elements can get quite slow because the map has to shuffle lots of elements around. 

### The Ugly

This map is obviously not as well tested as `std::unorderered_map`. It shoud be very stable for most use cases, but there might still be untested corner cases, where the map simply gives incorrect results! As far as I know, none of these bugs should be left. But I wouldn't bet my house on it.

## Alternatives

There are lots of `std::unorderd_map` challengers, here are a few interesting ones that I had a look at:

* [google/absl::Hash](https://abseil.io/blog/20180927-swisstables) Brand new and from google, but a bit difficult
to set up. Searches are extremely fast, about 30% faster than `robin_hood::unordered_map`. Insertion & deletion is
about 40% slower though. clear() frees all memory, so reinserting will be as slow as initial insertion was. For
integral types this seems to use exactly the same amount of memory as `robin_hood::unordered_map`, but for larger
data structures (when `robin_hood::unordered_map` switches to it's node-based mode) memory usage blows up to 2.4
times as much. Use this if lookup performance is absolutely critical for you.

* [skarupke/flat_hash_map](https://github.com/skarupke/flat_hash_map/blob/master/flat_hash_map.hpp): Very fast,
especially for integral types. Sometimes faster sometimes slower as `robin_hood::unordered_map`. Memory usage
tends to be much higher for non-integral types.

* [google/dense_hash_map](http://goog-sparsehash.sourceforge.net/doc/dense_hash_map.html) Old and fast but does
not have a modern C++ interface.


## Blog Posts

* [Part 1: Hopscotch & Robin Hood Hashing](http://martin.ankerl.com/2016/09/15/very-fast-hashmap-in-c-part-1/)
* [Part 2: Implementation Variants](http://martin.ankerl.com/2016/09/21/very-fast-hashmap-in-c-part-2/)
* [Part 3: Benchmark Results](http://martin.ankerl.com/2016/09/21/very-fast-hashmap-in-c-part-3/)

## License

Licensed under the MIT License. See the [LICENSE](https://github.com/martinus/robin-hood-hashing/blob/master/LICENSE) file for details.

by martinus
