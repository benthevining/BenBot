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

#include "support/Fixtures.hpp"
#include <algorithm>
#include <catch2/catch_test_macros.hpp>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iterator>
#include <libchess/game/Position.hpp>
#include <libchess/notation/PGN.hpp>
#include <random>
#include <sstream>
#include <string>
#include <tuner/Dataset.hpp>
#include <tuner/Extract.hpp>
#include <vector>

namespace {

using ben_bot::tuner::extract;
using ben_bot::tuner::extract_positions;
using ben_bot::tuner::ExtractOptions;
using ben_bot::tuner::ExtractStats;
using ben_bot::tuner::parse_move_comment;

constexpr auto TAGS = "[tuner][extract]";

// games.pgn has two fastchess self-play games (a White win & a Black win, each with 16 book plies),
// a game from a KQK position that ends in mate (every engine move after the first few has a mate
// score), and an unfinished game
[[nodiscard]] auto read_file(const std::string& name) -> std::string
{
    std::ifstream file { tuner_tests::data_file(name) };

    REQUIRE(file.is_open());

    return { std::istreambuf_iterator<char> { file }, std::istreambuf_iterator<char> { } };
}

[[nodiscard]] auto run_extract(const ExtractOptions& options, const std::uint64_t seed, ExtractStats& stats)
    -> std::string
{
    std::istringstream input { read_file("games.pgn") };

    std::ostringstream output;

    std::mt19937_64 rng { seed };

    extract(input, output, options, rng, stats);

    return output.str();
}

} // namespace

TEST_CASE("Extract - parse_move_comment", TAGS)
{
    REQUIRE(parse_move_comment("book").isBook);
    REQUIRE(parse_move_comment(" book ").isBook);

    REQUIRE(parse_move_comment("+0.18/5 0.139s, n=7818").score == 18);
    REQUIRE(parse_move_comment("-1.08/4 0.169s, n=17671").score == -108);
    REQUIRE(parse_move_comment("0.00/12 0.1s").score == 0);
    REQUIRE(parse_move_comment("-10.92/5 0.090s, n=6701, White wins by adjudication").score == -1092);

    for (const auto* mate : { "+M11/10 0.071s, n=8185", "-M2/1 0.000s, n=2", "+M1/1 0.000s, n=28, White mates" }) {
        const auto comment = parse_move_comment(mate);

        INFO(mate);
        REQUIRE(comment.isMate);
        REQUIRE(not comment.score.has_value());
    }

    for (const auto* noScore : { "", "White mates", "some text" }) {
        const auto comment = parse_move_comment(noScore);

        INFO(noScore);
        REQUIRE(not comment.isBook);
        REQUIRE(not comment.isMate);
        REQUIRE(not comment.score.has_value());
    }
}

TEST_CASE("Extract - counts & labels", TAGS)
{
    ExtractStats stats;

    const auto text = run_extract({ }, 1uz, stats);

    REQUIRE(stats.games == 4uz);
    REQUIRE(stats.skippedGames == 1uz);      // the unfinished game
    REQUIRE(stats.openingPositions == 56uz); // 16 book + 8 plies in each self-play game, & 8 plies of the KQK game
    REQUIRE(stats.inCheck == 5uz);           // 3 + 1 + 1
    REQUIRE(stats.mateScores == 6uz);
    REQUIRE(stats.candidates == 121uz); // 31 + 90 + 0
    REQUIRE(stats.extracted == 20uz);   // 10 from each self-play game

    std::istringstream lines { text };

    std::string line;

    auto numLines     = 0uz;
    auto numWhiteWins = 0uz;
    auto numBlackWins = 0uz;

    while (std::getline(lines, line)) {
        ++numLines;

        const auto parsed = ben_bot::tuner::parse_labelled_line(line);

        INFO(line);
        REQUIRE(parsed.has_value());
        REQUIRE(not parsed->position.is_check());

        // each self-play game's engine moves all have scores
        REQUIRE(ben_bot::tuner::parse_line_score(line).has_value());

        if (parsed->result == 1.)
            ++numWhiteWins;
        else if (parsed->result == 0.)
            ++numBlackWins;
    }

    REQUIRE(numLines == 20uz);
    REQUIRE(numWhiteWins == 10uz);
    REQUIRE(numBlackWins == 10uz);
}

TEST_CASE("Extract - the book plies & in-check positions are skipped", TAGS)
{
    const auto games = chess::notation::parse_all_pgns(read_file("games.pgn"));

    REQUIRE(games.size() == 4uz);

    static constexpr ExtractOptions options { .positionsPerGame = 0uz, .skipPlies = 8uz };

    const std::vector expectedCounts { 31uz, 90uz, 0uz, 0uz };
    const std::vector expectedResults { 1., 0., 1., 0.5 };

    std::mt19937_64 rng { 1uz };

    for (auto i = 0uz; i < games.size(); ++i) {
        const auto& game = games.at(i);

        ExtractStats stats;

        const auto positions = extract_positions(game, options, rng, stats);

        INFO("game " << i);
        REQUIRE(positions.size() == expectedCounts.at(i));

        // the positions in the book & the 8 plies after it
        std::vector<chess::game::Position::Hash> opening;

        auto position = game.get_starting_position();

        for (const auto& move : game.moves.moves) {
            if (opening.size() == stats.openingPositions)
                break;

            opening.push_back(position.hash);
            position.make_move(move.move);
        }

        for (const auto& [labelled, score] : positions) {
            REQUIRE(std::abs(labelled.result - expectedResults.at(i)) < 1e-9);
            REQUIRE(not labelled.position.is_check());
            REQUIRE(std::ranges::find(opening, labelled.position.hash) == opening.end());
        }
    }

    // the first position after the opening in the first game is before 13. axb5 {+0.03/5 ...},
    // and the next is before 13... cxb5 {-0.10/6 ...}, which is +0.10 from White's point of view
    ExtractStats stats;

    const auto first = extract_positions(games.front(), options, rng, stats);

    REQUIRE(stats.openingPositions == 24uz);
    REQUIRE(first.at(0uz).score == 3);
    REQUIRE(first.at(1uz).score == 10);
}

TEST_CASE("Extract - the same seed gives the same positions", TAGS)
{
    ExtractStats stats;

    const auto first  = run_extract({ }, 1uz, stats);
    const auto second = run_extract({ }, 1uz, stats);
    const auto other  = run_extract({ }, 2uz, stats);

    REQUIRE(first == second);
    REQUIRE(first != other);

    // the sampled positions are a subset of all of them, in the same order
    ExtractStats allStats;

    const auto all = run_extract({ .positionsPerGame = 0uz }, 1uz, allStats);

    REQUIRE(allStats.extracted == 121uz);

    std::istringstream sampled { first };

    std::string line;

    auto searchFrom = 0uz;

    while (std::getline(sampled, line)) {
        const auto found = all.find(line + '\n', searchFrom);

        INFO(line);
        REQUIRE(found != std::string::npos);

        searchFrom = found + line.size();
    }
}
