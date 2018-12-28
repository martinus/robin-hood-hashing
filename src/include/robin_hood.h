// MIT License
//
// Copyright (c) 2018 Martin Ankerl
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef ROBIN_HOOD_MAP_H_INCLUDED
#define ROBIN_HOOD_MAP_H_INCLUDED

#include <algorithm>
#include <functional>
#include <utility>

#include <iostream>

// mark unused members with this macro
#define ROBIN_HOOD_UNUSED(identifier)

#if SIZE_MAX == UINT32_MAX
#define ROBIN_HOOD_BITNESS 32
#elif SIZE_MAX == UINT64_MAX
#define ROBIN_HOOD_BITNESS 64
#else
#error Unsupported bitness
#endif

#ifdef _WIN32
#define ROBIN_HOOD_NOINLINE __declspec(noinline)
#else
#if __GNUC__ >= 4
#define ROBIN_HOOD_NOINLINE __attribute__((noinline))
#else
#define ROBIN_HOOD_NOINLINE
#endif
#endif

#if defined(__GNUC__) || defined(__clang__)
#define ROBIN_HOOD_ATTRIBUTE_MAY_ALIAS __attribute__((__may_alias__))
#else
#define ROBIN_HOOD_ATTRIBUTE_MAY_ALIAS
#endif

namespace robin_hood {

namespace detail {

// make sure this is not inlined as it is slow and dramatically enlarges code, thus making other inlinings more difficult.
// Throws are also generally the slow path.
template <class E, class... Args>
static ROBIN_HOOD_NOINLINE void doThrow(Args&&... args) {
	throw E(std::forward<Args>(args)...);
}

template <class E, class T, class... Args>
static T* assertNotNull(T* t, Args&&... args) {
	if (nullptr == t) {
		doThrow<E>(std::forward<Args>(args)...);
	}
	return t;
}

static inline uint64_t ror64(uint64_t v, int r) {
	return (v >> r) | (v << (64 - r));
}

// Allocates bulks of memory for objects of type T. This deallocates the memory in the destructor, and keeps a linked list of the allocated memory
// around. Overhead per allocation is the size of a pointer.
template <class T, size_t MinNumAllocs = 4, size_t MaxNumAllocs = 256>
class BulkPoolAllocator {
public:
	BulkPoolAllocator()
		: mHead(0)
		, mListForFree(0) {}

	// does not copy anything, just creates a new allocator.
	BulkPoolAllocator(const BulkPoolAllocator& ROBIN_HOOD_UNUSED(o))
		: mHead(0)
		, mListForFree(0) {}

	BulkPoolAllocator(BulkPoolAllocator&& o)
		: mHead(o.mHead)
		, mListForFree(o.mListForFree) {
		o.mListForFree = 0;
		o.mHead = 0;
	}

	BulkPoolAllocator& operator=(BulkPoolAllocator&& o) {
		reset();
		mHead = o.mHead;
		mListForFree = o.mListForFree;
		o.mListForFree = 0;
		o.mHead = 0;
		return *this;
	}

	BulkPoolAllocator& operator=(const BulkPoolAllocator& ROBIN_HOOD_UNUSED(o)) {
		// does not do anything
		return *this;
	}

	~BulkPoolAllocator() {
		reset();
	}

	// Deallocates all allocated memory.
	void reset() {
		while (mListForFree) {
			T* tmp = *mListForFree;
			free(mListForFree);
			mListForFree = reinterpret_cast<T**>(tmp);
		}
		mHead = 0;
	}

	// allocates, but does NOT initialize. Use in-place new constructor, e.g.
	//   T* obj = pool.allocate();
	//   new (obj) T();
	T* allocate() {
		T* tmp = mHead;
		if (!tmp) {
			tmp = performAllocation();
		}

		mHead = *reinterpret_cast<T**>(tmp);
		return tmp;
	}

	// does not actually deallocate but puts it in store.
	// make sure you have already called the destructor! e.g. with
	//  obj->~T();
	//  pool.deallocate(obj);
	void deallocate(T* obj) {
		*reinterpret_cast<T**>(obj) = mHead;
		mHead = obj;
	}

	// Adds an already allocated block of memory to the allocator. This allocator is from now on responsible for freeing the data (with free()). If
	// the provided data is not large enough to make use of, it is immediately freed. Otherwise it is reused and freed in the destructor.
	void addOrFree(void* ptr, const size_t numBytes) {
		// calculate number of available elements in ptr
		if (numBytes < ALIGNMENT + ALIGNED_SIZE) {
			// not enough data for at least one element. Free and return.
			free(ptr);
		} else {
			add(ptr, numBytes);
		}
	}

	void swap(BulkPoolAllocator<T, MinNumAllocs, MaxNumAllocs>& other) {
		using std::swap;
		swap(mHead, other.mHead);
		swap(mListForFree, other.mListForFree);
	}

private:
	// iterates the list of allocated memory to calculate how many to alloc next.
	// Recalculating this each time saves us a size_t member.
	// This ignores the fact that memory blocks might have been added manually with addOrFree. In practice, this should not matter much.
	size_t calcNumElementsToAlloc() const {
		T** tmp = mListForFree;
		size_t numAllocs = MinNumAllocs;

		while (numAllocs * 2 <= MaxNumAllocs && tmp) {
			tmp = *reinterpret_cast<T***>(tmp);
			numAllocs *= 2;
		}

		return numAllocs;
	}

