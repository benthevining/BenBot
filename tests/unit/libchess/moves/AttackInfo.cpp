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

#include <array>
#include <catch2/catch_test_macros.hpp>
#include <libchess/board/Bitboard.hpp>
#include <libchess/board/File.hpp>
#include <libchess/board/Masks.hpp>
#include <libchess/board/Pieces.hpp>
#include <libchess/board/Rank.hpp>
#include <libchess/board/Square.hpp>
#include <libchess/game/Position.hpp>
#include <libchess/moves/AttackInfo.hpp>
#include <libchess/moves/Attacks.hpp>
#include <libchess/moves/Patterns.hpp>
#include <libchess/moves/PseudoLegal.hpp>
#include <libchess/notation/FEN.hpp>
#include <libchess/pieces/Colors.hpp>
#include <libchess/pieces/PieceTypes.hpp>
#include <string_view>

inline constexpr auto TAGS { "[moves][attacks][AttackInfo]" };

using chess::board::Bitboard;
using chess::board::File;
using chess::board::Pieces;
using chess::board::Rank;
using chess::board::Square;
using chess::game::Position;
using chess::moves::AttackInfo;
using chess::moves::compute_attacks;
using chess::notation::from_fen;
using chess::pieces::Color;
using PieceType = chess::pieces::Type;

namespace patterns     = chess::moves::patterns;
namespace pseudo_legal = chess::moves::pseudo_legal;

namespace {

// start position, Kiwipete, the other standard perft positions, plus some positions
// with pins, promotions, en passant and stacked sliders
inline constexpr std::array FENS {
    std::string_view { "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1" },
    std::string_view { "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1" },
    std::string_view { "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1" },
    std::string_view { "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1" },
    std::string_view { "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8" },
    std::string_view { "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10" },
    std::string_view { "4k3/8/8/1b6/8/3N4/4K3/8 w - - 0 1" },                             // pinned knight
    std::string_view { "1k6/1P1P4/8/8/8/8/2p1p3/6K1 b - - 0 1" },                         // promotions
    std::string_view { "4k3/8/8/2pP4/8/8/8/4K3 w - c6 0 2" },                             // en passant
    std::string_view { "3qk3/3r4/3r4/8/8/3R4/3Q4/3RK2B w - - 0 1" },                      // stacked sliders
    std::string_view { "r1b2rk1/pp1nqppp/2p1p3/3n4/2BP4/2N1PN2/PPQ2PPP/R4RK1 w - - 0 1" } // uneven mobility
};

// independent reference implementation, using the setwise Kogge-Stone generators
// instead of magic bitboards, and without masking out friendly pieces
[[nodiscard]] auto reference_attacks(
    const Pieces& pieces, const Position& position, const Color side)
    -> AttackInfo::Side
{
    const auto emptySquares = position.occupied().inverse();

    AttackInfo::Side result;

    result.pawns = side == Color::White
                     ? patterns::pawn_attacks<Color::White>(pieces.pawns)
                     : patterns::pawn_attacks<Color::Black>(pieces.pawns);

    result.knights = patterns::knight(pieces.knights);
    result.bishops = pseudo_legal::bishop(pieces.bishops, emptySquares, { });
    result.rooks   = pseudo_legal::rook(pieces.rooks, emptySquares, { });
    result.queens  = pseudo_legal::queen(pieces.queens, emptySquares, { });
    result.king    = patterns::king(pieces.king);

    result.all = result.pawns | result.knights | result.bishops
               | result.rooks | result.queens | result.king;

    // brute force: count the attackers of each square, one piece at a time
    std::array<int, 64> numAttackers { };

    auto add_piece = [&numAttackers](const Bitboard attacks) {
        for (const auto square : attacks.squares())
            ++numAttackers.at(square.index());
    };

    for (const auto square : pieces.pawns.squares()) {
        const auto pawn = Bitboard::from_square(square);

        add_piece(side == Color::White
                      ? patterns::pawn_attacks<Color::White>(pawn)
                      : patterns::pawn_attacks<Color::Black>(pawn));
    }

    for (const auto square : pieces.knights.squares())
        add_piece(patterns::knight(Bitboard::from_square(square)));

    for (const auto square : pieces.bishops.squares())
        add_piece(pseudo_legal::bishop(Bitboard::from_square(square), emptySquares, { }));

    for (const auto square : pieces.rooks.squares())
        add_piece(pseudo_legal::rook(Bitboard::from_square(square), emptySquares, { }));

    for (const auto square : pieces.queens.squares())
        add_piece(pseudo_legal::queen(Bitboard::from_square(square), emptySquares, { }));

    add_piece(patterns::king(pieces.king));

    for (const auto square : chess::board::masks::ALL.squares())
        if (numAttackers.at(square.index()) >= 2)
            result.twice.set(square);

    return result;
}

// the squares that chess::moves::num_squares_attacked() counts: pawn, knight & king attacks
// include squares with friendly pieces on them, but slider attacks don't
[[nodiscard]] auto counted_squares(
    const AttackInfo::Side& attacks, const Pieces& pieces, const bool includeKing)
    -> Bitboard
{
    auto result = attacks.pawns | attacks.knights
                | ((attacks.bishops | attacks.rooks | attacks.queens) & pieces.occupied.inverse());

    if (includeKing)
        result |= attacks.king;

    return result;
}

} // namespace

