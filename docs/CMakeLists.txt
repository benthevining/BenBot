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
set (DOXYGEN_USE_MDFILE_AS_MAINPAGE "${BenBot_SOURCE_DIR}/README.md")
set (DOXYGEN_LAYOUT_FILE "${CMAKE_CURRENT_LIST_DIR}/DoxygenLayout.xml")
set (DOXYGEN_CITE_BIB_FILES "${CMAKE_CURRENT_SOURCE_DIR}/references.bib")

set (DOXYGEN_FILE_PATTERNS *.hpp)
set (DOXYGEN_INCLUDE_FILE_PATTERNS *.hpp)

set (DOXYGEN_STRIP_FROM_PATH "${BenBot_SOURCE_DIR}")
set (DOXYGEN_STRIP_FROM_INC_PATH
     "${BenBot_SOURCE_DIR}/libchess/include" "${BenBot_SOURCE_DIR}/libbenbot/include"
     "${BenBot_SOURCE_DIR}/ben-bot/include" "${BenBot_SOURCE_DIR}/ben-bot/resources/include"
)

set (DOXYGEN_OUTPUT_DIRECTORY "${BenBot_BINARY_DIR}/generated-docs")

#

get_target_property (libutil_headers ben_bot::libutil HEADER_SET)
get_target_property (libchess_headers ben_bot::libchess HEADER_SET)
get_target_property (libbenbot_headers ben_bot::libbenbot HEADER_SET)
get_target_property (resource_headers ben_bot::resources HEADER_SET)
get_target_property (gui_resource_headers ben_bot::gui_resources HEADER_SET)

doxygen_add_docs (
    ben_bot_docs
    "${DOXYGEN_LAYOUT_FILE}"
    "${DOXYGEN_CITE_BIB_FILES}"
    "${DOXYGEN_USE_MDFILE_AS_MAINPAGE}"
    "${CMAKE_CURRENT_LIST_DIR}/uci-spec.md"
    "${CMAKE_CURRENT_LIST_DIR}/top-level-groups.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/CodeStyle.md"
    ${libutil_headers}
    ${libchess_headers}
    ${libbenbot_headers}
    ${resource_headers}
    ${gui_resource_headers}
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
