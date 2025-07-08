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
#include <string>
#include <string_view>
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

protected:
    // Will be called with everything in the "setoption" command after the option name
    virtual void handle_setvalue(string_view arguments) = 0;
};

/** A boolean toggle option, which may be either on or off.

    @ingroup uci
 */
struct BoolOption final : Option {
    BoolOption(string name, bool defaultValue);

    using Value = bool;

    /** Returns this option's current value, as set by the last
        call to ``parse()``.
     */
    [[nodiscard]] bool get_value() const noexcept { return value; }

    /** Returns this option's default value. */
    [[nodiscard]] bool get_default_value() const noexcept { return optionDefault; }

    [[nodiscard]] string_view get_name() const noexcept override { return optionName; }

    [[nodiscard]] string get_declaration_string() const override;

private:
    void handle_setvalue(string_view arguments) override;

    string optionName;

    bool optionDefault { true };

    bool value { optionDefault };
};

/** An integer option.

    @ingroup uci
 */
struct IntOption final : Option {
    IntOption(
        string name,
        int minValue, int maxValue,
        int defaultValue);

    using Value = int;

    /** Returns this option's current value, as set by the last
        call to ``parse()``.
     */
    [[nodiscard]] int get_value() const noexcept { return value; }

    /** Returns this option's default value. */
    [[nodiscard]] int get_default_value() const noexcept { return optionDefault; }

    [[nodiscard]] string_view get_name() const noexcept override { return optionName; }

    [[nodiscard]] string get_declaration_string() const override;

private:
    void handle_setvalue(string_view arguments) override;

    string optionName;

    int optionMin { 0 };
    int optionMax { 100 };

    int optionDefault { 0 };

    int value { optionDefault };
};

/** A multiple-choice option that can have one of several predefined string values.

    @ingroup uci
 */
struct ComboOption final : Option {
    ComboOption(
        string              name,
        std::vector<string> values,
        string              defaultValue);

    using Value = string_view;

    [[nodiscard]] string_view get_value() const noexcept { return value; }

    /** Returns this option's default value. */
    [[nodiscard]] string_view get_default_value() const noexcept { return optionDefault; }

    [[nodiscard]] string_view get_name() const noexcept override { return optionName; }

    [[nodiscard]] string get_declaration_string() const override;

private:
    void handle_setvalue(string_view arguments) override;

    string optionName;

    std::vector<string> possibleValues;

    string optionDefault;

    string value { optionDefault };
};

/** An option that can have any arbitrary string value.
    String options are empty by default.

    @ingroup cli
 */
struct StringOption final : Option {
    StringOption(string name, string defaultValue);

    using Value = string_view;

    [[nodiscard]] string_view get_value() const noexcept { return value; }

    [[nodiscard]] string_view get_name() const noexcept override { return optionName; }

    [[nodiscard]] string get_declaration_string() const override;

private:
    void handle_setvalue(string_view arguments) override;

    string optionName;

    string value;
};

/** A triggerable action.

    @ingroup uci
 */
struct Action final : Option {
    using Callback = std::function<void()>;

    Action(string name, Callback&& action);

    using Value = void;

    [[nodiscard]] string_view get_name() const noexcept override { return optionName; }

    [[nodiscard]] string get_declaration_string() const override;

private:
    void handle_setvalue(string_view arguments) override;

    string optionName;

    Callback callback;
};

} // namespace chess::uci
