/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

/** @defgroup pieces Piece types
    Classes for working with the various chess piece types, and associated information.
 */

/** @file
    This file defines some constants for encoding information about the various piece types.
    @ingroup pieces
 */

#pragma once

#include <cstddef> // IWYU pragma: keep - for size_t
#include <cstdint> // IWYU pragma: keep - for std::uint_fast8_t
#include <format>
#include <magic_enum/magic_enum.hpp>

/** This namespace contains classes for encoding information about the various chess piece types.
    @ingroup pieces
 */
namespace chess::pieces {

using std::size_t;

/** This enum identifies the different types of pieces.

    Black & white pawns have different enums to encode the differing movement direction;
    therefore, a Type enum on its own completely encodes the movement mechanics
    (except for castling or en passant).

    @ingroup pieces
    @see utf8 values

    @todo from_string()
 */
enum class Type : std::uint_fast8_t {
    WhitePawn, ///< A White pawn.
    BlackPawn, ///< A Black pawn.
    Knight,    ///< A knight.
    Bishop,    ///< A bishop.
    Rook,      ///< A rook.
    Queen,     ///< A queen.
    King       ///< A king.
};

/// @ingroup pieces
/// @{

/** Returns true if ``type`` is ``Knight`` or ``Bishop``.
    @see is_heavy()
 */
[[nodiscard, gnu::const]] constexpr bool is_minor(const Type type) noexcept
{
    return type == Type::Knight || type == Type::Bishop;
}

/** Returns true if ``type`` is ``Rook`` or ``Queen``.
    @see is_minor()
 */
[[nodiscard, gnu::const]] constexpr bool is_heavy(const Type type) noexcept
{
    return type == Type::Rook || type == Type::Queen;
}

/** Returns true if ``type`` is ``Bishop``, ``Rook``, or ``Queen``. */
[[nodiscard, gnu::const]] constexpr bool is_sliding(const Type type) noexcept
{
    return type == Type::Bishop || type == Type::Rook || type == Type::Queen;
}

/// @}

/** This namespace contains constants encoding the material values of the various piece types.
    The king is not assigned a material value, as it can never be legally captured in a non-checkmated position.
    @ingroup pieces
 */
namespace values {

    /// @ingroup pieces
    /// @{

    /** Returns the material value of a pawn (1). */
    [[nodiscard, gnu::const]] static consteval size_t pawn() noexcept
    {
        return 1uz;
    }

    /** Returns the material value of a knight (3). */
    [[nodiscard, gnu::const]] static consteval size_t knight() noexcept
    {
        return 3uz;
    }

    /** Returns the material value of a bishop (4). */
    [[nodiscard, gnu::const]] static consteval size_t bishop() noexcept
    {
        return 4uz;
    }

    /** Returns the material value of a rook (5). */
    [[nodiscard, gnu::const]] static consteval size_t rook() noexcept
    {
        return 5uz;
    }

    /** Returns the material value of a queen (9). */
    [[nodiscard, gnu::const]] static consteval size_t queen() noexcept
    {
        return 9uz;
    }

    /// @}

} // namespace values

} // namespace chess::pieces

namespace std {

/** A formatter specialization for chess piece types.

    The formatter accepts the following format specifier arguments:
    @li ``s|S``: Tells the formatter to print a short (single-letter) version of the piece type
    @li ``l|L``: Tells the formatter to print a long version of the piece type

    If no arguments are specified, the formatter prints the short version of the piece type by default.

    @see chess::pieces::Type
    @ingroup pieces
 */
template <>
struct formatter<chess::pieces::Type> final {
    template <typename ParseContext>
    constexpr typename ParseContext::iterator parse(ParseContext& ctx);

    template <typename FormatContext>
    typename FormatContext::iterator format(
        chess::pieces::Type piece, FormatContext& ctx) const;

private:
    bool useShort { true };
};

/*
                         ___                           ,--,
      ,---,            ,--.'|_                ,--,   ,--.'|
    ,---.'|            |  | :,'             ,--.'|   |  | :
    |   | :            :  : ' :             |  |,    :  : '    .--.--.
    |   | |   ,---.  .;__,'  /    ,--.--.   `--'_    |  ' |   /  /    '
  ,--.__| |  /     \ |  |   |    /       \  ,' ,'|   '  | |  |  :  /`./
 /   ,'   | /    /  |:__,'| :   .--.  .-. | '  | |   |  | :  |  :  ;_
.   '  /  |.    ' / |  '  : |__  \__\/: . . |  | :   '  : |__ \  \    `.
'   ; |:  |'   ;   /|  |  | '.'| ," .--.; | '  : |__ |  | '.'| `----.   \
|   | '/  ''   |  / |  ;  :    ;/  /  ,.  | |  | '.'|;  :    ;/  /`--'  /__  ___  ___
|   :    :||   :    |  |  ,   /;  :   .'   \;  :    ;|  ,   /'--'.     /  .\/  .\/  .\
 \   \  /   \   \  /    ---`-' |  ,     .-./|  ,   /  ---`-'   `--'---'\  ; \  ; \  ; |
  `----'     `----'             `--`---'     ---`-'                     `--" `--" `--"

 */

template <typename ParseContext>
constexpr typename ParseContext::iterator
formatter<chess::pieces::Type>::parse(ParseContext& ctx)
{
    auto it = ctx.begin();

    if (it == ctx.end() || *it == '}')
        return it;

    do {
        switch (*it) {
            case 's': [[fallthrough]];
            case 'S':
                useShort = true;
                break;

            case 'l': [[fallthrough]];
            case 'L':
                useShort = false;
                break;

            default:
                throw std::format_error { "Unrecognized format argument" };
        }

        ++it;
    } while (! (it == ctx.end() || *it == '}'));

    ctx.advance_to(it);

    return it;
}

template <typename FormatContext>
typename FormatContext::iterator
formatter<chess::pieces::Type>::format(
    const chess::pieces::Type piece, FormatContext& ctx) const
{
    using PieceType = chess::pieces::Type;

    if (useShort) {
        const auto character = [piece] {
            switch (piece) {
                case PieceType::Knight: return 'N';
                case PieceType::Bishop: return 'B';
                case PieceType::Rook  : return 'R';
                case PieceType::Queen : return 'Q';
                case PieceType::King  : return 'K';
                default               : return 'P';
            }
        }();

        return std::format_to(ctx.out(), "{}", character);
    }

    switch (piece) {
        case PieceType::WhitePawn: [[fallthrough]];
        case PieceType::BlackPawn:
            return std::format_to(ctx.out(), "{}", "Pawn");

        default:
            return std::format_to(ctx.out(), "{}", magic_enum::enum_name(piece));
    }
}

} // namespace std