	// WARNING: Underflow if numBytes < ALIGNMENT! This is guarded in addOrFree().
	void add(void* ptr, const size_t numBytes) {
		const size_t numElements = (numBytes - ALIGNMENT) / ALIGNED_SIZE;

		T** data = reinterpret_cast<T**>(ptr);

		// link free list
		*reinterpret_cast<T***>(data) = mListForFree;
		mListForFree = data;

		// create linked list for newly allocated data
		T* const headT = reinterpret_cast<T*>(reinterpret_cast<char*>(ptr) + ALIGNMENT);

		char* const head = reinterpret_cast<char*>(headT);

		// Visual Studio compiler automatically unrolls this loop, which is pretty cool
		for (size_t i = 0; i < numElements; ++i) {
			*reinterpret_cast<char**>(head + i * ALIGNED_SIZE) = head + (i + 1) * ALIGNED_SIZE;
		}

		// last one points to 0
		*reinterpret_cast<T**>(head + (numElements - 1) * ALIGNED_SIZE) = mHead;
		mHead = headT;
	}

	// Called when no memory is available (mHead == 0).
	// Don't inline this slow path.
	ROBIN_HOOD_NOINLINE T* performAllocation() {
		const size_t numElementsToAlloc = calcNumElementsToAlloc();

		// alloc new memory: [prev |T, T, ... T]
		// std::cout << (sizeof(T*) + ALIGNED_SIZE * numElementsToAlloc) << " bytes" << std::endl;
		size_t bytes = ALIGNMENT + ALIGNED_SIZE * numElementsToAlloc;
		add(assertNotNull<std::bad_alloc>(malloc(bytes)), bytes);
		return mHead;
	}

	// enforce byte alignment of the T's
	static const size_t ALIGNMENT = (std::max)(std::alignment_of<T>::value, std::alignment_of<T*>::value);
	static const size_t ALIGNED_SIZE = ((sizeof(T) - 1) / ALIGNMENT + 1) * ALIGNMENT;

	static_assert(MinNumAllocs >= 1, "MinNumAllocs");
	static_assert(MaxNumAllocs >= MinNumAllocs, "MaxNumAllocs");
	static_assert(ALIGNED_SIZE >= sizeof(T*), "ALIGNED_SIZE");
	static_assert(0 == (ALIGNED_SIZE % sizeof(T*)), "ALIGNED_SIZE mod");
	static_assert(ALIGNMENT >= sizeof(T*), "ALIGNMENT");

	T* mHead;
	T** mListForFree;
};

template <class T, size_t MinSize, size_t MaxSize, bool IsDirect>
struct NodeAllocator;

// dummy allocator that does nothing
template <class T, size_t MinSize, size_t MaxSize>
struct NodeAllocator<T, MinSize, MaxSize, true> {

	// we are not using the data, so just free it.
	void addOrFree(void* ptr, size_t ROBIN_HOOD_UNUSED(numBytes)) {
		free(ptr);
	}
};

template <class T, size_t MinSize, size_t MaxSize>
struct NodeAllocator<T, MinSize, MaxSize, false> : public BulkPoolAllocator<T, MinSize, MaxSize> {};

// All empty maps initial mInfo point to this infobyte. That way lookup in an empty map
// always returns false, and this is a very hot byte.
//
// we have to use data >1byte (at least 2 bytes), because initially we set mShift to 63 (has to be <63),
// so initial index will be 0 or 1.
static uint64_t sDummyInfoByte = 0;

template <class First, class Second>
struct Pair;

// A custom Pair implementation is used in the map because std::pair is not is_trivially_copyable, which means it would  not be allowed to be used
// in std::memcpy. This struct is copyable, which is also tested.
template <class First, class Second>
struct Pair {
	using first_type = First;
	using second_type = Second;

	// pair constructors are explicit so we don't accidentally call this ctor when we don't have to.
	explicit Pair(std::pair<First, Second> const& pair)
		: first(pair.first)
		, second(pair.second) {}

	// pair constructors are explicit so we don't accidentally call this ctor when we don't have to.
	explicit Pair(std::pair<First, Second>&& pair)
		: first(std::move(pair.first))
		, second(std::move(pair.second)) {}

	constexpr Pair(const First& firstArg, const Second& secondArg)
		: first(firstArg)
		, second(secondArg) {}

	constexpr Pair(First&& firstArg, Second&& secondArg)
		: first(std::move(firstArg))
		, second(std::move(secondArg)) {}

	template <typename FirstArg, typename SecondArg>
	constexpr Pair(FirstArg&& firstArg, SecondArg&& secondArg)
		: first(std::forward<FirstArg>(firstArg))
		, second(std::forward<SecondArg>(secondArg)) {}

	template <class... Args1, class... Args2>
	Pair(std::piecewise_construct_t, std::tuple<Args1...> firstArgs, std::tuple<Args2...> secondArgs)
		: Pair(firstArgs, secondArgs, std::index_sequence_for<Args1...>(), std::index_sequence_for<Args2...>()) {}

	// constructor called from the std::piecewise_construct_t ctor
	template <class Tuple1, class Tuple2, size_t... Indexes1, size_t... Indexes2>
	Pair(Tuple1& val1, Tuple2& val2, std::index_sequence<Indexes1...>, std::index_sequence<Indexes2...>)
		: first(std::get<Indexes1>(std::move(val1))...)
		, second(std::get<Indexes2>(std::move(val2))...) {}

	first_type& getFirst() {
		return first;
	}
	first_type const& getFirst() const {
		return first;
	}
	second_type& getSecond() {
		return second;
	}
	second_type const& getSecond() const {
		return second;
	}

