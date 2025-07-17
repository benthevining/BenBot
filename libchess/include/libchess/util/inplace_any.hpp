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
    This file defines the inplace_any class.
    @ingroup util
 */

#pragma once

#include <cassert>
#include <concepts>
#include <cstddef>
#include <format>
#include <memory>
#include <new>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <typeindex>
#include <utility>

namespace chess::util {

using std::size_t;

#ifndef DOXYGEN
namespace detail {
    template <typename T>
    [[nodiscard]] static consteval bool is_nothrow_copyable()
    {
        using Decayed = std::decay_t<T>;

        return std::is_nothrow_copy_constructible_v<Decayed>
           and (std::is_nothrow_copy_assignable_v<Decayed> or not std::copyable<Decayed>);
    }

    template <typename T>
    [[nodiscard]] static consteval bool is_nothrow_movable()
    {
        using Decayed = std::decay_t<T>;

        return std::is_nothrow_move_constructible_v<Decayed>
           and (std::is_nothrow_move_assignable_v<Decayed> or not std::movable<Decayed>);
    }
} // namespace detail
#endif // DOXYGEN

/** A type-erasing object wrapper similar to ``std::any``, but uses stack memory instead of heap allocations.

    Note that this object can only hold types that are copy-constructible and move-constructible (though they
    don't have to be copy- or move-assignable).

    @tparam Size Maximum size of the object that the inplace_any can hold.
    @tparam Alignment Maximum alignment of the object that the inplace_any can hold.

    @ingroup util
 */
template <size_t Size, size_t Alignment = 8uz>
class inplace_any final {
    // needed to disambiguate constructors/assignment operators taking an object from the special member funcs
    template <typename T>
    static constexpr bool NotBaseOf = not std::is_base_of_v<inplace_any, std::decay_t<T>>;

public:
    /** Returns true if this object can store the given type. */
    template <typename T>
    [[nodiscard]] static consteval bool can_store()
    {
        using Decayed = std::decay_t<T>;

        return sizeof(Decayed) <= Size and alignof(Decayed) <= Alignment
           and std::is_destructible_v<Decayed>
           and std::copy_constructible<Decayed> and std::move_constructible<Decayed>
           and not std::is_void_v<Decayed>;
    }

    /// @name Constructors
    /// @{
    /** Creates an empty inplace_any object. */
    constexpr inplace_any() noexcept = default;

    /** Copy constructor. */
    inplace_any(const inplace_any& other) // NOLINT
        : dispatcher(other.dispatcher)
    {
        if (other.has_value()) {
            assert(dispatcher != nullptr);
            auto& mutableOther = const_cast<inplace_any&>(other); // NOLINT
            dispatcher(storage, Func::CopyConstruct, &mutableOther);
        }
    }

    /** Move constructor. */
    inplace_any(inplace_any&& other) noexcept // NOLINT
        : dispatcher(other.dispatcher)
    {
        if (other.has_value()) {
            assert(dispatcher != nullptr);
            dispatcher(storage, Func::MoveConstruct, &other);
        }
    }

    /** Creates an inplace_any object holding the passed object. */
    template <typename T>
    explicit inplace_any(T&& object) // NOLINT
        noexcept(std::is_nothrow_move_constructible_v<std::decay_t<T>>)
        requires(NotBaseOf<T> and can_store<T>())
    {
        emplace<std::decay_t<T>>(std::forward<T>(object));
    }

    /** Creates an inplace_any object by constructing an object in-place with the given arguments. */
    template <typename T, typename... Args>
    explicit(sizeof...(Args) == 0uz) inplace_any(std::in_place_type_t<T>, Args&&... args) // NOLINT
        noexcept(std::is_nothrow_constructible_v<std::decay_t<T>, Args...>)
        requires(can_store<T>())
    {
        emplace<std::decay_t<T>>(std::forward<Args>(args)...);
    }
    /// @}

    /// @name Assignment operators
    ///@{
    /** Copy assignment operator. */
    inplace_any& operator=(const inplace_any& other);

    /** Move assignment operator. */
    inplace_any& operator=(inplace_any&& other) noexcept;

    /** Copies the input ``object`` into this inplace_any.
        If this inplace_any was previously holding an object of a different type, it is first
        destroyed, and then a new object is copy-constructed. If this inplace_any was previously
        holding an object of the same type, then it is copy assigned.
     */
    template <typename T>
    inplace_any& operator=(const T& object)
        noexcept(detail::is_nothrow_copyable<T>())
        requires(NotBaseOf<T> and can_store<T>());

