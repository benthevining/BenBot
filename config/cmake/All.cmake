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

include_guard (GLOBAL)

include ("${CMAKE_CURRENT_LIST_DIR}/Sanitizers.cmake")
include ("${CMAKE_CURRENT_LIST_DIR}/Coverage.cmake")
include ("${CMAKE_CURRENT_LIST_DIR}/Hardening.cmake")
include ("${CMAKE_CURRENT_LIST_DIR}/Warnings.cmake")

# General settings

set_property (GLOBAL PROPERTY USE_FOLDERS YES)

set_property (
    GLOBAL PROPERTY REPORT_UNDEFINED_PROPERTIES "${CMAKE_BINARY_DIR}/UndefinedProperties.log"
)

# MSVC static runtime
block ()
get_cmake_property (debug_configs DEBUG_CONFIGURATIONS)

if (NOT debug_configs)
    set (debug_configs Debug)
endif ()

list (JOIN debug_configs "," debug_configs)

set (CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:${debug_configs}>:Debug>" CACHE STRING "")
endblock ()

# Enhance error reporting and compiler messages
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options (-fcolor-diagnostics)
elseif (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    add_compile_options (-fdiagnostics-color=always)
elseif (CMAKE_CXX_COMPILER_ID MATCHES "MSVC" AND MSVC_VERSION GREATER 1900)
    add_compile_options (/diagnostics:column)
endif ()
