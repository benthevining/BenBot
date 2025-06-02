/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

#include <algorithm>
#include <array>
#include <charconv>
#include <concepts>
#include <libchess/moves/Move.hpp>
#include <libchess/notation/FEN.hpp>
#include <libchess/notation/UCI.hpp>
#include <libchess/uci/CommandParsing.hpp>
#include <libchess/util/Strings.hpp>
#include <string_view>
#include <vector>

namespace chess::uci {

using util::split_at_first_space;
using util::trim;

Position parse_position_options(const std::string_view options)
{
    // position [fen <fenstring> | startpos ]  moves <move1> .... <movei>
    // options doesn't include the "position" token itself

    Position position {};

    auto [secondWord, rest] = split_at_first_space(options);

    secondWord = trim(secondWord);

    if (secondWord == "fen") {
        const auto [fenString, afterFEN] = split_at_first_space(rest);

        position = notation::from_fen(fenString);

        rest = afterFEN;
    }

    auto [moveToken, moves] = split_at_first_space(rest);

    moveToken = trim(moveToken);

    if (moveToken != "moves")
        return position;

    while (! moves.empty()) {
        const auto [firstMove, rest2] = split_at_first_space(moves);

        position.make_move(
            notation::from_uci(position, firstMove));

        moves = rest2;
    }

    return position;
}

namespace {

    // consumes one argument from ``options``, writes its value into ``value``,
    // and returns the rest of the ``options`` that are left
    [[nodiscard]] std::string_view parse_option_value(
        const std::string_view options, std::integral auto& value)
    {
        auto [valueStr, rest] = split_at_first_space(options);

        valueStr = trim(valueStr);

        std::from_chars(
            valueStr.data(), valueStr.data() + valueStr.length(), value);

        return rest;
    }

    // consumes all the moves following the "searchmoves" token,
    // and returns the rest of the ``options`` that are left
    [[nodiscard]] std::string_view parse_searchmoves(
        std::string_view options, const Position& currentPosition,
        std::vector<moves::Move>& output)
    {
        using std::operator""sv;

        static constexpr std::array argumentTokens {
            "searchmoves"sv, "ponder"sv, "wtime"sv, "btime"sv, "winc"sv, "binc"sv,
            "movestogo"sv, "depth"sv, "nodes"sv, "mate"sv, "movetime"sv, "infinite"sv
        };

        while (! options.empty()) {
            auto [firstMove, rest] = split_at_first_space(options);

            firstMove = trim(firstMove);

            if (std::ranges::contains(argumentTokens, firstMove))
                return options; // NOLINT

            options = rest;

            output.emplace_back(
                notation::from_uci(currentPosition, firstMove));
        }

        return options; // NOLINT
    }

} // namespace

GoCommandOptions parse_go_options(
    std::string_view options, const Position& currentPosition)
{
    // options doesn't include the "go" token itself

    GoCommandOptions ret;

    while (! options.empty()) {
        auto [firstWord, rest] = split_at_first_space(options);

        firstWord = trim(firstWord);

        options = rest;

        if (firstWord == "ponder") {
            ret.ponderMode = true;
            continue;
        }

        if (firstWord == "infinite") {
            ret.infinite = true;
            continue;
        }

        if (firstWord == "wtime") {
            options = parse_option_value(rest, ret.whiteMsLeft);
            continue;
        }

        if (firstWord == "btime") {
            options = parse_option_value(rest, ret.blackMsLeft);
            continue;
        }

        if (firstWord == "winc") {
            options = parse_option_value(rest, ret.whiteIncMs);
            continue;
        }

        if (firstWord == "binc") {
            options = parse_option_value(rest, ret.blackIncMs);
            continue;
        }

        if (firstWord == "movestogo") {
            options = parse_option_value(rest, ret.movesToGo);
            continue;
        }

        if (firstWord == "depth") {
            options = parse_option_value(rest, ret.depth);
            continue;
        }

        if (firstWord == "nodes") {
            options = parse_option_value(rest, ret.nodes);
            continue;
        }

        if (firstWord == "mate") {
            options = parse_option_value(rest, ret.mateIn);
            continue;
        }

        if (firstWord == "movetime") {
            options = parse_option_value(rest, ret.searchTime);
            continue;
        }

        if (firstWord == "searchmoves") {
            options = parse_searchmoves(rest, currentPosition, ret.moves);
        }
    }

    return ret;
}

} // namespace chess::uci
