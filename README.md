robin-hood-hashing
==================

Hashtable based on Robin Hood Hashing.

In general, this map is both faster and uses less memory than `std::unordered_map` implementations. It features:

## Installation & Usage

1. Add `robin_hood_map.h` to your C++ project.
1. Use `robin_hood::map` instead of `std::unordered_map`.

For most use cases, that's it! Enjoy less memory usage and more performance. In our use cases, we usually see these improvements:

* 3 times faster insertion than `std::unordered_map`
* 2 times faster lookup than `std::unordered_map`
* 2 - 3 times less memory usage than `std::unordered_map`

Of course, your milage may vary. There are an infinite number of use cases, so please do your own benchmarks.


See detailed description of the different variants here:

* [Part 1: Hopscotch & Robin Hood Hashing](http://martin.ankerl.com/2016/09/15/very-fast-hashmap-in-c-part-1/)
* [Part 2: Implementation Variants](http://martin.ankerl.com/2016/09/21/very-fast-hashmap-in-c-part-2/)
* [Part 3: Benchmark Results](http://martin.ankerl.com/2016/09/21/very-fast-hashmap-in-c-part-3/)

by martinus