	First first;
	Second second;
};

} // namespace detail

struct is_transparent_tag {};

// A thin wrapper around std::hash, performing a single multiplication to (hopefully) get nicely randomized upper bits, which are used by the
// unordered_map.
template <typename T>
class hash : public std::hash<T> {
public:
	size_t operator()(T const& obj) const {
		return std::hash<T>::operator()(obj);
	}
};

// specialization used for uint64_t and int64_t. Uses 128bit multiplication
template <>
class hash<uint64_t> {
public:
	size_t operator()(uint64_t const& obj) const {
#ifdef __SIZEOF_INT128__
		// 930566152204 masksum, 322103 geomean for 0xb3e739ce9947bb0d 0xa5f287615c7098e6
		static constexpr const auto factor = ((unsigned __int128){0xb3e739ce9947bb0d} << 64) | 0xa5f287615c7098e6;
		return static_cast<size_t>((factor * (unsigned __int128)obj) >> 64);
#else
		// murmurhash 3 finalizer
		uint64_t h = obj;
		h ^= h >> 33;
		h *= 0xff51afd7ed558ccd;
		h ^= h >> 33;
		h *= 0xc4ceb9fe1a85ec53;
		h ^= h >> 33;
		return static_cast<size_t>(h);
#endif
	}
};

template <>
class hash<int64_t> {
public:
	size_t operator()(int64_t const& obj) const {
		return hash<uint64_t>{}(obj);
	}
};

template <>
class hash<uint32_t> {
public:
	size_t operator()(uint32_t const& h) const {
#if ROBIN_HOOD_BITNESS == 32
		static constexpr uint64_t factor = 0xe02b61472f2e2abf;
		return static_cast<size_t>((factor * h) >> 32);
#else
		return hash<uint64_t>{}(h);
#endif
	}
};

template <>
class hash<int32_t> {
public:
	size_t operator()(int32_t const& obj) const {
		return hash<uint32_t>{}(obj);
	}
};

/*
// integral types simply used with multiplicative hashing
template <typename Arg>
typename std::enable_if_t<std::is_integral<Arg>::value, size_t> doHash(Arg const& obj) const {
	static constexpr uint64_t ah = 0xe02b61472f2e2abf;
	static constexpr uint64_t al = 0x90c9f3e278ea1ac7;
#if ROBIN_HOOD_BITNESS == 64
	static constexpr const unsigned __int128 factor = (u128{ah} << 64) | al;
	return (factor * obj) >> 64;
#else
	// from https://stackoverflow.com/a/22847373/48181
	uint64_t bl = obj;
	uint64_t p0 = al * bl;
	uint64_t p2 = ah * bl;

	return (p2 >> 32) + (((p0 >> 32) + (uint32_t)p2) >> 32);
	// can't use 128bit multiplication
#endif
}

// non-integral types use std::hash, without any modifications. Hopefully it's good enough.
template <typename Arg>
typename std::enable_if_t<!std::is_integral<Arg>::value, size_t> doHash(Arg const& obj) const {
	return std::hash<Arg>::operator()(obj);
}

public:
size_t operator()(T const& obj) const {
	return doHash<T>(obj);
}
}; // namespace robin_hood

// Thin wrapper around std::hash, providing very good mixing of the std::hash function. This uses the MurmurHash3 finalizer, which is quite fast and
// has extremely good mixing capabilities. Use this if you are unsure of the quality of your hash.
template <typename T>
struct hash_safe : public std::hash<T> {
	size_t operator()(T const& obj) const {
		// 17196638 swaps, 21068829 equals, capacity=524288
		size_t h = std::hash<T>::operator()(obj);
#if ROBIN_HOOD_BITNESS == 64
		h ^= (h >> 33);
		h *= UINT64_C(0xff51afd7ed558ccd);
		h ^= (h >> 33);
		h *= UINT64_C(0xc4ceb9fe1a85ec53);
		h ^= (h >> 33);
#else
		h ^= h >> 16;
		h *= 0x85ebca6b;
		h ^= h >> 13;
		h *= 0xc2b2ae35;
		h ^= h >> 16;
#endif
		return h;
	}
};
*/
// A highly optimized hashmap implementation, using the Robin Hood algorithm.
//
// In most cases, this map should be usable as a drop-in replacement for std::unordered_map, but be about 2x faster in most cases
// and require much less allocations.
//
// This implementation uses the following memory layout:
//
// [Node, Node, ... Node | info, info, ... infoSentinel ]
//
// * Node: either a DataNode that directly has the std::pair<key, val> as member,
//   or a DataNode with a pointer to std::pair<key,val>. Which DataNode representation to use depends
//   on how fast the swap() operation is. Heuristically, this is automatically choosen based on sizeof().
//   there are always 2^n Nodes.
//
// * info: Each Node in the map has a corresponding info byte, so there are 2^n info bytes.
//   Each byte is initialized to 0, meaning the corresponding Node is empty. Set to 1 means the corresponding
//   node contains data. Set to 2 means the corresponding Node is filled, but it actually belongs to the
//   previous position and was pushed out because that place is already taken.
//
// * infoSentinel: Sentinel byte set to 1, so that iterator's ++ can stop at end() without the need for a idx
//   variable.
template <class Key, class T, class Hash = std::hash<Key>, class KeyEqual = std::equal_to<Key>,
		  // Use direct map only when move does not throw, so swap and resize is possible without copying stuff.
		  // also make sure data is not too large, then swap might be slow.
		  bool IsDirect = sizeof(Key) + sizeof(T) <= sizeof(void*) * 3 &&
						  std::is_nothrow_move_constructible<std::pair<Key, T>>::value&& std::is_nothrow_move_assignable<std::pair<Key, T>>::value>

class unordered_map : public Hash, public KeyEqual, detail::NodeAllocator<detail::Pair<Key, T>, 4, 16384, IsDirect> {
	// configuration defaults
	static constexpr uint8_t MaxLoadFactor128 = 102; // 1 byte
	static constexpr size_t InitialNumElements = 4;

	using DataPool = detail::NodeAllocator<detail::Pair<Key, T>, 4, 16384, IsDirect>;

public:
	using key_type = Key;
	using mapped_type = T;
	using value_type = detail::Pair<Key, T>;
	using size_type = size_t;
	using hasher = Hash;
	using key_equal = KeyEqual;
	using Self = unordered_map<key_type, mapped_type, hasher, key_equal, IsDirect>;

private:
	// Primary template for the data node. We have special implementations for small and big objects.
	// For large objects it is assumed that swap() is fairly slow, so we allocate these on the heap
	// so swap merely swaps a pointer.
	template <class M, bool>
	class DataNode {};

