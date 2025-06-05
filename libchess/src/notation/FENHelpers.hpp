/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

// This file contains code common to FEN and EPD handling

#pragma once

#include <cassert>
#include <cstddef> // IWYU pragma: keep - for size_t
#include <format>
#include <iterator>
#include <libchess/board/Bitboard.hpp>
#include <libchess/board/File.hpp>
#include <libchess/board/Rank.hpp>
#include <libchess/board/Square.hpp>
#include <libchess/game/CastlingRights.hpp>
#include <libchess/game/Position.hpp>
#include <libchess/pieces/Colors.hpp>
#include <libchess/pieces/PieceTypes.hpp>
#include <magic_enum/magic_enum.hpp>
#include <optional>
#include <ranges>
#include <stdexcept>
#include <string_view>

namespace chess::notation::fen_helpers {

using board::Square;
using game::Position;
using pieces::Color;
using std::size_t;

[[nodiscard, gnu::const]] constexpr char int_to_char(const size_t value) noexcept
{
    assert(value <= 9uz);

    return '0' + static_cast<char>(value);
}

void write_rank(
    const Position&       position,
    const board::Rank     rank,
    const board::Bitboard allOccupied, const board::Bitboard whitePieces,
    std::output_iterator<char> auto outputIt)
{
    auto consecutiveEmpty { 0uz };

    for (const auto file : magic_enum::enum_values<board::File>()) {
        const Square square { file, rank };

        if (! allOccupied.test(square)) {
            ++consecutiveEmpty;
            continue;
        }

        if (consecutiveEmpty > 0uz) {
            *outputIt        = int_to_char(consecutiveEmpty);
            consecutiveEmpty = 0uz;
        }

        if (whitePieces.test(square)) {
            const auto type = position.whitePieces.get_piece_on(square);

            assert(type.has_value());

            *outputIt = pieces::to_char(*type, true);

            continue;
        }

        const auto type = position.blackPieces.get_piece_on(square);

        assert(type.has_value());

        *outputIt = pieces::to_char(*type, false);
    }

    if (consecutiveEmpty > 0uz)
        *outputIt = int_to_char(consecutiveEmpty);

    if (rank != board::Rank::One)
        *outputIt = '/';
}

void write_piece_positions(
    const Position&                 position,
    std::output_iterator<char> auto outputIt)
{
    const auto whitePieces = position.whitePieces.occupied;
    const auto blackPieces = position.blackPieces.occupied;
    const auto allOccupied = whitePieces | blackPieces;

    for (const auto rank : std::views::reverse(magic_enum::enum_values<board::Rank>()))
        write_rank(position, rank, allOccupied, whitePieces, outputIt);
}

void write_castling_rights(
    const game::CastlingRights&     whiteRights,
    const game::CastlingRights&     blackRights,
    std::output_iterator<char> auto outputIt)
{
    if (whiteRights.neither() && blackRights.neither()) {
        *outputIt = '-';
        return;
    }

    if (whiteRights.kingside)
        *outputIt = 'K';

    if (whiteRights.queenside)
        *outputIt = 'Q';

    if (blackRights.kingside)
        *outputIt = 'k';

    if (blackRights.queenside)
        *outputIt = 'q';
}

void write_en_passant_target_square(
    const std::optional<Square>     targetSquare,
    std::output_iterator<char> auto outputIt)
{
    if (! targetSquare.has_value()) {
        *outputIt = '-';
        return;
    }

    const auto [file, rank] = *targetSquare;

    *outputIt = board::file_to_char(file, false);
    *outputIt = board::rank_to_char(rank);
}

// returns the rest of the piece positions fragment that was left after parsing this rank
[[nodiscard]] inline std::string_view parse_rank(
    const board::Rank rank, std::string_view fenFragment, Position& position)
{
    const auto rankStart = Square { board::File::A, rank }.index();
    const auto rankEnd   = rankStart + 8uz;

    auto index = rankStart;

    do {
        if (fenFragment.empty())
            throw std::invalid_argument {
                "Unexpected end of piece positions FEN fragment"
            };

        switch (fenFragment.front()) {
            case 'p': position.blackPieces.pawns.set(index); break;
            case 'P': position.whitePieces.pawns.set(index); break;
            case 'n': position.blackPieces.knights.set(index); break;
            case 'N': position.whitePieces.knights.set(index); break;
            case 'b': position.blackPieces.bishops.set(index); break;
            case 'B': position.whitePieces.bishops.set(index); break;
            case 'r': position.blackPieces.rooks.set(index); break;
            case 'R': position.whitePieces.rooks.set(index); break;
            case 'q': position.blackPieces.queens.set(index); break;
            case 'Q': position.whitePieces.queens.set(index); break;
            case 'k': position.blackPieces.king.set(index); break;
            case 'K':
                position.whitePieces.king.set(index);
                break;

                // NB. these are all 1 less because index is incremented down below
            case '1': break;
            case '2': ++index; break;
            case '3': index += 2uz; break;
            case '4': index += 3uz; break;
            case '5': index += 4uz; break;
            case '6': index += 5uz; break;
            case '7': index += 6uz; break;
            case '8': index += 7uz; break;

            case '/':
                return fenFragment.substr(1uz);

            default:
                throw std::invalid_argument {
                    std::format("Unexpected char in piece positions FEN fragment: {}", fenFragment.front())
                };
        }

        ++index;
        fenFragment = fenFragment.substr(1uz);
    } while (index < rankEnd);

    if (! fenFragment.empty() && fenFragment.front() == '/')
        return fenFragment.substr(1uz);

    return fenFragment; // NOLINT
}

inline void parse_piece_positions(
    std::string_view fenFragment, Position& position)
{
    for (const auto rank : std::views::reverse(magic_enum::enum_values<board::Rank>()))
        fenFragment = parse_rank(rank, fenFragment, position);

    position.whitePieces.refresh_occupied();
    position.blackPieces.refresh_occupied();
}

inline void parse_side_to_move(
    const std::string_view fenFragment, Position& position)
{
    if (fenFragment.length() != 1uz)
        throw std::invalid_argument {
            std::format("Expected single character for side to move, got: {}", fenFragment)
        };

    const bool isBlack = fenFragment.front() == 'b';

    position.sideToMove = isBlack ? Color::Black : Color::White;
}

inline void parse_castling_rights(
    const std::string_view fenFragment, Position& position)
{
    if (fenFragment.contains('-')) {
        position.whiteCastlingRights.king_moved();
        position.blackCastlingRights.king_moved();
        return;
    }

    position.whiteCastlingRights.kingside  = fenFragment.contains('K');
    position.whiteCastlingRights.queenside = fenFragment.contains('Q');
    position.blackCastlingRights.kingside  = fenFragment.contains('k');
    position.blackCastlingRights.queenside = fenFragment.contains('q');
}

inline void parse_en_passant_target_square(
    const std::string_view fenFragment, Position& position)
{
    if (fenFragment.contains('-')) {
        position.enPassantTargetSquare = std::nullopt;
        return;
    }

    position.enPassantTargetSquare = Square::from_string(fenFragment);
}

} // namespace chess::notation::fen_helpers
