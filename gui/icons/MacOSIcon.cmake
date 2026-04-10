# ======================================================================================
#
# ░▒▓███████▓▒░░▒▓████████▓▒░▒▓███████▓▒░       ░▒▓███████▓▒░ ░▒▓██████▓▒░▒▓████████▓▒░
# ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░
# ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░
# ░▒▓███████▓▒░░▒▓██████▓▒░ ░▒▓█▓▒░░▒▓█▓▒░      ░▒▓███████▓▒░░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░
# ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░
# ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░
# ░▒▓███████▓▒░░▒▓████████▓▒░▒▓█▓▒░░▒▓█▓▒░      ░▒▓███████▓▒░ ░▒▓██████▓▒░  ░▒▓█▓▒░
#
# ======================================================================================

include_guard (GLOBAL)

if (NOT APPLE)
    return ()
endif ()

include ("${CMAKE_CURRENT_LIST_DIR}/SVGtoPNG.cmake")

if (NOT INKSCAPE_PROGRAM)
    return ()
endif ()

set (icon_set "${CMAKE_CURRENT_BINARY_DIR}/Queen-White.iconset")

file (MAKE_DIRECTORY "${icon_set}")

unset (png_images)

foreach (size IN ITEMS 16 32 64 128 256 512)
    set (png_out "${icon_set}/icon_${size}x${size}.png")

    # cmake-format: off
    add_svg_to_png_command (
        INPUT  "${BENBOT_APP_ICON_SVG}"
        OUTPUT "${png_out}"
        OUT_WIDTH  "${size}"
        OUT_HEIGHT "${size}"
        COMMENT "Generating ${size}x${size} image for MacOS iconset..."
    )
    # cmake-format: on

    set (png2x_out "${icon_set}/icon_${size}x${size}@2x.png")

    math (EXPR size2x "${size} * 2")

    # cmake-format: off
    add_svg_to_png_command (
        INPUT  "${BENBOT_APP_ICON_SVG}"
        OUTPUT "${png2x_out}"
        OUT_WIDTH  "${size2x}"
        OUT_HEIGHT "${size2x}"
        COMMENT "Generating ${size}x${size}@2x image for MacOS iconset..."
    )
    # cmake-format: on

    list (APPEND png_images "${png_out}" "${png2x_out}")
endforeach ()

find_program (
    ICONUTIL_PROGRAM iconutil
    DOC "iconutil program used for converting iconset to .icns" REQUIRED # iconutil is a system tool
                                                                         # that should never be
                                                                         # missing on MacOS
)

set (icns_output "${CMAKE_CURRENT_BINARY_DIR}/icon.icns")

add_custom_command (
    OUTPUT "${icns_output}"
    COMMAND "${ICONUTIL_PROGRAM}" --convert icns --output "${icns_output}" "${icon_set}"
    DEPENDS ${png_images}
    COMMENT "Generating MacOS icns from iconset..."
    VERBATIM USES_TERMINAL
)

target_sources (benbot_gui PRIVATE "${icns_output}")

set_source_files_properties ("${icns_output}" PROPERTIES MACOSX_PACKAGE_LOCATION Resources)

set_target_properties (benbot_gui PROPERTIES MACOSX_BUNDLE_ICON_FILE "icon.icns")
