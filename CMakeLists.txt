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

cmake_minimum_required (VERSION 3.30.0 FATAL_ERROR)

project (
    BenBot
    VERSION 1.4.13
    LANGUAGES CXX C
    DESCRIPTION "A chess engine"
    HOMEPAGE_URL "https://github.com/benthevining/BenBot"
)

option (BENBOT_DOCS "Build the documentation (requires Doxygen)" "${PROJECT_IS_TOP_LEVEL}")

option (BENBOT_TESTS "Build the tests" "${PROJECT_IS_TOP_LEVEL}")

set (CMAKE_CXX_STANDARD 23)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

include (FetchContent)
include (CPackComponent)

cpack_add_component_group (
    ben_bot_all DISPLAY_NAME "BenBot" DESCRIPTION "All components of BenBot" EXPANDED BOLD_TITLE
)

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
    set (CPACK_PACKAGE_VENDOR "Ben Vining")
    set (CPACK_PACKAGE_DESCRIPTION "UCI chess engine")
    set (CPACK_PACKAGE_FILE_NAME
         "ben_bot-${BenBot_VERSION}-${CMAKE_SYSTEM_NAME}-${CMAKE_CXX_COMPILER_ID}"
    )
    set (CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_LIST_DIR}/LICENSE")
    set (CPACK_STRIP_FILES TRUE)
    set (CPACK_INSTALL_CMAKE_PROJECTS "${BenBot_BINARY_DIR}" BenBot ben_bot .)

    include (CPack)
endif ()
