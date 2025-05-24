# ======================================================================================
#
# libchess - a chess engine by Ben Vining
#
# ======================================================================================

find_package (Doxygen OPTIONAL_COMPONENTS dot)

if (NOT DOXYGEN_FOUND)
    return ()
endif ()

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
set (DOXYGEN_WARN_LOGFILE "${PROJECT_SOURCE_DIR}/logs/Doxygen.log")
set (DOXYGEN_FILE_PATTERNS *.hpp)
set (DOXYGEN_INCLUDE_FILE_PATTERNS *.hpp)

# general options
set (DOXYGEN_PREDEFINED DOXYGEN=1)
set (DOXYGEN_EXTRACT_ALL YES)
set (DOXYGEN_MULTILINE_CPP_IS_BRIEF YES)
set (DOXYGEN_ALWAYS_DETAILED_SEC YES)
set (DOXYGEN_DISTRIBUTE_GROUP_DOC YES)
set (DOXYGEN_SOURCE_BROWSER YES)
set (DOXYGEN_GENERATE_TREEVIEW YES)
set (DOXYGEN_BUILTIN_STL_SUPPORT YES)
set (DOXYGEN_GROUP_NESTED_COMPOUNDS YES)
set (DOXYGEN_DISABLE_INDEX NO)
set (DOXYGEN_EXTERNAL_PAGES NO)
set (DOXYGEN_EXTRACT_PRIV_VIRTUAL YES)
set (DOXYGEN_GENERATE_XML YES)
set (DOXYGEN_HIDE_FRIEND_COMPOUNDS YES)
set (DOXYGEN_INLINE_INHERITED_MEMB YES)
set (DOXYGEN_NUM_PROC_THREADS 0) # value 0 means Doxygen will use all available cores
set (DOXYGEN_SHOW_HEADERFILE NO)
set (DOXYGEN_SORT_GROUP_NAMES YES)
set (DOXYGEN_SORT_MEMBERS_CTORS_1ST YES)
set (DOXYGEN_TEMPLATE_RELATIONS YES)
set (DOXYGEN_TIMESTAMP DATE)

# dot
set (DOXYGEN_DOT_IMAGE_FORMAT svg)
set (DOXYGEN_DOT_TRANSPARENT YES)
set (DOXYGEN_DOT_MULTI_TARGETS YES)
set (DOXYGEN_INTERACTIVE_SVG YES)
set (DOXYGEN_DOT_GRAPH_MAX_NODES 75)

# HTML
set (DOXYGEN_EXT_LINKS_IN_WINDOW YES)
set (DOXYGEN_HTML_FORMULA_FORMAT svg)
set (DOXYGEN_USE_MATHJAX YES)
set (DOXYGEN_HTML_DYNAMIC_SECTIONS YES)

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

set (html_dir "${DOXYGEN_OUTPUT_DIRECTORY}/html")

add_custom_target (
    libchess_open_docs COMMAND "${open_cmd}" "${html_dir}/index.html"
    COMMENT "Opening generated HTML documentation..."
)

add_dependencies (libchess_open_docs libchess_docs)
