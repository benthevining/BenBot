/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

#include <catch2/catch_test_macros.hpp>
#include <libchess/board/Bitboard.hpp>
#include <libchess/board/Distances.hpp>
#include <libchess/board/File.hpp>
#include <libchess/board/Masks.hpp>
#include <libchess/board/Rank.hpp>
#include <libchess/board/Square.hpp>
#include <libchess/moves/Patterns.hpp>
#include <libchess/pieces/Colors.hpp>

static constexpr auto TAGS { "[moves][Generation][patterns]" };

using chess::board::Bitboard;
using chess::board::File;
using chess::board::Rank;
using chess::board::Square;
using chess::pieces::Color;

namespace board_masks    = chess::board::masks;
namespace starting_masks = board_masks::starting;
namespace move_gen       = chess::moves::patterns;

TEST_CASE("Patterns - pawn pushes", TAGS)
{
    SECTION("White")
    {
        SECTION("From starting position")
        {
            static constexpr auto startingPos = starting_masks::white::PAWNS;

            static constexpr auto pushes = move_gen::pawn_pushes<Color::White>(startingPos);

            STATIC_REQUIRE(pushes == board_masks::ranks::THREE);
        }

        SECTION("From H7")
        {
            static constexpr auto board = Bitboard::from_square(Square { File::H, Rank::Seven });

            static constexpr auto pushes = move_gen::pawn_pushes<Color::White>(board);

            STATIC_REQUIRE(pushes.count() == 1uz);

            STATIC_REQUIRE(pushes.test({ File::H, Rank::Eight }));
        }

        SECTION("Empty")
        {
            static constexpr Bitboard empty;

            static constexpr auto pushes = move_gen::pawn_pushes<Color::White>(empty);

            STATIC_REQUIRE(pushes.none());
        }
    }

    SECTION("Black")
    {
        SECTION("From starting position")
        {
            static constexpr auto startingPos = starting_masks::black::PAWNS;

            static constexpr auto pushes = move_gen::pawn_pushes<Color::Black>(startingPos);

            STATIC_REQUIRE(pushes == board_masks::ranks::SIX);
        }

        SECTION("From E2")
        {
            static constexpr auto board = Bitboard::from_square(Square { File::E, Rank::Two });

            static constexpr auto pushes = move_gen::pawn_pushes<Color::Black>(board);

            STATIC_REQUIRE(pushes.count() == 1uz);

            STATIC_REQUIRE(pushes.test({ File::E, Rank::One }));
        }

        SECTION("Empty")
        {
            static constexpr Bitboard empty;

            static constexpr auto pushes = move_gen::pawn_pushes<Color::Black>(empty);

            STATIC_REQUIRE(pushes.none());
        }
    }
}

TEST_CASE("Patterns - pawn double pushes", TAGS)
{
    SECTION("White")
    {
        SECTION("From starting position")
        {
            static constexpr auto startingPos = starting_masks::white::PAWNS;

            static constexpr auto pushes = move_gen::pawn_double_pushes<Color::White>(startingPos);

            STATIC_REQUIRE(pushes == board_masks::ranks::FOUR);
        }

        SECTION("Pawns not on starting rank")
        {
            static constexpr auto pawns = board_masks::ranks::ONE | board_masks::ranks::THREE;

            static constexpr auto pushes = move_gen::pawn_double_pushes<Color::White>(pawns);

            STATIC_REQUIRE(pushes.none());
        }

        SECTION("Empty")
        {
            static constexpr Bitboard empty;

            static constexpr auto pushes = move_gen::pawn_double_pushes<Color::White>(empty);

            STATIC_REQUIRE(pushes.none());
        }
    }

    SECTION("Black")
    {
        SECTION("From starting position")
        {
            static constexpr auto startingPos = starting_masks::black::PAWNS;

            static constexpr auto pushes = move_gen::pawn_double_pushes<Color::Black>(startingPos);

            STATIC_REQUIRE(pushes == board_masks::ranks::FIVE);
        }

        SECTION("Pawns not on starting rank")
        {
            static constexpr auto pawns = board_masks::ranks::EIGHT | board_masks::ranks::SIX;

            static constexpr auto pushes = move_gen::pawn_double_pushes<Color::Black>(pawns);

            STATIC_REQUIRE(pushes.none());
        }

        SECTION("Empty")
        {
            static constexpr Bitboard empty;

            static constexpr auto pushes = move_gen::pawn_double_pushes<Color::Black>(empty);

            STATIC_REQUIRE(pushes.none());
        }
    }
}

