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
#include <chrono>
#include <expected>
#include <iterator>
#include <libchess/moves/Move.hpp>
#include <libchess/notation/FEN.hpp>
#include <libchess/notation/UCI.hpp>
#include <libchess/uci/CommandParsing.hpp>
#include <libchess/util/Strings.hpp>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

namespace chess::uci {

using std::chrono::milliseconds;
using std::string_view;

using util::split_at_first_space;
using util::trim;

// Note that in all UCI parsing, we need to use trim() defensively a lot,
// because UCI allows arbitrary whitespace between tokens. Also note that
// split_at_first_space() will return a pair whose first element is empty
// if its input string began with a space!

auto parse_position_options(string_view options)
    -> std::expected<Position, std::string>
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

        const auto fenPos = notation::from_fen(fenString);

        if (not fenPos.has_value())
            return std::unexpected(fenPos.error());

        position = fenPos.value();

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

        const auto parsed = notation::from_uci(position, firstMove);

        if (not parsed.has_value())
            return std::unexpected(parsed.error());

        const auto move = parsed.value();

        if (move.is_null())
            return std::unexpected("Found null move in move list");

        position.make_move(move);

        moves = trim(rest2);
    }

    return position;
}

namespace {
    // splits the input string into sections before and after the given whitespace-delimited token
    // for input string "foo bar baz" and token "bar", this would return ["foo", "bar baz"]
    [[nodiscard, gnu::const]] constexpr auto split_at_token(
        const string_view input, const string_view token)
        -> std::pair<string_view, string_view>
    {
        const auto tokenStart = input.find(token);

        if (tokenStart == string_view::npos)
            return std::make_pair(input, "");

        return std::make_pair(
            input.substr(0uz, tokenStart),
            input.substr(tokenStart));
    }

    [[nodiscard, gnu::const]] auto skip_first_word(const string_view input)
        -> string_view
    {
        [[maybe_unused]] const auto [firstWord, rest] = split_at_first_space(input);

        return trim(rest);
    }
} // namespace

auto parse_register_options(string_view options) -> RegisterOptions
{
    // options doesn't include the "register" token itself

    options = trim(options);

    RegisterNowOptions opts;

    while (not options.empty()) {
        auto [firstWord, rest] = split_at_first_space(options);

        firstWord = trim(firstWord);

        if (firstWord == "later")
            return std::nullopt;

        if (firstWord == "name") {
            const auto [name, rest2] = split_at_token(rest, "code");

            opts.name = std::string { trim(name) };

            options = trim(rest2);

            continue;
        }

        if (firstWord == "code") {
            const auto [code, rest2] = split_at_token(rest, "name");

            opts.code = std::string { trim(code) };

            options = trim(rest2);

            continue;
        }

        // unrecognized token, don't want to loop forever
        options = skip_first_word(options);
    }

    return opts;
}

namespace {

    // consumes one argument from ``options``,
    // and returns pair of the option value & the rest of the ``options`` that are left
    [[nodiscard]] auto parse_int_value(const string_view options) -> std::pair<size_t, string_view>
    {
        const auto [valueStr, rest] = split_at_first_space(options);

        return {
            util::int_from_string<size_t>(trim(valueStr)),
            trim(rest)
        };
    }

    // consumes all the moves following the "searchmoves" token,
    // and returns the rest of the ``options`` that are left
    [[nodiscard]] auto parse_searchmoves(
        string_view options, const Position& currentPosition,
        std::output_iterator<moves::Move> auto outputIt)
        -> string_view
    {
        using namespace std::literals::string_view_literals; // NOLINT

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

            if (const auto move = notation::from_uci(currentPosition, firstMove))
                *outputIt = move.value();
        }

        return options; // NOLINT
    }

} // namespace

auto parse_go_options(
    string_view options, const Position& currentPosition)
    -> GoCommandOptions
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

            ret.whiteTimeLeft = milliseconds { wtime };
            options           = rest2;

            continue;
        }

        if (firstWord == "btime") {
            const auto [btime, rest2] = parse_int_value(rest);

            ret.blackTimeLeft = milliseconds { btime };
            options           = rest2;

            continue;
        }

        if (firstWord == "winc") {
            const auto [winc, rest2] = parse_int_value(rest);

            ret.whiteInc = milliseconds { winc };
            options      = rest2;

            continue;
        }

        if (firstWord == "binc") {
            const auto [binc, rest2] = parse_int_value(rest);

            ret.blackInc = milliseconds { binc };
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

            ret.searchTime = milliseconds { time };
            options        = rest2;

            continue;
        }

        if (firstWord == "searchmoves") {
            options = parse_searchmoves(
                rest, currentPosition, std::back_inserter(ret.moves));

            continue;
        }

        // unrecognized token, don't want to loop forever
        options = skip_first_word(options);
    }

    return ret;
}

} // namespace chess::uci
