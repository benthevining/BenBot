/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

#include <catch2/catch_test_macros.hpp>
#include <libchess/board/BitboardMasks.hpp>
#include <libchess/moves/Move.hpp>
#include <libchess/pieces/Colors.hpp>

static constexpr auto TAGS { "[moves][Move]" };

using chess::pieces::Color;

namespace moves          = chess::moves;
namespace bitboard_masks = chess::board::masks;

using PieceType = chess::pieces::Type;

static constexpr auto whiteKingStartSquare = bitboard_masks::starting::white::king().squares().front();
static constexpr auto blackKingStartSquare = bitboard_masks::starting::black::king().squares().front();

TEST_CASE("Move - castle_kingside()", TAGS)
{
    SECTION("White")
    {
        static constexpr auto move = moves::castle_kingside(Color::White);

        STATIC_REQUIRE(move.from == whiteKingStartSquare);
        STATIC_REQUIRE(move.to.rank == move.from.rank);
        STATIC_REQUIRE(move.piece == PieceType::King);
        STATIC_REQUIRE(! move.is_promotion());
        STATIC_REQUIRE(move.is_castling());
    }

    SECTION("Black")
    {
        static constexpr auto move = moves::castle_kingside(Color::Black);

        STATIC_REQUIRE(move.from == blackKingStartSquare);
        STATIC_REQUIRE(move.to.rank == move.from.rank);
        STATIC_REQUIRE(move.piece == PieceType::King);
        STATIC_REQUIRE(! move.is_promotion());
        STATIC_REQUIRE(move.is_castling());
    }
}

TEST_CASE("Move - castle_queenside()", TAGS)
{
    SECTION("White")
    {
        static constexpr auto move = moves::castle_queenside(Color::White);

        STATIC_REQUIRE(move.from == whiteKingStartSquare);
        STATIC_REQUIRE(move.to.rank == move.from.rank);
        STATIC_REQUIRE(move.piece == PieceType::King);
        STATIC_REQUIRE(! move.is_promotion());
        STATIC_REQUIRE(move.is_castling());
    }

    SECTION("Black")
    {
        static constexpr auto move = moves::castle_queenside(Color::Black);

        STATIC_REQUIRE(move.from == blackKingStartSquare);
        STATIC_REQUIRE(move.to.rank == move.from.rank);
        STATIC_REQUIRE(move.piece == PieceType::King);
        STATIC_REQUIRE(! move.is_promotion());
        STATIC_REQUIRE(move.is_castling());
    }
}
