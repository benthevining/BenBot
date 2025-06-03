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
#include <iterator>
#include <libchess/moves/Move.hpp>
#include <libchess/notation/FEN.hpp>
#include <libchess/notation/UCI.hpp>
#include <libchess/uci/CommandParsing.hpp>
#include <libchess/util/Strings.hpp>
#include <string_view>

namespace chess::uci {

using util::split_at_first_space;
using util::trim;

// Note that in all UCI parsing, we need to use trim() defensively a lot,
// because UCI allows arbitrary whitespace between tokens. Also note that
// split_at_first_space() will return a pair whose first element is empty
// if its input string began with a space.

Position parse_position_options(std::string_view options)
{
    // position [fen <fenstring> | startpos ]  moves <move1> .... <movei>
    // options doesn't include the "position" token itself

    options = trim(options);

    Position position {};

    auto [secondWord, rest] = split_at_first_space(options);

    secondWord = trim(secondWord);

    if (secondWord == "fen") {
        // we have to take care here, because the FEN string won't be quoted,
        // so we search for the "moves" delimiter (which may be absent)
        const auto movesTokenIdx = rest.find("moves");

        const bool isNPos = movesTokenIdx == std::string_view::npos;

        const auto fenString = isNPos ? rest : rest.substr(0uz, movesTokenIdx);

        position = notation::from_fen(fenString);

        if (isNPos) {
            // the "moves" token wasn't found, so assume that the FEN string
            // was the last thing in the position command
            return position;
        }

        rest = trim(rest.substr(movesTokenIdx));
    } else {
        rest = trim(rest);
    }

    auto [moveToken, moves] = split_at_first_space(rest);

    moveToken = trim(moveToken);

    // code defensively against unrecognized tokens
    if (moveToken != "moves")
        return position;

    moves = trim(moves);

    while (! moves.empty()) {
        const auto [firstMove, rest2] = split_at_first_space(moves);

        position.make_move(
            notation::from_uci(position, firstMove));

        moves = trim(rest2);
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

        return trim(rest);
    }

    // consumes all the moves following the "searchmoves" token,
    // and returns the rest of the ``options`` that are left
    [[nodiscard]] std::string_view parse_searchmoves(
        std::string_view options, const Position& currentPosition,
        std::output_iterator<moves::Move> auto outputIt)
    {
        using std::operator""sv;

        // we could instead find the first token that doesn't successfully parse as
        // a UCI move, but from_uci() throws on failure, so instead we detect the
        // other delimiter tokens
        static constexpr std::array argumentTokens {
            "ponder"sv, "wtime"sv, "btime"sv, "winc"sv, "binc"sv, "infinite"sv,
            "movestogo"sv, "depth"sv, "nodes"sv, "mate"sv, "movetime"sv
        };

        while (! options.empty()) {
            auto [firstMove, rest] = split_at_first_space(options);

            firstMove = trim(firstMove);

            if (std::ranges::contains(argumentTokens, firstMove))
                return options; // NOLINT

            options = trim(rest);

            *outputIt = notation::from_uci(currentPosition, firstMove);
        }

        return options; // NOLINT
    }

} // namespace

GoCommandOptions parse_go_options(
    std::string_view options, const Position& currentPosition)
{
    // options doesn't include the "go" token itself

    options = trim(options);

    GoCommandOptions ret;

    while (! options.empty()) {
        auto [firstWord, rest] = split_at_first_space(options);

        firstWord = trim(firstWord);
        rest      = trim(rest);

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
            options = parse_searchmoves(
                rest, currentPosition, std::back_inserter(ret.moves));
        }
    }

    return ret;
}

} // namespace chess::uci
