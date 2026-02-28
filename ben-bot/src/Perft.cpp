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

#include <ben-bot/Engine.hpp>
#include <format>
#include <functional>
#include <libchess/moves/Move.hpp>
#include <libchess/moves/Perft.hpp>
#include <libchess/uci/Printing.hpp>
#include <libchess/util/Strings.hpp>
#include <print>
#include <string>

namespace ben_bot {

namespace util = chess::util;

using uci::printing::info_string;

namespace {
    using chess::moves::PerftResult;

    void print_root_nodes(
        const PerftResult&                      result,
        const std::function<std::string(Move)>& printMove)
    {
        for (const auto& [move, numChildren] : result.rootNodes) {
            info_string(std::format(
                "Move {}: {} child nodes",
                printMove(move), numChildren));
        }
    }

    void print_results(const PerftResult& result)
    {
        info_string(std::format("Nodes: {}", result.nodes));
        info_string(std::format("Captures: {}", result.captures));
        info_string(std::format("En passant captures: {}", result.enPassantCaptures));
        info_string(std::format("Castles: {}", result.castles));
        info_string(std::format("Promotions: {}", result.promotions));
        info_string(std::format("Checks: {}", result.checks));
        info_string(std::format("Checkmates: {}", result.checkmates));

        // NB. the python wrapper script relies on this being printed last
        info_string(std::format("Stalemates: {}", result.stalemates));
    }
} // namespace

void Engine::run_perft(const string_view arguments) const
{
    const auto depth = util::strings::int_from_string(
        util::strings::trim(arguments),
        4uz);

    info_string(std::format("Running perft depth {}...", depth));

    const auto result = chess::moves::perft(
        depth, searcher.context.options.position);

    std::println("");

    print_root_nodes(
        result,
        [this](const Move move) { return pretty_print_move(move); });

    std::println("");
    print_results(result);
}

} // namespace ben_bot
