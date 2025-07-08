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

FetchContent_Declare (
    cmrc
    SYSTEM
    GIT_REPOSITORY "https://github.com/vector-of-bool/cmrc.git"
    GIT_TAG origin/master
    GIT_PROGRESS ON
)

FetchContent_MakeAvailable (cmrc)

cmrc_add_resource_library (ben_bot_resources ALIAS ben_bot::resources book.json)

add_executable (ben_bot)

target_link_libraries (ben_bot PRIVATE libbenbot::libbenbot ben_bot::resources)

file (GLOB_RECURSE ben_bot_sources LIST_DIRECTORIES false
      CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/src/*.hpp" "${CMAKE_CURRENT_LIST_DIR}/src/*.cpp"
)

target_sources (ben_bot PRIVATE ${ben_bot_sources})

target_include_directories (ben_bot PRIVATE "${CMAKE_CURRENT_LIST_DIR}/src")

source_group (TREE "${CMAKE_CURRENT_LIST_DIR}" FILES ${ben_bot_sources})

#

set (lichess_bot_dir "${PROJECT_SOURCE_DIR}/lichess-bot")

if (IS_DIRECTORY "${lichess_bot_dir}")
    configure_file (config.yml "${lichess_bot_dir}/config.yml" @ONLY)
endif ()
