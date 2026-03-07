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

#include <concepts>
#include <format>
#include <libbenbot/engine/Engine.hpp>
#include <libchess/moves/Move.hpp>
#include <libchess/moves/Perft.hpp>
#include <libchess/uci/Printing.hpp>
#include <libchess/util/Strings.hpp>
#include <nlohmann/json.hpp>
#include <print>
#include <string>

namespace ben_bot {

namespace util = chess::util;

using uci::printing::info_string;

namespace {
    using chess::moves::PerftResult;

    template <typename Func>
    concept MovePrinter = std::regular_invocable<Func, Move>
                      and std::same_as<std::invoke_result_t<Func, Move>, std::string>;

    template <MovePrinter Printer>
    void print_root_nodes(
        const PerftResult& result,
        const Printer&     printMove)
    {
        for (const auto& [move, numChildren] : result.rootNodes) {
            info_string(std::format(
                "Move {}: {} child nodes",
                printMove(move), numChildren));
        }
    }

    void print_results(
        const PerftResult& result)
    {
        info_string(std::format("Nodes: {}", result.nodes));
        info_string(std::format("Captures: {}", result.captures));
        info_string(std::format("En passant captures: {}", result.enPassantCaptures));
        info_string(std::format("Castles: {}", result.castles));
        info_string(std::format("Promotions: {}", result.promotions));
        info_string(std::format("Checks: {}", result.checks));
        info_string(std::format("Checkmates: {}", result.checkmates));
        info_string(std::format("Stalemates: {}", result.stalemates));
    }

    template <MovePrinter Printer>
    void pretty_print(
        const PerftResult& result,
        const Printer&     printMove)
    {
        std::println("");
        print_root_nodes(result, printMove);

        std::println("");
        print_results(result);
    }

    void print_json(
        const PerftResult& result)
    {
        nlohmann::json json;

        json["captures"]    = result.captures;
        json["castles"]     = result.castles;
        json["checkmates"]  = result.checkmates;
        json["checks"]      = result.checks;
        json["en_passants"] = result.enPassantCaptures;
        json["promotions"]  = result.promotions;
        json["stalemates"]  = result.stalemates;
        json["totalNodes"]  = result.nodes;

        std::println("{}", json.dump());
    }
} // namespace

void Engine::run_perft(
    const string_view arguments) const
{
    using util::strings::trim;

    const auto [depthStr, jsonFlag] = util::strings::split_at_first_space(arguments);

    const auto depth = util::strings::int_from_string(trim(depthStr), 4uz);

    info_string(std::format("Running perft depth {}...", depth));

    const auto result = chess::moves::perft(
        depth, searcher.context.options.position);

    if (trim(jsonFlag) == "json") {
        print_json(result);
    } else {
        pretty_print(result,
            [this](const Move move) { return pretty_print_move(move); });
    }
}

} // namespace ben_bot
