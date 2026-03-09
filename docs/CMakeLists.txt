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

add_feature_info (benbot_docs DOXYGEN_FOUND "Building BenBot docs with Doxygen")

if (NOT DOXYGEN_FOUND)
    return ()
endif ()

include ("${CMAKE_CURRENT_LIST_DIR}/DoxygenSettings.cmake")

# inputs & filepaths
set (DOXYGEN_OUTPUT_DIRECTORY "${BenBot_BINARY_DIR}/generated-docs")
set (DOXYGEN_USE_MDFILE_AS_MAINPAGE "${BenBot_SOURCE_DIR}/README.md")
set (DOXYGEN_LAYOUT_FILE "${CMAKE_CURRENT_LIST_DIR}/DoxygenLayout.xml")
set (DOXYGEN_FILE_PATTERNS *.hpp)
set (DOXYGEN_INCLUDE_FILE_PATTERNS *.hpp)
set (DOXYGEN_STRIP_FROM_PATH "${BenBot_SOURCE_DIR}")
set (DOXYGEN_STRIP_FROM_INC_PATH
     "${BenBot_SOURCE_DIR}/libchess/include" "${BenBot_SOURCE_DIR}/libbenbot/include"
     "${BenBot_SOURCE_DIR}/ben-bot/include" "${BenBot_SOURCE_DIR}/ben-bot/resources/include"
)

# generate the engine docs page

set (engine_doc_page "${CMAKE_CURRENT_BINARY_DIR}/engine_docs.txt")

# all files must exist at configure time when using doxygen_add_docs()'s stamp file option
file (TOUCH "${engine_doc_page}")

set (generator_script "${CMAKE_CURRENT_LIST_DIR}/GenerateEngineDocs.cmake")

add_custom_command (
    OUTPUT "${engine_doc_page}"
    COMMAND "${CMAKE_COMMAND}" -D "OUTPUT_FILE=${engine_doc_page}" -D
            "ENGINE_PATH=$<TARGET_FILE:ben_bot::ben_bot>" -P "${generator_script}"
    DEPENDS ben_bot::ben_bot "${generator_script}" "${CMAKE_CURRENT_LIST_DIR}/engine-docs.txt"
    COMMENT "Generating engine docs page..."
    VERBATIM USES_TERMINAL
)

#

get_target_property (libchess_headers ben_bot::libchess HEADER_SET)
get_target_property (libbenbot_headers ben_bot::libbenbot HEADER_SET)
get_target_property (resource_headers ben_bot::resources HEADER_SET)

doxygen_add_docs (
    ben_bot_docs
    "${DOXYGEN_LAYOUT_FILE}"
    "${DOXYGEN_USE_MDFILE_AS_MAINPAGE}"
    "${CMAKE_CURRENT_LIST_DIR}/uci-spec.txt"
    "${CMAKE_CURRENT_LIST_DIR}/top-level-groups.txt"
    "${CMAKE_CURRENT_LIST_DIR}/CodeStyle.md"
    "${engine_doc_page}"
    ${libchess_headers}
    ${libbenbot_headers}
    ${resource_headers}
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
