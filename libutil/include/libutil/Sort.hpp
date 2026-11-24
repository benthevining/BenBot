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

/** @defgroup sort Sorting
    A sort that gives the same result with every standard library.

    @ingroup util
 */

/** @file
    This file provides a sort whose order for equal elements doesn't depend on the standard library.
    @ingroup sort
 */

#pragma once

#include <cstddef>
#include <iterator>
#include <span>
#include <utility>

/** This namespace provides a portable sort.
    @ingroup sort
 */
namespace util::sort {

namespace detail {

    // elements in ranges this short are sorted by insertion sort at the end,
    // instead of being partitioned further
    inline constexpr auto INSERTION_THRESHOLD { 16z };

    template <typename T, typename Compare>
    constexpr void insertion_sort(const std::span<T> range, Compare& comp)
    {
        for (auto idx = 1uz; idx < range.size(); ++idx) {
            auto value = std::move(range[idx]);
            auto pos   = idx;

            for (; pos > 0uz and comp(value, range[pos - 1uz]); --pos)
                range[pos] = std::move(range[pos - 1uz]);

            range[pos] = std::move(value);
        }
    }

    template <typename T, typename Compare>
    constexpr void sift_down(
        const std::span<T> range, std::size_t root, const std::size_t size, Compare& comp)
    {
        while (true) {
            auto child = (root * 2uz) + 1uz;

            if (child >= size)
                return;

            if (child + 1uz < size and comp(range[child], range[child + 1uz]))
                ++child;

            if (not comp(range[root], range[child]))
                return;

            std::swap(range[root], range[child]);
            root = child;
        }
    }

    // the fallback when partitioning goes badly, to keep the worst case at O(n log n)
    template <typename T, typename Compare>
    constexpr void heap_sort(const std::span<T> range, Compare& comp)
    {
        const auto size = range.size();

        for (auto idx = size / 2uz; idx > 0uz; --idx)
            sift_down(range, idx - 1uz, size, comp);

        for (auto end = size; end > 1uz; --end) {
            std::swap(range[0uz], range[end - 1uz]);
            sift_down(range, 0uz, end - 1uz, comp);
        }
    }

    // puts the median of the first, middle & last elements at the front, as the pivot
    template <typename T, typename Compare>
    constexpr void move_median_to_front(const std::span<T> range, Compare& comp)
    {
        auto& first  = range[1uz];
        auto& middle = range[range.size() / 2uz];
        auto& last   = range.back();

        auto* median = &first;

        if (comp(first, middle)) {
            if (comp(middle, last))
                median = &middle;
            else if (comp(first, last))
                median = &last;
        } else if (comp(first, last)) {
            // first is already the median
        } else if (comp(middle, last)) {
            median = &last;
        } else {
            median = &middle;
        }

        std::swap(range.front(), *median);
    }

    // partitions the range around its first element & returns the index where
    // the second part starts. Elements equal to the pivot may go on either side.
    template <typename T, typename Compare>
    constexpr auto partition(const std::span<T> range, Compare& comp) -> std::size_t
    {
        const auto& pivot = range.front();

        auto left  = 1uz;
        auto right = range.size();

        while (true) {
            while (comp(range[left], pivot))
                ++left;

            --right;

            while (comp(pivot, range[right]))
                --right;

            if (left >= right)
                return left;

            std::swap(range[left], range[right]);
            ++left;
        }
    }

    template <typename T, typename Compare>
    constexpr void introsort(std::span<T> range, int depthLimit, Compare& comp)
    {
        while (std::ssize(range) > INSERTION_THRESHOLD) {
            if (depthLimit == 0) {
                heap_sort(range, comp);
                return;
            }

            --depthLimit;

            move_median_to_front(range, comp);

            const auto cut = partition(range, comp);

            introsort(range.subspan(cut), depthLimit, comp);

            range = range.first(cut);
        }
    }

} // namespace detail

/** Sorts the range so that ``comp(b, a)`` is false for every ``a`` before ``b``.

    Like ``std::sort``, this isn't stable. Unlike ``std::sort``, the order it leaves
    equal elements in is the same with every compiler & standard library, so results
    that depend on it (like the engine's node counts) are the same on every platform.

    @ingroup sort
 */
template <typename T, typename Compare>
constexpr void portable_sort(const std::span<T> range, Compare comp)
{
    if (range.size() < 2uz)
        return;

    auto depthLimit = 0;

    for (auto size = range.size(); size > 1uz; size /= 2uz)
        depthLimit += 2;

    detail::introsort(range, depthLimit, comp);
    detail::insertion_sort(range, comp);
}

} // namespace util::sort