    /** Moves the input ``object`` into this inplace_any.
        If this inplace_any was previously holding an object of a different type, it is first
        destroyed, and then a new object is move-constructed. If this inplace_any was previously
        holding an object of the same type, then it is move assigned.
     */
    template <typename T>
    inplace_any& operator=(T&& object)
        noexcept(detail::is_nothrow_movable<T>())
        requires(NotBaseOf<T> and can_store<T>());
    /// @}

    /** Destructor. */
    ~inplace_any() noexcept { reset(); }

    /** Returns true if this inplace_any currently holds an object.
        @see empty()
     */
    [[nodiscard]] bool has_value() const noexcept { return dispatcher != nullptr; }

    /** Returns true if this inplace_any currently holds an object.
        @see has_value(), empty()
     */
    explicit operator bool() const noexcept { return has_value(); }

    /** Returns true if this inplace_any does not currently hold an object.
        @see has_value()
     */
    [[nodiscard]] bool empty() const noexcept { return not has_value(); }

    /** Constructs an object inside this inplace_any, using the given arguments.
        If the inplace_any was previously holding an object, it is destroyed first.
     */
    template <typename T, typename... Args>
    std::decay_t<T>& emplace(Args&&... args)
        noexcept(std::is_nothrow_constructible_v<std::decay_t<T>, Args...>)
        requires(can_store<T>())
    {
        reset();

        using Stored = std::decay_t<T>;

        dispatcher = &call_method_on_object<Stored>;

        std::construct_at(
            reinterpret_cast<Stored*>(storage), // NOLINT
            std::forward<Args>(args)...);

        return *std::launder(reinterpret_cast<Stored*>(storage)); // NOLINT
    }

    /** Destroys the held object, if any. */
    void reset() noexcept
    {
        if (empty())
            return;

        assert(dispatcher != nullptr);

        dispatcher(storage, Func::Destruct, nullptr);

        dispatcher = nullptr;
    }

    /** Returns the held object.

        @throws std::logic_error An exception will be thrown if the inplace_any is empty,
        or holds a different type than the requested type.

        @see try_get()
     */
    template <typename T>
    [[nodiscard]] std::decay_t<T>& get()
        requires(can_store<T>())
    {
        using Stored = std::decay_t<T>;

        if (not holds_type<Stored>())
            throw std::logic_error {
                std::format(
                    "Wrong type requested from inplace_any (holds {}, requested {})",
                    std::string { get_stored_type().name() },
                    std::string { std::type_index { typeid(Stored) }.name() })
            };

        return *std::launder(reinterpret_cast<Stored*>(storage)); // NOLINT
    }

    /** Returns the held object.

        @throws std::logic_error An exception will be thrown if the inplace_any is empty,
        or holds a different type than the requested type.

        @see try_get()
     */
    template <typename T>
    [[nodiscard]] std::add_const_t<std::decay_t<T>>& get() const
        requires(can_store<T>())
    {
        using Stored = std::decay_t<T>;

        if (not holds_type<Stored>())
            throw std::logic_error {
                std::format(
                    "Wrong type requested from inplace_any (holds {}, requested {})",
                    std::string { get_stored_type().name() },
                    std::string { std::type_index { typeid(Stored) }.name() })
            };

        return *std::launder(reinterpret_cast<const Stored*>(storage)); // NOLINT
    }

    /** Returns a pointer to the held object, or ``nullptr`` if this inplace_any doesn't
        hold the requested type.
        @see get()
     */
    template <typename T>
    [[nodiscard]] std::decay_t<T>* try_get() noexcept
        requires(can_store<T>())
    {
        using Stored = std::decay_t<T>;

        if (holds_type<Stored>())
            return std::launder(reinterpret_cast<Stored*>(storage)); // NOLINT

        return nullptr;
    }

    /** Returns a pointer to the held object, or ``nullptr`` if this inplace_any doesn't
        hold the requested type.
        @see get()
     */
    template <typename T>
    [[nodiscard]] std::add_const_t<std::decay_t<T>>* try_get() const noexcept
        requires(can_store<T>())
    {
        using Stored = std::decay_t<T>;

        if (holds_type<Stored>())
            return std::launder(reinterpret_cast<const Stored*>(storage)); // NOLINT

        return nullptr;
    }