	// Small: just allocate on the stack.
	template <class M>
	class DataNode<M, true> {
	public:
		template <class... Args>
		DataNode(M& ROBIN_HOOD_UNUSED(map), Args&&... args)
			: mData(std::forward<Args>(args)...) {}

		DataNode(M& ROBIN_HOOD_UNUSED(map), DataNode<M, true>&& n)
			: mData(std::move(n.mData)) {}

		// doesn't do anything
		void destroy(M& ROBIN_HOOD_UNUSED(map)) {}
		void destroyDoNotDeallocate() {}

		value_type const* operator->() const {
			return &mData;
		}
		value_type* operator->() {
			return &mData;
		}

		const value_type& operator*() const {
			return mData;
		}

		value_type& operator*() {
			return mData;
		}

		typename value_type::first_type& getFirst() {
			return mData.first;
		}

		typename value_type::first_type const& getFirst() const {
			return mData.first;
		}

		typename value_type::second_type& getSecond() {
			return mData.second;
		}

		typename value_type::second_type const& getSecond() const {
			return mData.second;
		}

		void swap(DataNode<M, true>& o) {
			using std::swap;
			swap(mData.first, o.mData.first);
			swap(mData.second, o.mData.second);
		}

	private:
		value_type mData;
	};

	// big object: allocate on heap.
	template <class M>
	class DataNode<M, false> {
	public:
		template <class... Args>
		DataNode(M& map, Args&&... args)
			: mData(map.allocate()) {
			new (mData) value_type(std::forward<Args>(args)...);
		}

		DataNode(M& map, DataNode<M, false>&& n)
			: mData(std::move(n.mData)) {}

		void destroy(M& map) {
			// don't deallocate, just put it into list of datapool.
			mData->~value_type();
			map.deallocate(mData);
		}

		void destroyDoNotDeallocate() {
			mData->~value_type();
		}

		value_type const* operator->() const {
			return mData;
		}

		value_type* operator->() {
			return mData;
		}

		const value_type& operator*() const {
			return *mData;
		}

		value_type& operator*() {
			return *mData;
		}

		typename value_type::first_type& getFirst() {
			return mData->first;
		}

		typename value_type::first_type const& getFirst() const {
			return mData->first;
		}

		typename value_type::second_type& getSecond() {
			return mData->second;
		}

		typename value_type::second_type const& getSecond() const {
			return mData->second;
		}

		void swap(DataNode<M, false>& o) {
			using std::swap;
			swap(mData, o.mData);
		}

	private:
		value_type* mData;
	};

	using Node = DataNode<Self, IsDirect>;

	size_t calcNumBytesInfo(size_t numElements) const {
		const size_t s = sizeof(uint8_t) * (numElements + 1);
		if (s / sizeof(uint8_t) != numElements + 1) {
			throwOverflowError();
		}
		return s;
	}
	size_t calcNumBytesNode(size_t numElements) const {
		const size_t s = sizeof(Node) * numElements;
		if (s / sizeof(Node) != numElements) {
			throwOverflowError();
		}
		return s;
	}
	size_t calcNumBytesTotal(size_t numElements) const {
		const size_t si = calcNumBytesInfo(numElements);
		const size_t sn = calcNumBytesNode(numElements);
		const size_t s = si + sn;
		if (s <= si || s <= sn) {
			throwOverflowError();
		}
		return s;
	}

	// Protects against bad hash functions, and makes sure the numbers are reasonably well spread.
	// I've played around with a few constants. The chosen one seems to work well in my benchmark.
	// basically any odd random 64bit number should work reasonably well. Also see
	// https://probablydance.com/2018/06/16/fibonacci-hashing-the-optimization-that-the-world-forgot-or-a-better-alternative-to-integer-modulo/
	//
	// Much better avalanching can be achieved with e.g. Murmur3 finalizer, but it is generally much slower.
	template <typename HashKey>
	size_t keyToIdx(HashKey&& key) const {
		return Hash::operator()(key) & mMask;
	}

	// forwards the index by one, wrapping around at the end
	void next(int& info, size_t& idx) const {
		idx = (idx + 1) & mMask;
		++info;
	}

	void nextWhileLess(int& info, size_t& idx) const {
		// unrolling this by hand did not bring any speedups.
		while (info < mInfo[idx]) {
			next(info, idx);
		}
	}

	void bubbleDown(size_t& idx, size_t const& insertion_idx) {
		while (idx != insertion_idx) {
			using std::swap;

			size_t const prev_idx = (idx - 1) & mMask;
			mKeyVals[idx].swap(mKeyVals[prev_idx]);
			swap(mInfo[idx], mInfo[prev_idx]);

			// increase the shifted up element
			if (0xFF == ++mInfo[idx]) {
				mMaxNumElementsAllowed = 0;
			}
			idx = prev_idx;
		}
	}

	// copy of find(), except that it returns iterator instead of const_iterator.
	template <class Other>
	size_t findIdx(const Other& key) const {
		size_t idx = keyToIdx(key);
		int info = 1;
		nextWhileLess(info, idx);

		// check while info matches with the source idx
		while (info == mInfo[idx]) {
			if (KeyEqual::operator()(key, mKeyVals[idx]->first)) {
				return idx;
			}
			next(info, idx);
		}

		// nothing found!
		return mMask + 1;
	}

	template <class M, bool UseMemcpy>
	struct Cloner;

	// fast path: Just copy data, without allocating anything.
	template <class M>
	struct Cloner<M, true> {
		void operator()(M const& source, M& target) const {
			// std::memcpy(target.mKeyVals, source.mKeyVals, target.calcNumBytesTotal(target.mMask + 1));
			auto src = reinterpret_cast<char const*>(source.mKeyVals);
			auto tgt = reinterpret_cast<char*>(target.mKeyVals);
			std::copy(src, src + target.calcNumBytesTotal(target.mMask + 1), tgt);
		}
	};

