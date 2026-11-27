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

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <format>
#include <istream>
#include <libbenbot/eval/Evaluation.hpp>
#include <libbenbot/eval/SEE.hpp>
#include <libchess/game/Position.hpp>
#include <libchess/moves/Move.hpp>
#include <libchess/moves/MoveGen.hpp>
#include <optional>
#include <ostream>
#include <string>
#include <string_view>
#include <tuner/Dataset.hpp>
#include <tuner/Parallel.hpp>
#include <tuner/Resolve.hpp>
#include <utility>
#include <vector>

namespace ben_bot::tuner {

namespace {

    using std::string;
    using std::string_view;

    // larger than any eval
    constexpr auto INFINITE_SCORE = 32000;

    // a mate in n plies scores MATE_SCORE - n
    constexpr auto MATE_SCORE = 31000;

    // the search stops here, even if the position isn't quiet; captures run out long before this
    constexpr auto MAX_PLY = 64uz;

    [[nodiscard]] auto trim(string_view text) noexcept -> string_view
    {
        const auto is_space = [](const char character) {
            return std::isspace(static_cast<unsigned char>(character)) != 0;
        };

        while (not text.empty() and is_space(text.front()))
            text.remove_prefix(1uz);

        while (not text.empty() and is_space(text.back()))
            text.remove_suffix(1uz);

        return text;
    }

    struct ScoredMove final {
        Move move;

        int see { 0 };
    };

    // the moves to search, best exchanges first. Ties keep the move generator's order, so the
    // search is the same every time
    [[nodiscard]] auto moves_to_search(const Position& position, const bool inCheck)
        -> std::vector<ScoredMove>
    {
        std::vector<ScoredMove> moves;

        for (const auto move : chess::moves::generate(position)) {
            if (inCheck) {
                moves.push_back({ .move = move });
                continue;
            }

            if (position.is_quiet(move) or move.is_under_promotion())
                continue;

            // skip captures that lose material once all recaptures are played out
            if (const auto see = eval::see(position, move); see >= 0)
                moves.push_back({ .move = move, .see = see });
        }

        std::ranges::stable_sort(moves, std::ranges::greater { }, &ScoredMove::see);

        return moves;
    }

    // a fail-soft alpha-beta quiescence search that fills pv with its principal variation
    [[nodiscard]] auto quiescence(
        const Position& position, int alpha, const int beta, const size_t ply, std::vector<Move>& pv)
        -> int
    {
        pv.clear();

        const auto inCheck = position.is_check();

        auto bestScore = -INFINITE_SCORE;

        if (not inCheck or ply >= MAX_PLY) {
            // the stand pat score is a lower bound, since we can always choose not to capture
            bestScore = static_cast<int>(eval::evaluate(position));

            if (bestScore >= beta or ply >= MAX_PLY)
                return bestScore;

            alpha = std::max(alpha, bestScore);
        }

        const auto moves = moves_to_search(position, inCheck);

        if (inCheck and moves.empty())
            return -MATE_SCORE + static_cast<int>(ply);

        std::vector<Move> childPV;

        for (const auto& [move, see] : moves) {
            const auto score = -quiescence(after_move(position, move), -beta, -alpha, ply + 1uz, childPV);

            bestScore = std::max(bestScore, score);

            if (score > alpha) {
                alpha = score;

                pv.assign(1uz, move);
                pv.insert(pv.end(), childPV.begin(), childPV.end());
            }

            if (score >= beta)
                break;
        }

        return bestScore;
    }

    struct PendingLine final {
        LabelledPosition labelled;

        std::optional<int> score;
    };

    struct ResolvedLine final {
        std::optional<string> text;

        bool changed { false };
    };

    [[nodiscard]] auto resolve_line(const PendingLine& pending) -> ResolvedLine
    {
        const auto resolved = resolve(pending.labelled.position);

        // a checkmate isn't a position the eval can learn from
        if (resolved.position.is_check())
            return { };

        return {
            .text    = format_labelled_line(resolved.position, pending.labelled.result, pending.score),
            .changed = not resolved.pv.empty()
        };
    }

} // namespace

auto resolve(const Position& position) -> Resolved
{
    Resolved resolved;

    resolved.score = quiescence(position, -INFINITE_SCORE, INFINITE_SCORE, 0uz, resolved.pv);

    resolved.position = position;

    for (const auto move : resolved.pv)
        resolved.position.make_move(move);

    return resolved;
}

void resolve_dataset(std::istream& input, std::ostream& output, const size_t numThreads, ResolveStats& stats)
{
    // the positions are resolved in batches of this many, so that they're never all in memory
    static constexpr auto BATCH_SIZE = 1uz << 14uz;

    static constexpr auto MAX_ERRORS_KEPT = 10uz;

    std::vector<PendingLine> batch;

    const auto flush = [&] {
        std::vector<ResolvedLine> results(batch.size());

        parallel_for(numThreads, batch.size(), [&batch, &results](const size_t, const size_t begin, const size_t end) {
            for (auto i = begin; i < end; ++i)
                results.at(i) = resolve_line(batch.at(i));
        });

        for (const auto& [text, changed] : results) {
            if (not text.has_value()) {
                ++stats.inCheck;
                continue;
            }

            if (changed)
                ++stats.changed;
            else
                ++stats.unchanged;

            output << *text << '\n';
        }

        batch.clear();
    };

    string line;

    while (std::getline(input, line)) {
        ++stats.lines;

        const auto trimmed = trim(line);

        if (trimmed.empty() or trimmed.front() == '#') {
            ++stats.skipped;
            continue;
        }

        auto parsed = parse_labelled_line(trimmed);

        if (not parsed.has_value()) {
            ++stats.parseErrors;

            if (stats.firstErrors.size() < MAX_ERRORS_KEPT)
                stats.firstErrors.emplace_back(std::format("line {}: {}", stats.lines, parsed.error()));

            continue;
        }

        batch.push_back({ .labelled = std::move(*parsed), .score = parse_line_score(trimmed) });

        if (batch.size() >= BATCH_SIZE)
            flush();
    }

    flush();
}

} // namespace ben_bot::tuner
