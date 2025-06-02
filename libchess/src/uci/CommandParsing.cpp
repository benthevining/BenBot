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
#include <libchess/notation/FEN.hpp>
#include <libchess/notation/UCI.hpp>
#include <libchess/uci/CommandParsing.hpp>
#include <libchess/util/Strings.hpp>
#include <string_view>

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

GoCommandOptions parse_go_options(
    std::string_view options, const Position& currentPosition)
{
    // args doesn't include the "go" token itself

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
            auto [wtime, rest2] = split_at_first_space(rest);

            wtime = trim(wtime);

            options = rest2;

            std::from_chars(
                wtime.data(), wtime.data() + wtime.length(), ret.whiteMsLeft);

            continue;
        }

        if (firstWord == "btime") {
            auto [btime, rest2] = split_at_first_space(rest);

            btime = trim(btime);

            options = rest2;

            std::from_chars(
                btime.data(), btime.data() + btime.length(), ret.blackMsLeft);

            continue;
        }

        if (firstWord == "winc") {
            auto [winc, rest2] = split_at_first_space(rest);

            winc = trim(winc);

            options = rest2;

            std::from_chars(
                winc.data(), winc.data() + winc.length(), ret.whiteIncMs);

            continue;
        }

        if (firstWord == "binc") {
            auto [binc, rest2] = split_at_first_space(rest);

            binc = trim(binc);

            options = rest2;

            std::from_chars(
                binc.data(), binc.data() + binc.length(), ret.blackIncMs);

            continue;
        }

        if (firstWord == "movestogo") {
            auto [movesToGo, rest2] = split_at_first_space(rest);

            movesToGo = trim(movesToGo);

            options = rest2;

            std::from_chars(
                movesToGo.data(), movesToGo.data() + movesToGo.length(), ret.movesToGo);

            continue;
        }

        if (firstWord == "depth") {
            auto [depth, rest2] = split_at_first_space(rest);

            depth = trim(depth);

            options = rest2;

            std::from_chars(
                depth.data(), depth.data() + depth.length(), ret.depth);

            continue;
        }

        if (firstWord == "nodes") {
            auto [nodes, rest2] = split_at_first_space(rest);

            nodes = trim(nodes);

            options = rest2;

            std::from_chars(
                nodes.data(), nodes.data() + nodes.length(), ret.nodes);

            continue;
        }

        if (firstWord == "mate") {
            auto [mate, rest2] = split_at_first_space(rest);

            mate = trim(mate);

            options = rest2;

            std::from_chars(
                mate.data(), mate.data() + mate.length(), ret.mateIn);

            continue;
        }

        if (firstWord == "movetime") {
            auto [movetime, rest2] = split_at_first_space(rest);

            movetime = trim(movetime);

            options = rest2;

            std::from_chars(
                movetime.data(), movetime.data() + movetime.length(), ret.searchTime);

            continue;
        }

        if (firstWord != "searchmoves")
            continue;

        // searchmoves

        using std::operator""sv;

        static constexpr std::array argumentTokens {
            "searchmoves"sv, "ponder"sv, "wtime"sv, "btime"sv, "winc"sv, "binc"sv,
            "movestogo"sv, "depth"sv, "nodes"sv, "mate"sv, "movetime"sv, "infinite"sv
        };

        while (! options.empty()) {
            auto [firstMove, rest2] = split_at_first_space(rest);

            firstMove = trim(firstMove);

            if (std::ranges::contains(argumentTokens, firstMove))
                break;

            options = rest2;

            ret.moves.emplace_back(
                notation::from_uci(currentPosition, firstMove));
        }
    }

    return ret;
}

} // namespace chess::uci