TEST_CASE("Patterns - pawn attacks", TAGS)
{
    SECTION("White")
    {
        SECTION("From D4")
        {
            static constexpr auto starting = Bitboard::from_square(Square { File::D, Rank::Four });

            static constexpr auto attacks = move_gen::pawn_attacks<Color::White>(starting);

            STATIC_REQUIRE(attacks.count() == 2uz);

            STATIC_REQUIRE(attacks.test({ File::C, Rank::Five }));
            STATIC_REQUIRE(attacks.test({ File::E, Rank::Five }));
        }

        SECTION("From A2")
        {
            static constexpr auto starting = Bitboard::from_square(Square { File::A, Rank::Two });

            static constexpr auto attacks = move_gen::pawn_attacks<Color::White>(starting);

            STATIC_REQUIRE(attacks.count() == 1uz);

            STATIC_REQUIRE(attacks.test({ File::B, Rank::Three }));
        }

        SECTION("From H5")
        {
            static constexpr auto starting = Bitboard::from_square(Square { File::H, Rank::Five });

            static constexpr auto attacks = move_gen::pawn_attacks<Color::White>(starting);

            STATIC_REQUIRE(attacks.count() == 1uz);

            STATIC_REQUIRE(attacks.test({ File::G, Rank::Six }));
        }

        SECTION("From A1, H3, and E7")
        {
            Bitboard board;

            board.set(Square { File::A, Rank::One });
            board.set(Square { File::H, Rank::Three });
            board.set(Square { File::E, Rank::Seven });

            const auto attacks = move_gen::pawn_attacks<Color::White>(board);

            REQUIRE(attacks.count() == 4uz);

            REQUIRE(attacks.test({ File::B, Rank::Two }));
            REQUIRE(attacks.test({ File::G, Rank::Four }));
            REQUIRE(attacks.test({ File::D, Rank::Eight }));
            REQUIRE(attacks.test({ File::F, Rank::Eight }));
        }

        SECTION("Empty")
        {
            static constexpr Bitboard empty;

            static constexpr auto attacks = move_gen::pawn_attacks<Color::White>(empty);

            STATIC_REQUIRE(attacks.none());
        }
    }

    SECTION("Black")
    {
        SECTION("From E3")
        {
            static constexpr auto starting = Bitboard::from_square(Square { File::E, Rank::Three });

            static constexpr auto attacks = move_gen::pawn_attacks<Color::Black>(starting);

            STATIC_REQUIRE(attacks.count() == 2uz);

            STATIC_REQUIRE(attacks.test({ File::D, Rank::Two }));
            STATIC_REQUIRE(attacks.test({ File::F, Rank::Two }));
        }

        SECTION("From A5")
        {
            static constexpr auto starting = Bitboard::from_square(Square { File::A, Rank::Five });

            static constexpr auto attacks = move_gen::pawn_attacks<Color::Black>(starting);

            STATIC_REQUIRE(attacks.count() == 1uz);

            STATIC_REQUIRE(attacks.test({ File::B, Rank::Four }));
        }

        SECTION("From H6")
        {
            static constexpr auto starting = Bitboard::from_square(Square { File::H, Rank::Six });

            static constexpr auto attacks = move_gen::pawn_attacks<Color::Black>(starting);

            STATIC_REQUIRE(attacks.count() == 1uz);

            STATIC_REQUIRE(attacks.test({ File::G, Rank::Five }));
        }

        SECTION("From A4, H8, and F3")
        {
            Bitboard board;

            board.set(Square { File::A, Rank::Four });
            board.set(Square { File::H, Rank::Eight });
            board.set(Square { File::F, Rank::Three });

            const auto attacks = move_gen::pawn_attacks<Color::Black>(board);

            REQUIRE(attacks.count() == 4uz);

            REQUIRE(attacks.test({ File::B, Rank::Three }));
            REQUIRE(attacks.test({ File::G, Rank::Seven }));
            REQUIRE(attacks.test({ File::E, Rank::Two }));
            REQUIRE(attacks.test({ File::G, Rank::Two }));
        }

        SECTION("Empty")
        {
            static constexpr Bitboard empty;

            static constexpr auto attacks = move_gen::pawn_attacks<Color::Black>(empty);

            STATIC_REQUIRE(attacks.none());
        }
    }
}