	template <class M>
	struct Cloner<M, false> {
		void operator()(M const& source, M& target) const {
			// make sure to copy initialize sentinel as well
			// std::memcpy(target.mInfo, source.mInfo, target.calcNumBytesInfo(target.mMask + 1));
			std::copy(source.mInfo, source.mInfo + target.calcNumBytesInfo(target.mMask + 1), target.mInfo);

			for (size_t i = 0; i < target.mMask + 1; ++i) {
				if (target.mInfo[i]) {
					new (target.mKeyVals + i) Node(target, *source.mKeyVals[i]);
				}
			}
		}
	};

	void cloneData(const unordered_map& o) {
		Cloner<unordered_map, IsDirect && std::is_trivially_copyable<Node>::value>()(o, *this);
	}

	// inserts a keyval that is guaranteed to be new, e.g. when the hashmap is resized.
	// @return index where the element was created
	size_t insert_move(Node&& keyval) {
		// we don't retry, fail if overflowing
		// don't need to check max num elements
		if (0 == mMaxNumElementsAllowed) {
			throwOverflowError();
		}

		size_t idx = keyToIdx(keyval->first);

		// skip forward. Use <= because we are certain that the element is not there.
		int info = 1;
		while (info <= mInfo[idx]) {
			idx = (idx + 1) & mMask;
			++info;
		}

		// key not found, so we are now exactly where we want to insert it.
		const size_t insertion_idx = idx;
		uint8_t insertion_info = static_cast<uint8_t>(info);
		if (0xFF == insertion_info) {
			mMaxNumElementsAllowed = 0;
		}

		// find an empty spot
		while (0 != mInfo[idx]) {
			next(info, idx);
		}

		// put at empty spot
		new (mKeyVals + idx) Node(std::move(keyval));
		mInfo[idx] = insertion_info;

		// bubble down into correct position
		bubbleDown(idx, insertion_idx);

		++mNumElements;
		return insertion_idx;
	}

	// generic iterator for both const_iterator and iterator.
	template <bool IsConst>
	class Iter {
	private:
		using NodePtr = typename std::conditional<IsConst, Node const*, Node*>::type;

	public:
		using difference_type = std::ptrdiff_t;
		using value_type = typename Self::value_type;
		using reference = typename std::conditional<IsConst, value_type const&, value_type&>::type;
		using pointer = typename std::conditional<IsConst, value_type const*, value_type*>::type;
		using iterator_category = std::forward_iterator_tag;

		// both const_iterator and iterator can be constructed from a non-const iterator
		Iter(Iter<false> const& other)
			: mKeyVals(other.mKeyVals)
			, mInfo(other.mInfo) {}

		Iter(NodePtr valPtr, uint8_t const* infoPtr)
			: mKeyVals(valPtr)
			, mInfo(infoPtr) {}

		// prefix increment. Undefined behavior if we are at end()!
		Iter& operator++() {
			do {
				mKeyVals++;
				mInfo++;
			} while (0 == *mInfo);
			return *this;
		}

		reference operator*() const {
			return **mKeyVals;
		}

		pointer operator->() const {
			return &**mKeyVals;
		}

		template <bool O>
		bool operator==(Iter<O> const& o) const {
			return mKeyVals == o.mKeyVals;
		}

		template <bool O>
		bool operator!=(Iter<O> const& o) const {
			return mKeyVals != o.mKeyVals;
		}

	private:
		friend class unordered_map<key_type, mapped_type, hasher, key_equal, IsDirect>;
		NodePtr mKeyVals;
		uint8_t const* mInfo;
	};

public:
	using iterator = Iter<false>;
	using const_iterator = Iter<true>;

	/// Creates an empty hash map. Nothing is allocated yet, this happens at the first insert.
	/// This tremendously speeds up ctor & dtor of a map that never receives an element. The
	/// penalty is payed at the first insert, and not before. Lookup of this empty map works
	/// because everybody points to sDummyInfoByte.
	/// parameter bucket_count is dictated by the standard, but we can ignore it.
	explicit unordered_map(size_t ROBIN_HOOD_UNUSED(bucket_count) = 0, const Hash& hash = Hash(), const KeyEqual& equal = KeyEqual())
		: Hash(hash)
		, KeyEqual(equal) {}

	template <class Iter>
	unordered_map(Iter first, Iter last, size_t ROBIN_HOOD_UNUSED(bucket_count) = 0, const Hash& hash = Hash(), const KeyEqual& equal = KeyEqual())
		: Hash(hash)
		, KeyEqual(equal) {
		insert(first, last);
	}

	unordered_map(std::initializer_list<value_type> init, size_t ROBIN_HOOD_UNUSED(bucket_count) = 0, const Hash& hash = Hash(),
				  const KeyEqual& equal = KeyEqual())
		: Hash(hash)
		, KeyEqual(equal) {
		insert(init.begin(), init.end());
	}

	unordered_map(unordered_map&& o)
		: Hash(std::move(static_cast<Hash&>(o)))
		, KeyEqual(std::move(static_cast<KeyEqual&>(o)))
		, DataPool(std::move(static_cast<DataPool&>(o)))
		, mKeyVals(std::move(o.mKeyVals))
		, mInfo(std::move(o.mInfo))
		, mNumElements(std::move(o.mNumElements))
		, mMask(std::move(o.mMask))
		, mMaxNumElementsAllowed(std::move(o.mMaxNumElementsAllowed)) {
		// set other's mask to 0 so its destructor won't do anything
		o.mMask = 0;
	}

	unordered_map& operator=(unordered_map&& o) {
		if (&o != this) {
			// different, move it
			destroy();
			mKeyVals = std::move(o.mKeyVals);
			mInfo = std::move(o.mInfo);
			mNumElements = std::move(o.mNumElements);
			mMask = std::move(o.mMask);
			mMaxNumElementsAllowed = std::move(o.mMaxNumElementsAllowed);
			Hash::operator=(std::move(static_cast<Hash&>(o)));
			KeyEqual::operator=(std::move(static_cast<KeyEqual&>(o)));
			DataPool::operator=(std::move(static_cast<DataPool&>(o)));
			// set other's mask to 0 so its destructor won't do anything
			o.mMask = 0;
		}
		return *this;
	}

