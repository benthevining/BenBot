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

#[[
This module creates a termcolor::termcolor interface library.
]]

set_package_properties (
    termcolor PROPERTIES URL "https://github.com/ikalnytskyi/termcolor"
    DESCRIPTION "Cross-platform color terminal output"
)

set (termcolor_FOUND TRUE)

if (TARGET termcolor::termcolor)
    return ()
endif ()

FetchContent_Declare (
    termcolor
    SYSTEM
    GIT_REPOSITORY "https://github.com/ikalnytskyi/termcolor.git"
    GIT_TAG "v${termcolor_FIND_VERSION}"
    GIT_SHALLOW ON
    SOURCE_SUBDIR foo # we need to disable adding termcolor as a subdirectory, because they list a
    # too-old cmake version
    OVERRIDE_FIND_PACKAGE
)

FetchContent_MakeAvailable (termcolor)

if (termcolor_SOURCE_DIR)
    set (termcolor_includes "${termcolor_SOURCE_DIR}/include")
elseif (termcolor_DIR)
    set (termcolor_includes "${termcolor_DIR}/include")
else ()
    message (FATAL_ERROR "termcolor include directory not found in expected variables")
endif ()

add_library (termcolor INTERFACE)

target_sources (
    termcolor INTERFACE FILE_SET HEADERS BASE_DIRS "${termcolor_includes}" FILES
                        "${termcolor_includes}/termcolor/termcolor.hpp"
)

add_library (termcolor::termcolor ALIAS termcolor)
