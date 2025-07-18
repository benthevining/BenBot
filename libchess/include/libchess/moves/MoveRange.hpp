/*
 * ======================================================================================
 *
 * ░▒▓███████▓▒░░▒▓████████▓▒░▒▓███████▓▒░       ░▒▓███████▓▒░ ░▒▓██████▓▒░▒▓████████▓▒░
 * ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░
 * ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░
 * ░▒▓███████▓▒░░▒▓██████▓▒░ ░▒▓█▓▒░░▒▓█▓▒░      ░▒▓███████▓▒░░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░
 * ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░
 * ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░
 * ░▒▓███████▓▒░░▒▓████████▓▒░▒▓█▓▒░░▒▓█▓▒░      ░▒▓███████▓▒░ ░▒▓██████▓▒░  ░▒▓█▓▒░
 *
 * ======================================================================================
 */

/** @file
    This file defines the MoveRange class.
    @ingroup moves
 */

#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint> // IWYU pragma: keep - for std::uint_least8_t
#include <iterator>
#include <libchess/moves/Move.hpp>
#include <libchess/util/inplace_any.hpp>
#include <memory>
#include <ranges>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace chess::moves {

/** A type-erased iterator whose value type is Move, and may wrap any type of iterator whose value
    type is Move.

    @ingroup moves
    @see MoveRange
 */
class MoveRangeIterator final {
    using Holder = util::inplace_any<1024uz, alignof(std::max_align_t)>;

public:
    using iterator_category = std::input_iterator_tag;
    using iterator_concept  = iterator_category;
    using value_type        = Move;
    using reference         = value_type;
    using pointer           = value_type;
    using difference_type   = std::ptrdiff_t;

    MoveRangeIterator() noexcept = default;

    // trick to get around not being able to specify constructor template arguments
    template <typename Type>
    struct TypeTag final { };

    struct IteratorTag { };
    struct SentinelTag { };

    template <typename It, typename Tag, typename Sentinel>
    explicit MoveRangeIterator(
        It&&                               it,
        [[maybe_unused]] TypeTag<Tag>      isSentinel,
        [[maybe_unused]] TypeTag<Sentinel> sentinelType) noexcept
        requires(not std::is_same_v<MoveRangeIterator, std::decay_t<It>>)
        : holder(std::forward<It>(it))
        , dispatcher(&call_method_on_iterator<It, std::is_same_v<Tag, SentinelTag>, Sentinel>)
    {
        static_assert(
            Holder::can_store<It>(),
            "If you hit this, you may need to increase the storage space size in MoveRangeIterator");
    }

    /** This method can be used to retrieve the underlying wrapped iterator type.
        Use with caution, as you must know the exact type of the underlying iterator!
     */
    template <class It>
    [[nodiscard]] It base() noexcept
        requires(Holder::can_store<It>())
    {
        assert(holder.holds_type<It>());
        return holder.get<It>();
    }

    [[nodiscard]] value_type operator*() const;

    MoveRangeIterator& operator++()
    {
        assert(dispatcher != nullptr);
        dispatcher(holder, Func::Increment, nullptr, nullptr);
        return *this;
    }

    [[nodiscard]] MoveRangeIterator operator++(int)
    {
        auto tmp = *this;
        operator++();
        return tmp;
    }

    bool operator==(const MoveRangeIterator& other) const;

private:
    enum class Func : std::uint_least8_t {
        Increment,   // ++
        Dereference, // *
        AreEqual     // ==
    };

    using DispatchFunc = void(Holder&, Func, std::byte*, const std::byte*);

    template <typename Iterator, bool IsSentinel, typename Sentinel>
    static void call_method_on_iterator(
        Holder& itHolder, Func opCode, std::byte* ret, const std::byte* arg);

    // mutable to avoid const_cast when we need to pass this into the dispatch func from const member funcs
    mutable Holder holder;

    DispatchFunc* dispatcher { nullptr };
};

/** A type-erased range whose value type is Move, and may wrap any type of range whose value
    type is move.

    @ingroup moves
    @see MoveRangeIterator
 */
struct MoveRange final {
    template <typename Range>
    explicit MoveRange(Range&& underlyingRange)
        requires(! std::is_same_v<MoveRange, std::decay_t<Range>>)
        : range { std::forward<Range>(underlyingRange) }
        , dispatcher { &call_method_on_range<Range> }
    {
        static_assert(
            Holder::can_store<Range>(),
            "If you hit this, you may need to increase the storage space size in MoveRange");

        static_assert(
            std::is_same_v<std::ranges::range_value_t<Range>, value_type>,
            "Range's value_type must be Move");
    }

    using iterator   = MoveRangeIterator;
    using value_type = Move;

    [[nodiscard]] iterator begin() { return get_iterator(Func::Begin); }
    [[nodiscard]] iterator end() { return get_iterator(Func::End); }

    [[nodiscard]] bool empty() const { return std::ranges::empty(*this); }

private:
    using Holder = util::inplace_any<4096uz, alignof(std::max_align_t)>;

