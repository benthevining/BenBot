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
    This file provides classes for declaring UCI @cite Meyer-Kahlen_2006 engine parameters.
    @ingroup uci
 */

#pragma once

#include <functional>
#include <magic_enum/magic_enum.hpp>
#include <ranges>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

namespace chess::uci {

using std::string;
using std::string_view;

/** Base class for UCI @cite Meyer-Kahlen_2006 options.

    @ingroup uci
 */
struct Option {
    Option() = default;

    virtual ~Option();

    Option(const Option&)            = default;
    Option(Option&&)                 = default;
    Option& operator=(const Option&) = default;
    Option& operator=(Option&&)      = default;

    /** Returns this option's name. */
    [[nodiscard]] virtual auto get_name() const noexcept -> string_view = 0;

    /** Returns the option's declaration string suitable for sending
        to the GUI. The returned string includes the "option" token.
     */
    [[nodiscard]] virtual auto get_declaration_string() const -> string = 0;

    /** Returns a textual representation of this option's type. */
    [[nodiscard]] virtual auto get_type() const noexcept -> string_view = 0;

    /** Returns the help string for this option. */
    [[nodiscard]] virtual auto get_help() const noexcept -> string_view = 0;

    /** Returns true if this option type has an associated value.
        False only for Action options.
     */
    [[nodiscard]] virtual auto has_value() const noexcept -> bool { return true; }

    /** Represents a variant that can hold any of the derived class's value types. */
    using Variant = std::variant<bool, int, string_view>;

    /** Returns this option's current value, as a variant.
        Note that if ``has_value()`` returns false, you must not call this method!
     */
    [[nodiscard]] virtual auto get_value_variant() const -> Variant = 0;

    /** Returns this option's default value, as a variant.
        Note that if ``has_value()`` returns false, you must not call this method!
     */
    [[nodiscard]] virtual auto get_default_value_variant() const -> Variant = 0;

    /** Resets the option's value to the default. */
    virtual void reset_to_default_value() = 0;

    /** Will be called with everything in the "setoption" command after the option name.
        This is typically not called directly by user code.
     */
    virtual void handle_setvalue(string_view arguments) = 0;
};

/** A boolean toggle option, which may be either on or off.

    @ingroup uci
 */
struct BoolOption final : Option {
    using Value    = bool;
    using Callback = std::function<void(bool)>;

    /** Creates a boolean option. */
    BoolOption(
        string name, bool defaultValue,
        string     helpString,
        Callback&& changeCallback = [](bool) { });

    /** Returns this option's current value, as set by the last
        call to ``parse()``.
     */
    [[nodiscard]] auto get_value() const noexcept -> bool { return value; }

    /** Sets the option's value directly. */
    void set_value(bool newValue);

    [[nodiscard]] auto get_value_variant() const -> Variant override { return value; }

    /** Returns this option's default value. */
    [[nodiscard]] auto get_default_value() const noexcept -> bool { return optionDefault; }

    [[nodiscard]] auto get_default_value_variant() const -> Variant override { return optionDefault; }

    void reset_to_default_value() override { value = optionDefault; }

    [[nodiscard]] auto get_name() const noexcept -> string_view override { return optionName; }

    [[nodiscard]] auto get_declaration_string() const -> string override;

    [[nodiscard]] auto get_type() const noexcept -> string_view override { return "Toggle"; }

    [[nodiscard]] auto get_help() const noexcept -> string_view override { return help; }

    void handle_setvalue(string_view arguments) override;

private:
    string optionName;

    bool optionDefault { true };

    bool value { optionDefault };

    string help;

    Callback onChange { [](bool) { } };
};

/** An integer option.

    @ingroup uci
 */
struct IntOption final : Option {
    using Value    = int;
    using Callback = std::function<void(int)>;

    /** Creates an integer option. */
    IntOption(
        string name,
        int minValue, int maxValue,
        int        defaultValue,
        string     helpString,
        Callback&& changeCallback = [](int) { });

    /** Returns this option's current value, as set by the last
        call to ``parse()``.
     */
    [[nodiscard]] auto get_value() const noexcept -> int { return value; }

    /** Sets the option's value directly. */
    void set_value(int newValue);

    [[nodiscard]] auto get_value_variant() const -> Variant override { return value; }

    /** Returns this option's default value. */
    [[nodiscard]] auto get_default_value() const noexcept -> int { return optionDefault; }

    [[nodiscard]] auto get_default_value_variant() const -> Variant override { return optionDefault; }

    void reset_to_default_value() override { value = optionDefault; }

    [[nodiscard]] auto get_name() const noexcept -> string_view override { return optionName; }

    [[nodiscard]] auto get_declaration_string() const -> string override;

    [[nodiscard]] auto get_type() const noexcept -> string_view override { return "Integer"; }

    [[nodiscard]] auto get_help() const noexcept -> string_view override { return help; }

    void handle_setvalue(string_view arguments) override;

private:
    string optionName;

    int optionMin { 0 };
    int optionMax { 100 };

    int optionDefault { 0 };

    int value { optionDefault };

    string help;

    Callback onChange { [](int) { } };
};

/** A multiple-choice option that can have one of several predefined string values.

    @ingroup uci
    @see EnumOption
 */
struct ComboOption : Option {
    using Value    = string_view;
    using Callback = std::function<void(string_view)>;

