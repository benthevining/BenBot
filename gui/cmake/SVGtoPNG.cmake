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

#[[
This module provides the add_svg_to_png_command function, which uses inkscape.
Before using add_svg_to_png_command() you should check ``if(INKSCAPE_PROGRAM)``.
]]

include_guard (GLOBAL)

find_program (
    INKSCAPE_PROGRAM inkscape
    DOC "inkscape executable, used for converting SVG to PNG for app icon generation"
)

add_feature_info (benbot_app_icon INKSCAPE_PROGRAM "Generating app icon (requires inkscape)")

#[[
    add_svg_to_png_command (
        INPUT <svg>
        OUTPUT <png>
        OUT_WIDTH <w>
        OUT_HEIGHT <w>
        COMMENT <comment>
    )
]]
function (add_svg_to_png_command)
    if (NOT INKSCAPE_PROGRAM)
        message (
            FATAL_ERROR
                "${CMAKE_CURRENT_FUNCTION} cannot be used because INKSCAPE_PROGRAM was not found!"
        )
    endif ()

    set (args # cmake-format: sortable
              COMMENT INPUT OUT_HEIGHT OUT_WIDTH OUTPUT
    )

    cmake_parse_arguments (ARG "" "${args}" "" ${ARGN})

    add_custom_command (
        OUTPUT "${ARG_OUTPUT}"
        COMMAND "${INKSCAPE_PROGRAM}" -o "${ARG_OUTPUT}" -w "${ARG_OUT_WIDTH}" -h
                "${ARG_OUT_HEIGHT}" "${ARG_INPUT}"
        DEPENDS "${ARG_INPUT}"
        COMMENT "${ARG_COMMENT}"
        VERBATIM USES_TERMINAL
    )
endfunction ()
