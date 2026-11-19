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

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <format>
#include <imgui.h>
#include <libgui/PieceTextures.hpp>
#include <libgui/Resources.hpp>
#include <lunasvg.h>
#include <magic_enum/magic_enum.hpp>
#include <span>
#include <stdexcept>
#include <type_traits>

namespace ben_bot::gui {

// PieceTextures stores the texture ID as a plain integer so that
// imgui can remain a private dependency of libgui
static_assert(std::is_same_v<ImTextureID, std::uint64_t>);

namespace {
    void render_sprite(
        PieceSpriteSheet& sheet, const PieceType type, const Color color)
    {
        const auto svg = resources::get_piece_sprite(type, color);

        const auto document = lunasvg::Document::loadFromData(svg.data(), svg.size());

        if (document == nullptr)
            throw std::runtime_error {
                std::format("Error parsing SVG for {} {}",
                    magic_enum::enum_name(color), magic_enum::enum_name(type))
            };

        const auto size = static_cast<int>(sheet.pieceSize);

        auto bitmap = document->renderToBitmap(size, size);

        if (bitmap.isNull())
            throw std::runtime_error {
                std::format("Error rendering SVG for {} {}",
                    magic_enum::enum_name(color), magic_enum::enum_name(type))
            };

        // lunasvg renders premultiplied ARGB, but imgui expects straight RGBA
        bitmap.convertToRGBA();

        const auto [column, row] = get_sprite_cell(type, color);

        static constexpr auto BytesPerPixel = 4uz;

        const auto sheetStride  = sheet.width() * BytesPerPixel;
        const auto bitmapStride = static_cast<std::size_t>(bitmap.stride());
        const auto rowBytes     = sheet.pieceSize * BytesPerPixel;

        const auto originX = column * sheet.pieceSize;
        const auto originY = row * sheet.pieceSize;

        const std::span src { bitmap.data(), bitmapStride * sheet.pieceSize };
        const std::span dst { sheet.pixels };

        for (auto y = 0uz; y < sheet.pieceSize; ++y) {
            std::ranges::copy(
                src.subspan(y * bitmapStride, rowBytes),
                dst.subspan(((originY + y) * sheetStride) + (originX * BytesPerPixel)).begin());
        }
    }
} // namespace

auto rasterize_piece_sprites(const std::size_t pieceSize) -> PieceSpriteSheet
{
    PieceSpriteSheet sheet { .pieceSize = pieceSize, .pixels = { } };

    sheet.pixels.resize(sheet.width() * sheet.height() * 4uz);

    for (const auto color : magic_enum::enum_values<Color>())
        for (const auto type : magic_enum::enum_values<PieceType>())
            render_sprite(sheet, type, color);

    return sheet;
}

} // namespace ben_bot::gui
