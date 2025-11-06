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

option (BENBOT_ALWAYS_REBUILD_RESOURCES
        "If on, the resources target is rebuilt every time a build is invoked" OFF
)

mark_as_advanced (BENBOT_ALWAYS_REBUILD_RESOURCES)

FetchContent_Declare (
    cmrc
    SYSTEM
    GIT_REPOSITORY "https://github.com/vector-of-bool/cmrc.git"
    GIT_TAG origin/master
    OVERRIDE_FIND_PACKAGE
)

FetchContent_MakeAvailable (cmrc)

set (resource_files # cmake-format: sortable
                    res/bench.epd res/license_header.txt
)

cmrc_add_resource_library (ben_bot_resources_internal ${resource_files})

add_library (ben_bot_resources STATIC)

target_link_libraries (ben_bot_resources PRIVATE ben_bot_resources_internal)

set_target_properties (
    ben_bot_resources
    PROPERTIES CXX_STANDARD_REQUIRED ON
               CXX_EXTENSIONS OFF
               CXX_VISIBILITY_PRESET hidden
               VISIBILITY_INLINES_HIDDEN ON
               DEBUG_POSTFIX -d
               POSITION_INDEPENDENT_CODE ON
               VERIFY_INTERFACE_HEADER_SETS ON
)

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

set (header_file "include/ben-bot/Resources.hpp")
set (source_files # cmake-format: sortable
                  src/BuildTime.cpp src/Resources.cpp
)

target_sources (
    ben_bot_resources PUBLIC FILE_SET HEADERS BASE_DIRS include FILES "${header_file}"
    PRIVATE ${source_files} "${generated_header}"
)

source_group (
    TREE "${CMAKE_CURRENT_LIST_DIR}" FILES "${header_file}" ${source_files} ${resource_files}
)

target_include_directories (ben_bot_resources PRIVATE "${generated_includes}")

if (BENBOT_ALWAYS_REBUILD_RESOURCES)
    add_custom_target (
        ben_bot_resources_toucher ALL COMMAND "${CMAKE_COMMAND}" -E touch_nocreate
                                              "${CMAKE_CURRENT_LIST_DIR}/src/BuildTime.cpp"
        COMMENT "Touching BuildTime.cpp to ensure it rebuilds..." VERBATIM
    )

    add_dependencies (ben_bot_resources ben_bot_resources_toucher)
endif ()

# warning flags
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    set (not_gnu "$<NOT:$<CXX_COMPILER_ID:GNU>>")
    set (c_or_objc "$<COMPILE_LANGUAGE:C,OBJC>")

    target_compile_options (
        ben_bot_resources_internal
        PRIVATE
            # cmake-format: sortable
            -Wno-newline-eof
            -Wno-exit-time-destructors
            "$<$<OR:${c_or_objc},${not_gnu}>:-Wno-missing-variable-declarations;-Wno-missing-prototypes>"
    )

    target_compile_options (
        ben_bot_resources PRIVATE # cmake-format: sortable
                                  -Wno-date-time -Wno-unreachable-code
    )
endif ()

add_library (ben_bot::resources ALIAS ben_bot_resources)
