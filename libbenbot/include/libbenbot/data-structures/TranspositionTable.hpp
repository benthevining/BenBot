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
    This file defines the transposition table data structure.
    @ingroup benbot_data_structures
 */

#pragma once

#include <cstddef> // IWYU pragma: keep - for size_t
#include <cstdint> // IWYU pragma: keep - for std::uint_least8_t
#include <libchess/game/Position.hpp>
#include <libchess/moves/Move.hpp>
#include <optional>
#include <span>
#include <utility>

namespace ben_bot {

namespace search {
    struct Bounds;
} // namespace search

using chess::game::Position;
using chess::moves::Move;
using std::size_t;

/** This enumeration defines types of evaluation values that
    different nodes in the search tree may be assigned.

    @ingroup benbot_data_structures
 */
enum class EvalType : std::uint_least8_t {
    Exact, ///< Indicates that the evaluation value is an exact evaluation. This also indicates that this is a PV node.
    Alpha, ///< Indicates that the evaluation value is a maximum evaluation; for example, if ``eval`` is 16, this means that the evaluation of this node was at most 16.
    Beta   ///< Indicates that the evaluation is a minimum evaluation; for example, if ``eval`` is 16, this means that the evaluation of this node was at least 16.
};

/** This POD struct contains the data stored in the transposition table
    for a position. The data is returned from the table by copy.

    @ingroup benbot_data_structures
 */
struct TTData final {
    /** The depth that the position was searched to. */
    size_t searchedDepth { 0uz }; // empty slots are marked with a depth of 0

    /** The evaluation of this position.
        See ``evalType`` to determine the exact meaning of this value.
     */
    int eval { 0 };

    /** Gives the exact meaning of the ``eval`` value. */
    EvalType evalType { EvalType::Alpha };

    /** If a conclusive best move was found in this position, it
        is stored here. Sometimes this may be ``nullopt`` if everything
        failed low (i.e. ``score <= alpha``).
     */
    std::optional<Move> bestMove;

    constexpr bool operator==(const TTData& other) const noexcept = default;
};

/** The transposition table data structure.

    @ingroup benbot_data_structures
 */
class TranspositionTable final {
public:
    TranspositionTable() { resize(5uz); }
    ~TranspositionTable() { deallocate(); }

    TranspositionTable(const TranspositionTable&)            = delete;
    TranspositionTable& operator=(const TranspositionTable&) = delete;

    TranspositionTable(TranspositionTable&& other) noexcept;
    TranspositionTable& operator=(TranspositionTable&& other) noexcept;

    /** Retrieves the stored record for the given position,
        or nullptr if the given position isn't in the table.
     */
    [[nodiscard]] std::optional<TTData> find(const Position& pos) const;

    /** Represents a probed evaluation from the table.
        This is a pair of the evaluation value and the value type.
     */
    using ProbedEval = std::pair<int, EvalType>;

    /** Similar to ``find()``, this function instead probes for an
        evaluation value of the given position, searched to at least
        the given depth and honoring the given search bounds.

        Returns pair of the evaluation value and the value type.
     */
    [[nodiscard]] std::optional<ProbedEval> probe_eval(
        const Position& pos, size_t depth, const search::Bounds& bounds) const;

    /** Returns the opponent's best response to the given move, if one
        is recorded.
     */
    [[nodiscard]] std::optional<Move> get_best_response(
        const Position& pos, const Move& move) const;

    /** Stores a record for a given position. */
    void store(const Position& pos, const TTData& record);

    /** Clears the contents of the table. Note that no memory is freed. */
    void clear();

    /** Resizes the table to a given size in megabytes.

        @throws std::bad_alloc An exception is thrown if ``sizeMB`` is 0 or
        if the allocation fails.
     */
    [[clang::reinitializes]] void resize(size_t sizeMB);

    /** Returns an estimate of the percentage of entries (permille) that
        have been written to during this search.
     */
    [[nodiscard]] size_t hashfull() const;

    /** This must be called at the beginning of a new search to keep
        track of entry aging.
     */
    void new_search() noexcept;

    /** Hints to the CPU to prefetch the memory bucket containing data
        for the given position. This function returns immediately.
     */
    void prefetch(const Position& pos) const noexcept;

private:
    struct Entry;
    struct Cluster;

    void deallocate();

    [[nodiscard]] Cluster& index_table(size_t clusterIdx) const noexcept;

    // this is the hash function
    [[nodiscard]] std::span<Entry> find_cluster(Position::Hash key) const noexcept;

    Cluster* table { nullptr };

    size_t clusterCount { 0uz };

    std::uint8_t generation { 0 };
};

} // namespace ben_bot
