➵ robin_hood
============

 [![Release](https://img.shields.io/github/release/martinus/robin-hood-hashing.svg)](https://github.com/martinus/robin-hood-hashing/releases) [![GitHub license](https://img.shields.io/github/license/martinus/robin-hood-hashing.svg)](https://raw.githubusercontent.com/martinus/robin-hood-hashing/master/LICENSE)
[![Travis CI Build Status](https://travis-ci.com/martinus/robin-hood-hashing.svg?branch=master)](https://travis-ci.com/martinus/robin-hood-hashing)
[![Appveyor Build Status](https://ci.appveyor.com/api/projects/status/github/martinus/robin-hood-hashing?branch=master&svg=true)](https://ci.appveyor.com/project/martinus/robin-hood-hashing)
[![Codacy Badge](https://api.codacy.com/project/badge/Grade/9308495247b542c9802016caa6fd3461)](https://www.codacy.com/app/martinus/robin-hood-hashing?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=martinus/robin-hood-hashing&amp;utm_campaign=Badge_Grade)
[![Total alerts](https://img.shields.io/lgtm/alerts/g/martinus/robin-hood-hashing.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/martinus/robin-hood-hashing/alerts/)
[![Language grade: C/C++](https://img.shields.io/lgtm/grade/cpp/g/martinus/robin-hood-hashing.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/martinus/robin-hood-hashing/context:cpp)

`robin_hood::unordered_map` is be a platform independent replacement for `std::unordered_map` which is both faster and more memory efficient for real-world use cases.

## Installation & Usage

1. Add [`robin_hood.h`](https://github.com/martinus/robin-hood-hashing/blob/master/src/include/robin_hood.h) to your C++ project.
1. Use `robin_hood::unordered_map` instead of `std::unordered_map`.

## Benchmarks

I've performed extensive tests with [map_benchmark](https://github.com/martinus/map_benchmark), which can generate nice graphs of memoryusage & runtime. Here are some of my results. In general, insertion & removal is by far the fastest of all maps I've tested. Find performance is 2-3 times faster than `std::unordered_map`, but slower than `absl::flat_hash_map`.

All benchmarks are done on an Intel i7-8700, Linux, compiled with `g++-8 -O3`.

### Insert
Measure runtime and memory usage when inserting 100M randomly generated `int`, then call `clear()`, then inserting again 100M `int`. [source](https://github.com/martinus/map_benchmark/blob/4f4ed87d1e73082bf1fde5e14e8c24b825c09db9/src/benchmarks/Insert.cpp#L5)

![Insert](https://raw.githubusercontent.com/martinus/robin-hood-hashing/master/doc/insert_int.png)
brown: `std::unordered_map<int, int>`, green: `robin_hood::unordered_map<int, int>`. std::unordered_map is 7 times slower and uses 2.7 times more memory. More results with other maps:

|                           | runtime [sec] | peak memory [MB] |
|--------------------------:|--------------:|-----------------:|
|     `absl::flat_hash_map` |          22.8 |         **1182** |
|     `absl::node_hash_map` |          82.1 |             5162 |
|    `robin_hood::node_map` |          24.0 |             2416 |
|    `robin_hood::flat_map` |      **10.4** |         **1182** |
|    `ska::bytell_hash_map` |          17.4 |             6252 |
|      `std::unordered_map` |          69.8 |             4982 |

### Random find
Insert 100k `uint32_t`, search 100M times. Repeat 8 times, so in total the map will contain 800k elements and 800M lookups are performed. [Source](https://github.com/martinus/map_benchmark/blob/4f4ed87d1e73082bf1fde5e14e8c24b825c09db9/src/benchmarks/RandomFindExisting.cpp#L11)

![Random Find](https://raw.githubusercontent.com/martinus/robin-hood-hashing/master/doc/random_find_existing.png)

Brown is `std::unordered_map<uint32_t, uint32_t>`, green is `robin_hood::unordered_map<uint32_t, uint32_t>`.  `robin_hood::unordered_map` is 2.9 times faster while using 4 times less memory.

|                           | runtime [sec] | peak memory [MB] |
|--------------------------:|--------------:|-----------------:|
|     `absl::flat_hash_map` |       **6.7** |         **14.1** |
|     `absl::node_hash_map` |           9.6 |             41.4 |
|    `robin_hood::node_map` |           9.8 |             18.9 |
|    `robin_hood::flat_map` |           9.2 |         **14.1** |
|    `ska::bytell_hash_map` |           8.6 |             22.2 |
|      `std::unordered_map` |          26.5 |             14.1 |

### Insert & Lookup `std::string`

Uses 20 byte long `std::string` as key, and `size_t` as value. Inserts & lookups 50M strings so that about 25% are distinct. Brown is `std::unordered_map<std::string, size_t>`, green is `robin_hood::unordered_map<std::string, size_t>`. Here the difference is not so big, mostly because g++'s hash implementation uses a fast Murmurhash2 which is very similar to robin_hood's implementation. It seems all the other implementations are not really tuned for such data types. [Source](https://github.com/martinus/map_benchmark/blob/4f4ed87d1e73082bf1fde5e14e8c24b825c09db9/src/benchmarks/Strings.cpp#L44)

![String 25% distinct](https://raw.githubusercontent.com/martinus/robin-hood-hashing/master/doc/string25.png)

|                           | runtime [sec] | peak memory [MB] |
|--------------------------:|--------------:|-----------------:|
|     `absl::flat_hash_map` |          14.9 |             1325 |
|     `absl::node_hash_map` |          15.5 |             1329 |
|    `robin_hood::node_map` |          17.0 |         **1133** |
|    `robin_hood::flat_map` |      **11.4** |             1300 |
|    `ska::bytell_hash_map` |          14.6 |             2090 |
|      `std::unordered_map` |          16.2 |             1497 |

### Random Distinct

A mixed workload, similar to the benchmark used in [attractivechaos/udb2](https://github.com/attractivechaos/udb2). 50M `operator[]` are performed with random keys. This is done 4 times, with different number of prabability of accessing existing elements: 5% distinct values, 25% distinct, 50%, and purely random numbers with 100% distinctness. [Source](https://github.com/martinus/map_benchmark/blob/4f4ed87d1e73082bf1fde5e14e8c24b825c09db9/src/benchmarks/RandomDistinct.cpp#L5)

![Random Distinct](https://raw.githubusercontent.com/martinus/robin-hood-hashing/master/doc/random_distinct2.png)

Brown is `std::unordered_map<int, int>`, green is `robin_hood::unordered_map<int, int>`. Again, peak memory for `robin_hood::unordered_map` is about 2.8 times lower while being more than 6 times faster.

|                           | runtime [sec] | peak memory [MB] |
|--------------------------:|--------------:|-----------------:|
|     `absl::flat_hash_map` |          18.7 |          **906** |
|     `absl::node_hash_map` |          53.9 |             2592 |
|    `robin_hood::node_map` |          25.8 |             1208 |
|    `robin_hood::flat_map` |       **7.7** |          **906** |
|    `ska::bytell_hash_map` |          10.9 |             1422 |
|      `std::unordered_map` |          49.2 |             2495 |

## Features

- **Two memory layouts**. Data is either stored in a flat map, or node based. Flat map's access is extremely fast due to no indirection, but references to elements are not stable. It also causes allocation spikes when resizing will need more memory for large objects. Node based map has stable references but is a bit slower due to indirection. The choice is yours; you can either use `robin_hood::flat_map` or `robin_hood::node_map` directly. If you use `robin_hood::unordered_map` It tries to choose the layout that seems appropriate for your data.

- **Custom allocator**. Node based representation has a custom bulk allocator that tries to make few memory allocations. All allocated memory is reused, so there won't be any allocation spikes. It's very fast as well.

- **Optimized hash**. `robin_hood::hash` has custom implementations for integer types and for `std::string`, and falls back to `std::hash` for everything else.

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
