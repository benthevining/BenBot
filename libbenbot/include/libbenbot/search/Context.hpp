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
    This file defines the search context class.
    @ingroup search
 */

#pragma once

#include <atomic>
#include <libbenbot/data-structures/KillerMoves.hpp>
#include <libbenbot/data-structures/TranspositionTable.hpp>
#include <libbenbot/eval/PieceSquareTables.hpp>
#include <libbenbot/search/Callbacks.hpp>
#include <libbenbot/search/Options.hpp>
#include <libchess/game/Position.hpp>
#include <libutil/Memory.hpp>
#include <optional>
#include <string_view>
#include <utility>

namespace ben_bot::search {

using chess::game::Position;

/** This struct encapsulates everything needed to perform a search.
    You can keep one of these alive between searches by simply updating
    the options and then calling ``search()`` again.

    @ingroup search
 */
struct Context final {
    /** Creates a search context with empty callbacks. */
    Context() = default;

    /** Creates a search context with a specified set of result callbacks. */
    explicit Context(Callbacks&& callbacksToUse)
        : callbacks { std::move(callbacksToUse) }
    {
    }

    Context(const Context&)            = delete;
    Context& operator=(const Context&) = delete;

    Context(Context&&)            = delete;
    Context& operator=(Context&&) = delete;

    ~Context() = default;

    /** Performs a search.
        Results will be propagated via the ``callbacks`` that have been
        assigned.

        The search may execute for a potentially unbounded amount of time.
        The search can be interrupted by calling the ``abort()`` method while
        ``search()`` is executing.
     */
    void search();

    /** This function may be called while ``search()`` is executing to interrupt
        the search. If a search is in progress, calling this method will cause the
        search routine to return at the next available point. This method returns
        immediately; to wait for the search to complete, call ``wait()`` after
        calling this method.
     */
    void abort() noexcept { exitFlag.store(true, std::memory_order::release); }

    /** Clears the transposition table.
        If a search is in progress, this method aborts it and blocks until it returns.
     */
    void clear_transposition_table()
    {
        abort();
        wait();
        transTable.clear();
    }

    /** Resizes the transposition table.
        If a search is in progress, this method aborts it and blocks until it returns.
     */
    void resize_transposition_table(const size_t sizeMB)
    {
        abort();
        wait();
        transTable.resize(sizeMB);
    }

    /** Probes the transposition table for the given position. */
    [[nodiscard]] auto probe_transposition_table(const Position& pos) const
        -> std::optional<TTData>
    {
        return transTable.find(pos);
    }

    /** Returns true if a search is currently in progress. */
    [[nodiscard]] auto in_progress() const noexcept -> bool { return activeFlag.load(std::memory_order::acquire); }

    /** Blocks the calling thread until the search in progress is complete.
        Returns immediately if no search was in progress when this function was called.
     */
    void wait() const;

    /** Sets whether the next search will be in ponder mode.
        When in ponder mode, the search will not exit until ``abort()`` or
        ``ponder_hit()`` are called.
     */
    void set_pondering(const bool isPonderMode) noexcept { pondering.store(isPonderMode, std::memory_order::release); }

    /** When in a ponder mode search, this exits the search. A ponder mode search
        will not exit until this method or ``abort()`` are called.
     */
    void ponder_hit() noexcept { pondering.store(false, std::memory_order::release); }

    /** Sets the position to be searched by the next search.
        If a search is in progress, this function aborts it and blocks until it completes.
     */
    void set_position(const Position& pos)
    {
        abort();
        wait();
        position = pos;

        // clear this so that all legal moves will be searched by default
        options.movesToSearch.clear();
    }

    /** Sets the options to be used by the next search.
        If a search is in progress, this function aborts it and blocks until it completes.
     */
    void set_options(const Options& opts)
    {
        abort();
        wait();
        options = opts;
    }

    /** Sets the options to be used by the next search.
        If a search is in progress, this function aborts it and blocks until it completes.
     */
    void set_options(const chess::uci::GoCommandOptions& opts)
    {
        abort();
        wait();
        options = Options::from_libchess(opts, position.is_white_to_move());
    }

    /** Sets the result callbacks that will be used for the next search.
        If a search is in progress, this function aborts it and blocks until it completes.
     */
    void set_callbacks(Callbacks&& callbacksToUse)
    {
        abort();
        wait();
        callbacks = std::move(callbacksToUse);
    }

    /** Returns the current position. */
    [[nodiscard]] auto get_position() const noexcept -> const Position& { return position; }

    /** Returns the current search parameters. */
    [[nodiscard]] auto get_options() const noexcept -> const Options& { return options; }

    /** Returns the current piece square tables. */
    [[nodiscard]] auto get_piece_square_tables() const noexcept -> const eval::PieceSquareTables& { return pieceSquareTables; }

    /** Loads piece square table data from a given JSON string.
        If a search is in progress, this function aborts it and blocks until it completes.
     */
    void load_piece_square_tables(
        const std::string_view data)
    {
        abort();
        wait();
        pieceSquareTables = eval::PieceSquareTables::from_string(data);
    }

private:
    Position position;

    Options options;

    Callbacks callbacks;

    TranspositionTable transTable;

    KillerMoves killerMoves;

    eval::PieceSquareTables pieceSquareTables { eval::PieceSquareTables::get_default() };

    // these flags are all on their own cache lines to prevent
    // false sharing between the searcher & main (input) threads
    alignas(util::memory::CacheLineSize) std::atomic_bool exitFlag { false };
    alignas(util::memory::CacheLineSize) std::atomic_bool activeFlag { false };
    alignas(util::memory::CacheLineSize) std::atomic_bool pondering { false };
};

} // namespace ben_bot::search
