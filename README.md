➵ robin_hood::unordered_map
============

 [![Release](https://img.shields.io/github/release/martinus/robin-hood-hashing.svg)](https://github.com/martinus/robin-hood-hashing/releases) [![GitHub license](https://img.shields.io/github/license/martinus/robin-hood-hashing.svg)](https://raw.githubusercontent.com/martinus/robin-hood-hashing/master/LICENSE)
[![Travis CI Build Status](https://travis-ci.com/martinus/robin-hood-hashing.svg?branch=master)](https://travis-ci.com/martinus/robin-hood-hashing)
[![Appveyor Build Status](https://ci.appveyor.com/api/projects/status/github/martinus/robin-hood-hashing?branch=master&svg=true)](https://ci.appveyor.com/project/martinus/robin-hood-hashing)
[![Codacy Badge](https://api.codacy.com/project/badge/Grade/9308495247b542c9802016caa6fd3461)](https://www.codacy.com/app/martinus/robin-hood-hashing?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=martinus/robin-hood-hashing&amp;utm_campaign=Badge_Grade)
[![Total alerts](https://img.shields.io/lgtm/alerts/g/martinus/robin-hood-hashing.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/martinus/robin-hood-hashing/alerts/)
[![Language grade: C/C++](https://img.shields.io/lgtm/grade/cpp/g/martinus/robin-hood-hashing.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/martinus/robin-hood-hashing/context:cpp)

`robin_hood::unordered_map` is a platform independent replacement for `std::unordered_map` which is both faster and more memory efficient for real-world use cases.

## Installation & Usage

1. Add [`robin_hood.h`](https://github.com/martinus/robin-hood-hashing/blob/master/src/include/robin_hood.h) to your C++ project.
1. Use `robin_hood::unordered_map` instead of `std::unordered_map`.

## Benchmarks

Please see extensive benchmarks in [doc/BENCHMARKS.md](doc/BENCHMARKS.md). In short: `robin_hood` is really fast, most of the time faster than all competitors that I've tested, and uses far less memory than `std::unordered_map`.

## Features

- **Two memory layouts**. Data is either stored in a flat array, or with node indirection. Access for `unordered_flat_map` is extremely fast due to no indirection, but references to elements are not stable. It also causes allocation spikes when the map resizes, and will need plenty of memory for large objects. Node based map has stable references and uses `const Key` in the pair. It is a bit slower due to indirection. The choice is yours; you can either use `robin_hood::unordered_flat_map` or `robin_hood::unordered_node_map` directly. If you use `robin_hood::unordered_map` It tries to choose the layout that seems appropriate for your data.

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

This map is obviously not as well tested as `std::unordered_map`. It shoud be very stable for most use cases, but there might still be untested corner cases, where the map simply gives incorrect results! As far as I know, none of these bugs should be left. But I wouldn't bet my house on it.

## Alternatives

There are lots of `std::unorderd_map` challengers, here are a few interesting ones that I had a look at:

* [google/absl::Hash](https://abseil.io/blog/20180927-swisstables) Brand new and from google, but a bit difficult
to set up. Lots of features, heavily optimized, with some SIMD tricks.

* [skarupke/flat_hash_map](https://github.com/skarupke/flat_hash_map/blob/master/flat_hash_map.hpp): Very fast,
especially for integral types. Designed as a response to google's absl::Hash. 

* [google/dense_hash_map](http://goog-sparsehash.sourceforge.net/doc/dense_hash_map.html) Old and fast but does
not have a modern C++ interface.

## Blog Posts

Beware, these posts are out of date now.

* [Part 1: Hopscotch & Robin Hood Hashing](http://martin.ankerl.com/2016/09/15/very-fast-hashmap-in-c-part-1/)
* [Part 2: Implementation Variants](http://martin.ankerl.com/2016/09/21/very-fast-hashmap-in-c-part-2/)
* [Part 3: Benchmark Results](http://martin.ankerl.com/2016/09/21/very-fast-hashmap-in-c-part-3/)

## License

Licensed under the MIT License. See the [LICENSE](https://github.com/martinus/robin-hood-hashing/blob/master/LICENSE) file for details.

by martinus
