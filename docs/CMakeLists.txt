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

find_package (Doxygen 1.16.1 OPTIONAL_COMPONENTS dot)

set_package_properties (
    Doxygen PROPERTIES URL "https://www.doxygen.nl/" DESCRIPTION "Code documentation generator"
)

add_feature_info (benbot_docs DOXYGEN_FOUND "Building BenBot docs with Doxygen")

if (NOT DOXYGEN_FOUND)
    return ()
endif ()

include ("${CMAKE_CURRENT_LIST_DIR}/DoxygenSettings.cmake")

# inputs & filepaths
set (DOXYGEN_PROJECT_ICON "${PROJECT_SOURCE_DIR}/gui/libgui/resources/res/pieces/Queen-White.svg")
set (DOXYGEN_PROJECT_LOGO "${DOXYGEN_PROJECT_ICON}")
set (DOXYGEN_USE_MDFILE_AS_MAINPAGE "${BenBot_SOURCE_DIR}/README.md")
set (DOXYGEN_LAYOUT_FILE "${CMAKE_CURRENT_LIST_DIR}/DoxygenLayout.xml")
set (DOXYGEN_CITE_BIB_FILES "${CMAKE_CURRENT_SOURCE_DIR}/references.bib")

set (DOXYGEN_FILE_PATTERNS *.hpp)
set (DOXYGEN_INCLUDE_FILE_PATTERNS *.hpp)

set (DOXYGEN_STRIP_FROM_PATH "${BenBot_SOURCE_DIR}")

set (DOXYGEN_OUTPUT_DIRECTORY "${BenBot_BINARY_DIR}/generated-docs")

#

unset (header_files)

foreach (lib_name IN ITEMS libutil libchess libbenbot resources libgui gui_resources)
    set (target_name "ben_bot::${lib_name}")

    get_target_property (lib_headers "${target_name}" HEADER_SET)
    list (APPEND header_files ${lib_headers})

    get_target_property (lib_include_base_dirs "${target_name}" HEADER_DIRS)
    list (APPEND DOXYGEN_STRIP_FROM_INC_PATH ${lib_include_base_dirs})
endforeach ()

doxygen_add_docs (
    ben_bot_docs
    "${DOXYGEN_LAYOUT_FILE}"
    "${DOXYGEN_CITE_BIB_FILES}"
    "${DOXYGEN_USE_MDFILE_AS_MAINPAGE}"
    "${CMAKE_CURRENT_LIST_DIR}/uci-spec.md"
    "${CMAKE_CURRENT_LIST_DIR}/top-level-groups.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/CodeStyle.md"
    ${header_files}
    WORKING_DIRECTORY "${BenBot_SOURCE_DIR}"
    COMMENT "Building Doxygen documentation..." USE_STAMP_FILE
)

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
