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

#include <algorithm>
#include <cassert>
#include <format>
#include <libchess/uci/Options.hpp>
#include <libutil/Strings.hpp>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace chess::uci {

using std::string;
using std::string_view;

using util::strings::split_at_first_space;
using util::strings::trim;

// defined out-of-line to address -Wweak-vtables
Option::~Option() = default;

BoolOption::BoolOption(
    string name, const bool defaultValue, string helpString, Callback&& changeCallback)
    : optionName { std::move(name) }
    , optionDefault { defaultValue }
    , help { std::move(helpString) }
    , onChange { std::move(changeCallback) }
{
}

auto BoolOption::get_declaration_string() const -> string
{
    return std::format(
        "option name {} type check default {}",
        optionName, optionDefault);
}

inline constexpr string_view VALUE_TOKEN { "value" };

void BoolOption::handle_setvalue(const string_view arguments)
{
    auto [valueToken, valueStr] = split_at_first_space(arguments);

    valueToken = trim(valueToken);

    if (valueToken != VALUE_TOKEN)
        return;

    valueStr = trim(valueStr);

    // check that the input string doesn't match the default value
    // (so that the default value is used if the input string isn't recognized)
    if (optionDefault) {
        value = valueStr != "false";
    } else {
        value = valueStr == "true";
    }

    onChange(value);
}

void BoolOption::set_value(const bool newValue)
{
    value = newValue;

    onChange(value);
}

/*------------------------------------------------------------------------------------------------------------------*/

IntOption::IntOption(
    string    name,
    const int minValue, const int maxValue,
    const int  defaultValue,
    string     helpString,
    Callback&& changeCallback)
    : optionName { std::move(name) }
    , optionMin { minValue }
    , optionMax { maxValue }
    , optionDefault { defaultValue }
    , help { std::move(helpString) }
    , onChange { std::move(changeCallback) }
{
    assert(std::cmp_greater_equal(optionMax, optionMin));
    assert(std::cmp_greater_equal(optionDefault, optionMin));
    assert(std::cmp_less_equal(optionDefault, optionMax));
}

auto IntOption::get_declaration_string() const -> string
{
    return std::format(
        "option name {} type spin default {} min {} max {}",
        optionName, optionDefault, optionMin, optionMax);
}

void IntOption::handle_setvalue(const string_view arguments)
{
    auto [valueToken, valueStr] = split_at_first_space(arguments);

    valueToken = trim(valueToken);

    if (valueToken != VALUE_TOKEN)
        return;

    const auto newValue = util::strings::int_from_string(
        trim(valueStr), value);

    value = std::clamp(newValue, optionMin, optionMax);

    onChange(value);
}

void IntOption::set_value(const int newValue)
{
    value = std::clamp(newValue, optionMin, optionMax);

    onChange(value);
}

/*------------------------------------------------------------------------------------------------------------------*/

ComboOption::ComboOption(
    string              name,
    std::vector<string> values,
    string              defaultValue,
    string              helpString,
    Callback&&          changeCallback)
    : optionName { std::move(name) }
    , possibleValues { std::move(values) }
    , optionDefault { std::move(defaultValue) }
    , help { std::move(helpString) }
    , onChange { std::move(changeCallback) }
{
    assert(std::ranges::contains(possibleValues, optionDefault));
}

auto ComboOption::get_declaration_string() const -> string
{
    auto result = std::format(
        "option name {} type combo default {}",
        optionName, optionDefault);

    for (const auto& val : possibleValues)
        result.append(
            std::format(" var {}", val));

    return result;
}

void ComboOption::handle_setvalue(const string_view arguments)
{
    auto [valueToken, valueStr] = split_at_first_space(arguments);

    valueToken = trim(valueToken);

    if (valueToken != VALUE_TOKEN)
        return;

    valueStr = trim(valueStr);

    if (std::ranges::contains(possibleValues, valueStr))
        value = valueStr;
    else
        value = optionDefault;

    onChange(value);
}

void ComboOption::set_value(string_view newValue)
{
    newValue = trim(newValue);

    if (std::ranges::contains(possibleValues, newValue))
        value = newValue;
    else
        value = optionDefault;

    onChange(value);
}

/*------------------------------------------------------------------------------------------------------------------*/

StringOption::StringOption(
    string     name,
    string     defaultValue,
    string     helpString,
    Callback&& changeCallback)
    : optionName { std::move(name) }
    , optionDefault { std::move(defaultValue) }
    , help { std::move(helpString) }
    , onChange { std::move(changeCallback) }
{
}

auto StringOption::get_declaration_string() const -> string
{
    return std::format(
        "option name {} type string default {}",
        optionName, value);
}

void StringOption::handle_setvalue(const string_view arguments)
{
    auto [valueToken, valueStr] = split_at_first_space(arguments);

    valueToken = trim(valueToken);

    if (valueToken != VALUE_TOKEN)
        return;

    value = trim(valueStr);

    onChange(value);
}

void StringOption::set_value(const string_view newValue)
{
    value = trim(newValue);

    onChange(value);
}

/*------------------------------------------------------------------------------------------------------------------*/

Action::Action(
    string     name,
    Callback&& action,
    string     helpString)
    : optionName { std::move(name) }
    , callback { std::move(action) }
    , help { std::move(helpString) }
{
    assert(callback != nullptr);
}

auto Action::get_declaration_string() const -> string
{
    return std::format("option name {} type button", optionName);
}

void Action::handle_setvalue(
    [[maybe_unused]] const string_view arguments)
{
    callback();
}

} // namespace chess::uci