	unordered_map(const unordered_map& o)
		: Hash(static_cast<const Hash&>(o))
		, KeyEqual(static_cast<const KeyEqual&>(o))
		, DataPool(static_cast<const DataPool&>(o)) {

		if (!o.empty()) {
			// not empty: create an exact copy. it is also possible to just iterate through all elements and insert them, but
			// copying is probably faster.

			mKeyVals = reinterpret_cast<Node*>(detail::assertNotNull<std::bad_alloc>(malloc(calcNumBytesTotal(o.mMask + 1))));
			// no need for calloc because clonData does memcpy
			mInfo = reinterpret_cast<uint8_t*>(mKeyVals + o.mMask + 1);
			mNumElements = o.mNumElements;
			mMask = o.mMask;
			mMaxNumElementsAllowed = o.mMaxNumElementsAllowed;
			cloneData(o);
		}
	}

	// Creates a copy of the given map. Copy constructor of each entry is used.
	unordered_map& operator=(const unordered_map& o) {
		if (&o == this) {
			// prevent assigning of itself
			return *this;
		}

		// we keep using the old allocator and not assign the new one, because we want to keep the memory available.
		// when it is the same size.
		if (o.empty()) {
			if (0 == mMask) {
				// nothing to do, we are empty too
				return *this;
			}

			// not empty: destroy what we have there
			// clear also resets mInfo to 0, that's sometimes not necessary.
			destroy();
			mKeyVals = reinterpret_cast<Node*>(&detail::sDummyInfoByte) - 1;
			mInfo = reinterpret_cast<uint8_t*>(&detail::sDummyInfoByte);
			Hash::operator=(static_cast<const Hash&>(o));
			KeyEqual::operator=(static_cast<const KeyEqual&>(o));
			mNumElements = 0;
			mMask = 0;
			mMaxNumElementsAllowed = 0;
			return *this;
		}

		// clean up old stuff
		destroyNodes();

		if (mMask != o.mMask) {
			// no luck: we don't have the same array size allocated, so we need to realloc.
			if (0 != mMask) {
				// only deallocate if we actually have data!
				free(mKeyVals);
			}

			mKeyVals = reinterpret_cast<Node*>(detail::assertNotNull<std::bad_alloc>(malloc(calcNumBytesTotal(o.mMask + 1))));

			// no need for calloc here because cloneData performs a memcpy.
			mInfo = reinterpret_cast<uint8_t*>(mKeyVals + o.mMask + 1);
			// sentinel is set in cloneData
		}
		Hash::operator=(static_cast<const Hash&>(o));
		KeyEqual::operator=(static_cast<const KeyEqual&>(o));
		mNumElements = o.mNumElements;
		mMask = o.mMask;
		mMaxNumElementsAllowed = o.mMaxNumElementsAllowed;
		cloneData(o);

		return *this;
	}

	// Swaps everything between the two maps.
	void swap(unordered_map& o) {
		using std::swap;
		swap(mKeyVals, o.mKeyVals);
		swap(mInfo, o.mInfo);
		swap(mNumElements, o.mNumElements);
		swap(mMask, o.mMask);
		swap(mMaxNumElementsAllowed, o.mMaxNumElementsAllowed);
		swap(static_cast<Hash&>(*this), static_cast<Hash&>(o));
		swap(static_cast<KeyEqual&>(*this), static_cast<KeyEqual&>(o));
		// no harm done in swapping datapool
		swap(static_cast<DataPool&>(*this), static_cast<DataPool&>(o));
	}

	// Clears all data, without resizing.
	void clear() {
		if (empty()) {
			// don't do anything! also important because we don't want to write to sDummyInfoByte, even
			// though we would just write 0 to it.
			return;
		}

		destroyNodes();

		// clear everything except the sentinel
		// std::memset(mInfo, 0, sizeof(uint8_t) * (mMask + 1));
		std::fill(mInfo, mInfo + (sizeof(uint8_t) * (mMask + 1)), 0);
	}

	/// Destroys the map and all it's contents.
	~unordered_map() {
		destroy();
	}

	/// Checks if both maps contain the same entries. Order is irrelevant.
	bool operator==(const unordered_map& other) const {
		if (other.size() != size()) {
			return false;
		}
		const_iterator myEnd = end();
		for (const_iterator otherIt = other.begin(), otherEnd = other.end(); otherIt != otherEnd; ++otherIt) {
			Self::const_iterator myIt = find(otherIt->first);
			if (myIt == myEnd || !(myIt->second == otherIt->second)) {
				return false;
			}
		}

		return true;
	}

	bool operator!=(const unordered_map& other) const {
		return !operator==(other);
	}

	mapped_type& operator[](const key_type& key) {
		return doCreateByKey(key);
	}

	mapped_type& operator[](key_type&& key) {
		return doCreateByKey(std::move(key));
	}

	template <class Iter>
	void insert(Iter first, Iter last) {
		for (; first != last; ++first) {
			// value_type ctor needed because this might be called with std::pair's
			insert(value_type(*first));
		}
	}

	template <class... Args>
	std::pair<iterator, bool> emplace(Args&&... args) {
		Node n{*this, std::forward<Args>(args)...};
		auto r = doInsert(std::move(n));
		if (!r.second) {
			// insertion not possible: destroy node
			n.destroy(*this);
		}
		return r;
	}

	std::pair<iterator, bool> insert(const value_type& keyval) {
		return doInsert(keyval);
	}

	std::pair<iterator, bool> insert(value_type&& keyval) {
		return doInsert(std::move(keyval));
	}

