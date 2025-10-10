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

/** @defgroup benbot_data_structures
    Data structures used by BenBot's search algorithm.

    @ingroup libbenbot
 */

/** @file
    This file defines the transposition table data structure.
    @ingroup benbot_data_structures
 */

#pragma once

#include <cstddef> // IWYU pragma: keep - for size_t
#include <cstdint> // IWYU pragma: keep - for std::uint_least8_t
#include <libbenbot/eval/Score.hpp>
#include <libchess/moves/Move.hpp>
#include <optional>
#include <span>
#include <utility>

namespace chess::game {
struct Position;
} // namespace chess::game

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
struct [[nodiscard]] TTData final {
    /** The depth that the position was searched to. */
    size_t searchedDepth { 0uz };

    /** The evaluation of this position.
        See ``evalType`` to determine the exact meaning of this value.
     */
    eval::Value eval { 0 };

    /** Gives the exact meaning of the ``eval`` value. */
    EvalType evalType { EvalType::Alpha };

    /** If a conclusive best move was found in this position, it
        is stored here. Sometimes this may be ``nullopt`` if everything
        failed low (i.e. ``score <= alpha``).
     */
    std::optional<Move> bestMove;

    constexpr auto operator==(const TTData& other) const noexcept -> bool = default;
};

/** The transposition table data structure.

    @ingroup benbot_data_structures
 */
class TranspositionTable final {
public:
    explicit TranspositionTable(const size_t initialSize = 16uz)
    {
        resize(initialSize);
    }

    ~TranspositionTable() { deallocate(); }

    TranspositionTable(const TranspositionTable&)            = delete;
    TranspositionTable& operator=(const TranspositionTable&) = delete;

    TranspositionTable(TranspositionTable&& other) noexcept;
    TranspositionTable& operator=(TranspositionTable&& other) noexcept;

    /** Retrieves the stored record for the given position,
        or nullptr if the given position isn't in the table.
     */
    [[nodiscard]] auto find(const Position& pos) const -> std::optional<TTData>;

    /** Represents a probed evaluation from the table.
        This is a pair of the evaluation value and the value type.
     */
    using ProbedEval = std::pair<eval::Value, EvalType>;

    /** Similar to ``find()``, this function instead probes for an
        evaluation value of the given position, searched to at least
        the given depth and honoring the given search bounds.

        Returns pair of the evaluation value and the value type.
     */
    [[nodiscard]] auto probe_eval(
        const Position& pos, size_t depth, const search::Bounds& bounds) const
        -> std::optional<ProbedEval>;

    /** Returns the opponent's best response to the given move, if one
        is recorded.
     */
    [[nodiscard]] auto get_best_response(
        const Position& pos, Move move) const
        -> std::optional<Move>;

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
    [[nodiscard]] auto hashfull() const -> size_t;

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

    [[nodiscard]] auto index_table(size_t clusterIdx) const noexcept -> Cluster&;

    // this is the hash function
    [[nodiscard]] auto find_cluster(std::uint64_t key) const noexcept -> std::span<Entry>;

    Cluster* table { nullptr };

    size_t clusterCount { 0uz };

    std::uint8_t generation { 0 };
};

} // namespace ben_bot
