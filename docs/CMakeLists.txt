# ======================================================================================
#
# libchess - a chess engine by Ben Vining
#
# ======================================================================================

find_package (Doxygen OPTIONAL_COMPONENTS dot)

if (NOT DOXYGEN_FOUND)
    return ()
endif ()

include ("${CMAKE_CURRENT_LIST_DIR}/DoxygenSettings.cmake")

if (TARGET Doxygen::dot)
    set (DOXYGEN_ENABLED_SECTIONS DOT_GRAPHS)
else ()
    unset (DOXYGEN_ENABLED_SECTIONS)
endif ()

# inputs & filepaths
set (DOXYGEN_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/generated-docs")
set (DOXYGEN_GENERATE_TAGFILE "${DOXYGEN_OUTPUT_DIRECTORY}/${PROJECT_NAME}.tag")
set (DOXYGEN_STRIP_FROM_PATH "${PROJECT_SOURCE_DIR}")
set (DOXYGEN_USE_MDFILE_AS_MAINPAGE "${PROJECT_SOURCE_DIR}/README.md")
set (DOXYGEN_FILE_PATTERNS *.hpp)
set (DOXYGEN_INCLUDE_FILE_PATTERNS *.hpp)

get_target_property (libchess_headers libchess::libchess HEADER_SET)

# cmake-format: off
doxygen_add_docs (
        libchess_docs
        "${DOXYGEN_USE_MDFILE_AS_MAINPAGE}" ${libchess_headers}
        WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
        COMMENT "Building Doxygen documentation..."
        USE_STAMP_FILE
)
# cmake-format: on

#

if (WIN32)
    set (open_cmd start)
else ()
    set (open_cmd open)
endif ()

add_custom_target (
    libchess_open_docs COMMAND "${open_cmd}" "${DOXYGEN_OUTPUT_DIRECTORY}/html/index.html"
    COMMENT "Opening generated HTML documentation..."
)

add_dependencies (libchess_open_docs libchess_docs)
