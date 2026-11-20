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

#include <expected>
#include <libchess/game/Position.hpp>
#include <libchess/moves/Move.hpp>
#include <libchess/notation/Algebraic.hpp>
#include <libchess/notation/ICCF.hpp>
#include <libchess/notation/MoveFormats.hpp>
#include <libchess/notation/UCI.hpp>
#include <string>
#include <string_view>

namespace chess::notation {

auto format_move(
    const MoveFormat format,
    const Position&  position,
    const Move       move,
    const bool       algPieceTypeAsUTF8) -> std::string
{
    switch (format) {
        case MoveFormat::Algebraic:
            return to_alg(position, move, algPieceTypeAsUTF8);

        case MoveFormat::ICCF:
            return to_iccf(move);

        default: [[fallthrough]];
        case MoveFormat::UCI:
            return to_uci(move);
    }
}

auto parse_move(
    const MoveFormat       format,
    const Position&        position,
    const std::string_view string)
    -> std::expected<Move, std::string>
{
    switch (format) {
        case MoveFormat::Algebraic:
            return from_alg(position, string);

        case MoveFormat::ICCF:
            return from_iccf(position, string);

        default: [[fallthrough]];
        case MoveFormat::UCI:
            return from_uci(position, string);
    }
}

} // namespace chess::notation
