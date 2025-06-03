/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
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
    [[nodiscard]] virtual std::string_view get_name() const noexcept = 0;

    /** Returns the option's declaration string suitable for sending
        to the GUI. The returned string includes the "option" token.
     */
    [[nodiscard]] virtual std::string get_declaration_string() const = 0;

    /** Parses the arguments following a "setoption" token from the GUI,
        and updates the current state of the option object.
        The ``arguments`` should not include the "setoption" token.
        Note that this may be called for any option; this class will take
        care of filtering out updates for other options.
     */
    void parse(std::string_view arguments);

protected:
    // Will be called with everything in the "setoption" command after the option name
    virtual void handle_setvalue(std::string_view arguments) = 0;
};

/** A boolean toggle option, which may be either on or off.

    @ingroup uci
 */
struct BoolOption final : Option {
    BoolOption(
        std::string name, bool defaultValue);

    /** Returns this option's current value, as set by the last
        call to ``parse()``.
     */
    [[nodiscard]] bool get_value() const noexcept { return value; }

    [[nodiscard]] std::string_view get_name() const noexcept override { return optionName; }

    [[nodiscard]] std::string get_declaration_string() const override;

private:
    void handle_setvalue(std::string_view arguments) override;

    std::string optionName;

    bool optionDefault { true };

    bool value { optionDefault };
};

/** An integer option.

    @ingroup uci
 */
struct IntOption final : Option {
    IntOption(
        std::string name,
        int minValue, int maxValue,
        int defaultValue);

    /** Returns this option's current value, as set by the last
        call to ``parse()``.
     */
    [[nodiscard]] int get_value() const noexcept { return value; }

    [[nodiscard]] std::string_view get_name() const noexcept override { return optionName; }

    [[nodiscard]] std::string get_declaration_string() const override;

private:
    void handle_setvalue(std::string_view arguments) override;

    std::string optionName;

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
        std::string              name,
        std::vector<std::string> values,
        std::string              defaultValue);

    [[nodiscard]] std::string_view get_value() const noexcept { return value; }

    [[nodiscard]] std::string_view get_name() const noexcept override { return optionName; }

    [[nodiscard]] std::string get_declaration_string() const override;

private:
    void handle_setvalue(std::string_view arguments) override;

    std::string optionName;

    std::vector<std::string> possibleValues;

    std::string optionDefault;

    std::string value { optionDefault };
};

/** An option that can have any arbitrary string value.

    @ingroup cli
 */
struct StringOption final : Option {
    StringOption(
        std::string name,
        std::string defaultValue);

    [[nodiscard]] std::string_view get_value() const noexcept { return value; }

    [[nodiscard]] std::string_view get_name() const noexcept override { return optionName; }

    [[nodiscard]] std::string get_declaration_string() const override;

private:
    void handle_setvalue(std::string_view arguments) override;

    std::string optionName;

    std::string value;
};

/** A triggerable action.

    @ingroup uci
 */
struct Action final : Option {
    using Callback = std::function<void()>;

    Action(
        std::string name,
        Callback&&  action);

    [[nodiscard]] std::string_view get_name() const noexcept override { return optionName; }

    [[nodiscard]] std::string get_declaration_string() const override;

private:
    void handle_setvalue(std::string_view arguments) override;

    std::string optionName;

    Callback callback;
};

} // namespace chess::uci
