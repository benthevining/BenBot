/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

#include <algorithm>
#include <cassert>
#include <charconv>
#include <format>
#include <libchess/uci/Options.hpp>
#include <libchess/util/Strings.hpp>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace chess::uci {
using util::split_at_first_space;
using util::trim;

void Option::parse(const std::string_view arguments)
{
    auto [firstWord, rest] = split_at_first_space(arguments);

    firstWord = trim(firstWord);

    if (firstWord != "name")
        return;

    auto [name, rest2] = split_at_first_space(rest);

    name = trim(name);

    if (name == get_name())
        handle_setvalue(rest2);
}

/*------------------------------------------------------------------------------------------------------------------*/

BoolOption::BoolOption(
    std::string name, const bool defaultValue)
    : optionName { std::move(name) }
    , optionDefault { defaultValue }
{
}

[[nodiscard]] std::string BoolOption::get_declaration_string()
{
    return std::format("name {} type check default {}", optionName, optionDefault);
}

void BoolOption::handle_setvalue(const std::string_view arguments)
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
    std::string name,
    const int minValue, const int maxValue,
    const int defaultValue)
    : optionName { std::move(name) }
    , optionMin { minValue }
    , optionMax { maxValue }
    , optionDefault { defaultValue }
{
    assert(optionDefault >= optionMin);
    assert(optionDefault <= optionMax);
}

std::string IntOption::get_declaration_string()
{
    return std::format(
        "name {} type spin default {} min {} max {}",
        optionName, optionDefault, optionMin, optionMax);
}

void IntOption::handle_setvalue(const std::string_view arguments)
{
    auto [valueToken, valueStr] = split_at_first_space(arguments);

    valueToken = trim(valueToken);

    if (valueToken != "value")
        return;

    valueStr = trim(valueStr);

    std::from_chars(
        valueStr.data(), valueStr.data() + valueStr.length(), value);

    value = std::clamp(value, optionMin, optionMax);
}

/*------------------------------------------------------------------------------------------------------------------*/

ComboOption::ComboOption(
    std::string              name,
    std::vector<std::string> values,
    std::string              defaultValue)
    : optionName { std::move(name) }
    , possibleValues { std::move(values) }
    , optionDefault { std::move(defaultValue) }
{
    assert(std::ranges::contains(possibleValues, optionDefault));
}

std::string ComboOption::get_declaration_string()
{
    auto result = std::format("name {} type combo default {}", optionName, optionDefault);

    for (const auto& value : possibleValues)
        result.append(
            std::format("var {}", value));

    return result;
}

void ComboOption::handle_setvalue(const std::string_view arguments)
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
    std::string name,
    std::string defaultValue)
    : optionName { std::move(name) }
    , value { std::move(defaultValue) }
{
}

std::string StringOption::get_declaration_string()
{
    return std::format("name {} type string default {}", optionName, value);
}

void StringOption::handle_setvalue(std::string_view arguments)
{
    auto [valueToken, valueStr] = split_at_first_space(arguments);

    valueToken = trim(valueToken);

    if (valueToken != "value")
        return;

    valueStr = trim(valueStr);

    value = valueStr;
}

/*------------------------------------------------------------------------------------------------------------------*/

Action::Action(
    std::string name,
    Callback&&  action)
    : optionName { std::move(name) }
    , callback { std::move(action) }
{
    assert(callback != nullptr);
}

std::string Action::get_declaration_string()
{
    return std::format("name {} type button", optionName);
}

void Action::handle_setvalue(
    [[maybe_unused]] const std::string_view arguments)
{
    callback();
}

} // namespace chess::uci
