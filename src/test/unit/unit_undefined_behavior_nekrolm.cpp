// This test was provided by github user Nekrolm in Issue #57:
// https://github.com/martinus/robin-hood-hashing/issues/57
//
// The test triggers undefined behavior because of unaligned read.
// Additionally, this has surfaced a bug where the info bytes were not correctly recalculated!
#include <robin_hood.h>

#include <app/doctest.h>

#include <climits>
#include <iostream>
#include <unordered_map>

struct VoidState {
    inline constexpr bool operator==(const VoidState& ROBIN_HOOD_UNUSED(x) /*unused*/) const
        noexcept {
        return true;
    }
    inline constexpr bool operator<(const VoidState& ROBIN_HOOD_UNUSED(x) /*unused*/) const
        noexcept {
        return false;
    }
    static inline constexpr size_t Hash() noexcept {
        return 0;
    }
};

inline constexpr size_t CombineHashes(size_t h1, size_t h2) noexcept {
    return h1 << 5U ^ h1 >> (CHAR_BIT * sizeof(size_t) - 5) ^ h2;
}

template <class T, class = typename std::enable_if<std::is_integral<T>::value>::type>
struct IntegralState {
private:
    T value{};

public:
    IntegralState() = default;

    explicit IntegralState(T x)
        : value(x) {}

    inline bool operator<(const IntegralState& other) const noexcept {
        return value < other.value;
    }

    inline bool operator==(const IntegralState& other) const noexcept {
        return value == other.value;
    }

    ROBIN_HOOD(NODISCARD) inline size_t Hash() const noexcept {
        static const robin_hood::hash<T> hasher;
        return hasher(value);
    }
};

template <class First, class Second>
struct StatePair {
private:
    First first_state{};
    Second second_state{};

public:
    StatePair() = default;

    StatePair(First f, Second s)
        : first_state(f)
        , second_state(s) {}

    ROBIN_HOOD(NODISCARD) inline const First& FirstState() const noexcept {
        return first_state;
    }
    ROBIN_HOOD(NODISCARD) inline const Second& SecondState() const noexcept {
        return second_state;
    }

    inline bool operator==(const StatePair other) const noexcept {
        return first_state == other.first_state && second_state == other.second_state;
    }

    inline bool operator<(const StatePair other) const noexcept {
        return (first_state == other.first_state) ? second_state < other.second_state
                                                  : first_state < other.first_state;
    }

    ROBIN_HOOD(NODISCARD) inline size_t Hash() const noexcept {
        return CombineHashes(second_state.Hash(), first_state.Hash());
    }
};

/*-----------------------------------------------------------------------
 * specializations for cases with VoidState to not store redundant fields
 */
template <class First>
struct StatePair<First, VoidState> {
private:
    First first_state{};

public:
    inline StatePair() noexcept = default;

    inline StatePair(First f, VoidState ROBIN_HOOD_UNUSED(x) /*unused*/) noexcept
        : first_state(std::move(f)) {}

    ROBIN_HOOD(NODISCARD) inline const First& FirstState() const noexcept {
        return first_state;
    }
    inline static VoidState SecondState() noexcept {
        return {};
    }

    inline bool operator==(const StatePair other) const noexcept {
        return first_state == other.first_state;
    }

    inline bool operator<(const StatePair other) const noexcept {
        return first_state < other.first_state;
    }

    ROBIN_HOOD(NODISCARD) inline size_t Hash() const noexcept {
        return first_state.Hash();
    }
};

template <class Second>
struct StatePair<VoidState, Second> {
private:
    Second second_state{};

public:
    explicit inline StatePair(VoidState ROBIN_HOOD_UNUSED(x) /*unused*/, Second s) noexcept
        : second_state(std::move(s)) {}

    inline StatePair() noexcept = default;

    inline static VoidState FirstState() noexcept {
        return {};
    }
    inline const Second& SecondState() const noexcept {
        return second_state;
    }

    inline bool operator==(const StatePair other) const noexcept {
        return second_state == other.second_state;
    }

    inline bool operator<(const StatePair other) const noexcept {
        return second_state < other.second_state;
    }

    ROBIN_HOOD(NODISCARD) inline size_t Hash() const noexcept {
        return second_state.Hash();
    }
};

namespace std {
template <class First, class Second>
class hash<StatePair<First, Second>> {
public:
    size_t operator()(const StatePair<First, Second>& s) const noexcept {
        return s.Hash();
    }
};

} // namespace std

class FstStatePair {
public:
    FstStatePair(int i, int j)
        : inner_state(i)
        , outer_state(j) {}

    FstStatePair() = default;

    inline bool operator==(const FstStatePair& other) const noexcept {
        return inner_state == other.inner_state && outer_state == other.outer_state;
    }

    inline bool operator<(const FstStatePair& other) const noexcept {
        return inner_state == other.inner_state ? outer_state < other.outer_state
                                                : inner_state < other.inner_state;
    }

    ROBIN_HOOD(NODISCARD) size_t Hash() const noexcept {
        static const robin_hood::hash<uint64_t> hasher;
        return hasher((static_cast<uint64_t>(inner_state) << 32U) |
                      static_cast<uint32_t>(outer_state));
    }

private:
    int inner_state = -1;
    int outer_state = -1;
};

template <typename FState>
using InternalStateId = StatePair<FstStatePair, FState>;

using FState = StatePair<IntegralState<int>, StatePair<IntegralState<int>, VoidState>>;
using IState = InternalStateId<FState>;

TYPE_TO_STRING(std::unordered_map<IState, int>);
TYPE_TO_STRING(robin_hood::unordered_flat_map<IState, int>);
TYPE_TO_STRING(robin_hood::unordered_node_map<IState, int>);

TEST_CASE_TEMPLATE("undefined_behavior_nekrolm", Map, robin_hood::unordered_flat_map<IState, int>,
                   robin_hood::unordered_node_map<IState, int>) {
    Map map;

    for (int i = 0; i < 1000; ++i) {
        for (int j = 0; j < 1000; ++j) {
            FstStatePair lhs(i, j);
            FState rhs{};
            IState s(lhs, rhs);
            map[s] = static_cast<int>(map.size());
        }
    }

    REQUIRE(map.size() == 1000000);
}