TEST_CASE("Patterns - knight moves", TAGS)
{
    using chess::board::knight_distance;

    SECTION("From D4")
    {
        static constexpr Square starting { File::D, Rank::Four };

        static constexpr auto moves = move_gen::knight(Bitboard::from_square(starting));

        STATIC_REQUIRE(moves.count() == 8uz);

        STATIC_REQUIRE(moves.test(Square { File::B, Rank::Five }));
        STATIC_REQUIRE(moves.test(Square { File::C, Rank::Six }));
        STATIC_REQUIRE(moves.test(Square { File::E, Rank::Six }));
        STATIC_REQUIRE(moves.test(Square { File::F, Rank::Five }));
        STATIC_REQUIRE(moves.test(Square { File::F, Rank::Three }));
        STATIC_REQUIRE(moves.test(Square { File::E, Rank::Two }));
        STATIC_REQUIRE(moves.test(Square { File::C, Rank::Two }));
        STATIC_REQUIRE(moves.test(Square { File::B, Rank::Three }));

        for (const auto square : moves.squares())
            REQUIRE(knight_distance(starting, square) == 1uz);
    }

    SECTION("From A1")
    {
        static constexpr Square starting { File::A, Rank::One };

        static constexpr auto moves = move_gen::knight(Bitboard::from_square(starting));

        STATIC_REQUIRE(moves.count() == 2u);

        STATIC_REQUIRE(moves.test(Square { File::B, Rank::Three }));
        STATIC_REQUIRE(moves.test(Square { File::C, Rank::Two }));

        for (const auto square : moves.squares())
            REQUIRE(knight_distance(starting, square) == 1uz);
    }

    SECTION("From A8")
    {
        static constexpr Square starting { File::A, Rank::Eight };

        static constexpr auto moves = move_gen::knight(Bitboard::from_square(starting));

        STATIC_REQUIRE(moves.count() == 2u);

        STATIC_REQUIRE(moves.test(Square { File::B, Rank::Six }));
        STATIC_REQUIRE(moves.test(Square { File::C, Rank::Seven }));

        for (const auto square : moves.squares())
            REQUIRE(knight_distance(starting, square) == 1uz);
    }

    SECTION("From H1")
    {
        static constexpr Square starting { File::H, Rank::One };

        static constexpr auto moves = move_gen::knight(Bitboard::from_square(starting));

        STATIC_REQUIRE(moves.count() == 2u);

        STATIC_REQUIRE(moves.test(Square { File::F, Rank::Two }));
        STATIC_REQUIRE(moves.test(Square { File::G, Rank::Three }));

        for (const auto square : moves.squares())
            REQUIRE(knight_distance(starting, square) == 1uz);
    }

    SECTION("From H8")
    {
        static constexpr Square starting { File::H, Rank::Eight };

        static constexpr auto moves = move_gen::knight(Bitboard::from_square(starting));

        STATIC_REQUIRE(moves.count() == 2u);

        STATIC_REQUIRE(moves.test(Square { File::F, Rank::Seven }));
        STATIC_REQUIRE(moves.test(Square { File::G, Rank::Six }));

        for (const auto square : moves.squares())
            REQUIRE(knight_distance(starting, square) == 1uz);
    }

    SECTION("From A1 and H1")
    {
        Bitboard starting;

        starting.set({ File::A, Rank::One });
        starting.set({ File::H, Rank::One });

        const auto moves = move_gen::knight(starting);

        REQUIRE(moves.count() == 4uz);

        REQUIRE(moves.test(Square { File::B, Rank::Three }));
        REQUIRE(moves.test(Square { File::C, Rank::Two }));
        REQUIRE(moves.test(Square { File::F, Rank::Two }));
        REQUIRE(moves.test(Square { File::G, Rank::Three }));
    }

    SECTION("Empty")
    {
        static constexpr Bitboard empty;

        static constexpr auto moves = move_gen::knight(empty);

        STATIC_REQUIRE(moves.none());
    }
}

