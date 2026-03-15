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

# Including this module globally enables coverage flags.

include_guard (GLOBAL)

include (FeatureSummary)

add_feature_info (
    coverage [[CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang" AND NOT WIN32]]
    "Coverage reporting flags for debug configurations"
)

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang" AND NOT WIN32)
    get_cmake_property (debug_configs DEBUG_CONFIGURATIONS)

    if (NOT debug_configs)
        set (debug_configs Debug)
    endif ()

    if (APPLE)
        # On MacOS, UBSAN seems to interfere with coverage collection, it erroneously reports 0-4%,
        # so just disable it
        list (REMOVE_ITEM debug_configs UBSAN)
    endif ()

    list (JOIN debug_configs "," debug_configs)

    set (config_debug "$<CONFIG:${debug_configs}>")

    add_compile_options ("$<${config_debug}:-g;-O0;--coverage>")
    add_link_options ("$<${config_debug}:--coverage>")

    if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        link_libraries ("$<${config_debug}:gcov>")
    endif ()

    set (clean_script "${CMAKE_SOURCE_DIR}/scripts/CleanOldCoverageOutput.cmake")

    add_custom_target (
        coverage-clean
        COMMAND "${CMAKE_COMMAND}" -D "BUILD_DIR=${CMAKE_BINARY_DIR}" -P "${clean_script}"
        COMMENT "Cleaning old coverage output files..."
        VERBATIM USES_TERMINAL
        SOURCES "${clean_script}"
    )
endif ()
