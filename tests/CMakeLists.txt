# ======================================================================================
#
# libchess - a chess engine by Ben Vining
#
# ======================================================================================

include (FetchContent)

FetchContent_Declare (
    Catch2
    SYSTEM
    GIT_REPOSITORY "https://github.com/catchorg/Catch2.git"
    GIT_TAG v3.8.0
    GIT_SHALLOW ON
    GIT_PROGRESS ON
    FIND_PACKAGE_ARGS 3.8.0
)

FetchContent_MakeAvailable (Catch2)

if (catch2_SOURCE_DIR)
    list (APPEND CMAKE_MODULE_PATH "${catch2_SOURCE_DIR}/extras")
elseif (Catch2_DIR)
    list (APPEND CMAKE_MODULE_PATH "${Catch2_DIR}")
endif ()

include (Catch)

#

add_executable (libchess_tests)

target_link_libraries (libchess_tests PRIVATE libchess::libchess Catch2::Catch2WithMain)

set (sources # cmake-format: sortable
             geometry/Bitboard.cpp geometry/Square.cpp
)

target_sources (libchess_tests PRIVATE ${sources})

source_group (TREE "${CMAKE_CURRENT_LIST_DIR}" FILES ${sources})

# cmake-format: off
catch_discover_tests (
    libchess_tests
    EXTRA_ARGS --verbosity high
               --warn NoAssertions
               --warn UnmatchedTestSpec
               --colour-mode none
               --reporter console::out=-
    TEST_PREFIX libchess.unit.
    PROPERTIES
        SKIP_REGULAR_EXPRESSION "SKIPPED:"
)
# cmake-format: on
