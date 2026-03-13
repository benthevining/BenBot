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

if (MSVC)
    add_compile_options (/sdl /DYNAMICBASE /guard:cf)
    add_link_options (/NXCOMPAT /CETCOMPAT)

    return ()
endif ()

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    get_cmake_property (debug_configs DEBUG_CONFIGURATIONS)

    if (NOT debug_configs)
        set (debug_configs Debug)
    endif ()

    list (JOIN debug_configs "," debug_configs)

    set (config_debug "$<CONFIG:${debug_configs}>")
    set (config_release "$<NOT:${config_debug}>")

    add_compile_options ("$<${config_release}:-U_FORTIFY_SOURCE>")

    add_compile_definitions (_GLIBCXX_ASSERTIONS "$<${config_release}:_FORTIFY_SOURCE=3>")

    include (CheckCXXCompilerFlag)

    check_cxx_compiler_flag (-fstack-protector-strong HAVE_fstack_protector_strong)
    if (HAVE_fstack_protector_strong)
        add_compile_options (-fstack-protector-strong)
    endif ()

    check_cxx_compiler_flag (-fcf-protection HAVE_fcf_protection)
    if (HAVE_fcf_protection)
        add_compile_options (-fcf-protection)
    endif ()

    if (LINUX OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        check_cxx_compiler_flag (-fstack-clash-protection HAVE_fstack_clash_protection)
        if (HAVE_fstack_clash_protection)
            add_compile_options (-fstack-clash-protection)
        endif ()
    endif ()
endif ()
