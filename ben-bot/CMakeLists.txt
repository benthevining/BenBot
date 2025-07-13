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

file (GLOB_RECURSE ben_bot_sources LIST_DIRECTORIES false
      CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/src/*.hpp" "${CMAKE_CURRENT_LIST_DIR}/src/*.cpp"
)

set (generated_includes "${CMAKE_CURRENT_BINARY_DIR}/generated/include/$<CONFIG>")
set (generated_header "${generated_includes}/BenBotConfig.hpp")

# we need to do string(CONFIGURE) and then file(GENERATE) because we want to expand both variable
# references and generator expressions

file (READ BenBotConfig.hpp config_header_content)

string (CONFIGURE "${config_header_content}" config_header_content @ONLY)

file (GENERATE OUTPUT "${generated_header}" CONTENT "${config_header_content}")

target_sources (ben_bot PRIVATE ${ben_bot_sources} "${generated_header}")

source_group (TREE "${CMAKE_CURRENT_LIST_DIR}" FILES ${ben_bot_sources})

target_include_directories (ben_bot PRIVATE "${CMAKE_CURRENT_LIST_DIR}/src" "${generated_includes}")

#

set (lichess_bot_dir "${PROJECT_SOURCE_DIR}/lichess-bot")

if (IS_DIRECTORY "${lichess_bot_dir}")
    configure_file (config.yml "${lichess_bot_dir}/config.yml" @ONLY)
endif ()
