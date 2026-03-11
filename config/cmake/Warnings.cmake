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

# Including this module enables some default warnings at directory scope.

include_guard (DIRECTORY)

if (MSVC)
    add_compile_options (
        /W4
        /Wall
        /WL
        /external:W0
        #
        /wd4820 # struct padding
        /wd4324 # struct padding due to alignas
        /wd4514 # unreferenced functions removed
        /wd4710 # function not inlined
        /wd4711 # function selected for inlining
        /wd5030 # unrecognized attributes
        /wd5246 # don't require braces around every subobject of std::array initializer list
        /wd5045 # don't tell us about Spectre mitigations
        /wd4868 # compiler may not enforce left-to-right evaluation in initializer lists
        /wd5264 # unused variables - this warning causes too many false positives
    )
    return ()
endif ()

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options (
        -pedantic
        -pedantic-errors
        -Wall
        -Wcast-align
        -Wconversion
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
        -Wunused
        #
        -Wno-c++98-compat
        -Wno-c++98-compat-pedantic
        -Wno-c++20-compat
        -Wno-pre-c++20-compat-pedantic
        -Wno-padded
        -Wno-poison-system-directories
        -Wno-global-constructors
        -Wno-exit-time-destructors
        -Wno-covered-switch-default
    )
endif ()

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options (
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
        -Weverything
    )

    if (WIN32)
        add_compile_options (-Wno-reserved-macro-identifier)
    endif ()

    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 21)
        add_compile_options (-Wno-nrvo)
    endif ()
elseif (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    add_compile_options (
        -Waggressive-loop-optimizations
        -Wpointer-arith
        -Wredundant-decls
        -Wwrite-strings
        -Wdelete-non-virtual-dtor
        -Wsuggest-override
        -Wtrampolines
        -Wbidi-chars=any
        #
        -Wno-attributes
    )
endif ()
