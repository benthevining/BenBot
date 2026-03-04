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

cmake_minimum_required (VERSION 4.0.0 FATAL_ERROR)

project (
    BenBot
    VERSION 1.7.2
    LANGUAGES CXX C
    DESCRIPTION "A chess engine"
    HOMEPAGE_URL "https://github.com/benthevining/BenBot"
)

option (BENBOT_DOCS "Build the documentation (requires Doxygen)" "${PROJECT_IS_TOP_LEVEL}")

option (BENBOT_TESTS "Build the tests" "${PROJECT_IS_TOP_LEVEL}")

# includes used throughout the various directories
include (CPackComponent)
include (FeatureSummary)
include (FetchContent)

cpack_add_component_group (
    ben_bot_all DISPLAY_NAME "BenBot" DESCRIPTION "All components of BenBot" EXPANDED BOLD_TITLE
)

FetchContent_Declare (
    termcolor
    SYSTEM
    GIT_REPOSITORY "https://github.com/ikalnytskyi/termcolor.git"
    GIT_TAG v2.1.0
    GIT_SHALLOW ON
    SOURCE_SUBDIR foo # we need to disable adding termcolor as a subdirectory, because they list a
                      # too-old cmake version
    FIND_PACKAGE_ARGS 2.1.0
)

FetchContent_MakeAvailable (termcolor)

if (termcolor_SOURCE_DIR)
    set (TERMCOLOR_INCLUDES "${termcolor_SOURCE_DIR}/include")
elseif (termcolor_DIR)
    set (TERMCOLOR_INCLUDES "${termcolor_DIR}/include")
endif ()

add_subdirectory (libchess)
add_subdirectory (libbenbot)
add_subdirectory (ben-bot)

if (BENBOT_TESTS)
    enable_testing ()

    add_subdirectory (tests)

    if (PROJECT_IS_TOP_LEVEL)
        include (CTest)
    endif ()
endif ()

if (BENBOT_DOCS)
    add_subdirectory (docs)
endif ()

if (PROJECT_IS_TOP_LEVEL)
    include (CPack)
    feature_summary (WHAT ALL DEFAULT_DESCRIPTION)
endif ()
