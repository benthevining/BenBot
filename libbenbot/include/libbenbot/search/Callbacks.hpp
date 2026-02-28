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

/** @file
    This file defines the search callbacks struct.
    @ingroup search
 */

#pragma once

#include <functional>
#include <string>

namespace chess::moves {
struct Move;
}

namespace ben_bot::search {

struct Options;
struct Result;

/** This struct encapsulates a set of functions that will be called to
    process search progress and results. Search results are always
    retrieved through these callbacks.

    @ingroup search
    @see Context
 */
struct Callbacks final {
    /** Function type that accepts a single Result argument. */
    using Callback = std::function<void(const Result&)>;

    /** Function object that will be invoked when a new search is started. */
    std::function<void(const Options&)> onSearchStart;

    /** Function object that will be invoked with results from a completed search. */
    Callback onSearchComplete;

    /** Function object that will be invoked with results from each iteration of
        the iterative deepening loop.
     */
    Callback onIteration;

    /** Can be safely called without checking if ``onSearchStart`` is null. */
    void search_start(const Options& options) const
    {
        if (onSearchStart != nullptr)
            onSearchStart(options);
    }

    /** Can be safely called without checking if ``onSearchComplete`` is null. */
    void search_complete(const Result& result) const
    {
        if (onSearchComplete != nullptr) {
            [[likely]];
            onSearchComplete(result);
        }
    }

    /** Can be safely called without checking if ``onIteration`` is null. */
    void iteration_complete(const Result& result) const
    {
        if (onIteration != nullptr) {
            [[likely]];
            onIteration(result);
        }
    }

    /** Creates a set of callbacks that print UCI-formatted information and bestmove
        output to standard output.

        @param isDebugMode Function object that should return true if debug information
        should be included in the information output.
     */
    [[nodiscard]] static auto make_uci_printer(
        std::function<bool()>&& isDebugMode)
        -> Callbacks;

    /** Creates a set of callbacks that print search information in a human-readable
        table-aligned format.

        @param printMove Function object that will be used to format Move objects to display
        the PV.

        @note The output produced by these callbacks does not conform to the UCI protocol!
     */
    [[nodiscard]] static auto make_pretty_printer(
        std::function<std::string(chess::moves::Move)>&& printMove)
        -> Callbacks;
};

} // namespace ben_bot::search
