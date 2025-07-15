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
#include <array>
#include <iterator>
#include <libchess/moves/Move.hpp>
#include <libchess/notation/FEN.hpp>
#include <libchess/notation/UCI.hpp>
#include <libchess/uci/CommandParsing.hpp>
#include <libchess/util/Strings.hpp>
#include <optional>
#include <string_view>
#include <utility>

namespace chess::uci {

using std::string_view;

using util::split_at_first_space;
using util::trim;

// Note that in all UCI parsing, we need to use trim() defensively a lot,
// because UCI allows arbitrary whitespace between tokens. Also note that
// split_at_first_space() will return a pair whose first element is empty
// if its input string began with a space!

Position parse_position_options(string_view options)
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

        const bool isNPos = movesTokenIdx == string_view::npos;

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

    if (moveToken != "moves") // code defensively against unrecognized tokens
        return position;

    moves = trim(moves);

    while (not moves.empty()) {
        const auto [firstMove, rest2] = split_at_first_space(moves);

        position.make_move(
            notation::from_uci(position, firstMove));

        moves = trim(rest2);
    }

    return position;
}

namespace {

    // consumes one argument from ``options``,
    // and returns pair of the option value & the rest of the ``options`` that are left
    [[nodiscard]] std::pair<size_t, string_view>
    parse_int_value(const string_view options)
    {
        const auto [valueStr, rest] = split_at_first_space(options);

        return {
            util::int_from_string<size_t>(trim(valueStr)),
            trim(rest)
        };
    }

    // consumes all the moves following the "searchmoves" token,
    // and returns the rest of the ``options`` that are left
    [[nodiscard]] string_view parse_searchmoves(
        string_view options, const Position& currentPosition,
        std::output_iterator<moves::Move> auto outputIt)
    {
        using namespace std::literals::string_view_literals; // NOLINT

        // we could instead find the first token that doesn't successfully parse as
        // a UCI move, but from_uci() throws on failure, so instead we detect the
        // other delimiter tokens
        static constexpr std::array argumentTokens {
            "ponder"sv, "wtime"sv, "btime"sv, "winc"sv, "binc"sv, "infinite"sv,
            "movestogo"sv, "depth"sv, "nodes"sv, "mate"sv, "movetime"sv
        };

        while (not options.empty()) {
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
    string_view options, const Position& currentPosition)
{
    // options doesn't include the "go" token itself

    options = trim(options);

    GoCommandOptions ret;

    while (not options.empty()) {
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
            const auto [wtime, rest2] = parse_int_value(rest);

            ret.whiteTimeLeft = Milliseconds { wtime };
            options           = rest2;

            continue;
        }

        if (firstWord == "btime") {
            const auto [btime, rest2] = parse_int_value(rest);

            ret.blackTimeLeft = Milliseconds { btime };
            options           = rest2;

            continue;
        }

        if (firstWord == "winc") {
            const auto [winc, rest2] = parse_int_value(rest);

            ret.whiteInc = Milliseconds { winc };
            options      = rest2;

            continue;
        }

        if (firstWord == "binc") {
            const auto [binc, rest2] = parse_int_value(rest);

            ret.blackInc = Milliseconds { binc };
            options      = rest2;

            continue;
        }

        if (firstWord == "movestogo") {
            const auto [mtg, rest2] = parse_int_value(rest);

            ret.movesToGo = mtg;
            options       = rest2;

            continue;
        }

        if (firstWord == "depth") {
            const auto [depth, rest2] = parse_int_value(rest);

            ret.depth = depth;
            options   = rest2;

            continue;
        }

        if (firstWord == "nodes") {
            const auto [nodes, rest2] = parse_int_value(rest);

            ret.nodes = nodes;
            options   = rest2;

            continue;
        }

        if (firstWord == "mate") {
            const auto [mate, rest2] = parse_int_value(rest);

            ret.mateIn = mate;
            options    = rest2;

            continue;
        }

        if (firstWord == "movetime") {
            const auto [time, rest2] = parse_int_value(rest);

            ret.searchTime = Milliseconds { time };
            options        = rest2;

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
