robin-hood-hashing
==================

Hashtable based on Robin Hood Hashing and HopScotch.
The HopScotch algorithm features:

* 3 times faster insertion than std::unordered_map
* 2 times faster lookup than std::unordered_map
* 2.6 times less memory usage than std::unordered_map

All benchmarks done with Visual Studio 2015, Update 3, 64 bit.

See detailed description of the different variants here:

* [Part 1: Hopscotch & Robin Hood Hashing](http://martin.ankerl.com/2016/09/15/very-fast-hashmap-in-c-part-1/)
* [Part 2: Implementation Variants](http://martin.ankerl.com/2016/09/21/very-fast-hashmap-in-c-part-2/)
* [Part 3: Benchmark Results](http://martin.ankerl.com/2016/09/21/very-fast-hashmap-in-c-part-3/)

by martinus
