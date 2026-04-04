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
#include <catch2/catch_test_macros.hpp>
#include <libchess/board/File.hpp>
#include <libchess/board/Rank.hpp>
#include <libchess/board/Square.hpp>
#include <libchess/moves/MoveGen.hpp>
#include <libchess/notation/Algebraic.hpp>
#include <libchess/notation/FEN.hpp>

inline constexpr auto TAGS { "[moves][EnPassant]" };

using chess::notation::from_fen;

TEST_CASE("En passant - illegal if capture reveals check", TAGS)
{
    const auto position = from_fen("4k3/8/8/p1K1Pp1r/Pp5p/6pP/6P1/8 w - f6 0 1")
                              .value();

    const auto move = chess::notation::from_alg(position, "exf6")
                          .value();

    REQUIRE_FALSE(position.is_legal(move));
}

TEST_CASE("Potentially legal EP target squares", TAGS)
{
    using chess::board::File;
    using chess::board::Rank;
    using chess::board::Square;

    using chess::moves::get_potentially_legal_en_passant_target_squares;
    using std::ranges::is_sorted;

    SECTION("Simple case")
    {
        const auto position = from_fen("rnbqkbnr/ppppp1pp/8/4Pp2/8/8/PPPP1PPP/RNBQKBNR w KQkq - 0 1")
                                  .value();

        const auto squares = get_potentially_legal_en_passant_target_squares(position);

        REQUIRE(squares.size() == 1uz);

        REQUIRE(is_sorted(squares));

        REQUIRE(squares.front() == Square { .file = File::F, .rank = Rank::Six });
    }

    SECTION("Multiple possibilities")
    {
        const auto position = from_fen("rnbqkbnr/p1p1p1pp/8/Pp1pPp2/8/8/1PPP1PPP/RNBQKBNR w KQkq - 0 1")
                                  .value();

        const auto squares = get_potentially_legal_en_passant_target_squares(position);

        REQUIRE(squares.size() == 3uz);

        REQUIRE(is_sorted(squares));

        REQUIRE(squares.front() == Square { .file = File::B, .rank = Rank::Six });

        REQUIRE(squares.at(1uz) == Square { .file = File::D, .rank = Rank::Six });

        REQUIRE(squares.back() == Square { .file = File::F, .rank = Rank::Six });
    }

    SECTION("Returned list should filter duplicates")
    {
        const auto position = from_fen("rnbqkbnr/p1p1p1p1/8/PpPpPpPp/8/8/1P1P1P1P/RNBQKBNR w KQkq - 0 1")
                                  .value();

        const auto squares = get_potentially_legal_en_passant_target_squares(position);

        REQUIRE(is_sorted(squares));

        REQUIRE(squares.size() == 4uz);
    }
}