TEST_CASE("Patterns - bishop moves", TAGS)
{
    using chess::board::are_on_same_diagonal;

    SECTION("From C5")
    {
        static constexpr Square starting { File::C, Rank::Five };

        static constexpr auto moves = move_gen::bishop(Bitboard::from_square(starting));

        STATIC_REQUIRE(moves.count() == 11uz);

        STATIC_REQUIRE(moves.test(Square { File::A, Rank::Seven }));
        STATIC_REQUIRE(moves.test(Square { File::B, Rank::Six }));
        STATIC_REQUIRE(moves.test(Square { File::D, Rank::Four }));
        STATIC_REQUIRE(moves.test(Square { File::E, Rank::Three }));
        STATIC_REQUIRE(moves.test(Square { File::F, Rank::Two }));
        STATIC_REQUIRE(moves.test(Square { File::G, Rank::One }));

        STATIC_REQUIRE(moves.test(Square { File::A, Rank::Three }));
        STATIC_REQUIRE(moves.test(Square { File::B, Rank::Four }));
        STATIC_REQUIRE(moves.test(Square { File::D, Rank::Six }));
        STATIC_REQUIRE(moves.test(Square { File::E, Rank::Seven }));
        STATIC_REQUIRE(moves.test(Square { File::F, Rank::Eight }));

        for (const auto square : moves.squares())
            REQUIRE(are_on_same_diagonal(starting, square));
    }

    SECTION("From F3")
    {
        static constexpr Square starting { File::F, Rank::Three };

        static constexpr auto moves = move_gen::bishop(Bitboard::from_square(starting));

        STATIC_REQUIRE(moves.count() == 11uz);

        STATIC_REQUIRE(moves.test(Square { File::A, Rank::Eight }));
        STATIC_REQUIRE(moves.test(Square { File::B, Rank::Seven }));
        STATIC_REQUIRE(moves.test(Square { File::C, Rank::Six }));
        STATIC_REQUIRE(moves.test(Square { File::D, Rank::Five }));
        STATIC_REQUIRE(moves.test(Square { File::E, Rank::Four }));
        STATIC_REQUIRE(moves.test(Square { File::G, Rank::Two }));
        STATIC_REQUIRE(moves.test(Square { File::H, Rank::One }));

        STATIC_REQUIRE(moves.test(Square { File::D, Rank::One }));
        STATIC_REQUIRE(moves.test(Square { File::E, Rank::Two }));
        STATIC_REQUIRE(moves.test(Square { File::G, Rank::Four }));
        STATIC_REQUIRE(moves.test(Square { File::H, Rank::Five }));

        for (const auto square : moves.squares())
            REQUIRE(are_on_same_diagonal(starting, square));
    }

    SECTION("From A1")
    {
        static constexpr Square starting { File::A, Rank::One };

        static constexpr auto moves = move_gen::bishop(Bitboard::from_square(starting));

        STATIC_REQUIRE(moves.count() == 7uz);

        STATIC_REQUIRE(moves.test(Square { File::B, Rank::Two }));
        STATIC_REQUIRE(moves.test(Square { File::C, Rank::Three }));
        STATIC_REQUIRE(moves.test(Square { File::D, Rank::Four }));
        STATIC_REQUIRE(moves.test(Square { File::E, Rank::Five }));
        STATIC_REQUIRE(moves.test(Square { File::F, Rank::Six }));
        STATIC_REQUIRE(moves.test(Square { File::G, Rank::Seven }));
        STATIC_REQUIRE(moves.test(Square { File::H, Rank::Eight }));

        for (const auto square : moves.squares())
            REQUIRE(are_on_same_diagonal(starting, square));
    }

    SECTION("From A8")
    {
        static constexpr Square starting { File::A, Rank::Eight };

        static constexpr auto moves = move_gen::bishop(Bitboard::from_square(starting));

        STATIC_REQUIRE(moves.count() == 7uz);

        STATIC_REQUIRE(moves.test(Square { File::B, Rank::Seven }));
        STATIC_REQUIRE(moves.test(Square { File::C, Rank::Six }));
        STATIC_REQUIRE(moves.test(Square { File::D, Rank::Five }));
        STATIC_REQUIRE(moves.test(Square { File::E, Rank::Four }));
        STATIC_REQUIRE(moves.test(Square { File::F, Rank::Three }));
        STATIC_REQUIRE(moves.test(Square { File::G, Rank::Two }));
        STATIC_REQUIRE(moves.test(Square { File::H, Rank::One }));

        for (const auto square : moves.squares())
            REQUIRE(are_on_same_diagonal(starting, square));
    }

    SECTION("From H1")
    {
        static constexpr Square starting { File::H, Rank::One };

        static constexpr auto moves = move_gen::bishop(Bitboard::from_square(starting));

        STATIC_REQUIRE(moves.count() == 7uz);

        STATIC_REQUIRE(moves.test(Square { File::A, Rank::Eight }));
        STATIC_REQUIRE(moves.test(Square { File::B, Rank::Seven }));
        STATIC_REQUIRE(moves.test(Square { File::C, Rank::Six }));
        STATIC_REQUIRE(moves.test(Square { File::D, Rank::Five }));
        STATIC_REQUIRE(moves.test(Square { File::E, Rank::Four }));
        STATIC_REQUIRE(moves.test(Square { File::F, Rank::Three }));
        STATIC_REQUIRE(moves.test(Square { File::G, Rank::Two }));

        for (const auto square : moves.squares())
            REQUIRE(are_on_same_diagonal(starting, square));
    }

    SECTION("From H8")
    {
        static constexpr Square starting { File::H, Rank::Eight };

        static constexpr auto moves = move_gen::bishop(Bitboard::from_square(starting));

        STATIC_REQUIRE(moves.count() == 7uz);

        STATIC_REQUIRE(moves.test(Square { File::A, Rank::One }));
        STATIC_REQUIRE(moves.test(Square { File::B, Rank::Two }));
        STATIC_REQUIRE(moves.test(Square { File::C, Rank::Three }));
        STATIC_REQUIRE(moves.test(Square { File::D, Rank::Four }));
        STATIC_REQUIRE(moves.test(Square { File::E, Rank::Five }));
        STATIC_REQUIRE(moves.test(Square { File::F, Rank::Six }));
        STATIC_REQUIRE(moves.test(Square { File::G, Rank::Seven }));

        for (const auto square : moves.squares())
            REQUIRE(are_on_same_diagonal(starting, square));
    }

    SECTION("From C4 and D4")
    {
        Bitboard starting;

        starting.set(Square { File::C, Rank::Four });
        starting.set(Square { File::D, Rank::Four });

        const auto moves = move_gen::bishop(starting);

        REQUIRE(moves == Bitboard { 0Xc061331e001e3361 });
    }
}

