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

find_package (Doxygen OPTIONAL_COMPONENTS dot)

if (NOT DOXYGEN_FOUND)
    message (VERBOSE "Doxygen not found, not building BenBot docs")
    return ()
endif ()

include ("${CMAKE_CURRENT_LIST_DIR}/DoxygenSettings.cmake")

if (TARGET Doxygen::dot)
    set (DOXYGEN_ENABLED_SECTIONS DOT_GRAPHS)
else ()
    unset (DOXYGEN_ENABLED_SECTIONS)
endif ()

# inputs & filepaths
set (DOXYGEN_OUTPUT_DIRECTORY "${BenBot_BINARY_DIR}/generated-docs")
set (DOXYGEN_GENERATE_TAGFILE "${DOXYGEN_OUTPUT_DIRECTORY}/BenBot.tag")
set (DOXYGEN_STRIP_FROM_PATH "${BenBot_SOURCE_DIR}")
set (DOXYGEN_USE_MDFILE_AS_MAINPAGE "${BenBot_SOURCE_DIR}/README.md")
set (DOXYGEN_LAYOUT_FILE "${CMAKE_CURRENT_LIST_DIR}/DoxygenLayout.xml")
set (DOXYGEN_FILE_PATTERNS *.hpp)
set (DOXYGEN_INCLUDE_FILE_PATTERNS *.hpp)

get_target_property (libchess_headers ben_bot::libchess HEADER_SET)
get_target_property (libbenbot_headers ben_bot::libbenbot HEADER_SET)

# cmake-format: off
doxygen_add_docs (
        ben_bot_docs
        "${DOXYGEN_USE_MDFILE_AS_MAINPAGE}" "${DOXYGEN_LAYOUT_FILE}"
        "${CMAKE_CURRENT_LIST_DIR}/uci-spec.txt"
        "${CMAKE_CURRENT_LIST_DIR}/CodeStyle.md"
        ${libchess_headers} ${libbenbot_headers}
        WORKING_DIRECTORY "${BenBot_SOURCE_DIR}"
        COMMENT "Building Doxygen documentation..."
        USE_STAMP_FILE
)
# cmake-format: on

set (html_output_dir "${DOXYGEN_OUTPUT_DIRECTORY}/html")

install (DIRECTORY "${html_output_dir}" TYPE DOC OPTIONAL EXCLUDE_FROM_ALL COMPONENT ben_bot_docs)

cpack_add_component (
    ben_bot_docs DISPLAY_NAME "BenBot docs" DESCRIPTION "BenBot Doxygen documentation"
    GROUP ben_bot_all
)

#

if (WIN32)
    set (open_cmd start)
else ()
    set (open_cmd open)
endif ()

add_custom_target (
    ben_bot_open_docs COMMAND "${open_cmd}" "${html_output_dir}/index.html"
    COMMENT "Opening generated HTML documentation..."
)

add_dependencies (ben_bot_open_docs ben_bot_docs)
