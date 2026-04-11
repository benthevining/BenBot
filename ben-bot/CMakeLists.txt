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

add_executable (ben_bot main.cpp)

target_compile_features (ben_bot PRIVATE cxx_std_23)

target_link_libraries (ben_bot PRIVATE ben_bot::libbenbot)

add_executable (ben_bot::ben_bot ALIAS ben_bot)

#

include (CheckIPOSupported)
include (CMakeDependentOption)

check_ipo_supported (RESULT ipo_supported LANGUAGES CXX)

cmake_dependent_option (BENBOT_IPO "Enable IPO/LTO" TRUE ipo_supported FALSE)

if (BENBOT_IPO)
    set_target_properties (ben_bot PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
endif ()

add_feature_info (benbot_ipo BENBOT_IPO "Interprocedural optimization for BenBot")

#

find_program (CODESIGN_PROGRAM codesign DOC "Path to the codesign executable")

set (
    BENBOT_CODESIGN_ID
    "$ENV{BENBOT_CODESIGN_ID}"
    CACHE
        STRING
        "Apple developer ID string used for signing BenBot, for example 'Developer ID Application: Your Name (2DO8NL92GO)'"
)

add_feature_info (
    benbot_codesigning "CODESIGN_PROGRAM AND BENBOT_CODESIGN_ID" "Signing of BenBot binaries"
)

if (CODESIGN_PROGRAM AND BENBOT_CODESIGN_ID)
    add_custom_command (
        TARGET ben_bot
        POST_BUILD
        COMMAND "${CODESIGN_PROGRAM}" --force --deep --strict --options=runtime --timestamp -s
                "${BENBOT_CODESIGN_ID}" -v "$<TARGET_FILE:ben_bot>"
        COMMENT "[codesign] - Signing ben_bot..."
        VERBATIM COMMAND_EXPAND_LISTS USES_TERMINAL
    )
endif ()

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

#

add_custom_target (
    bench
    COMMAND ben_bot::ben_bot bench
    WORKING_DIRECTORY "$<TARGET_FILE_DIR:ben_bot>"
    COMMENT "Running ben_bot bench..."
    VERBATIM USES_TERMINAL
)

add_dependencies (bench ben_bot::ben_bot)