TEST_CASE("Patterns - rook moves", TAGS)
{
    SECTION("From D3")
    {
        static constexpr Square starting { File::D, Rank::Three };

        static constexpr auto moves = move_gen::rook(Bitboard::from_square(starting));

        STATIC_REQUIRE(moves.count() == 14uz);

        STATIC_REQUIRE(moves.test(Square { File::A, Rank::Three }));
        STATIC_REQUIRE(moves.test(Square { File::B, Rank::Three }));
        STATIC_REQUIRE(moves.test(Square { File::C, Rank::Three }));
        STATIC_REQUIRE(moves.test(Square { File::E, Rank::Three }));
        STATIC_REQUIRE(moves.test(Square { File::F, Rank::Three }));
        STATIC_REQUIRE(moves.test(Square { File::G, Rank::Three }));
        STATIC_REQUIRE(moves.test(Square { File::H, Rank::Three }));

        STATIC_REQUIRE(moves.test(Square { File::D, Rank::One }));
        STATIC_REQUIRE(moves.test(Square { File::D, Rank::Two }));
        STATIC_REQUIRE(moves.test(Square { File::D, Rank::Four }));
        STATIC_REQUIRE(moves.test(Square { File::D, Rank::Five }));
        STATIC_REQUIRE(moves.test(Square { File::D, Rank::Six }));
        STATIC_REQUIRE(moves.test(Square { File::D, Rank::Seven }));
        STATIC_REQUIRE(moves.test(Square { File::D, Rank::Eight }));
    }

    SECTION("From A1")
    {
        static constexpr Square starting { File::A, Rank::One };

        static constexpr auto moves = move_gen::rook(Bitboard::from_square(starting));

        STATIC_REQUIRE(moves.count() == 14uz);

        STATIC_REQUIRE(moves.test(Square { File::B, Rank::One }));
        STATIC_REQUIRE(moves.test(Square { File::C, Rank::One }));
        STATIC_REQUIRE(moves.test(Square { File::D, Rank::One }));
        STATIC_REQUIRE(moves.test(Square { File::E, Rank::One }));
        STATIC_REQUIRE(moves.test(Square { File::F, Rank::One }));
        STATIC_REQUIRE(moves.test(Square { File::G, Rank::One }));
        STATIC_REQUIRE(moves.test(Square { File::H, Rank::One }));

        STATIC_REQUIRE(moves.test(Square { File::A, Rank::Two }));
        STATIC_REQUIRE(moves.test(Square { File::A, Rank::Three }));
        STATIC_REQUIRE(moves.test(Square { File::A, Rank::Four }));
        STATIC_REQUIRE(moves.test(Square { File::A, Rank::Five }));
        STATIC_REQUIRE(moves.test(Square { File::A, Rank::Six }));
        STATIC_REQUIRE(moves.test(Square { File::A, Rank::Seven }));
        STATIC_REQUIRE(moves.test(Square { File::A, Rank::Eight }));
    }

    SECTION("From B7 and E3")
    {
        Bitboard starting;

        starting.set(Square { File::B, Rank::Seven });
        starting.set(Square { File::E, Rank::Three });

        const auto moves = move_gen::rook(starting);

        REQUIRE(moves == Bitboard { 0X12fd121212ef1212 });
    }
}