TEST_CASE("AttackInfo - matches reference attack sets", TAGS)
{
    for (const auto fen : FENS) {
        INFO(fen);

        const auto position = from_fen(fen).value();

        const auto info = compute_attacks(position);

        REQUIRE(info.white == reference_attacks(position.whitePieces, position, Color::White));
        REQUIRE(info.black == reference_attacks(position.blackPieces, position, Color::Black));

        REQUIRE(&info.side<Color::White>() == &info.white);
        REQUIRE(&info.side<Color::Black>() == &info.black);
        REQUIRE(&info.side(Color::White) == &info.white);
        REQUIRE(&info.side(Color::Black) == &info.black);
    }
}

TEST_CASE("AttackInfo - same squares as num_squares_attacked()", TAGS)
{
    namespace masks = chess::board::masks;

    for (const auto fen : FENS) {
        INFO(fen);

        const auto position = from_fen(fen).value();

        const auto info = compute_attacks(position);

        for (const bool includeKing : { true, false }) {
            const auto white = counted_squares(info.white, position.whitePieces, includeKing);
            const auto black = counted_squares(info.black, position.blackPieces, includeKing);

            auto check_target = [&](const Bitboard target) {
                REQUIRE((target & white).count()
                        == chess::moves::num_squares_attacked<Color::White>(
                            position.whitePieces, target, position.blackPieces.occupied, includeKing));

                REQUIRE((target & black).count()
                        == chess::moves::num_squares_attacked<Color::Black>(
                            position.blackPieces, target, position.whitePieces.occupied, includeKing));
            };

            check_target(masks::ALL);
            check_target(masks::CENTER);

            for (const auto square : masks::ALL.squares())
                check_target(Bitboard::from_square(square));
        }
    }
}

TEST_CASE("AttackInfo - defended pieces are included", TAGS)
{
    static const Position startPos;

    const auto info = compute_attacks(startPos);

    // knight on b1 defends d2, rook on a1 defends a2 & b1
    REQUIRE(info.white.knights.test(Square { File::D, Rank::Two }));
    REQUIRE(info.white.rooks.test(Square { File::A, Rank::Two }));
    REQUIRE(info.white.rooks.test(Square { File::B, Rank::One }));

    // the rook can't see past its own pieces
    REQUIRE_FALSE(info.white.rooks.test(Square { File::A, Rank::Three }));

    // every white piece except the rooks is defended by something
    const auto rooks = Bitboard::from_square(Square { File::A, Rank::One })
                     | Bitboard::from_square(Square { File::H, Rank::One });

    REQUIRE((info.white.all & startPos.whitePieces.occupied)
            == (startPos.whitePieces.occupied & rooks.inverse()));

    REQUIRE(info.white.pawns == chess::board::masks::ranks::THREE);
    REQUIRE(info.black.pawns == chess::board::masks::ranks::SIX);
}

TEST_CASE("AttackInfo - squares attacked twice", TAGS)
{
    // the white knights on b1 & c4 both attack d2 & a3, and the pawns on c3 & e3 both attack d4
    const auto position = from_fen("4k3/8/8/8/2N5/2P1P3/8/1N5K w - - 0 1").value();

    const auto info = compute_attacks(position);

    REQUIRE(info.white.twice.test(Square { File::D, Rank::Two }));
    REQUIRE(info.white.twice.test(Square { File::A, Rank::Three }));
    REQUIRE(info.white.twice.test(Square { File::D, Rank::Four }));

    // only the c3 pawn attacks b4, and only the b1 knight attacks c3
    REQUIRE_FALSE(info.white.twice.test(Square { File::B, Rank::Four }));
    REQUIRE_FALSE(info.white.twice.test(Square { File::C, Rank::Three }));

    REQUIRE(info.black.twice.none());
}

TEST_CASE("AttackInfo - per-piece callback", TAGS)
{
    for (const auto fen : FENS) {
        INFO(fen);

        const auto position = from_fen(fen).value();

        AttackInfo fromCallback;

        auto numCalls = 0uz;

        const auto info = compute_attacks(
            position,
            [&](const Color color, const PieceType type, const Bitboard attacks, const AttackInfo& partial) {
                ++numCalls;

                // pawn & king attacks are filled in before any callbacks happen
                REQUIRE(partial.white.pawns == patterns::pawn_attacks<Color::White>(position.whitePieces.pawns));
                REQUIRE(partial.black.pawns == patterns::pawn_attacks<Color::Black>(position.blackPieces.pawns));
                REQUIRE(partial.white.king == patterns::king(position.whitePieces.king));
                REQUIRE(partial.black.king == patterns::king(position.blackPieces.king));

                auto& side = color == Color::White ? fromCallback.white : fromCallback.black;

                if (type == PieceType::Knight)
                    side.knights |= attacks;
                else if (type == PieceType::Bishop)
                    side.bishops |= attacks;
                else if (type == PieceType::Rook)
                    side.rooks |= attacks;
                else if (type == PieceType::Queen)
                    side.queens |= attacks;
                else
                    FAIL("unexpected piece type");
            });

        auto num_pieces = [](const Pieces& pieces) {
            return pieces.knights.count() + pieces.bishops.count() + pieces.rooks.count() + pieces.queens.count();
        };

        REQUIRE(numCalls == num_pieces(position.whitePieces) + num_pieces(position.blackPieces));

        REQUIRE(fromCallback.white.knights == info.white.knights);
        REQUIRE(fromCallback.white.bishops == info.white.bishops);
        REQUIRE(fromCallback.white.rooks == info.white.rooks);
        REQUIRE(fromCallback.white.queens == info.white.queens);
        REQUIRE(fromCallback.black.knights == info.black.knights);
        REQUIRE(fromCallback.black.bishops == info.black.bishops);
        REQUIRE(fromCallback.black.rooks == info.black.rooks);
        REQUIRE(fromCallback.black.queens == info.black.queens);
    }
}