    /** Returns a ``std::type_index`` representing the type of the owned object, if any.
        If the inplace_any is empty, returns the ``std::type_index`` for ``void``.
        @see holdsType()
     */
    [[nodiscard]] std::type_index get_stored_type() const noexcept
    {
        if (empty())
            return { typeid(void) };

        std::type_index ret { typeid(void) };

        dispatcher(reinterpret_cast<std::byte*>(&ret), Func::GetTypeInfo, nullptr); // NOLINT

        return ret;
    }

    /** Returns true if this inplace_any holds an object of the given type.
        @see getStoredType()
     */
    template <typename T>
    [[nodiscard]] bool holds_type() const noexcept
    {
        if constexpr (can_store<T>()) {
            using Stored = std::decay_t<T>;

            return dispatcher == &call_method_on_object<Stored>;
        } else {
            return false;
        }
    }

    /** Swaps two inplace_any objects.
        If the two Anys both hold objects of the same type, then this will call the user-defined ``swap``
        function found by ADL, if any; otherwise, this is a primitive swap operation on the internal byte
        buffer and metadata.
     */
    void swap(inplace_any& other) noexcept
    {
        if (has_value() and dispatcher == other.dispatcher) {
            assert(dispatcher != nullptr);
            dispatcher(storage, Func::Swap, &other);
            return;
        }

        using std::swap;

        swap(storage, other.storage);
        swap(dispatcher, other.dispatcher);
    }

private:
    enum class Func : std::uint_least8_t {
        Destruct,
        CopyAssign,
        CopyConstruct,
        MoveAssign,
        MoveConstruct,
        Swap,
        GetTypeInfo
    };

    using DispatchFunc = void(std::byte*, Func, inplace_any*);

    template <typename T>
    static void call_method_on_object(std::byte* obj, Func func, inplace_any* arg)
        noexcept(detail::is_nothrow_copyable<T>()
                 and detail::is_nothrow_movable<T>()
                 and std::is_nothrow_swappable_v<T>);

    template <typename T>
    static void call_destructor(std::byte* obj) noexcept // std::destroy_at isn't noexcept for some reason, but a type that throws from its destructor is probably ill-formed
    {
        std::destroy_at(std::launder(reinterpret_cast<T*>(obj))); // NOLINT
    }

    template <typename T>
    static void call_copy_construct(std::byte* obj, const inplace_any* arg)
        noexcept(std::is_nothrow_copy_constructible_v<T>)
    {
        assert(arg != nullptr);
        assert(arg->holds_type<T>());

        const auto& otherObj = arg->get<T>();

        std::construct_at(
            reinterpret_cast<T*>(obj), // NOLINT
            otherObj);
    }

    template <typename T>
    static void call_copy_assign(std::byte* obj, const inplace_any* arg)
        noexcept(std::is_nothrow_copy_assignable_v<T>)
        requires std::copyable<T>
    {
        assert(arg != nullptr);
        assert(arg->holds_type<T>());

        const auto& otherObj = arg->get<T>();

        auto& thisObj = *std::launder(reinterpret_cast<T*>(obj)); // NOLINT

        thisObj = otherObj;
    }

    template <typename T>
    static void call_move_construct(std::byte* obj, inplace_any* arg)
        noexcept(std::is_nothrow_move_constructible_v<T>)
    {
        assert(arg != nullptr);
        assert(arg->holds_type<T>());

        auto& otherObj = arg->get<T>();

        std::construct_at(
            reinterpret_cast<T*>(obj), // NOLINT
            std::move(otherObj));
    }

    template <typename T>
    static void call_move_assign(std::byte* obj, inplace_any* arg)
        noexcept(std::is_nothrow_move_assignable_v<T>)
        requires std::movable<T>
    {
        assert(arg != nullptr);
        assert(arg->holds_type<T>());

        auto& otherObj = arg->get<T>();
        auto& thisObj  = *std::launder(reinterpret_cast<T*>(obj)); // NOLINT

        thisObj = std::move(otherObj);
    }

    template <typename T>
    static void call_swap(std::byte* obj, inplace_any* arg)
        noexcept(std::is_nothrow_swappable_v<T>)
    {
        static_assert(std::is_swappable_v<T>);

        assert(arg != nullptr);
        assert(arg->holds_type<T>());

        auto& otherObj = arg->get<T>();
        auto& thisObj  = *std::launder(reinterpret_cast<T*>(obj)); // NOLINT

        using std::swap;

        swap(thisObj, otherObj);
    }

    alignas(Alignment) std::byte storage[Size] {}; // NOLINT

