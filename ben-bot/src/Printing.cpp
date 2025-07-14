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

#include "Data.hpp"
#include "Engine.hpp"
#include "TextTable.hpp"
#include <cassert>
#include <cmath>
#include <cstddef> // IWYU pragma: keep - for size_t
#include <format>
#include <iostream>
#include <libbenbot/data-structures/TranspositionTable.hpp>
#include <libbenbot/eval/Evaluation.hpp>
#include <libbenbot/eval/Score.hpp>
#include <libbenbot/search/Search.hpp>
#include <libchess/game/Position.hpp>
#include <libchess/notation/FEN.hpp>
#include <libchess/notation/UCI.hpp>
#include <optional>
#include <print>
#include <string>
#include <variant>

namespace ben_bot {

using Result = search::Callbacks::Result;
using eval::Score;
using std::size_t;

using chess::notation::to_uci;
using std::println;

namespace {

    [[nodiscard]] std::string get_score_string(const Score score)
    {
        if (! score.is_mate()) {
            // NB. we pass score.value directly here instead of going through
            // Score's formatter because that extra indirection appears to cost
            // enough time to observably cost some Elo
            return std::format("cp {}", score.value);
        }

        auto plyToMate = score.ply_to_mate();

        if (plyToMate > 0uz)
            ++plyToMate;

        // plies -> moves
        const auto mateIn = plyToMate / 2uz;

        auto mateVal = static_cast<int>(mateIn);

        if (score < 0)
            mateVal *= -1;

        return std::format("mate {}", mateVal);
    }

    [[nodiscard]] size_t get_nodes_per_second(const Result& res)
    {
        const auto seconds = static_cast<double>(res.duration.count()) * 0.001;

        assert(seconds > 0.);

        const auto nps = static_cast<double>(res.nodesSearched) / seconds;

        return static_cast<size_t>(std::round(nps));
    }

    // extracts the PV from the transposition table
    [[nodiscard]] std::string get_pv_string(
        Position position, Move bestMove, const TranspositionTable& transTable)
    {
        auto result = std::format("pv {}", to_uci(bestMove));

        while (true) {
            const auto nextMove = transTable.get_best_response(position, bestMove);

            if (! nextMove.has_value())
                break;

            result.append(1uz, ' ');
            result.append(to_uci(*nextMove));

            position.make_move(bestMove);

            bestMove = *nextMove;
        }

        return result;
    }

    [[nodiscard]] std::string get_extra_stats_string(
        const Result& res, const bool isDebugMode)
    {
        if ((! isDebugMode) || res.nodesSearched == 0uz)
            return {};

        auto get_pcnt = [totalNodes = static_cast<double>(res.nodesSearched)](const size_t value) {
            return (static_cast<double>(value) / totalNodes) * 100.;
        };

        return std::format(
            " string TT hits {} ({}%) Beta cutoffs {} ({}%) MDP cutoffs {} ({}%)",
            res.transpositionTableHits, get_pcnt(res.transpositionTableHits),
            res.betaCutoffs, get_pcnt(res.betaCutoffs),
            res.mdpCutoffs, get_pcnt(res.mdpCutoffs));
    }

    [[nodiscard]] std::string get_ponder_move_string(const std::optional<Move> ponderMove)
    {
        if (! ponderMove.has_value())
            return {};

        return std::format(
            " ponder {}",
            to_uci(*ponderMove));
    }

} // namespace

template <bool PrintBestMove>
void Engine::print_uci_info(const Result& res) const
{
    const auto& currPos = searcher.context.options.position;

    const auto& transTable = searcher.context.transTable;

    println(
        "info depth {} score {} time {} nodes {} nps {} {}{}",
        res.depth, get_score_string(res.score), res.duration.count(),
        res.nodesSearched, get_nodes_per_second(res),
        get_pv_string(currPos, res.bestMove, transTable),
        get_extra_stats_string(res, debugMode.load()));

    if constexpr (PrintBestMove) {
        println("bestmove {}{}",
            to_uci(res.bestMove),
            get_ponder_move_string(
                transTable.get_best_response(currPos, res.bestMove)));

        // Because these callbacks are executed on the searcher background thread,
        // without this flush here, the output may not actually be written when we
        // expect, leading to timeouts or GUIs thinking we've hung/disconnected.
        // Because the best move is always printed last after all info output, we
        // can do the flush only in this branch.
        std::cout.flush();
    }
}

template void Engine::print_uci_info<true>(const Result&) const;
template void Engine::print_uci_info<false>(const Result&) const;

void Engine::print_book_hit() const
{
    if (debugMode.load())
        println("info string Opening book hit!");
}

void Engine::print_logo_and_version() const
{
    println("{}", get_ascii_logo());

    println(
        "{}, version {}, by {}",
        get_name(), get_version_string(), get_author());
}

void Engine::print_help() const
{
    print_logo_and_version();

    println();

    println(
        "All standard UCI commands are supported, as well as the following non-standard commands:");

    println();

    TextTable table;

    table.append_column("Command")
        .append_column("Notes");

    for (const auto& command : customCommands) {
        table.new_row()
            .append_column(std::format("{} {}", command.name, command.argsHelp))
            .append_column(command.description);
    }

    println("{}", table.to_string());
}

void Engine::print_options() const
{
    println();
    println("The following UCI options are supported:");
    println();

    TextTable table;

    table.append_column("Option")
        .append_column("Type")
        .append_column("Notes")
        .append_column("Default")
        .append_column("Current");

    for (const auto* option : options) {
        table.new_row()
            .append_column(option->get_name())
            .append_column(option->get_type())
            .append_column(option->get_help());

        if (option->has_value()) {
            std::visit(
                [&table](auto defaultValue) {
                    table.append_column(std::format("{}", defaultValue));
                },
                option->get_default_value_variant());

            std::visit(
                [&table](auto value) {
                    table.append_column(std::format("{}", value));
                },
                option->get_value_variant());
        }
    }

    println("{}", table.to_string());
}

void Engine::print_current_position() const
{
    const auto& pos = searcher.context.options.position;

    println("{}", print_utf8(pos));
    println("{}", chess::notation::to_fen(pos));
    println();

    // print eval
    if (const auto* record = searcher.context.transTable.find(pos)) {
        const auto score = Score::from_tt({ record->eval, record->evalType }, 0uz);

        println("TT hit: {}", get_score_string(score));
    }

    println("Static eval: {}", eval::evaluate(pos));
}

void Engine::print_compiler_info()
{
    println(
        "Compiled by {} version {} for {}",
        get_compiler_name(), get_compiler_version(), get_system_name());

    println(
        "Build configuration: {}", get_build_config());
}

} // namespace ben_bot
