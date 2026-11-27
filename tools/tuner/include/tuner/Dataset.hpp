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
    This file defines the tuner's dataset: labelled positions, stored as the sparse
    coefficients of their eval traces.
    @ingroup tuner
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <expected>
#include <filesystem>
#include <iosfwd>
#include <libchess/game/Position.hpp>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

/** The eval tuner.
    @ingroup tuner
 */
namespace ben_bot::tuner {

using chess::game::Position;
using std::size_t;

/** A position, with the result of the game it came from.
    @ingroup tuner
 */
struct LabelledPosition final {
    /** The position. */
    Position position;

    /** The game result from White's point of view: 1 for a White win,
        0.5 for a draw, and 0 for a Black win.
     */
    double result { 0.5 };
};

/** Parses one line of a labelled dataset.

    The line starts with a FEN (the last two fields are optional, as in EPD), followed by
    the result in one of these forms:

    - ``[1.0]``, ``[0.5]`` or ``[0.0]`` (any number in ``[0, 1]`` between brackets)
    - ``"1-0"``, ``"1/2-1/2"`` or ``"0-1"`` (quoted, as in zurichess's ``c9 "1-0";``)
    - ``1-0``, ``1/2-1/2`` or ``0-1`` as the line's last word

    Anything between the FEN & the result (such as EPD opcodes) is ignored.
    The result is always from White's point of view.

    @ingroup tuner
 */
[[nodiscard]] auto parse_labelled_line(std::string_view line)
    -> std::expected<LabelledPosition, std::string>;

/** Returns the search score written after a line's bracketed result, as in ``<FEN> [1.0] 34``,
    or ``nullopt`` if there isn't one. The score is in centipawns from White's point of view.
    ``parse_labelled_line()`` ignores it.

    @ingroup tuner
    @see format_labelled_line()
 */
[[nodiscard]] auto parse_line_score(std::string_view line) -> std::optional<int>;

/** Writes a position as a dataset line: its FEN, then the result in brackets (``[1.0]``,
    ``[0.5]`` or ``[0.0]``), then the search score if there is one.

    @ingroup tuner
    @see parse_labelled_line(), parse_line_score()
 */
[[nodiscard]] auto format_labelled_line(
    const Position& position, double result, std::optional<int> score = std::nullopt)
    -> std::string;

/** One coefficient of a position's eval trace.
    @ingroup tuner
 */
struct Tuple final {
    /** The weight's index in ``eval::Weights::values``. */
    std::uint16_t index { 0 };

    /** The weight's White coefficient minus its Black coefficient. */
    std::int16_t coeff { 0 };
};

/** One position in the dataset.
    The position itself isn't kept, only its trace.

    @ingroup tuner
 */
struct Entry final {
    /** The game result from White's point of view, in ``[0, 1]``. */
    float result { 0.5f };

    /** The game phase, in ``[0, eval::MAX_PHASE]``. */
    std::uint16_t phase { 0 };

    /** The number of tuples that this position has. */
    std::uint16_t numTuples { 0 };

    /** The index of this position's first tuple in ``Dataset::tuples``. */
    std::uint32_t firstTuple { 0 };

    /** The parts of the eval that the tuner doesn't change, from White's point of view:
        the trace's ``base``, plus the non-tunable terms times their default weights.
     */
    std::int32_t baseMg { 0 };

    /** The endgame part of ``baseMg``. */
    std::int32_t baseEg { 0 };
};

/** A set of traced positions.
    Each position only stores the tuples of its tunable weights that have non-zero
    coefficients. That's typically about 25 of the ~500 weights, which keeps
    the dataset small and makes each epoch a pass over only the non-zero coefficients.

    @ingroup tuner
 */
struct Dataset final {
    /** The positions. */
    std::vector<Entry> entries;

    /** All the positions' tuples. */
    std::vector<Tuple> tuples;

    /** Returns the number of positions. */
    [[nodiscard]] auto size() const noexcept -> size_t { return entries.size(); }

    /** Returns true if there are no positions. */
    [[nodiscard]] auto empty() const noexcept -> bool { return entries.empty(); }

    /** Returns the tuples of the given position. */
    [[nodiscard]] auto tuples_of(const Entry& entry) const noexcept -> std::span<const Tuple>
    {
        return std::span { tuples }.subspan(entry.firstTuple, entry.numTuples);
    }

    /** Appends another dataset's positions to this one. */
    void append(const Dataset& other);
};

/** Traces the given positions and returns them as a dataset, in the same order.

    Each position is checked: the eval rebuilt from its trace must equal ``evaluate()``
    exactly, and the dataset's own linear model with the default weights must be within
    1 cp of it (the engine truncates when it blends the phases, and the tuner doesn't).
    Throws ``std::runtime_error`` naming the position if either check fails.

    @ingroup tuner
 */
[[nodiscard]] auto build_dataset(std::span<const LabelledPosition> positions, size_t numThreads)
    -> Dataset;

/** Options for loading a dataset from files.
    @ingroup tuner
 */
struct LoadOptions final {
    /** The fraction of the positions to put in the validation set, in ``[0, 1)``. */
    double validationFraction { 0.1 };

    /** The seed used to choose the validation positions. */
    std::uint64_t seed { 1 };

    /** The number of threads used to trace the positions. */
    size_t numThreads { 1 };

    /** If non-zero, stop after this many positions have been accepted. */
    size_t limit { 0 };
};

/** Counts of what happened while loading a dataset.
    @ingroup tuner
 */
struct LoadStats final {
    /** The number of lines read. */
    size_t lines { 0 };

    /** The number of empty or comment (``#``) lines skipped. */
    size_t skipped { 0 };

    /** The number of lines that couldn't be parsed. */
    size_t parseErrors { 0 };

    /** The first few parse errors. */
    std::vector<std::string> firstErrors;

    /** The number of positions dropped because the side to move is in check. */
    size_t inCheck { 0 };

    /** The number of positions dropped because an earlier line had the same position. */
    size_t duplicates { 0 };

    /** The number of positions that were kept. */
    size_t accepted { 0 };
};

/** A dataset, split into training & validation positions.
    @ingroup tuner
 */
struct SplitDataset final {
    /** The positions to train on. */
    Dataset training;

    /** The positions held out for validation. */
    Dataset validation;
};

/** Loads labelled positions from a stream, one per line (see ``parse_labelled_line()``).

    Positions where the side to move is in check are dropped, as are positions that have
    already been seen (by Zobrist hash, so the move counters are ignored); the first
    occurrence is kept. Unparsable lines are counted and skipped. The positions are
    traced as they're read, so the positions themselves are never all in memory at once.

    @ingroup tuner
 */
[[nodiscard]] auto load_dataset(std::istream& input, const LoadOptions& options, LoadStats& stats)
    -> SplitDataset;

/** Loads labelled positions from the given files, with duplicates removed across all of them.
    Throws ``std::runtime_error`` if a file can't be opened.

    @ingroup tuner
 */
[[nodiscard]] auto load_dataset(
    std::span<const std::filesystem::path> files, const LoadOptions& options, LoadStats& stats)
    -> SplitDataset;

} // namespace ben_bot::tuner