    enum class Func : std::uint_least8_t {
        Begin,
        End
    };

    using DispatchFunc = iterator(Holder&, Func);

    template <typename Range>
    [[nodiscard]] static iterator call_method_on_range(
        Holder& holder, Func opCode);

    [[nodiscard]] iterator get_iterator(Func opCode);

    Holder range;

    DispatchFunc* dispatcher { nullptr };
};

/*
                         ___                           ,--,
      ,---,            ,--.'|_                ,--,   ,--.'|
    ,---.'|            |  | :,'             ,--.'|   |  | :
    |   | :            :  : ' :             |  |,    :  : '    .--.--.
    |   | |   ,---.  .;__,'  /    ,--.--.   `--'_    |  ' |   /  /    '
  ,--.__| |  /     \ |  |   |    /       \  ,' ,'|   '  | |  |  :  /`./
 /   ,'   | /    /  |:__,'| :   .--.  .-. | '  | |   |  | :  |  :  ;_
.   '  /  |.    ' / |  '  : |__  \__\/: . . |  | :   '  : |__ \  \    `.
'   ; |:  |'   ;   /|  |  | '.'| ," .--.; | '  : |__ |  | '.'| `----.   \
|   | '/  ''   |  / |  ;  :    ;/  /  ,.  | |  | '.'|;  :    ;/  /`--'  /__  ___  ___
|   :    :||   :    |  |  ,   /;  :   .'   \;  :    ;|  ,   /'--'.     /  .\/  .\/  .\
 \   \  /   \   \  /    ---`-' |  ,     .-./|  ,   /  ---`-'   `--'---'\  ; \  ; \  ; |
  `----'     `----'             `--`---'     ---`-'                     `--" `--" `--"

 */

// when IsSentinel is true, Iterator is actually the range's sentinel type
// and Sentinel is the range's iterator type
template <typename Iterator, bool IsSentinel, typename Sentinel>
void MoveRangeIterator::call_method_on_iterator(
    Holder& itHolder, const Func opCode, std::byte* ret, const std::byte* arg)
{
    auto& it = itHolder.get<Iterator>();

    switch (opCode) {
        case (Func::Increment): {
            if constexpr (IsSentinel) {
                throw std::logic_error { "Cannot increment sentinel!" };
            } else {
                ++it;
            }
            return;
        }

        case (Func::Dereference): {
            if constexpr (IsSentinel) {
                throw std::logic_error { "Cannot dereference sentinel!" };
            } else {
                assert(ret != nullptr);
                std::construct_at(reinterpret_cast<value_type*>(ret), *it); // NOLINT
            }
            return;
        }

        case (Func::AreEqual): {
            assert(ret != nullptr);
            assert(arg != nullptr);

            auto& boolRet = *reinterpret_cast<bool*>(ret); // NOLINT

            const auto& otherIt = *reinterpret_cast<const MoveRangeIterator*>(arg); // NOLINT

            if constexpr (IsSentinel) {
                if (otherIt.holder.holds_type<Iterator>()) {
                    boolRet = true;
                    return;
                }

                auto& other = otherIt.holder.get<Sentinel>();

                boolRet = other == it;
            } else {
                if (not otherIt.holder.holds_type<Sentinel>()) {
                    boolRet = false;
                    return;
                }

                auto& sentinel = otherIt.holder.get<Sentinel>();

                boolRet = it == sentinel;
            }
        }
    }
}

inline bool MoveRangeIterator::operator==(const MoveRangeIterator& other) const
{
    if (holder.empty() && other.holder.empty())
        return true; // both iterators are null

    assert(dispatcher != nullptr);

    bool ret { false };

    dispatcher(holder, Func::AreEqual,
        reinterpret_cast<std::byte*>(&ret),          // NOLINT
        reinterpret_cast<const std::byte*>(&other)); // NOLINT

    return ret;
}

inline auto MoveRangeIterator::operator*() const -> value_type
{
    assert(dispatcher != nullptr);

    alignas(value_type) std::byte buf[sizeof(value_type)]; // NOLINT

    dispatcher(holder, Func::Dereference, &buf[0uz], nullptr);

    return *std::launder(reinterpret_cast<value_type*>(buf)); // NOLINT
}

template <typename Range>
auto MoveRange::call_method_on_range(
    Holder& holder, const Func opCode) -> iterator
{
    auto& range = holder.get<Range>();

    switch (opCode) {
        case Func::Begin:
            return iterator {
                std::ranges::begin(range),
                iterator::TypeTag<iterator::IteratorTag> {},
                iterator::TypeTag<std::ranges::sentinel_t<Range>> {}
            };

        case Func::End:
            return iterator {
                std::ranges::end(range),
                iterator::TypeTag<iterator::SentinelTag> {},
                iterator::TypeTag<std::ranges::iterator_t<Range>> {}
            };

        default:
            std::unreachable();
    }
}

inline auto MoveRange::get_iterator(const Func opCode) -> iterator
{
    assert(dispatcher != nullptr);
    return dispatcher(range, opCode);
}

} // namespace chess::moves
