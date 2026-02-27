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
    This file provides classes for declaring UCI engine parameters.
    @ingroup uci
 */

#pragma once

#include <functional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace chess::uci {

using std::string;
using std::string_view;

/** Base class for UCI options.

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

    [[nodiscard]] auto get_value_variant() const -> Variant override { return value; }

    /** Returns this option's default value. */
    [[nodiscard]] auto get_default_value() const noexcept -> bool { return optionDefault; }

    [[nodiscard]] auto get_default_value_variant() const -> Variant override { return optionDefault; }

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

    [[nodiscard]] auto get_value_variant() const -> Variant override { return value; }

    /** Returns this option's default value. */
    [[nodiscard]] auto get_default_value() const noexcept -> int { return optionDefault; }

    [[nodiscard]] auto get_default_value_variant() const -> Variant override { return optionDefault; }

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
 */
struct ComboOption final : Option {
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

    [[nodiscard]] auto get_value_variant() const -> Variant override { return get_value(); }

    /** Returns this option's default value. */
    [[nodiscard]] auto get_default_value() const noexcept -> string_view { return optionDefault; } // cppcheck-suppress returnByReference

    [[nodiscard]] auto get_default_value_variant() const -> Variant override { return get_default_value(); }

    [[nodiscard]] auto get_name() const noexcept -> string_view override { return optionName; }

    [[nodiscard]] auto get_declaration_string() const -> string override;

    [[nodiscard]] auto get_type() const noexcept -> string_view override { return "Multichoice"; }

    [[nodiscard]] auto get_help() const noexcept -> string_view override { return help; }

    void handle_setvalue(string_view arguments) override;

private:
    string optionName;

    std::vector<string> possibleValues;

    string optionDefault;

    string value { optionDefault };

    string help;

    Callback onChange { [](string_view) { } };
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

    [[nodiscard]] auto get_value_variant() const -> Variant override { return get_value(); }

    [[nodiscard]] auto get_default_value_variant() const -> Variant override { return string_view { }; }

    [[nodiscard]] auto get_name() const noexcept -> string_view override { return optionName; }

    [[nodiscard]] auto get_declaration_string() const -> string override;

    [[nodiscard]] auto get_type() const noexcept -> string_view override { return "String"; }

    [[nodiscard]] auto get_help() const noexcept -> string_view override { return help; }

    void handle_setvalue(string_view arguments) override;

private:
    string optionName;

    string value;

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

} // namespace chess::uci
