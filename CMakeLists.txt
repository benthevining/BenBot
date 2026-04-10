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
    VERSION 1.7.5
    LANGUAGES CXX C
    DESCRIPTION "A chess engine"
    HOMEPAGE_URL "https://github.com/benthevining/BenBot"
)

option (BENBOT_GUI "Build the imgui app" "${PROJECT_IS_TOP_LEVEL}")

# includes used throughout the various directories
include (CPackComponent)
include (FeatureSummary)
include (FetchContent)

cpack_add_component_group (
    ben_bot_all DISPLAY_NAME "BenBot" DESCRIPTION "All components of BenBot" EXPANDED BOLD_TITLE
)

add_subdirectory (libutil)
add_subdirectory (libchess)
add_subdirectory (libbenbot)
add_subdirectory (ben-bot)

if (BENBOT_GUI)
    add_subdirectory (gui EXCLUDE_FROM_ALL)
endif ()

if (PROJECT_IS_TOP_LEVEL)
    include (CTest)
endif ()

if (BUILD_TESTING)
    enable_testing ()

    add_subdirectory (tests)
endif ()

add_subdirectory (docs EXCLUDE_FROM_ALL)

if (PROJECT_IS_TOP_LEVEL)
    include (CPack)
    feature_summary (WHAT ALL DEFAULT_DESCRIPTION)
endif ()
