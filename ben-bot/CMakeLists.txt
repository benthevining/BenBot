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

target_compile_features (ben_bot PRIVATE cxx_std_23)

target_link_libraries (ben_bot PRIVATE ben_bot::libbenbot ben_bot::resources)

set_target_properties (
    ben_bot PROPERTIES CXX_STANDARD_REQUIRED ON CXX_EXTENSIONS OFF DEBUG_POSTFIX -d
)

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

add_executable (ben_bot::ben_bot ALIAS ben_bot)

add_custom_target (
    bench COMMAND ben_bot::ben_bot bench --no-loop COMMENT "Running ben_bot bench..." VERBATIM
                                                                                      USES_TERMINAL
)

add_dependencies (bench ben_bot::ben_bot)

#

# cmake-format: off
install (TARGETS ben_bot COMPONENT ben_bot
         # this list can be a bit fussy - the goal is to simply
         # ignore anything that cmake can't auto-resolve for us
         RUNTIME_DEPENDENCIES
            PRE_EXCLUDE_REGEXES api-ms- ext-ms- wpaxholder wtdccm
            POST_EXCLUDE_REGEXES .dll # a last resort, I couldn't find
                                      # any other way to stop CMake from
                                      # complaining about AzureAttestManager.dll
                                      # and AzureAttestNormal.dll (pre-exclude
                                      # regexes didn't work for some reason)
         FRAMEWORK DESTINATION Library/Frameworks
)
# cmake-format: on

cpack_add_component (
    ben_bot DISPLAY_NAME BenBot DESCRIPTION "BenBot UCI chess engine" GROUP ben_bot_all
)
