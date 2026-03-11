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

#[=======================================================================[.rst:
Coverage.cmake
----------------------

Including this module enables support for code coverage reports.

This module sets ``CTEST_COVERAGE_COMMAND`` to ``gcov``'s location, if it can be found.

Variables
^^^^^^^^^^

.. variable:: GCOV_PROGRAM

  Path to the ``gcov`` executable.

.. variable:: GENINFO_PROGRAM

  Path to the ``geninfo`` executable.

.. variable:: GENHTML_PROGRAM

  Path to the ``genhtml`` executable.

Targets
^^^^^^^^^

Each target will only exist if all required tools could be found.

.. target:: coverage-clean

  Executes a script to remove all previously generated coverage files from the build tree.
  This can be useful if you get test output containing errors mentioning failure to merge
  previous coverage output, etc.

.. target:: coverage-report

  Builds a coverage report from coverage files found in the build tree. Requires that gcov,
  geninfo, and genhtml could all be found.

.. target:: open-coverage

  First builds ``coverage-report``, then opens the generated HTML in your default browser.

#]=======================================================================]

include_guard (GLOBAL)

include (FeatureSummary)

# set up compiler flags to generate coverage output

get_cmake_property (debug_configs DEBUG_CONFIGURATIONS)

if (NOT debug_configs)
    set (debug_configs Debug)
endif ()

list (JOIN debug_configs "," debug_configs)

set (config_debug "$<CONFIG:${debug_configs}>")

if (MSVC)
    add_compile_options ("$<${config_debug}:/fsanitize-coverage=edge>")
    return ()
endif ()

add_compile_options ("$<${config_debug}:-g;--coverage>")
add_link_options ("$<${config_debug}:--coverage>")

include (CheckCXXCompilerFlag)

check_cxx_compiler_flag (-fprofile-abs-path HAVE_cxx_fprofile_abs_path)

if (HAVE_cxx_fprofile_abs_path)
    add_compile_options ("$<${config_debug}:-fprofile-abs-path>")
endif ()

check_cxx_compiler_flag (-fprofile-update=atomic HAVE_cxx_fprofile_update_atomic)

if (HAVE_cxx_fprofile_update_atomic)
    add_compile_options ("$<${config_debug}:-fprofile-update=atomic>")
endif ()

check_cxx_compiler_flag (-fprofile-arcs HAVE_cxx_fprofile_arcs)

if (HAVE_cxx_fprofile_arcs)
    add_compile_options ("$<${config_debug}:-fprofile-arcs>")
endif ()

check_cxx_compiler_flag (-ftest-coverage HAVE_cxx_ftest_coverage)

if (HAVE_cxx_ftest_coverage)
    add_compile_options ("$<${config_debug}:-ftest-coverage>")
endif ()

if (CMAKE_CXX_COMPILER MATCHES "GNU")
    link_libraries (gcov)
endif ()

# add custom target to clean old coverage output

set (COVERAGE_OUTPUT_DIR "${CMAKE_BINARY_DIR}/coverage/$<CONFIG>")
set (COVERAGE_INFO_FILE "${COVERAGE_OUTPUT_DIR}/coverage.info")

add_custom_target (
    coverage-clean
    COMMAND
        "${CMAKE_COMMAND}" -D "COVERAGE_OUTPUT_DIR=${COVERAGE_OUTPUT_DIR}" -D
        "CMAKE_BINARY_DIR=${CMAKE_BINARY_DIR}" -D "CMAKE_SOURCE_DIR=${CMAKE_SOURCE_DIR}" -P
        "${CMAKE_CURRENT_LIST_DIR}/detail/DeleteOldCoverageOutput.cmake"
    COMMENT "[coverage] - Cleaning old coverage output files..."
    VERBATIM
)

# set up coverage reports

find_program (GCOV_PROGRAM gcov DOC "gcov executable")
find_program (GENINFO_PROGRAM geninfo DOC "geninfo executable")
find_program (GENHTML_PROGRAM genhtml DOC "genhtml executable")

mark_as_advanced (GCOV_PROGRAM GENINFO_PROGRAM GENHTML_PROGRAM)

add_feature_info (
    CoverageReports "GENINFO_PROGRAM AND GENHTML_PROGRAM"
    "Custom targets to generate/open coverage report (requires geninfo & genhtml)"
)

if (NOT (GENINFO_PROGRAM AND GENHTML_PROGRAM))
    return ()
endif ()

include (FetchContent)

add_custom_command (
    OUTPUT "${COVERAGE_INFO_FILE}"
    COMMAND
        "${GENINFO_PROGRAM}" "${CMAKE_SOURCE_DIR}" --exclude "${FETCHCONTENT_BASE_DIR}" --gcov-tool
        "${GCOV_PROGRAM}" --branch-coverage --no-external --follow --forget-test-names
        --demangle-cpp --keep-going --rc derive_function_end_line=0 -o "${COVERAGE_INFO_FILE}"
        --ignore-errors empty,inconsistent,format,unsupported,category,range,source,unused
    COMMENT "[coverage] - Running geninfo on .gcda coverage output files..."
    VERBATIM
)

set (index_html "${COVERAGE_OUTPUT_DIR}/index.html")

add_custom_command (
    OUTPUT "${index_html}"
    COMMAND
        "${GENHTML_PROGRAM}" "${COVERAGE_INFO_FILE}" --header-title
        "${CMAKE_PROJECT_NAME} coverage report" --prefix "${CMAKE_SOURCE_DIR}" --precision 1
        --filter "brace,blank,range,function" --show-zero-columns --suppress-aliases
        --simplified-colors --elide-path-mismatch --sort --dark-mode --synthesize-missing
        --show-navigation --legend --function-coverage --branch-coverage --demangle-cpp
        --forget-test-names --keep-going -o "${COVERAGE_OUTPUT_DIR}" --ignore-errors
        empty,inconsistent,format,unsupported,category,range,source
    COMMAND "${CMAKE_COMMAND}" -E echo
            "Coverage report generated. Open ${index_html} in your browser."
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    DEPENDS "${COVERAGE_INFO_FILE}"
    COMMENT "[coverage] - Running genhtml to create coverage report..."
    VERBATIM
)

add_custom_target (coverage-report DEPENDS "${index_html}")

set_property (DIRECTORY APPEND PROPERTY ADDITIONAL_CLEAN_FILES "${COVERAGE_OUTPUT_DIR}")

if (WIN32)
    set (open_cmd start)
else ()
    set (open_cmd open)
endif ()

add_custom_target (open-coverage COMMAND "${open_cmd}" "${index_html}")

add_dependencies (open-coverage coverage-report)

set_target_properties (coverage-report coverage-clean open-coverage PROPERTIES FOLDER coverage)
