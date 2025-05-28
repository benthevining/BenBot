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
#include <string>

namespace chess::notation {

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
                const board::Square square { file, rank };

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
        position.sideToMove == pieces::Color::White ? 'w' : 'b');

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

} // namespace chess::notation
