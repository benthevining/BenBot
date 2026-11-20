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

/** @file
    This file defines the piece sprite sheet used to render pieces on the board.
    @ingroup libgui
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <libchess/pieces/Colors.hpp>
#include <libchess/pieces/PieceTypes.hpp>
#include <utility>
#include <vector>

namespace ben_bot::gui {

using PieceType = chess::pieces::Type;
using chess::pieces::Color;

/** A sheet of rasterized piece sprites, stored as non-premultiplied 8-bit RGBA pixels.

    Each column holds one piece type (in the order of the ``PieceType`` enum), and each
    row holds one color (White on top). Each sprite is ``pieceSize`` pixels square.

    @ingroup libgui
 */
struct PieceSpriteSheet final {
    /** The number of sprite columns (one per piece type). */
    static constexpr auto NumColumns { 6uz };

    /** The number of sprite rows (one per color). */
    static constexpr auto NumRows { 2uz };

    /** The width & height of each sprite, in pixels. */
    std::size_t pieceSize { 0uz };

    /** The RGBA pixel data, with ``width() * 4`` bytes per row. */
    std::vector<std::uint8_t> pixels;

    /** Returns the width of the whole sheet, in pixels. */
    [[nodiscard]] constexpr auto width() const noexcept -> std::size_t { return pieceSize * NumColumns; }

    /** Returns the height of the whole sheet, in pixels. */
    [[nodiscard]] constexpr auto height() const noexcept -> std::size_t { return pieceSize * NumRows; }
};

/** The size (in pixels) that the main application should use for each
    sprite when rasterizing the sprite sheet. This is larger than the squares
    will usually be drawn, so the texture should be created with mipmaps.

    @ingroup libgui
 */
inline constexpr auto DefaultPieceSpriteSize { 256uz };

/** Rasterizes the embedded piece SVGs into a sprite sheet.
    Throws an exception if any of the SVGs cannot be parsed or rendered.

    @ingroup libgui
    @relates PieceSpriteSheet
 */
[[nodiscard]] auto rasterize_piece_sprites(std::size_t pieceSize) -> PieceSpriteSheet;

/** The normalized texture coordinates of a single sprite within the sprite sheet.
    @ingroup libgui
 */
struct SpriteUVs final {
    float u0 { 0.f }; ///< Left edge.
    float v0 { 0.f }; ///< Top edge.
    float u1 { 0.f }; ///< Right edge.
    float v1 { 0.f }; ///< Bottom edge.
};

/** Returns the column & row of the given piece's sprite within the sprite sheet.
    @ingroup libgui
    @relates PieceSpriteSheet
 */
[[nodiscard]] constexpr auto get_sprite_cell(
    const PieceType type, const Color color) noexcept -> std::pair<std::size_t, std::size_t>
{
    return {
        static_cast<std::size_t>(std::to_underlying(type)),
        color == Color::White ? 0uz : 1uz
    };
}

/** Returns the texture coordinates of the given piece's sprite within the sprite sheet.
    @ingroup libgui
    @relates PieceSpriteSheet
 */
[[nodiscard]] constexpr auto get_sprite_uvs(
    const PieceType type, const Color color) noexcept -> SpriteUVs
{
    const auto [column, row] = get_sprite_cell(type, color);

    static constexpr auto colWidth  = 1.f / static_cast<float>(PieceSpriteSheet::NumColumns);
    static constexpr auto rowHeight = 1.f / static_cast<float>(PieceSpriteSheet::NumRows);

    return {
        .u0 = static_cast<float>(column) * colWidth,
        .v0 = static_cast<float>(row) * rowHeight,
        .u1 = static_cast<float>(column + 1uz) * colWidth,
        .v1 = static_cast<float>(row + 1uz) * rowHeight
    };
}

/** Holds the backend texture created from the piece sprite sheet.

    libgui doesn't know which rendering backend is in use, so the main application
    is responsible for uploading the sprite sheet to the GPU and storing the
    resulting texture handle here (and destroying the texture at shutdown).

    @ingroup libgui
 */
struct PieceTextures final {
    /** The sprite sheet texture, as an ``ImTextureID``.
        This is 0 if the texture hasn't been created.
     */
    std::uint64_t textureID { 0 };

    /** Returns true if the texture has been created. */
    [[nodiscard]] constexpr auto is_valid() const noexcept -> bool { return textureID != 0; }
};

} // namespace ben_bot::gui
