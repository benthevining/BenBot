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
#include <cmath>
#include <cstdlib>
#include <exception>
#include <istream>
#include <iterator>
#include <libchess/game/Result.hpp>
#include <libchess/notation/PGN.hpp>
#include <libchess/pieces/Colors.hpp>
#include <numeric>
#include <optional>
#include <ostream>
#include <random>
#include <string>
#include <string_view>
#include <tuner/Dataset.hpp>
#include <tuner/Extract.hpp>
#include <utility>
#include <vector>

namespace ben_bot::tuner {

namespace {

    using std::string;
    using std::string_view;

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

    [[nodiscard]] auto result_from_white_pov(const chess::game::Result result) noexcept -> double
    {
        using chess::game::Result;

        if (result == Result::WhiteWon)
            return 1.;

        if (result == Result::BlackWon)
            return 0.;

        return 0.5;
    }

    // picks count of the candidates at random, and returns them in their original order. This uses
    // the generator's output directly, because std::uniform_int_distribution differs between
    // standard libraries, and the same seed should give the same dataset on every platform
    [[nodiscard]] auto sample(
        std::vector<ExtractedPosition> candidates, const size_t count,
        std::mt19937_64& rng) // cppcheck-suppress constParameterReference
        -> std::vector<ExtractedPosition>
    {
        if (count == 0uz or candidates.size() <= count)
            return candidates;

        std::vector<size_t> indices(candidates.size());

        std::iota(indices.begin(), indices.end(), 0uz);

        // the first count steps of a Fisher-Yates shuffle
        for (auto i = 0uz; i < count; ++i) {
            const auto j = i + static_cast<size_t>(rng() % (indices.size() - i));

            std::swap(indices.at(i), indices.at(j));
        }

        indices.resize(count);

        std::ranges::sort(indices);

        std::vector<ExtractedPosition> chosen;

        chosen.reserve(count);

        std::ranges::transform(indices, std::back_inserter(chosen), [&candidates](const size_t index) {
            return std::move(candidates.at(index));
        });

        return chosen;
    }

} // namespace

auto parse_move_comment(string_view comment) -> MoveComment
{
    comment = trim(comment);

    if (comment == "book")
        return { .isBook = true, .isMate = false, .score = std::nullopt };

    // the score is the first word, up to the / before the depth
    const auto score = comment.substr(0uz, comment.find_first_of("/ \t,"));

    if (score.contains('M') or score.contains('#'))
        return { .isBook = false, .isMate = true, .score = std::nullopt };

    if (score.empty())
        return { };

    const string copy { score };

    char* end { nullptr };

    const auto pawns = std::strtod(copy.c_str(), &end);

    if (end != copy.c_str() + copy.size() or not std::isfinite(pawns))
        return { };

    return { .isBook = false, .isMate = false, .score = static_cast<int>(std::lround(pawns * 100.)) };
}

auto extract_positions(
    const chess::notation::GameRecord& game, const ExtractOptions& options,
    std::mt19937_64& rng, ExtractStats& stats)
    -> std::vector<ExtractedPosition>
{
    ++stats.games;

    if (not game.result.has_value()) {
        ++stats.skippedGames;
        return { };
    }

    const auto result = result_from_white_pov(*game.result);

    const auto& moves = game.moves.moves;

    auto bookPlies = 0uz;

    while (bookPlies < moves.size() and parse_move_comment(moves.at(bookPlies).comment).isBook)
        ++bookPlies;

    const auto firstPly = bookPlies + options.skipPlies;

    std::vector<ExtractedPosition> candidates;

    auto position = game.get_starting_position();

    // each position is the one before a move, so its score is the one in that move's comment
    for (auto ply = 0uz; ply < moves.size(); ++ply) {
        const auto& move = moves.at(ply);

        if (ply < firstPly)
            ++stats.openingPositions;
        else if (position.is_check())
            ++stats.inCheck;
        else if (const auto comment = parse_move_comment(move.comment); comment.isMate)
            ++stats.mateScores;
        else {
            // the comment's score is from the point of view of the side that played the move
            auto score = comment.score;

            if (score.has_value() and position.sideToMove == chess::pieces::Color::Black)
                score = -*score;

            candidates.push_back({ .labelled = { .position = position, .result = result }, .score = score });
        }

        position.make_move(move.move);
    }

    stats.candidates += candidates.size();

    auto chosen = sample(std::move(candidates), options.positionsPerGame, rng);

    stats.extracted += chosen.size();

    return chosen;
}

void extract(
    std::istream& pgn, std::ostream& output, const ExtractOptions& options,
    std::mt19937_64& rng, ExtractStats& stats)
{
    string gameText;

    const auto finish_game = [&] {
        if (trim(gameText).empty())
            return;

        // a game that libchess can't parse is skipped, rather than stopping a long run
        const auto game = [&gameText] -> std::optional<chess::notation::GameRecord> {
            try {
                if (auto parsed = chess::notation::from_pgn(gameText))
                    return std::move(*parsed);
            } catch (const std::exception&) { // NOLINT(bugprone-empty-catch)
            }

            return std::nullopt;
        }();

        gameText.clear();

        if (not game.has_value()) {
            ++stats.games;
            ++stats.skippedGames;
            return;
        }

        for (const auto& [labelled, score] : extract_positions(*game, options, rng, stats))
            output << format_labelled_line(labelled.position, labelled.result, score) << '\n';
    };

    string line;

    auto inMoveText = false;

    // a game ends at the first tag line after its moves, as in chess::notation::parse_all_pgns()
    while (std::getline(pgn, line)) {
        if (line.ends_with('\r'))
            line.pop_back();

        const auto isTag = line.starts_with('[');

        if (isTag and inMoveText) {
            finish_game();
            inMoveText = false;
        }

        if (not isTag and not trim(line).empty())
            inMoveText = true;

        gameText.append(line);
        gameText.push_back('\n');
    }

    finish_game();
}

} // namespace ben_bot::tuner