TEST_CASE("Patterns - queen moves", TAGS)
{
    SECTION("From D4")
    {
        static constexpr Square starting { File::D, Rank::Four };

        static constexpr auto moves = move_gen::queen(Bitboard::from_square(starting));

        STATIC_REQUIRE(moves.count() == 27uz);

        STATIC_REQUIRE(moves.test(Square { File::A, Rank::One }));
        STATIC_REQUIRE(moves.test(Square { File::A, Rank::Four }));
        STATIC_REQUIRE(moves.test(Square { File::A, Rank::Seven }));
        STATIC_REQUIRE(moves.test(Square { File::B, Rank::Two }));
        STATIC_REQUIRE(moves.test(Square { File::B, Rank::Four }));
        STATIC_REQUIRE(moves.test(Square { File::B, Rank::Six }));
        STATIC_REQUIRE(moves.test(Square { File::C, Rank::Three }));
        STATIC_REQUIRE(moves.test(Square { File::C, Rank::Four }));
        STATIC_REQUIRE(moves.test(Square { File::C, Rank::Five }));
        STATIC_REQUIRE(moves.test(Square { File::D, Rank::One }));
        STATIC_REQUIRE(moves.test(Square { File::D, Rank::Two }));
        STATIC_REQUIRE(moves.test(Square { File::D, Rank::Three }));
        STATIC_REQUIRE(moves.test(Square { File::D, Rank::Five }));
        STATIC_REQUIRE(moves.test(Square { File::D, Rank::Six }));
        STATIC_REQUIRE(moves.test(Square { File::D, Rank::Seven }));
        STATIC_REQUIRE(moves.test(Square { File::D, Rank::Eight }));
        STATIC_REQUIRE(moves.test(Square { File::E, Rank::Three }));
        STATIC_REQUIRE(moves.test(Square { File::E, Rank::Four }));
        STATIC_REQUIRE(moves.test(Square { File::E, Rank::Five }));
        STATIC_REQUIRE(moves.test(Square { File::F, Rank::Two }));
        STATIC_REQUIRE(moves.test(Square { File::F, Rank::Four }));
        STATIC_REQUIRE(moves.test(Square { File::F, Rank::Six }));
        STATIC_REQUIRE(moves.test(Square { File::G, Rank::One }));
        STATIC_REQUIRE(moves.test(Square { File::G, Rank::Four }));
        STATIC_REQUIRE(moves.test(Square { File::G, Rank::Seven }));
        STATIC_REQUIRE(moves.test(Square { File::H, Rank::Four }));
        STATIC_REQUIRE(moves.test(Square { File::H, Rank::Eight }));
    }

    SECTION("From C4 and F7")
    {
        Bitboard starting;

        starting.set(Square { File::C, Rank::Four });
        starting.set(Square { File::F, Rank::Seven });

        const auto moves = move_gen::queen(starting);

        REQUIRE(moves == Bitboard { 0X74df75aefb2e3524 });
    }
}

