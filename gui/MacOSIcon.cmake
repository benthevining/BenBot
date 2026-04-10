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

find_program (
    INKSCAPE_PROGRAM inkscape
    DOC "inkscape executable, used for converting SVG to PNG for icns file generation"
)

add_feature_info (
    benbot_app_icon INKSCAPE_PROGRAM "Generating app icon (requires inkscape on MacOS)"
)

if (NOT INKSCAPE_PROGRAM)
    return ()
endif ()

set (icon_set "${CMAKE_CURRENT_BINARY_DIR}/Queen-White.iconset")

file (MAKE_DIRECTORY "${icon_set}")

unset (png_images)

foreach (size IN ITEMS 16 32 64 128 256 512)
    set (png_out "${icon_set}/icon_${size}x${size}.png")

    add_custom_command (
        OUTPUT "${png_out}"
        COMMAND "${INKSCAPE_PROGRAM}" -o "${png_out}" -w "${size}" -h "${size}"
                "${BENBOT_APP_ICON_SVG}"
        DEPENDS "${svg_input}"
        COMMENT "Generating ${size}x${size} image for MacOS iconset"
        VERBATIM USES_TERMINAL
    )

    set (png2x_out "${icon_set}/icon_${size}x${size}@2x.png")

    math (EXPR size2x "${size} * 2")

    add_custom_command (
        OUTPUT "${png2x_out}"
        COMMAND "${INKSCAPE_PROGRAM}" -o "${png2x_out}" -w "${size2x}" -h "${size2x}"
                "${BENBOT_APP_ICON_SVG}"
        DEPENDS "${svg_input}"
        COMMENT "Generating ${size}x${size}@2x image for MacOS iconset"
        VERBATIM USES_TERMINAL
    )

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
    COMMENT "Generating MacOS icns from iconset"
    VERBATIM USES_TERMINAL
)

target_sources (benbot_gui PRIVATE "${icns_output}")

set_source_files_properties ("${icns_output}" PROPERTIES MACOSX_PACKAGE_LOCATION Resources)

set_target_properties (benbot_gui PROPERTIES MACOSX_BUNDLE_ICON_FILE "icon.icns")
