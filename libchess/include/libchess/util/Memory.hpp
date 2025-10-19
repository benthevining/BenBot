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

/** @defgroup memory Memory utilities
    Memory handling utility functions used throughout the code.

    @ingroup util
 */

/** @file
    This file provides some memory management utility functions.
    @ingroup memory
 */

#pragma once

#include <cstddef> // IWYU pragma: keep - for size_t

namespace chess::util {

using std::size_t;

/** Allocates memory aligned by page size, with a minimum
    alignment of 4096 bytes. Memory allocated by this function
    must be freed by calling ``page_aligned_free()``. Returns
    ``nullptr`` if the memory could not be allocated.

    @ingroup memory
    @see page_aligned_free()
 */
[[nodiscard, gnu::alloc_size(1), gnu::malloc, clang::ownership_returns(malloc)]]
auto page_aligned_alloc(size_t size) -> void*;

/** Frees page-aligned memory allocated by ``page_aligned_alloc()``.
    This is a no-op if ``mem`` is ``nullptr``.

    @ingroup memory
    @see page_aligned_alloc()
 */
[[clang::ownership_takes(malloc, 1)]] void page_aligned_free([[clang::noescape]] void* mem);

/** Hints the CPU to prefetch the page that the given memory address
    is on. This function is nonblocking, and may be a no-op depending
    on the target platform.

    @ingroup memory
 */
void prefetch(const void* mem);

} // namespace chess::util
