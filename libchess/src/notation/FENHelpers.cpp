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

#include "FENHelpers.hpp"
#include <array>
#include <expected>
#include <format>
#include <iterator>
#include <libchess/board/Bitboard.hpp>
#include <libchess/board/File.hpp>
#include <libchess/board/Rank.hpp>
#include <libchess/game/CastlingRights.hpp>
#include <libchess/game/Position.hpp>
#include <libchess/pieces/Colors.hpp>
#include <libchess/pieces/PieceTypes.hpp>
#include <libchess/util/Strings.hpp>
#include <magic_enum/magic_enum.hpp>
#include <optional>
#include <ranges>
#include <string>
#include <string_view>
#include <utility>

namespace chess::notation::fen_helpers {

using pieces::Color;
using std::string;
using std::string_view;

using magic_enum::enum_values;

namespace {

    void write_rank(
        const Position&   position,
        const board::Rank rank,
        string&           output)
    {
        using util::write_integer;

        const auto whitePieces = position.whitePieces.occupied;
        const auto allOccupied = whitePieces | position.blackPieces.occupied;

        auto consecutiveEmpty { 0uz };

        for (const auto file : enum_values<board::File>()) {
            const Square square { .file = file, .rank = rank };

            if (not allOccupied.test(square)) {
                ++consecutiveEmpty;
                continue;
            }

            if (consecutiveEmpty > 0uz) {
                write_integer<1uz>(consecutiveEmpty, output);
                consecutiveEmpty = 0uz;
            }

            if (whitePieces.test(square)) {
                const auto type = position.whitePieces.get_piece_on(square);

                output.push_back(to_char(type.value(), true));

                continue;
            }

            const auto type = position.blackPieces.get_piece_on(square);

            output.push_back(to_char(type.value(), false));
        }

        if (consecutiveEmpty > 0uz)
            write_integer<1uz>(consecutiveEmpty, output);

        if (rank != board::Rank::One)
            output.push_back('/');
    }

} // namespace

void write_piece_positions(
    const Position& position,
    string&         output)
{
    for (const auto rank : std::views::reverse(enum_values<board::Rank>()))
        write_rank(position, rank, output);
}

void write_castling_rights(
    const game::CastlingRights& whiteRights,
    const game::CastlingRights& blackRights,
    string&                     output)
{
    if (whiteRights.neither() and blackRights.neither()) {
        output.push_back('-');
        return;
    }

    if (whiteRights.kingside)
        output.push_back('K');

    if (whiteRights.queenside)
        output.push_back('Q');

    if (blackRights.kingside)
        output.push_back('k');

    if (blackRights.queenside)
        output.push_back('q');
}

void write_en_passant_target_square(
    const std::optional<Square> targetSquare,
    string&                     output)
{
    using Placeholder = std::optional<int>;

    targetSquare
        .and_then([&output](const Square epSquare) {
            output.push_back(file_to_char(epSquare.file));
            output.push_back(rank_to_char(epSquare.rank));

            return Placeholder { 1 }; // return placeholder with value
        })
        .or_else([&output] {
            output.push_back('-');

            return Placeholder {};
        });
}

namespace {

    // returns the rest of the piece positions fragment that was left after parsing this rank
    [[nodiscard]] auto parse_rank(
        const board::Rank rank, string_view fenFragment, Position& position)
        -> std::expected<string_view, string>
    {
        const auto rankStart = Square { .file = board::File::A, .rank = rank }.index();
        const auto rankEnd   = rankStart + 8uz;

        auto index = rankStart;

        do {
            if (fenFragment.empty())
                return std::unexpected("Unexpected end of piece positions FEN fragment");

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
                    return std::unexpected(
                        std::format(
                            "Unexpected char in piece positions FEN fragment: {}",
                            fenFragment.front()));
            }

            ++index;
            fenFragment = fenFragment.substr(1uz);
        } while (std::cmp_less(index, rankEnd));

        if (not fenFragment.empty() and fenFragment.front() == '/')
            return fenFragment.substr(1uz);

        return fenFragment; // NOLINT
    }

} // namespace

std::expected<void, string> parse_piece_positions(
    string_view fenFragment, Position& position)
{
    for (const auto rank : std::views::reverse(enum_values<board::Rank>())) {
        const auto left = parse_rank(rank, fenFragment, position);

        if (not left.has_value())
            return std::unexpected(left.error());

        fenFragment = left.value();
    }

    position.whitePieces.refresh_occupied();
    position.blackPieces.refresh_occupied();

    return {};
}

std::expected<void, string> parse_side_to_move(
    const string_view fenFragment, Position& position)
{
    if (fenFragment.length() != 1uz) {
        return std::unexpected(
            std::format(
                "Expected single character for side to move, got: {}",
                fenFragment));
    }

    const bool isBlack = fenFragment.front() == 'b';

    position.sideToMove = isBlack ? Color::Black : Color::White;

    return {};
}

void parse_castling_rights(
    const string_view fenFragment, Position& position)
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
    const string_view fenFragment, Position& position)
{
    if (fenFragment.contains('-')) {
        position.enPassantTargetSquare = std::nullopt;
        return;
    }

    position.enPassantTargetSquare = Square::from_string(fenFragment);
}

} // namespace chess::notation::fen_helpers
