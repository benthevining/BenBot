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

#[=======================================================================[.rst:
Warnings.cmake
----------------------

Including this module globally enables some default warnings.

#]=======================================================================]

include_guard (GLOBAL)

if (MSVC)
    add_compile_options (
        # cmake-format: sortable
        /W4
        /Wall
        /WL
        /external:W0
        /wd4820
        /wd5045
        /wd4514
        /wd5030
        /wd4625
        /wd4626
        /wd5027
        /wd4061
        /wd4464
        /wd4710
        /wd4711
        /wd5026
        /wd4623
        /wd5246
        /wd4868
    )
    return ()
endif ()

add_compile_options (
    # cmake-format: sortable
    -pedantic
    -pedantic-errors
    -Wall
    -Wcast-align
    -Wconversion
    -Werror=implicit
    -Werror=incompatible-pointer-types
    -Werror=int-conversion
    -Werror=format-security
    -Wextra
    -Wformat
    -Wformat=2
    -Woverloaded-virtual
    -Wimplicit-fallthrough
    -Wpedantic
    -Wreorder
    -Wshadow
    -Wsign-compare
    -Wsign-conversion
    -Wstrict-aliasing
    -Wuninitialized
    -Wunreachable-code
    -Wunused-parameter
    -Wnon-virtual-dtor
    -Wzero-as-null-pointer-constant
)

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    add_compile_options (
        # cmake-format: sortable
        --extra-warnings
        -fcolor-diagnostics
        -Wbool-conversion
        -Wconditional-uninitialized
        -Wconstant-conversion
        -Wextra-semi
        -Wint-conversion
        -Wnullable-to-nonnull-conversion
        -Wshadow-all
        -Wshift-sign-overflow
        -Wshorten-64-to-32
        -Wunused-variable
        -Wno-missing-designated-field-initializers
    )
elseif ("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
    add_compile_options (
        # cmake-format: sortable
        -Wabi
        -Wno-strict-overflow
        -Wpointer-arith
        -Wredundant-decls
        -Wno-undef
        -Wwrite-strings
        -Wdelete-non-virtual-dtor
        -Wsuggest-override
        -Wtrampolines
        -Wbidi-chars=any
    )
endif ()
