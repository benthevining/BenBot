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

    virtual ~Option() = default;

    Option(const Option&)            = default;
    Option(Option&&)                 = default;
    Option& operator=(const Option&) = default;
    Option& operator=(Option&&)      = default;

    /** Returns this option's name. */
    [[nodiscard]] virtual string_view get_name() const noexcept = 0;

    /** Returns the option's declaration string suitable for sending
        to the GUI. The returned string includes the "option" token.
     */
    [[nodiscard]] virtual string get_declaration_string() const = 0;

    /** Parses the arguments following a "setoption" token from the GUI,
        and updates the current state of the option object.
        The ``arguments`` should not include the "setoption" token.
        Note that this may be called for any option; this base class will
        take care of filtering out updates for other options.

        @returns True if this call had an effect; false if the ``arguments``
        were for a different option.
     */
    bool parse(string_view arguments);

    /** Returns a textual representation of this option's type. */
    [[nodiscard]] virtual string_view get_type() const noexcept = 0;

    /** Returns the help string for this option. */
    [[nodiscard]] virtual string_view get_help() const noexcept = 0;

    /** Returns true if this option type has an associated value.
        False only for Action options.
     */
    [[nodiscard]] virtual bool has_value() const noexcept { return true; }

    /** Represents a variant that can hold any of the derived class's value types. */
    using Variant = std::variant<bool, int, string_view>;

    /** Returns this option's current value, as a variant.
        Note that if ``has_value()`` returns false, you must not call this method!
     */
    [[nodiscard]] virtual Variant get_value_variant() const = 0;

    /** Returns this option's default value, as a variant.
        Note that if ``has_value()`` returns false, you must not call this method!
     */
    [[nodiscard]] virtual Variant get_default_value_variant() const = 0;

protected:
    // Will be called with everything in the "setoption" command after the option name
    virtual void handle_setvalue(string_view arguments) = 0;
};

/** A boolean toggle option, which may be either on or off.

    @ingroup uci
 */
struct BoolOption final : Option {
    BoolOption(string name, bool defaultValue, string helpString);

    using Value = bool;

    /** Returns this option's current value, as set by the last
        call to ``parse()``.
     */
    [[nodiscard]] bool get_value() const noexcept { return value; }

    [[nodiscard]] Variant get_value_variant() const override { return value; }

    /** Returns this option's default value. */
    [[nodiscard]] bool get_default_value() const noexcept { return optionDefault; }

    [[nodiscard]] Variant get_default_value_variant() const override { return optionDefault; }

    [[nodiscard]] string_view get_name() const noexcept override { return optionName; }

    [[nodiscard]] string get_declaration_string() const override;

    [[nodiscard]] string_view get_type() const noexcept override { return "Toggle"; }

    [[nodiscard]] string_view get_help() const noexcept override { return help; }

private:
    void handle_setvalue(string_view arguments) override;

    string optionName;

    bool optionDefault { true };

    bool value { optionDefault };

    string help;
};

/** An integer option.

    @ingroup uci
 */
struct IntOption final : Option {
    IntOption(
        string name,
        int minValue, int maxValue,
        int    defaultValue,
        string helpString);

    using Value = int;

    /** Returns this option's current value, as set by the last
        call to ``parse()``.
     */
    [[nodiscard]] int get_value() const noexcept { return value; }

    [[nodiscard]] Variant get_value_variant() const override { return value; }

    /** Returns this option's default value. */
    [[nodiscard]] int get_default_value() const noexcept { return optionDefault; }

    [[nodiscard]] Variant get_default_value_variant() const override { return optionDefault; }

    [[nodiscard]] string_view get_name() const noexcept override { return optionName; }

    [[nodiscard]] string get_declaration_string() const override;

    [[nodiscard]] string_view get_type() const noexcept override { return "Integer"; }

    [[nodiscard]] string_view get_help() const noexcept override { return help; }

private:
    void handle_setvalue(string_view arguments) override;

    string optionName;

    int optionMin { 0 };
    int optionMax { 100 };

    int optionDefault { 0 };

    int value { optionDefault };

    string help;
};

/** A multiple-choice option that can have one of several predefined string values.

    @ingroup uci
 */
struct ComboOption final : Option {
    ComboOption(
        string              name,
        std::vector<string> values,
        string              defaultValue,
        string              helpString);

    using Value = string_view;

    [[nodiscard]] string_view get_value() const noexcept { return value; }

    [[nodiscard]] Variant get_value_variant() const override { return get_value(); }

    /** Returns this option's default value. */
    [[nodiscard]] string_view get_default_value() const noexcept { return optionDefault; }

    [[nodiscard]] Variant get_default_value_variant() const override { return get_default_value(); }

    [[nodiscard]] string_view get_name() const noexcept override { return optionName; }

    [[nodiscard]] string get_declaration_string() const override;

    [[nodiscard]] string_view get_type() const noexcept override { return "Multichoice"; }

    [[nodiscard]] string_view get_help() const noexcept override { return help; }

private:
    void handle_setvalue(string_view arguments) override;

    string optionName;

    std::vector<string> possibleValues;

    string optionDefault;

    string value { optionDefault };

    string help;
};

/** An option that can have any arbitrary string value.
    String options are empty by default.

    @ingroup cli
 */
struct StringOption final : Option {
    StringOption(string name, string defaultValue, string helpString);

    using Value = string_view;

    [[nodiscard]] string_view get_value() const noexcept { return value; }

    [[nodiscard]] Variant get_value_variant() const override { return get_value(); }

    [[nodiscard]] Variant get_default_value_variant() const override { return string_view {}; }

    [[nodiscard]] string_view get_name() const noexcept override { return optionName; }

    [[nodiscard]] string get_declaration_string() const override;

    [[nodiscard]] string_view get_type() const noexcept override { return "String"; }

    [[nodiscard]] string_view get_help() const noexcept override { return help; }

private:
    void handle_setvalue(string_view arguments) override;

    string optionName;

    string value;

    string help;
};

/** A triggerable action.

    @ingroup uci
 */
struct Action final : Option {
    using Callback = std::function<void()>;

    Action(string name, Callback&& action, string helpString);

    using Value = void;

    [[nodiscard]] Variant get_value_variant() const override { throw_value_error(); }
    [[nodiscard]] Variant get_default_value_variant() const override { throw_value_error(); }

    [[nodiscard]] string_view get_name() const noexcept override { return optionName; }

    [[nodiscard]] string get_declaration_string() const override;

    [[nodiscard]] string_view get_type() const noexcept override { return "Button"; }

    [[nodiscard]] string_view get_help() const noexcept override { return help; }

    [[nodiscard]] bool has_value() const noexcept override { return false; }

private:
    void handle_setvalue(string_view arguments) override;

    [[noreturn]] static void throw_value_error()
    {
        throw std::logic_error { "get_value_variant() called on option of Action type" };
    }

    string optionName;

    Callback callback;

    string help;
};

} // namespace chess::uci
