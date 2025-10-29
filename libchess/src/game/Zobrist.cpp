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

#include "game/Zobrist.hpp"
#include "game/ZobristKeys.hpp"
#include <cstdint>
#include <functional>
#include <libchess/board/BitboardIndex.hpp>
#include <libchess/board/File.hpp>
#include <libchess/board/Masks.hpp>
#include <libchess/board/Pieces.hpp>
#include <libchess/board/Square.hpp>
#include <libchess/game/Position.hpp>
#include <libchess/moves/Move.hpp>
#include <libchess/pieces/Colors.hpp>
#include <magic_enum/magic_enum.hpp>
#include <numeric>
#include <optional>
#include <utility>

namespace chess::game::zobrist {

using board::File;
using pieces::Color;

namespace {

    using Hash = Position::Hash;

    [[nodiscard]] constexpr auto en_passant_key(const File file) -> Hash
    {
        return keys::EN_PASSANT_KEYS.at(std::to_underlying(file));
    }

    [[nodiscard]] constexpr auto piece_key(
        const PieceType type, const Color side, const Square& square)
        -> Hash
    {
        static constexpr auto squares = static_cast<size_t>(board::NUM_SQUARES);

        const auto typeOffset = squares * std::to_underlying(type) * 2uz;
        const auto sideOffset = squares * std::to_underlying(side);

        const auto index = typeOffset + sideOffset + square.index();

        return keys::PIECE_KEYS.at(index);
    }

    template <Color Side>
    [[nodiscard, gnu::const]] constexpr auto add_piece_positions(
        const Hash prevValue, const PieceType type, const Position& position)
        -> Hash
    {
        const auto squares = position.pieces_for<Side>().get_type(type).squares();

        // map squares to piece keys, then fold left applying ^= to prevValue
        return std::transform_reduce(
            squares.begin(), squares.end(),
            prevValue,
            std::bit_xor<> {},
            [type](const Square square) {
                return piece_key(type, Side, square);
            });
    }

    [[nodiscard, gnu::const]] constexpr auto apply_en_passant(
        const Hash prevValue, const std::optional<Square> epSquare)
        -> Hash
    {
        return epSquare
            .transform([prevValue](const Square square) {
                return prevValue ^ en_passant_key(square.file);
            })
            .value_or(prevValue);
    }

} // namespace

auto CastlingRightsChanges::update_hash(Hash value) const noexcept -> Hash
{
    if (whiteKingside)
        value ^= keys::WHITE_KINGSIDE_CASTLE;

    if (whiteQueenside)
        value ^= keys::WHITE_QUEENSIDE_CASTLE;

    if (blackKingside)
        value ^= keys::BLACK_KINGSIDE_CASTLE;

    if (blackQueenside)
        value ^= keys::BLACK_QUEENSIDE_CASTLE;

    return value;
}

auto calculate(const Position& pos) -> Hash
{
    Hash value { UINT64_C(0) };

    if (pos.is_black_to_move())
        value ^= keys::BLACK_TO_MOVE;

    for (const auto type : magic_enum::enum_values<PieceType>()) {
        value = add_piece_positions<Color::White>(value, type, pos);
        value = add_piece_positions<Color::Black>(value, type, pos);
    }

    if (pos.whiteCastlingRights.kingside)
        value ^= keys::WHITE_KINGSIDE_CASTLE;

    if (pos.whiteCastlingRights.queenside)
        value ^= keys::WHITE_QUEENSIDE_CASTLE;

    if (pos.blackCastlingRights.kingside)
        value ^= keys::BLACK_KINGSIDE_CASTLE;

    if (pos.blackCastlingRights.queenside)
        value ^= keys::BLACK_QUEENSIDE_CASTLE;

    value = apply_en_passant(value, pos.enPassantTargetSquare);

    return value;
}

auto update(
    const Position& pos, const Move& move,
    const std::optional<Square>  newEPTarget,
    const CastlingRightsChanges& rightsChanges)
    -> Hash
{
    auto value = pos.hash;

    value ^= keys::BLACK_TO_MOVE; // just toggle these bits in/out every other move

    // remove old EP target
    value = apply_en_passant(value, pos.enPassantTargetSquare);

    // add new EP target
    value = apply_en_passant(value, newEPTarget);

    // remove moved-from square
    value ^= piece_key(move.piece(), pos.sideToMove, move.from());

    // add moved-to square
    value ^= piece_key(
        move.is_promotion() ? move.promoted_type().value() : move.piece(),
        pos.sideToMove, move.to());

    // remove captured piece
    if (pos.is_capture(move)) {
        const auto otherColor = pos.is_white_to_move() ? Color::Black : Color::White;

        if (pos.is_en_passant(move)) {
            value ^= piece_key(
                PieceType::Pawn, otherColor,
                get_en_passant_captured_square(
                    pos.enPassantTargetSquare.value(),
                    pos.is_white_to_move()));
        } else {
            const auto capturedType = pos.their_pieces().get_piece_on(move.to());

            value ^= piece_key(
                capturedType.value(), otherColor, move.to());
        }
    } else if (move.is_castling()) {
        if (move.to().is_kingside())
            value ^= board::masks::kingside_castle_rook_pos_mask(pos.sideToMove).to_int();
        else
            value ^= board::masks::queenside_castle_rook_pos_mask(pos.sideToMove).to_int();
    }

    value = rightsChanges.update_hash(value);

    return value;
}

auto after_null_move(const Position& pos) -> Hash
{
    auto value = pos.hash;

    value ^= keys::BLACK_TO_MOVE; // just toggle these bits in/out every other move

    // remove old EP target
    value = apply_en_passant(value, pos.enPassantTargetSquare);

    return value;
}

} // namespace chess::game::zobrist
