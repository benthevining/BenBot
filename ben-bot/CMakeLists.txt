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

add_subdirectory (resources)

add_executable (ben_bot)

target_link_libraries (ben_bot PRIVATE ben_bot::libbenbot ben_bot::resources)

file (GLOB_RECURSE benbot_headers LIST_DIRECTORIES false CONFIGURE_DEPENDS
                                                         "${CMAKE_CURRENT_LIST_DIR}/include/*.hpp"
)

file (GLOB benbot_sources LIST_DIRECTORIES false CONFIGURE_DEPENDS
                                                 "${CMAKE_CURRENT_LIST_DIR}/src/*.cpp"
)

target_sources (
    ben_bot PRIVATE FILE_SET HEADERS BASE_DIRS include FILES ${benbot_headers}
    PRIVATE ${benbot_sources}
)

source_group (TREE "${CMAKE_CURRENT_LIST_DIR}" FILES ${benbot_headers} ${benbot_sources})

# this is to differentiate artifacts from different builds
set_target_properties (
    ben_bot PROPERTIES OUTPUT_NAME
                       "ben_bot-${BenBot_VERSION}-${CMAKE_SYSTEM_NAME}-${CMAKE_CXX_COMPILER_ID}"
)

add_executable (ben_bot::ben_bot ALIAS ben_bot)

#

install (TARGETS ben_bot COMPONENT ben_bot)

cpack_add_component (
    ben_bot DISPLAY_NAME BenBot DESCRIPTION "BenBot UCI chess engine" GROUP ben_bot_all
)

#

file (GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/lichess-config/$<CONFIG>.yml" INPUT config.yml)

set (release_config_file "${CMAKE_CURRENT_BINARY_DIR}/lichess-config/Release.yml")

set (lichess_bot_dir "${PROJECT_SOURCE_DIR}/lichess-bot")

if (IS_DIRECTORY "${lichess_bot_dir}")
    file (COPY_FILE "${release_config_file}" "${lichess_bot_dir}/config.yml" ONLY_IF_DIFFERENT
          INPUT_MAY_BE_RECENT
    )
endif ()

#[[
install (
    FILES "${release_config_file}"
    TYPE SYSCONF
    CONFIGURATIONS Release
    COMPONENT ben_bot_lichess
    RENAME config.yml
    EXCLUDE_FROM_ALL
)

cpack_add_component (
    ben_bot_lichess
    DISPLAY_NAME "Lichess configuration"
    DESCRIPTION "BenBot Lichess configuration file"
    DEPENDS ben_bot
    GROUP ben_bot_all
)
]]
