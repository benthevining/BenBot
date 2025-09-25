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

#include <functional>
#include <iostream>
#include <libbenbot/search/Callbacks.hpp>
#include <libbenbot/search/Context.hpp>
#include <libchess/uci/Printing.hpp>

namespace ben_bot::search {

namespace {

    namespace uci_printing = chess::uci::printing;

    template <bool PrintBestMove>
    void print_uci_info(
        const Result&  res,
        const bool     debugMode,
        const Context& context)
    {
        uci_printing::search_info(res.to_libchess(debugMode));

        if constexpr (PrintBestMove) {
            const auto& currPos    = context.options.position;
            const auto& transTable = context.transTable;

            uci_printing::best_move(
                res.bestMove,
                transTable.get_best_response(currPos, res.bestMove));

            // Because these callbacks are executed on the searcher background thread,
            // without this flush here, the output may not actually be written when we
            // expect, leading to timeouts or GUIs thinking we've hung/disconnected.
            // Because the best move is always printed last after all info output, we
            // can do the flush only in this branch.
            std::cout.flush();
        }
    }

} // namespace

Callbacks Callbacks::make_uci_printer(
    const Context&        context,
    std::function<bool()> isDebugMode)
{
    return {
        .onSearchComplete = [&context, isDebugMode](const Result& res) { print_uci_info<true>(res, isDebugMode(), context); },
        .onIteration = [&context, isDebugMode](const Result& res) { print_uci_info<false>(res, isDebugMode(), context); }
    };
}

} // namespace ben_bot::search