    /** Creates a multiple-choice option. */
    ComboOption(
        string              name,
        std::vector<string> values,
        string              defaultValue,
        string              helpString,
        Callback&&          changeCallback = [](string_view) { });

    [[nodiscard]] auto get_value() const noexcept -> string_view { return value; } // cppcheck-suppress returnByReference

    /** Sets the option's value directly. */
    void set_value(string_view newValue);

    [[nodiscard]] auto get_value_variant() const -> Variant override { return get_value(); }

    /** Returns this option's default value. */
    [[nodiscard]] auto get_default_value() const noexcept -> string_view { return optionDefault; } // cppcheck-suppress returnByReference

    [[nodiscard]] auto get_default_value_variant() const -> Variant override { return get_default_value(); }

    void reset_to_default_value() override { value = optionDefault; }

    [[nodiscard]] auto get_name() const noexcept -> string_view override { return optionName; }

    [[nodiscard]] auto get_declaration_string() const -> string override;

    [[nodiscard]] auto get_type() const noexcept -> string_view override { return "Multichoice"; }

    [[nodiscard]] auto get_help() const noexcept -> string_view override { return help; }

    void handle_setvalue(string_view arguments) override;

    /** Returns the possible value strings that the combo option may be assigned to. */
    [[nodiscard]] auto get_possible_values() const noexcept
        -> std::span<const string>
    {
        return possibleValues;
    }

private:
    string optionName;

    std::vector<string> possibleValues;

    string optionDefault;

    string value { optionDefault };

    string help;

    Callback onChange { [](string_view) { } };
};

/** This simple concept matches any enumeration type. */
template <typename T>
concept Enum = std::is_enum_v<T>;

/** A ComboOption with options for each value of an enum.

    @ingroup uci
    @see ComboOption
 */
template <Enum T>
struct EnumOption final : ComboOption {
    /** Creates an enum option. */
    EnumOption(
        string name, T defaultValue, string helpString);

    /** Returns the option's current value as an enum. */
    [[nodiscard]] auto get_enum_value() const -> T;
};

/** An option that can have any arbitrary string value.

    @ingroup uci
 */
struct StringOption final : Option {
    using Value    = string_view;
    using Callback = std::function<void(string_view)>;

    /** Creates a string option. */
    StringOption(
        string     name,
        string     defaultValue,
        string     helpString,
        Callback&& changeCallback = [](string_view) { });

    [[nodiscard]] auto get_value() const noexcept -> string_view
    {
        if (value == "<empty>")
            return { };

        return value;
    }

    /** Sets the option's value directly. */
    void set_value(string_view newValue);

    [[nodiscard]] auto get_value_variant() const -> Variant override { return get_value(); }

    [[nodiscard]] auto get_default_value_variant() const -> Variant override { return optionDefault; }

    void reset_to_default_value() override { value = optionDefault; }

    [[nodiscard]] auto get_name() const noexcept -> string_view override { return optionName; }

    [[nodiscard]] auto get_declaration_string() const -> string override;

    [[nodiscard]] auto get_type() const noexcept -> string_view override { return "String"; }

    [[nodiscard]] auto get_help() const noexcept -> string_view override { return help; }

    void handle_setvalue(string_view arguments) override;

private:
    string optionName;

    string optionDefault;

    string value { optionDefault };

    string help;

    Callback onChange { [](string_view) { } };
};

/** A triggerable action.

    @ingroup uci
 */
struct Action final : Option {
    using Value    = void;
    using Callback = std::function<void()>;

    /** Creates an action option. */
    Action(
        string     name,
        Callback&& action,
        string     helpString);

    [[nodiscard]] auto get_value_variant() const -> Variant override { throw_value_error(); }
    [[nodiscard]] auto get_default_value_variant() const -> Variant override { throw_value_error(); }

    /** Does nothing. */
    void reset_to_default_value() override { }

    [[nodiscard]] auto get_name() const noexcept -> string_view override { return optionName; }

    [[nodiscard]] auto get_declaration_string() const -> string override;

    [[nodiscard]] auto get_type() const noexcept -> string_view override { return "Button"; }

    [[nodiscard]] auto get_help() const noexcept -> string_view override { return help; }

    [[nodiscard]] auto has_value() const noexcept -> bool override { return false; }

    void handle_setvalue(string_view arguments) override;

private:
    [[noreturn]] static void throw_value_error()
    {
        throw std::logic_error { "get_value_variant() called on option of Action type" };
    }

    string optionName;

    Callback callback;

    string help;
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

namespace detail {
    template <Enum T>
    [[nodiscard]] auto get_all_value_strings()
        -> std::vector<string>
    {
        return magic_enum::enum_names<T>()
             | std::views::transform([](const string_view str) { return string { str }; })
             | std::ranges::to<std::vector>();
    }
} // namespace detail

template <Enum T>
EnumOption<T>::EnumOption(
    string name, T defaultValue, string helpString)
    : ComboOption {
        std::move(name),
        detail::get_all_value_strings<T>(),
        std::string { magic_enum::enum_name(defaultValue) },
        std::move(helpString)
    }
{
}

template <Enum T>
auto EnumOption<T>::get_enum_value() const -> T
{
    return magic_enum::enum_cast<T>(get_value())
        .value();
}

} // namespace chess::uci
