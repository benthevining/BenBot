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

# Including this module adds build configurations for all available sanitizer types.

include_guard (GLOBAL)

set (config_types_desc "Semicolon separated list of supported build configuration types")

set (CMAKE_CONFIGURATION_TYPES "Debug;Release;MinSizeRel;RelWithDebInfo"
     CACHE STRING "${config_types_desc}"
)

set (CMAKE_BUILD_TYPE Debug CACHE STRING "Default build configuration")

set_property (CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS ${CMAKE_CONFIGURATION_TYPES})

set_property (GLOBAL PROPERTY DEBUG_CONFIGURATIONS Debug)

#

function (__add_build_config name flags linkerFlags)
    set (CMAKE_CXX_FLAGS_${name} "${flags}"
         CACHE STRING "Flags used by the C++ compiler during ${name} builds"
    )

    set (CMAKE_C_FLAGS_${name} "${flags}"
         CACHE STRING "Flags used by the C compiler during ${name} builds"
    )

    set (CMAKE_EXE_LINKER_FLAGS_${name} "${linkerFlags}"
         CACHE STRING "Flags used for linking binaries during ${name} builds"
    )

    set (CMAKE_SHARED_LINKER_FLAGS_${name} "${linkerFlags}"
         CACHE STRING "Flags used for linking shared libraries during ${name} builds"
    )

    set (CMAKE_MODULE_LINKER_FLAGS_${name} "${linkerFlags}"
         CACHE STRING "Flags used for linking module libraries during ${name} builds"
    )

    set_property (CACHE CMAKE_BUILD_TYPE APPEND PROPERTY STRINGS "${name}")
    set_property (GLOBAL APPEND PROPERTY DEBUG_CONFIGURATIONS "${name}")

    if (NOT "${name}" IN_LIST CMAKE_CONFIGURATION_TYPES)
        set (CMAKE_CONFIGURATION_TYPES "${CMAKE_CONFIGURATION_TYPES};${name}"
             CACHE STRING "${config_types_desc}" FORCE
        )
    endif ()

    mark_as_advanced (
        CMAKE_CXX_FLAGS_${name} CMAKE_C_FLAGS_${name} CMAKE_EXE_LINKER_FLAGS_${name}
        CMAKE_SHARED_LINKER_FLAGS_${name} CMAKE_MODULE_LINKER_FLAGS_${name}
    )
endfunction ()

# ASAN

if (MSVC)
    set (asan_flags "/fsanitize=address /fsanitize-address-use-after-return /DEBUG /Zi")
    set (asan_link_flags "/DEBUG")
else ()
    set (asan_flags
         "-g -fno-omit-frame-pointer -fsanitize=address -fsanitize-address-use-after-scope"
    )
    set (asan_link_flags "${asan_flags}")
endif ()

__add_build_config (ASAN "${asan_flags}" "${asan_link_flags}")

# UBSAN

if (NOT MSVC)
    __add_build_config (UBSAN "-g -fsanitize=undefined" "-fsanitize=undefined")
endif ()

# TSAN

if (APPLE)
    __add_build_config (TSAN "-g -fsanitize=thread" "-fsanitize=thread")
endif ()
