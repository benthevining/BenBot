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
    cmrc SYSTEM GIT_REPOSITORY "https://github.com/vector-of-bool/cmrc.git" GIT_TAG origin/master
)

FetchContent_MakeAvailable (cmrc)

# cmake-format: off
cmrc_add_resource_library (
        ben_bot_resources_internal
        res/license_header.txt
        res/bench.epd
)
# cmake-format: on

add_library (ben_bot_resources STATIC)

target_link_libraries (ben_bot_resources PRIVATE ben_bot_resources_internal)

# generate the config header that provides info such as compiler name, build config, etc.

set (config_header "${CMAKE_CURRENT_LIST_DIR}/src/BenBotConfig.hpp")

file (READ "${config_header}" config_header_content)
list (APPEND CMAKE_CONFIGURE_DEPENDS "${config_header}")

set (generated_includes "${CMAKE_CURRENT_BINARY_DIR}/generated/include/$<CONFIG>")
set (generated_header "${generated_includes}/BenBotConfig.hpp")

# we need to do string(CONFIGURE) and then file(GENERATE) because we want to expand both variable
# references and generator expressions
string (CONFIGURE "${config_header_content}" config_header_content @ONLY)
file (GENERATE OUTPUT "${generated_header}" CONTENT "${config_header_content}")

target_sources (
    ben_bot_resources PUBLIC FILE_SET HEADERS BASE_DIRS include FILES include/ben-bot/Resources.hpp
    PRIVATE src/Resources.cpp src/BuildTime.cpp "${generated_header}"
)

target_include_directories (ben_bot_resources PRIVATE "${generated_includes}")

add_library (ben_bot::resources ALIAS ben_bot_resources)