	size_t count(const key_type& key) const {
		return findIdx(key) == (mMask + 1) ? 0 : 1;
	}

	const_iterator find(const key_type& key) const {
		const size_t idx = findIdx(key);
		return const_iterator(mKeyVals + idx, mInfo + idx);
	}

	template <class OtherKey>
	const_iterator find(const OtherKey& key, is_transparent_tag) const {
		const size_t idx = findIdx(key);
		return const_iterator(mKeyVals + idx, mInfo + idx);
	}

	iterator find(const key_type& key) {
		const size_t idx = findIdx(key);
		return iterator(mKeyVals + idx, mInfo + idx);
	}

	template <class OtherKey>
	iterator find(const OtherKey& key, is_transparent_tag) {
		const size_t idx = findIdx(key);
		return iterator(mKeyVals + idx, mInfo + idx);
	}

	iterator begin() {
		if (empty()) {
			return end();
		}
		return ++iterator(mKeyVals - 1, mInfo - 1);
	}
	const_iterator begin() const {
		return cbegin();
	}
	const_iterator cbegin() const {
		if (empty()) {
			return cend();
		}
		return ++const_iterator(mKeyVals - 1, mInfo - 1);
	}

	iterator end() {
		// no need to supply valid info pointer: end() must not be dereferenced, and only node pointer is compared.
		return iterator(reinterpret_cast<Node*>(mInfo), 0);
	}
	const_iterator end() const {
		return cend();
	}
	const_iterator cend() const {
		return const_iterator(reinterpret_cast<Node*>(mInfo), 0);
	}

	iterator erase(const_iterator pos) {
		// its safe to perform const cast here
		return erase(iterator(const_cast<Node*>(pos.mKeyVals), const_cast<uint8_t*>(pos.mInfo)));
	}

	// Erases element at pos, returns iterator to the next element.
	iterator erase(iterator pos) {
		// we assume that pos always points to a valid entry, and not end().

		// perform backward shift deletion: shift elements to the left
		// until we find one that is either empty or has zero offset.
		size_t idx = pos.mKeyVals - mKeyVals;
		size_t nextIdx = (idx + 1) & mMask;
		while (mInfo[nextIdx] > 1) {
			mInfo[idx] = (uint8_t)(mInfo[nextIdx] - 1);
			mKeyVals[idx].swap(mKeyVals[nextIdx]);
			idx = nextIdx;
			nextIdx = (idx + 1) & mMask;
		}

		mInfo[idx] = 0;
		mKeyVals[idx].destroy(*this);
		mKeyVals[idx].~Node();
		--mNumElements;

		if (*pos.mInfo) {
			// we've backward shifted, return this again
			return pos;
		}

		// no backward shift, return next element
		return ++pos;
	}

	size_t erase(const key_type& key) {
		size_t idx = keyToIdx(key);

		int info = 1;
		nextWhileLess(info, idx);

		// check while info matches with the source idx
		while (info == mInfo[idx]) {
			if (KeyEqual::operator()(key, mKeyVals[idx]->first)) {
				// found it! perform backward shift deletion: shift elements to the left
				// until we find one that is either empty or has zero offset.
				size_t nextIdx = (idx + 1) & mMask;
				while (mInfo[nextIdx] > 1) {
					mInfo[idx] = (uint8_t)(mInfo[nextIdx] - 1);
					mKeyVals[idx].swap(mKeyVals[nextIdx]);
					idx = nextIdx;
					nextIdx = (idx + 1) & mMask;
				}

				mInfo[idx] = 0;
				mKeyVals[idx].destroy(*this);
				mKeyVals[idx].~Node();

				--mNumElements;
				return 1;
			}
			next(info, idx);
		}

		// nothing found to delete
		return 0;
	}

	size_type size() const {
		return mNumElements;
	}

	size_type max_size() const {
		return static_cast<size_type>(-1);
	}

	bool empty() const {
		return 0 == mNumElements;
	}

	float max_load_factor() const {
		return MaxLoadFactor128 / 128.0f;
	}

	// Average number of elements per bucket. Since we allow only 1 per bucket
	float load_factor() const {
		return static_cast<float>(size()) / (mMask + 1);
	}

	size_t mask() const {
		return mMask;
	}

private:
	ROBIN_HOOD_NOINLINE void throwOverflowError() const {
		throw std::overflow_error("robin_hood::map overflow");
	}

	void init_data(size_t max_elements) {
		mNumElements = 0;
		mMask = max_elements - 1;
		mMaxNumElementsAllowed = calcMaxNumElementsAllowed128(max_elements, MaxLoadFactor128);

		// calloc also zeroes everything
		mKeyVals = reinterpret_cast<Node*>(detail::assertNotNull<std::bad_alloc>(calloc(1, calcNumBytesTotal(max_elements))));
		mInfo = reinterpret_cast<uint8_t*>(mKeyVals + max_elements);

		// set sentinel
		mInfo[max_elements] = 1;
	}

	template <class Arg>
	mapped_type& doCreateByKey(Arg&& key) {
		while (true) {
			size_t idx = keyToIdx(key);

			int info = 1;
			nextWhileLess(info, idx);

			// while we potentially have a match
			while (info == mInfo[idx]) {
				if (KeyEqual::operator()(key, mKeyVals[idx]->first)) {
					// key already exists, do not insert.
					return mKeyVals[idx]->second;
				}
				next(info, idx);
			}

			// unlikely that this evaluates to true
			if (mNumElements >= mMaxNumElementsAllowed) {
				increase_size();
				continue;
			}

			// key not found, so we are now exactly where we want to insert it.
			size_t const insertion_idx = idx;
			uint8_t const insertion_info = static_cast<uint8_t>(info);
			if (0xFF == insertion_info) {
				// might overflow next time, set to 0 so we increase size next time
				mMaxNumElementsAllowed = 0;
			}

			// find an empty spot
			while (0 != mInfo[idx]) {
				next(info, idx);
			}

			// put at empty spot. This forwards all arguments into the node where the object is constructed exactly where it is needed.
			new (mKeyVals + idx) Node(*this, std::piecewise_construct, std::forward_as_tuple(std::forward<Arg>(key)), std::forward_as_tuple());

			// mKeyVals[idx]->first = std::move(key);
			mInfo[idx] = insertion_info;

			// bubble down into correct position
			bubbleDown(idx, insertion_idx);

			++mNumElements;
			return mKeyVals[insertion_idx]->second;
		}
	}

