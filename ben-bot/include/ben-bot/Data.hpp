/*
 * ======================================================================================
 *
 * ░▒▓███████▓▒░░▒▓████████▓▒░▒▓███████▓▒░       ░▒▓███████▓▒░ ░▒▓██████▓▒░▒▓████████▓▒░
 * ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░
 * ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░
 * ░▒▓███████▓▒░░▒▓██████▓▒░ ░▒▓█▓▒░░▒▓█▓▒░      ░▒▓███████▓▒░░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░
 * ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░
 * ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░
 * ░▒▓███████▓▒░░▒▓████████▓▒░▒▓█▓▒░░▒▓█▓▒░      ░▒▓███████▓▒░ ░▒▓██████▓▒░  ░▒▓█▓▒░
 *
 * ======================================================================================
 */

/** @defgroup benbot_data ben-bot data
    Binary data embedded into the engine executable.
 */

#pragma once

#include <string_view>

namespace ben_bot {

using std::string_view;

/// @ingroup benbot_data
/// @{

/** Returns the engine's opening book as a PGN text. */
[[nodiscard]] string_view get_opening_book_pgn_text();

/** Returns the default set of bench positions as an EPD text. */
[[nodiscard]] string_view get_bench_epd_text();

/** Returns the engine's ASCII logo. */
[[nodiscard]] string_view get_ascii_logo();

/** Returns the engine's version string. */
[[nodiscard, gnu::const]] string_view get_version_string();

/** Returns the name of the compiler used to build the engine. */
[[nodiscard, gnu::const]] string_view get_compiler_name();

/** Returns the version of the compiler used to build the engine. */
[[nodiscard, gnu::const]] string_view get_compiler_version();

/** Returns the name of the target system that the engine was built for. */
[[nodiscard, gnu::const]] string_view get_system_name();

/** Returns the name of the build configuration that the engine was built for. */
[[nodiscard, gnu::const]] string_view get_build_config();

/// @}

} // namespace ben_bot
