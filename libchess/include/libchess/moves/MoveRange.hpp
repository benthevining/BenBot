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
#include <iterator>
#include <libchess/moves/Move.hpp>
#include <libchess/util/inplace_any.hpp>
#include <memory>
#include <ranges>
#include <type_traits>
#include <utility>

namespace chess::moves {

/** A type-erased iterator whose value type is Move, and may wrap any type of iterator whose value
    type is Move.

    @ingroup moves
    @see MoveRange
 */
class MoveRangeIterator final {
    using Holder = util::inplace_any<64uz, alignof(std::max_align_t)>;

public:
    using iterator_category = std::input_iterator_tag;
    using iterator_concept  = iterator_category;
    using value_type        = Move;
    using reference         = value_type;
    using pointer           = value_type;
    using difference_type   = std::ptrdiff_t;

    MoveRangeIterator() noexcept = default;

    template <typename It>
    explicit MoveRangeIterator(It&& it) noexcept
        requires(not std::is_same_v<MoveRangeIterator, std::decay_t<It>>)
        : holder(std::forward<It>(it))
        , dispatcher(&call_method_on_iterator<It>)
    {
        static_assert(
            Holder::can_store<It>(),
            "If you hit this, you may need to increase the storage space size in MoveRangeIterator");

        static_assert(
            std::is_base_of_v<iterator_category, typename std::iterator_traits<It>::iterator_category>,
            "It must be at least an input iterator");
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

    [[nodiscard]] value_type operator*() const noexcept;

    MoveRangeIterator& operator++() noexcept
    {
        assert(dispatcher != nullptr);
        dispatcher(holder, Func::Increment, nullptr, nullptr);
        return *this;
    }

    [[nodiscard]] MoveRangeIterator operator++(int) noexcept
    {
        auto tmp = *this;
        operator++();
        return tmp;
    }

    bool operator==(const MoveRangeIterator& other) const noexcept;

    bool operator<(const MoveRangeIterator& other) const noexcept { return get_bool_result(other, Func::LessThan); }
    bool operator<=(const MoveRangeIterator& other) const noexcept { return get_bool_result(other, Func::LessThanEqualTo); }
    bool operator>(const MoveRangeIterator& other) const noexcept { return get_bool_result(other, Func::GreaterThan); }
    bool operator>=(const MoveRangeIterator& other) const noexcept { return get_bool_result(other, Func::GreaterThanEqualTo); }

private:
    enum class Func : std::uint_least8_t {
        Increment,         // ++
        Dereference,       // *
        AreEqual,          // ==
        LessThan,          // <
        LessThanEqualTo,   // <=
        GreaterThan,       // >
        GreaterThanEqualTo // >=
    };

    using DispatchFunc = void(Holder&, Func, std::byte*, const std::byte*);

    template <typename Iterator>
    static void call_method_on_iterator(
        Holder& itHolder, Func opCode, std::byte* ret, const std::byte* arg);

    template <typename Iterator>
    static void call_bool_method_on_iterator_pair(const Iterator& it, Func opCode, std::byte* ret, const std::byte* arg);

    [[nodiscard]] bool get_bool_result(const MoveRangeIterator& other, Func opCode) const noexcept;

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

private:
    using Holder = util::inplace_any<128uz, alignof(std::max_align_t)>;

    enum class Func {
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

template <typename Iterator>
void MoveRangeIterator::call_method_on_iterator(Holder& itHolder, Func opCode, std::byte* ret, const std::byte* arg)
{
    auto& it = itHolder.get<Iterator>();

    switch (opCode) {
        case (Func::Increment):
            ++it;
            return;

        case (Func::Dereference): {
            assert(ret != nullptr);
            std::construct_at(reinterpret_cast<value_type*>(ret), *it); // NOLINT
            return;
        }

        case (Func::AreEqual)       : [[fallthrough]];
        case (Func::LessThan)       : [[fallthrough]];
        case (Func::LessThanEqualTo): [[fallthrough]];
        case (Func::GreaterThan)    : [[fallthrough]];
        case (Func::GreaterThanEqualTo):
            return call_bool_method_on_iterator_pair(it, opCode, ret, arg);
    }
}

template <typename Iterator>
void MoveRangeIterator::call_bool_method_on_iterator_pair(
    const Iterator&  it,
    const Func       opCode,
    std::byte*       ret,
    const std::byte* arg)
{
    assert(ret != nullptr);
    assert(arg != nullptr);

    auto& boolRet = *reinterpret_cast<bool*>(ret); // NOLINT

    const auto& otherBufIt = *reinterpret_cast<const MoveRangeIterator*>(arg); // NOLINT
    const auto& otherIt    = otherBufIt.holder.get<Iterator>();

    switch (opCode) {
        case (Func::AreEqual):
            boolRet = it == otherIt;
            return;

        case (Func::LessThan):
            boolRet = it < otherIt;
            return;

        case (Func::LessThanEqualTo):
            boolRet = it <= otherIt;
            return;

        case (Func::GreaterThan):
            boolRet = it > otherIt;
            return;

        case (Func::GreaterThanEqualTo):
            boolRet = it >= otherIt;
            return;

        default:
            std::unreachable();
    }
}

inline auto MoveRangeIterator::operator*() const noexcept -> value_type
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
            return iterator { std::ranges::begin(range) };

        case Func::End:
            return iterator { std::ranges::end(range) };

        default:
            std::unreachable();
    }
}

inline auto MoveRange::get_iterator(Func opCode) -> iterator
{
    assert(dispatcher != nullptr);
    return dispatcher(range, opCode);
}

} // namespace chess::moves
