/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

#include <array>
#include <cassert>
#include <charconv>
#include <concepts>
#include <cstddef> // IWYU pragma: keep - for size_t
#include <format>
#include <iterator>
#include <libchess/board/File.hpp>
#include <libchess/board/Rank.hpp>
#include <libchess/board/Square.hpp>
#include <libchess/game/CastlingRights.hpp>
#include <libchess/game/Position.hpp>
#include <libchess/notation/FEN.hpp>
#include <libchess/pieces/Colors.hpp>
#include <libchess/pieces/PieceTypes.hpp>
#include <magic_enum/magic_enum.hpp>
#include <optional>
#include <ranges>
#include <stdexcept>
#include <string>
#include <utility>

namespace chess::notation {

using board::Square;
using pieces::Color;
using std::size_t;

namespace {

    [[nodiscard, gnu::const]] constexpr char int_to_char(const size_t value) noexcept
    {
        assert(value <= 9uz);

        return '0' + value;
    }

    void write_piece_positions(
        const Position& position,
        auto            outputIt)
    {
        const auto whitePieces = position.whitePieces.occupied();
        const auto blackPieces = position.blackPieces.occupied();

        const auto allOccupied = whitePieces | blackPieces;

        for (const auto rank : std::views::reverse(magic_enum::enum_values<board::Rank>())) {
            size_t consecutiveEmpty { 0uz };

            for (const auto file : magic_enum::enum_values<board::File>()) {
                const Square square { file, rank };

                if (! allOccupied.test(square)) {
                    ++consecutiveEmpty;
                    continue;
                }

                if (consecutiveEmpty != 0uz)
                    *outputIt = int_to_char(consecutiveEmpty);

                consecutiveEmpty = 0uz;

                if (whitePieces.test(square)) {
                    const auto type = position.whitePieces.get_piece_on(square);

                    assert(type.has_value());

                    *outputIt = pieces::to_char(*type, true);

                    continue;
                }

                assert(blackPieces.test(square));

                const auto type = position.blackPieces.get_piece_on(square);

                assert(type.has_value());

                *outputIt = pieces::to_char(*type, false);
            }

            if (consecutiveEmpty != 0uz)
                *outputIt = int_to_char(consecutiveEmpty);

            if (rank != board::Rank::One)
                *outputIt = '/';
        }
    }

    void write_castling_rights(
        const game::CastlingRights& whiteRights,
        const game::CastlingRights& blackRights,
        auto                        outputIt)
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
        const std::optional<board::Square> targetSquare,
        auto                               outputIt)
    {
        if (! targetSquare.has_value()) {
            *outputIt = '-';
            return;
        }

        const auto [file, rank] = *targetSquare;

        *outputIt = board::file_to_char(file, false);
        *outputIt = board::rank_to_char(rank);
    }

    template <size_t MaxLen>
    void write_integer(
        const std::integral auto value,
        auto                     outputIt)
    {
        std::array<char, MaxLen> buffer {};

        const auto result = std::to_chars(buffer.begin(), buffer.end(), value);

        for (const auto* ptr = buffer.begin(); ptr != result.ptr; ++ptr)
            *outputIt = *ptr;
    }

} // namespace

std::string to_fen(const Position& position)
{
    std::string fen;

    // see https://www.talkchess.com/forum/viewtopic.php?t=49083
    // 92 seems to be the upper bound for FEN string length
    fen.reserve(92uz);

    write_piece_positions(position, std::back_inserter(fen));

    fen.push_back(' ');

    // side to move
    fen.push_back(
        position.sideToMove == Color::White ? 'w' : 'b');

    fen.push_back(' ');

    write_castling_rights(
        position.whiteCastlingRights, position.blackCastlingRights, std::back_inserter(fen));

    fen.push_back(' ');

    write_en_passant_target_square(
        position.enPassantTargetSquare, std::back_inserter(fen));

    fen.push_back(' ');

    // halfmove clock
    write_integer<3uz>(
        position.halfmoveClock, std::back_inserter(fen));

    fen.push_back(' ');

    // full move counter
    write_integer<4uz>(
        position.fullMoveCounter, std::back_inserter(fen));

    return fen;
}

namespace {

    [[nodiscard]] std::pair<std::string_view, std::string_view> split_at_first_space(
        const std::string_view input)
    {
        const auto spaceIdx = input.find(' ');

        if (spaceIdx == std::string_view::npos)
            throw std::invalid_argument {
                std::format("Expected space in FEN string: {}", input)
            };

        return {
            input.substr(0uz, spaceIdx),
            input.substr(spaceIdx + 1uz)
        };
    }

    [[nodiscard]] std::string_view parse_rank(
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
                case 'K': position.whitePieces.king.set(index); break;

                case '1': ++index; break;
                case '2': index += 2uz; break;
                case '3': index += 3uz; break;
                case '4': index += 4uz; break;
                case '5': index += 5uz; break;
                case '6': index += 6uz; break;
                case '7': index += 7uz; break;
                case '8': index += 8uz; break;

                case '/':
                    return fenFragment;

                default:
                    throw std::invalid_argument {
                        std::format("Unexpected char in piece positions FEN fragment: {}", fenFragment.front())
                    };
            }

            ++index;
            fenFragment = fenFragment.substr(1uz);
        } while (index < rankEnd);

        return fenFragment;
    }

    void parse_piece_positions(
        std::string_view fenFragment, Position& position)
    {
        for (const auto rank : std::views::reverse(magic_enum::enum_values<board::Rank>()))
            fenFragment = parse_rank(rank, fenFragment, position);
    }

    void parse_side_to_move(
        const std::string_view fenFragment, Position& position)
    {
        if (fenFragment.length() != 1uz)
            throw std::invalid_argument {
                std::format("Expected single character for side to move, got: {}", fenFragment)
            };

        const bool isBlack = fenFragment.front() == 'b';

        position.sideToMove = isBlack ? Color::Black : Color::White;
    }

    void parse_castling_rights(
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

    void parse_en_passant_target_square(
        const std::string_view fenFragment, Position& position)
    {
        if (fenFragment.contains('-')) {
            position.enPassantTargetSquare = std::nullopt;
            return;
        }

        position.enPassantTargetSquare = Square::from_string(fenFragment);
    }

} // namespace

Position from_fen(const std::string_view fenString)
{
    Position position;

    const auto [piecePositions, rest1] = split_at_first_space(fenString);

    parse_piece_positions(piecePositions, position);

    const auto [sideToMove, rest2] = split_at_first_space(rest1);

    parse_side_to_move(sideToMove, position);

    const auto [castlingRights, rest3] = split_at_first_space(rest2);

    parse_castling_rights(castlingRights, position);

    const auto [epTarget, rest4] = split_at_first_space(rest3);

    parse_en_passant_target_square(epTarget, position);

    const auto [halfMoveClock, fullMoveCounter] = split_at_first_space(rest4);

    std::from_chars(
        halfMoveClock.begin(), halfMoveClock.end(), position.halfmoveClock);

    std::from_chars(
        fullMoveCounter.begin(), fullMoveCounter.end(), position.fullMoveCounter);

    return position;
}

} // namespace chess::notation
