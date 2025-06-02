# ======================================================================================
#
# libchess - a chess engine by Ben Vining
#
# ======================================================================================

cmake_minimum_required (VERSION 3.30.0 FATAL_ERROR)

project (
    libchess
    VERSION 0.0.1
    LANGUAGES CXX C
    DESCRIPTION "A chess engine"
    HOMEPAGE_URL "https://github.com/benthevining/libchess"
)

option (LIBCHESS_DOCS "Build the documentation (requires Doxygen)" "${PROJECT_IS_TOP_LEVEL}")

option (LIBCHESS_TESTS "Build the tests" "${PROJECT_IS_TOP_LEVEL}")

add_subdirectory (libchess)
add_subdirectory (ben-bot)

if (LIBCHESS_TESTS)
    enable_testing ()

    configure_file (config/CTestCustom.cmake CTestCustom.cmake @ONLY)

    add_subdirectory (tests)

    if (PROJECT_IS_TOP_LEVEL)
        include (CTest)
    endif ()
endif ()

if (LIBCHESS_DOCS)
    add_subdirectory (docs)
endif ()
