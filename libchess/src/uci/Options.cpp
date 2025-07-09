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
#include <libchess/util/Strings.hpp>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace chess::uci {

using std::string;
using std::string_view;

using util::split_at_first_space;
using util::trim;

BoolOption::BoolOption(
    string name, const bool defaultValue, string helpString)
    : optionName { std::move(name) }
    , optionDefault { defaultValue }
    , help { std::move(helpString) }
{
}

string BoolOption::get_declaration_string() const
{
    return std::format(
        "option name {} type check default {}",
        optionName, optionDefault);
}

void BoolOption::handle_setvalue(const string_view arguments)
{
    auto [valueToken, valueStr] = split_at_first_space(arguments);

    valueToken = trim(valueToken);

    if (valueToken != "value")
        return;

    valueStr = trim(valueStr);

    // check that the input string doesn't match the default value
    // (so that the default value is used if the input string isn't recognized)
    if (optionDefault) {
        value = valueStr != "false";
    } else {
        value = valueStr == "true";
    }
}

/*------------------------------------------------------------------------------------------------------------------*/

IntOption::IntOption(
    string    name,
    const int minValue, const int maxValue,
    const int defaultValue,
    string    helpString)
    : optionName { std::move(name) }
    , optionMin { minValue }
    , optionMax { maxValue }
    , optionDefault { defaultValue }
    , help { std::move(helpString) }
{
    assert(optionDefault >= optionMin);
    assert(optionDefault <= optionMax);
}

string IntOption::get_declaration_string() const
{
    return std::format(
        "option name {} type spin default {} min {} max {}",
        optionName, optionDefault, optionMin, optionMax);
}

void IntOption::handle_setvalue(const string_view arguments)
{
    auto [valueToken, valueStr] = split_at_first_space(arguments);

    valueToken = trim(valueToken);

    if (valueToken != "value")
        return;

    const auto newValue = util::int_from_string(trim(valueStr), value);

    value = std::clamp(newValue, optionMin, optionMax);
}

/*------------------------------------------------------------------------------------------------------------------*/

ComboOption::ComboOption(
    string              name,
    std::vector<string> values,
    string              defaultValue,
    string              helpString)
    : optionName { std::move(name) }
    , possibleValues { std::move(values) }
    , optionDefault { std::move(defaultValue) }
    , help { std::move(helpString) }
{
    assert(std::ranges::contains(possibleValues, optionDefault));
}

string ComboOption::get_declaration_string() const
{
    auto result = std::format(
        "option name {} type combo default {}",
        optionName, optionDefault);

    for (const auto& value : possibleValues)
        result.append(
            std::format(" var {}", value));

    return result;
}

void ComboOption::handle_setvalue(const string_view arguments)
{
    auto [valueToken, valueStr] = split_at_first_space(arguments);

    valueToken = trim(valueToken);

    if (valueToken != "value")
        return;

    valueStr = trim(valueStr);

    if (std::ranges::contains(possibleValues, valueStr)) {
        value = valueStr;
    } else {
        value = optionDefault;
    }
}

/*------------------------------------------------------------------------------------------------------------------*/

StringOption::StringOption(
    string name,
    string defaultValue,
    string helpString)
    : optionName { std::move(name) }
    , value { std::move(defaultValue) }
    , help { std::move(helpString) }
{
}

string StringOption::get_declaration_string() const
{
    return std::format(
        "option name {} type string default {}",
        optionName, value);
}

void StringOption::handle_setvalue(const string_view arguments)
{
    auto [valueToken, valueStr] = split_at_first_space(arguments);

    valueToken = trim(valueToken);

    if (valueToken != "value")
        return;

    value = trim(valueStr);
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

string Action::get_declaration_string() const
{
    return std::format("option name {} type button", optionName);
}

void Action::handle_setvalue(
    [[maybe_unused]] const string_view arguments)
{
    callback();
}

} // namespace chess::uci
