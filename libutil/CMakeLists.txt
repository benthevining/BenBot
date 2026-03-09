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

add_library (libutil STATIC)

target_compile_features (libutil PUBLIC cxx_std_23)

# target_link_libraries (libutil PUBLIC magic_enum::magic_enum beman::inplace_vector)

set_target_properties (
    libutil PROPERTIES CXX_VISIBILITY_PRESET hidden VISIBILITY_INLINES_HIDDEN ON
                       VERIFY_INTERFACE_HEADER_SETS ON
)

file (GLOB_RECURSE libutil_headers LIST_DIRECTORIES false
      CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/include/libutil/*.hpp"
)

file (GLOB_RECURSE libutil_sources LIST_DIRECTORIES false
      CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/src/*.[c,h]pp" # include private headers too
)

target_sources (
    libutil PUBLIC FILE_SET HEADERS BASE_DIRS include FILES ${libutil_headers}
    PRIVATE ${libutil_sources}
)

target_include_directories (libutil PRIVATE "${CMAKE_CURRENT_LIST_DIR}/src/")

set (memory_src_dir "${CMAKE_CURRENT_LIST_DIR}/src/util/memory")

set_property (
    SOURCE "${memory_src_dir}/PageAlignedAlloc.cpp" APPEND PROPERTY INCLUDE_DIRECTORIES
                                                                    "${memory_src_dir}"
)

source_group (TREE "${CMAKE_CURRENT_LIST_DIR}" FILES ${libutil_headers} ${libutil_sources})

add_library (ben_bot::libutil ALIAS libutil)