TEST_CASE("Patterns - king moves", TAGS)
{
    SECTION("From G2")
    {
        static constexpr Square starting { File::G, Rank::Two };

        static constexpr auto moves = move_gen::king(Bitboard::from_square(starting));

        STATIC_REQUIRE(moves.count() == 8u);

        STATIC_REQUIRE(moves.test(Square { File::F, Rank::Three }));
        STATIC_REQUIRE(moves.test(Square { File::G, Rank::Three }));
        STATIC_REQUIRE(moves.test(Square { File::H, Rank::Three }));
        STATIC_REQUIRE(moves.test(Square { File::F, Rank::Two }));
        STATIC_REQUIRE(moves.test(Square { File::H, Rank::Two }));
        STATIC_REQUIRE(moves.test(Square { File::F, Rank::One }));
        STATIC_REQUIRE(moves.test(Square { File::G, Rank::One }));
        STATIC_REQUIRE(moves.test(Square { File::H, Rank::One }));
    }

    SECTION("From A1")
    {
        static constexpr Square starting { File::A, Rank::One };

        static constexpr auto moves = move_gen::king(Bitboard::from_square(starting));

        STATIC_REQUIRE(moves.count() == 3u);

        STATIC_REQUIRE(moves.test(Square { File::A, Rank::Two }));
        STATIC_REQUIRE(moves.test(Square { File::B, Rank::One }));
        STATIC_REQUIRE(moves.test(Square { File::B, Rank::Two }));
    }

    SECTION("From A8")
    {
        static constexpr Square starting { File::A, Rank::Eight };

        static constexpr auto moves = move_gen::king(Bitboard::from_square(starting));

        STATIC_REQUIRE(moves.count() == 3u);

        STATIC_REQUIRE(moves.test(Square { File::A, Rank::Seven }));
        STATIC_REQUIRE(moves.test(Square { File::B, Rank::Eight }));
        STATIC_REQUIRE(moves.test(Square { File::B, Rank::Seven }));
    }

    SECTION("From H1")
    {
        static constexpr Square starting { File::H, Rank::One };

        static constexpr auto moves = move_gen::king(Bitboard::from_square(starting));

        STATIC_REQUIRE(moves.count() == 3u);

        STATIC_REQUIRE(moves.test(Square { File::H, Rank::Two }));
        STATIC_REQUIRE(moves.test(Square { File::G, Rank::Two }));
        STATIC_REQUIRE(moves.test(Square { File::G, Rank::One }));
    }

    SECTION("From H8")
    {
        static constexpr Square starting { File::H, Rank::Eight };

        static constexpr auto moves = move_gen::king(Bitboard::from_square(starting));

        STATIC_REQUIRE(moves.count() == 3u);

        STATIC_REQUIRE(moves.test(Square { File::H, Rank::Seven }));
        STATIC_REQUIRE(moves.test(Square { File::G, Rank::Seven }));
        STATIC_REQUIRE(moves.test(Square { File::G, Rank::Eight }));
    }

    SECTION("From C1")
    {
        static constexpr Square starting { File::C, Rank::One };

        static constexpr auto moves = move_gen::king(Bitboard::from_square(starting));

        STATIC_REQUIRE(moves.count() == 5u);

        STATIC_REQUIRE(moves.test(Square { File::B, Rank::One }));
        STATIC_REQUIRE(moves.test(Square { File::B, Rank::Two }));
        STATIC_REQUIRE(moves.test(Square { File::C, Rank::Two }));
        STATIC_REQUIRE(moves.test(Square { File::D, Rank::Two }));
        STATIC_REQUIRE(moves.test(Square { File::D, Rank::One }));
    }

    SECTION("From A1 and H8")
    {
        Bitboard starting;

        starting.set({ File::A, Rank::One });
        starting.set({ File::H, Rank::Eight });

        const auto moves = move_gen::king(starting);

        REQUIRE(moves.count() == 6uz);

        REQUIRE(moves.test(Square { File::A, Rank::Two }));
        REQUIRE(moves.test(Square { File::B, Rank::One }));
        REQUIRE(moves.test(Square { File::B, Rank::Two }));
        REQUIRE(moves.test(Square { File::H, Rank::Seven }));
        REQUIRE(moves.test(Square { File::G, Rank::Seven }));
        REQUIRE(moves.test(Square { File::G, Rank::Eight }));
    }

    SECTION("Empty")
    {
        static constexpr Bitboard empty;

        static constexpr auto moves = move_gen::king(empty);

        STATIC_REQUIRE(moves.none());
    }
}
