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

#include <catch2/catch_test_macros.hpp>
#include <libchess/board/File.hpp>
#include <libchess/board/Masks.hpp>
#include <libchess/board/Rank.hpp>
#include <libchess/moves/Move.hpp>
#include <libchess/pieces/Colors.hpp>
#include <magic_enum/magic_enum.hpp>

static constexpr auto TAGS { "[moves][Move]" };

using magic_enum::enum_values;

using chess::board::File;
using chess::board::Rank;
using chess::pieces::Color;

namespace moves          = chess::moves;
namespace bitboard_masks = chess::board::masks;

using PieceType = chess::pieces::Type;

static constexpr auto whiteKingStartSquare = bitboard_masks::starting::white::KING.squares().front();
static constexpr auto blackKingStartSquare = bitboard_masks::starting::black::KING.squares().front();

TEST_CASE("Move - castle_kingside()", TAGS)
{
    SECTION("White")
    {
        static constexpr auto move = moves::castle_kingside(Color::White);

        STATIC_REQUIRE(move.from == whiteKingStartSquare);
        STATIC_REQUIRE(move.to.rank == move.from.rank);
        STATIC_REQUIRE(move.piece == PieceType::King);
        STATIC_REQUIRE(not move.is_promotion());
        STATIC_REQUIRE(move.is_castling());
    }

    SECTION("Black")
    {
        static constexpr auto move = moves::castle_kingside(Color::Black);

        STATIC_REQUIRE(move.from == blackKingStartSquare);
        STATIC_REQUIRE(move.to.rank == move.from.rank);
        STATIC_REQUIRE(move.piece == PieceType::King);
        STATIC_REQUIRE(not move.is_promotion());
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
        STATIC_REQUIRE(not move.is_promotion());
        STATIC_REQUIRE(move.is_castling());
    }

    SECTION("Black")
    {
        static constexpr auto move = moves::castle_queenside(Color::Black);

        STATIC_REQUIRE(move.from == blackKingStartSquare);
        STATIC_REQUIRE(move.to.rank == move.from.rank);
        STATIC_REQUIRE(move.piece == PieceType::King);
        STATIC_REQUIRE(not move.is_promotion());
        STATIC_REQUIRE(move.is_castling());
    }
}

TEST_CASE("Move - promotion()", TAGS)
{
    SECTION("Queen promotion")
    {
        SECTION("White")
        {
            for (const auto file : enum_values<File>()) {
                const auto move = moves::promotion(file, Color::White);

                REQUIRE(move.from.file == file);
                REQUIRE(move.to.file == file);

                REQUIRE(move.from.rank == Rank::Seven);
                REQUIRE(move.to.rank == Rank::Eight);

                REQUIRE(move.piece == PieceType::Pawn);
                REQUIRE(move.promotedType.has_value());
                REQUIRE(*move.promotedType == PieceType::Queen);

                REQUIRE(move.is_promotion());
                REQUIRE(not move.is_under_promotion());
                REQUIRE(not move.is_castling());
            }
        }

        SECTION("Black")
        {
            for (const auto file : enum_values<File>()) {
                const auto move = moves::promotion(file, Color::Black);

                REQUIRE(move.from.file == file);
                REQUIRE(move.to.file == file);

                REQUIRE(move.from.rank == Rank::Two);
                REQUIRE(move.to.rank == Rank::One);

                REQUIRE(move.piece == PieceType::Pawn);
                REQUIRE(move.promotedType.has_value());
                REQUIRE(*move.promotedType == PieceType::Queen);

                REQUIRE(move.is_promotion());
                REQUIRE(not move.is_under_promotion());
                REQUIRE(not move.is_castling());
            }
        }
    }

    SECTION("Under promotion")
    {
        for (const auto color : enum_values<Color>()) {
            for (const auto file : enum_values<File>()) {
                for (const auto promotedType : { PieceType::Knight, PieceType::Bishop, PieceType::Rook }) {
                    const auto move = moves::promotion(file, color, promotedType);

                    REQUIRE(move.from.file == file);
                    REQUIRE(move.to.file == file);

                    REQUIRE(move.piece == PieceType::Pawn);
                    REQUIRE(move.promotedType.has_value());
                    REQUIRE(*move.promotedType == promotedType);

                    REQUIRE(move.is_promotion());
                    REQUIRE(move.is_under_promotion());
                    REQUIRE(not move.is_castling());
                }
            }
        }
    }
}