	// This is exactly the same code as operator[], except for the return values
	template <class Arg>
	std::pair<iterator, bool> doInsert(Arg&& keyval) {
		while (true) {
			size_t idx = keyToIdx(keyval.getFirst());

			int info = 1;
			nextWhileLess(info, idx);

			// while we potentially have a match
			while (info == mInfo[idx]) {
				if (KeyEqual::operator()(keyval.getFirst(), mKeyVals[idx]->first)) {
					// key already exists, do NOT insert.
					// see http://en.cppreference.com/w/cpp/container/unordered_map/insert
					return std::make_pair<iterator, bool>(iterator(mKeyVals + idx, mInfo + idx), false);
				}
				next(info, idx);
			}

			// unlikely that this evaluates to true
			if (mNumElements >= mMaxNumElementsAllowed) {
				increase_size();
				continue;
			}

			// key not found, so we are now exactly where we want to insert it.
			const size_t insertion_idx = idx;
			uint8_t insertion_info = static_cast<uint8_t>(info);
			if (0xFF == insertion_info) {
				mMaxNumElementsAllowed = 0;
			}

			// find an empty spot
			while (0 != mInfo[idx]) {
				next(info, idx);
			}

			// put at empty spot
			new (mKeyVals + idx) Node(*this, std::forward<Arg>(keyval));
			mInfo[idx] = insertion_info;

			// bubble down into correct position
			bubbleDown(idx, insertion_idx);

			++mNumElements;
			return std::make_pair(iterator(mKeyVals + insertion_idx, mInfo + insertion_idx), true);
		}
	}

	size_t calcMaxNumElementsAllowed128(size_t maxElements, uint8_t maxLoadFactor128) {
		// make sure we can't get an overflow, use floatingpoint arithmetic if necessary.
		return (maxElements > static_cast<size_t>(-1) / 128) ? static_cast<size_t>((static_cast<double>(maxElements) * maxLoadFactor128) / 128.0)
															 : (maxElements * maxLoadFactor128) / 128;
	}

	void increase_size() {
		// nothing allocated yet? just allocate 4 elements
		if (0 == mMask) {
			init_data(InitialNumElements);
			return;
		}

		// it seems we have a really bad hash function! don't try to resize again
		if (mNumElements * 2 < calcMaxNumElementsAllowed128(mMask + 1, MaxLoadFactor128)) {
			throwOverflowError();
		}

		// std::cout << (100.0*mNumElements / (mMask + 1)) << "% full, resizing" << std::endl;
		Node* oldKeyVals = mKeyVals;
		uint8_t* oldInfo = mInfo;

		const size_t oldMaxElements = mMask + 1;

		// resize operation: move stuff
		init_data(oldMaxElements * 2);

		for (size_t i = 0; i < oldMaxElements; ++i) {
			if (oldInfo[i]) {
				insert_move(std::move(oldKeyVals[i]));
				// destroy the node but DON'T destroy the data.
				oldKeyVals[i].~Node();
			}
		}

		// don't destroy old data: put it into the pool instead
		DataPool::addOrFree(oldKeyVals, calcNumBytesTotal(oldMaxElements));
	}

	// destroys all nodes (without clearing mInfo)
	// WARNING don't call when empty, because of sentinel.
	void destroyNodes() {
		mNumElements = 0;
		if (IsDirect && std::is_trivially_destructible<Node>::value) {
			return;
		}

		// clear also resets mInfo to 0, that's sometimes not necessary.
		for (size_t idx = 0; idx <= mMask; ++idx) {
			if (0 != mInfo[idx]) {
				Node& n = mKeyVals[idx];
				n.destroy(*this);
				n.~Node();
			}
		}
	}

	void destroyNodesDoNotDeallocate() {
		mNumElements = 0;

		if (IsDirect && std::is_trivially_destructible<Node>::value) {
			return;
		}

		// clear also resets mInfo to 0, that's sometimes not necessary.
		for (size_t idx = 0; idx <= mMask; ++idx) {
			if (0 != mInfo[idx]) {
				Node& n = mKeyVals[idx];
				n.destroyDoNotDeallocate();
				n.~Node();
			}
		}
	}

	void destroy() {
		if (0 == mMask) {
			// don't deallocate! we are pointing to sDummyInfoByte.
			return;
		}

		destroyNodesDoNotDeallocate();
		free(mKeyVals);
	}

	// members are sorted so no padding occurs
	Node* mKeyVals = reinterpret_cast<Node*>(reinterpret_cast<uint8_t*>(&detail::sDummyInfoByte) - sizeof(Node)); // 8 byte  8
	uint8_t* mInfo = reinterpret_cast<uint8_t*>(&detail::sDummyInfoByte);                                         // 8 byte 16
	size_t mNumElements = 0;                                                                                      // 8 byte 24
	size_t mMask = 0;                                                                                             // 8 byte 32
	size_t mMaxNumElementsAllowed = 0;                                                                            // 8 byte 40
};

template <class Key, class T, class Hash = robin_hood::hash<Key>, class KeyEqual = std::equal_to<Key>>
using flat_map = unordered_map<Key, T, Hash, KeyEqual, true>;

template <class Key, class T, class Hash = robin_hood::hash<Key>, class KeyEqual = std::equal_to<Key>>
using node_map = unordered_map<Key, T, Hash, KeyEqual, false>;

} // namespace robin_hood

#endif