    DispatchFunc* dispatcher { nullptr };
};

/** Swaps two inplace_any objects.
    @relates inplace_any
    @ingroup util
 */
template <size_t S, size_t A>
void swap(inplace_any<S, A>& first, inplace_any<S, A>& second) noexcept
{
    first.swap(second);
}

template <size_t S, size_t A>
auto inplace_any<S, A>::operator=(const inplace_any& other) -> inplace_any&
{
    if (this == std::addressof(other)) [[unlikely]]
        return *this;

    if (other.empty()) {
        reset();
        return *this;
    }

    if (dispatcher != other.dispatcher)
        reset();

    const auto opCode = has_value() ? Func::CopyAssign : Func::CopyConstruct;

    dispatcher = other.dispatcher;

    auto& mutableOther = const_cast<inplace_any&>(other); // NOLINT

    assert(dispatcher != nullptr);

    dispatcher(storage, opCode, &mutableOther);

    return *this;
}

template <size_t S, size_t A>
auto inplace_any<S, A>::operator=(inplace_any&& other) noexcept -> inplace_any&
{
    if (this == std::addressof(other)) [[unlikely]]
        return *this;

    if (other.empty()) {
        reset();
        return *this;
    }

    if (dispatcher != other.dispatcher)
        reset();

    const auto opCode = has_value() ? Func::MoveAssign : Func::MoveConstruct;

    dispatcher = other.dispatcher;

    assert(dispatcher != nullptr);

    dispatcher(storage, opCode, &other);

    return *this;
}

// clang-format off
template <size_t S, size_t A>
template <typename T>
auto inplace_any<S, A>::operator=(const T& object)
    noexcept(detail::is_nothrow_copyable<T>()) -> inplace_any&
    requires(NotBaseOf<T> and can_store<T>())
// clang-format on
{
    using Stored = std::decay_t<T>;

    if (not holds_type<Stored>()) {
        emplace<Stored>(std::forward<T>(object));
        return *this;
    }

    if constexpr (std::copyable<Stored>) {
        auto& thisObj = *std::launder(reinterpret_cast<Stored*>(storage)); // NOLINT
        thisObj       = object;
    } else {
        // destroy current object, copy-construct a new one
        emplace<Stored>(std::forward<T>(object));
    }

    return *this;
}

// clang-format off
template <size_t S, size_t A>
template <typename T>
auto inplace_any<S, A>::operator=(T&& object)
    noexcept(detail::is_nothrow_movable<T>()) -> inplace_any&
    requires(NotBaseOf<T> and can_store<T>())
// clang-format on
{
    using Stored = std::decay_t<T>;

    if (not holds_type<Stored>()) {
        emplace<Stored>(std::forward<T>(object));
        return *this;
    }

    if constexpr (std::movable<Stored>) {
        auto& thisObj = *std::launder(reinterpret_cast<Stored*>(storage)); // NOLINT
        thisObj       = std::forward<T>(object);
    } else {
        // destroy current object, move-construct a new one
        emplace<Stored>(std::forward<T>(object));
    }

    return *this;
}

template <size_t S, size_t A>
template <typename T>
void inplace_any<S, A>::call_method_on_object(
    std::byte* obj, const Func func, inplace_any* arg)
    noexcept(detail::is_nothrow_copyable<T>()
             and detail::is_nothrow_movable<T>()
             and std::is_nothrow_swappable_v<T>)
{
    static_assert(can_store<T>());
    static_assert(std::is_same_v<T, std::decay_t<T>>);

    assert(obj != nullptr);

    switch (func) {
        case (Func::Destruct):
            return call_destructor<T>(obj);

        case (Func::Swap):
            return call_swap<T>(obj, arg);

        case (Func::CopyConstruct):
            return call_copy_construct<T>(obj, arg);

        case (Func::MoveConstruct):
            return call_move_construct<T>(obj, arg);

        case (Func::CopyAssign): {
            // copy-assign if supported, otherwise destroy & copy-construct
            if constexpr (std::copyable<T>) {
                call_copy_assign<T>(obj, arg);
            } else {
                call_destructor<T>(obj);
                call_copy_construct<T>(obj, arg);
            }

            return;
        }

        case (Func::MoveAssign): {
            // move-assign if supported, otherwise destroy & move-construct
            if constexpr (std::movable<T>) {
                call_move_assign<T>(obj, arg);
            } else {
                call_destructor<T>(obj);
                call_move_construct<T>(obj, arg);
            }

            return;
        }

        case (Func::GetTypeInfo): {
            auto& index = *reinterpret_cast<std::type_index*>(obj); // NOLINT

            index = typeid(T);
        }
    }
}

} // namespace chess::util
