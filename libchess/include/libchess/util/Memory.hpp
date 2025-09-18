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

/** @file
    This file provides some memory management utility functions.
    @ingroup util
 */

#pragma once

#include <cstddef> // IWYU pragma: keep - for size_t
#include <memory>
#include <memory_resource>
#include <stdexcept>
#include <vector>

namespace chess::util {

using std::size_t;

/** Allocates memory aligned by page size, with a minimum
    alignment of 4096 bytes. Memory allocated by this function
    must be freed by calling ``page_aligned_free()``. Returns
    ``nullptr`` if the memory could not be allocated.

    @ingroup util
    @see PageAlignedMemoryResource, PageAlignedVector
 */
[[nodiscard]] void* page_aligned_alloc(size_t size);

/** Frees page-aligned memory allocated by ``page_aligned_alloc()``.
    This is a no-op if ``mem`` is ``nullptr``.

    @ingroup util
    @see PageAlignedMemoryResource, PageAlignedVector
 */
void page_aligned_free(void* mem);

/** This RAII struct encapsulates a memory buffer allocated using ``page_aligned_alloc()``
    and handed to a ``std::pmr::monotonic_buffer_resource``, for usage with e.g.
    ``std::pmr::vector``.

    @ingroup util
    @see PageAlignedVector
 */
struct PageAlignedMemoryResource final {
    /** The memory resource type owned by this object. */
    using Resource = std::pmr::monotonic_buffer_resource;

    /** Creates the memory resource and allocates its buffer. */
    explicit PageAlignedMemoryResource(const size_t bufSizeBytes)
        : buffer { page_aligned_alloc(bufSizeBytes) }
        , resource { buffer, bufSizeBytes }
    {
    }

    ~PageAlignedMemoryResource() { page_aligned_free(buffer); }

    PageAlignedMemoryResource(const PageAlignedMemoryResource&)            = delete;
    PageAlignedMemoryResource& operator=(const PageAlignedMemoryResource&) = delete;

    PageAlignedMemoryResource(PageAlignedMemoryResource&&)            = delete;
    PageAlignedMemoryResource& operator=(PageAlignedMemoryResource&&) = delete;

    /** Returns the underlying memory resource. */
    [[nodiscard]] const Resource& get_resource() const noexcept { return resource; }

private:
    void* buffer;

    Resource resource;
};

/** This class encapsulates a ``std::vector`` that allocates its items using
    a ``PageAlignedMemoryResource``. When the vector is resized, the old one
    and its resource are simply destroyed.

    @ingroup util
    @see PageAlignedMemoryResource
 */
template <typename T>
class PageAlignedVector final {
public:
    /** Deletes all items in the vector and frees its memory. */
    void clear() { vec.reset(); }

    /** Deletes any items in the vector, frees its storage, and reserves
        a new storage buffer large for at least ``maxSize`` objects.
     */
    void resize(const size_t maxSize)
    {
        clear();

        vec = std::make_unique<InternalVec>(maxSize);
    }

    /** Returns the maximum number of elements the vector can hold. */
    [[nodiscard]] size_t capacity() const noexcept
    {
        if (vec == nullptr)
            return 0uz;

        return vec->maxSize;
    }

    /** Returns a reference to the element in the vector at the given index.

        @throws std::out_of_range An exception will be thrown if the vector
        is empty or the index is out of range.
     */
    [[nodiscard]] T& at(const size_t idx)
    {
        if (vec == nullptr)
            throw std::out_of_range { "PageAlignedVector::at() - internal vector doesn't exist!" };

        return vec->at(idx);
    }

    /** Returns a reference to the element in the vector at the given index.

        @throws std::out_of_range An exception will be thrown if the vector
        is empty or the index is out of range.
     */
    [[nodiscard]] const T& at(const size_t idx) const
    {
        if (vec == nullptr)
            throw std::out_of_range { "PageAlignedVector::at() - internal vector doesn't exist!" };

        return vec->at(idx);
    }

private:
    struct InternalVec final {
        explicit InternalVec(const size_t maxNumItems)
            : maxSize { maxNumItems }
            , resource { maxNumItems * sizeof(T) }
        {
        }

        size_t maxSize;

        PageAlignedMemoryResource resource;

        std::pmr::polymorphic_allocator<T> alloc { &resource.get_resource() };

        std::pmr::vector<T> vector { alloc };
    };

    std::unique_ptr<InternalVec> vec;
};

} // namespace chess::util
