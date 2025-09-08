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

#include "MagicsImpl.hpp" // NOLINT(build/include_subdir)
#include <beman/inplace_vector/inplace_vector.hpp>
#include <cstddef> // IWYU pragma: keep - for std::ptrdiff_t
#include <cstdlib>
#include <filesystem>
#include <format>
#include <fstream>
#include <iterator>
#include <libchess/board/Bitboard.hpp>
#include <libchess/board/BitboardIndex.hpp>
#include <libchess/board/Square.hpp>
#include <libchess/moves/PseudoLegal.hpp>
#include <print>
#include <ranges>
#include <span>
#include <string_view>
#include <vector>

constexpr std::string_view FILE_HEADER {
    R"(#pragma once

#include <array>
#include <libchess/board/Bitboard.hpp>

namespace chess::moves::magics {

    using board::Bitboard;

    static constexpr std::array MAGIC_MOVES {
)"
};

constexpr std::string_view FILE_FOOTER {
    R"(
};

} // namespace chess::moves::magics
)"
};

namespace chess::moves::magics {
using board::Bitboard;

namespace {

    // returns the next permutation of the given set
    // used to generate all permutations of possible blockers
    [[nodiscard, gnu::const]] constexpr Bitboard permute(
        const Bitboard set, const Bitboard subset) noexcept
    {
        return Bitboard { subset.to_int() - set.to_int() } & set;
    }

    [[nodiscard]] constexpr std::vector<Bitboard> generate_magic_moves()
    {
        std::vector<Bitboard> result;

        result.resize(88772uz);

        for (auto i = 0uz; i < 64uz; ++i) {
            const auto square = board::Square::from_index(static_cast<board::BitboardIndex>(i));

            // Bishops
            {
                Bitboard occupied;

                do {
                    auto& value = result.at(calc_bishop_index(i, occupied));

                    value = pseudo_legal::bishop(
                        Bitboard::from_square(square),
                        occupied.inverse(), {});

                    occupied = permute(BISHOP_MASKS.at(i), occupied);
                } while (occupied.any());
            }

            // Rooks
            {
                Bitboard occupied;

                do {
                    auto& value = result.at(calc_rook_index(i, occupied));

                    value = pseudo_legal::rook(
                        Bitboard::from_square(square),
                        occupied.inverse(), {});

                    occupied = permute(ROOK_MASKS.at(i), occupied);
                } while (occupied.any());
            }
        }

        return result;
    }

} // namespace

} // namespace chess::moves::magics

int main(const int argc, const char** argv)
{
    static constexpr auto MAX_ARGS = 2uz;

    const beman::inplace_vector<std::string_view, MAX_ARGS> argStorage {
        argv,
        std::next(argv, static_cast<std::ptrdiff_t>(argc))
    };

    std::span args { argStorage };

    args = args.subspan(1uz); // consume program name

    if (args.empty()) {
        std::println("Missing required positional argument (file output path)");
        return EXIT_FAILURE;
    }

    const std::filesystem::path outputFile { args.front() };

    std::filesystem::remove(outputFile);

    std::filesystem::create_directories(outputFile.parent_path());

    std::ofstream fileStream { outputFile };

    const auto magicMoves = chess::moves::magics::generate_magic_moves();

    fileStream << FILE_HEADER;

    for (auto i = 0uz; i < magicMoves.size(); ++i) {
        fileStream << std::format(
            "Bitboard{{ {}ULL }}",
            magicMoves.at(i).to_int());

        if (i != magicMoves.size() - 1uz)
            fileStream << ",\n";
    }

    fileStream << FILE_FOOTER;

    return EXIT_SUCCESS;
}
