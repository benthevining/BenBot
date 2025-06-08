/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

#include <array>
#include <cstddef> // IWYU pragma: keep - for size_t
#include <cstdint> // IWYU pragma: keep - for std::uint64_t
#include <libchess/board/BitboardIndex.hpp>
#include <libchess/board/Masks.hpp>
#include <libchess/moves/Magics.hpp>
#include <libchess/moves/Patterns.hpp>
#include <libchess/moves/PseudoLegal.hpp>
#include <utility>

namespace chess::moves::magics {

namespace {

    using board::BitboardIndex;
    using std::size_t;
    using MagicInfo = std::pair<std::uint64_t, int>;

    constexpr std::array bishop_stuff {
        MagicInfo { 0x007fbfbfbfbfbfffULL, 5378 }, MagicInfo { 0x0000a060401007fcULL, 4093 },
        MagicInfo { 0x0001004008020000ULL, 4314 }, MagicInfo { 0x0000806004000000ULL, 6587 },
        MagicInfo { 0x0000100400000000ULL, 6491 }, MagicInfo { 0x000021c100b20000ULL, 6330 },
        MagicInfo { 0x0000040041008000ULL, 5609 }, MagicInfo { 0x00000fb0203fff80ULL, 22236 },
        MagicInfo { 0x0000040100401004ULL, 6106 }, MagicInfo { 0x0000020080200802ULL, 5625 },
        MagicInfo { 0x0000004010202000ULL, 16785 }, MagicInfo { 0x0000008060040000ULL, 16817 },
        MagicInfo { 0x0000004402000000ULL, 6842 }, MagicInfo { 0x0000000801008000ULL, 7003 },
        MagicInfo { 0x000007efe0bfff80ULL, 4197 }, MagicInfo { 0x0000000820820020ULL, 7356 },
        MagicInfo { 0x0000400080808080ULL, 4602 }, MagicInfo { 0x00021f0100400808ULL, 4538 },
        MagicInfo { 0x00018000c06f3fffULL, 29531 }, MagicInfo { 0x0000258200801000ULL, 45393 },
        MagicInfo { 0x0000240080840000ULL, 12420 }, MagicInfo { 0x000018000c03fff8ULL, 15763 },
        MagicInfo { 0x00000a5840208020ULL, 5050 }, MagicInfo { 0x0000020008208020ULL, 4346 },
        MagicInfo { 0x0000804000810100ULL, 6074 }, MagicInfo { 0x0001011900802008ULL, 7866 },
        MagicInfo { 0x0000804000810100ULL, 32139 }, MagicInfo { 0x000100403c0403ffULL, 57673 },
        MagicInfo { 0x00078402a8802000ULL, 55365 }, MagicInfo { 0x0000101000804400ULL, 15818 },
        MagicInfo { 0x0000080800104100ULL, 5562 }, MagicInfo { 0x00004004c0082008ULL, 6390 },
        MagicInfo { 0x0001010120008020ULL, 7930 }, MagicInfo { 0x000080809a004010ULL, 13329 },
        MagicInfo { 0x0007fefe08810010ULL, 7170 }, MagicInfo { 0x0003ff0f833fc080ULL, 27267 },
        MagicInfo { 0x007fe08019003042ULL, 53787 }, MagicInfo { 0x003fffefea003000ULL, 5097 },
        MagicInfo { 0x0000101010002080ULL, 6643 }, MagicInfo { 0x0000802005080804ULL, 6138 },
        MagicInfo { 0x0000808080a80040ULL, 7418 }, MagicInfo { 0x0000104100200040ULL, 7898 },
        MagicInfo { 0x0003ffdf7f833fc0ULL, 42012 }, MagicInfo { 0x0000008840450020ULL, 57350 },
        MagicInfo { 0x00007ffc80180030ULL, 22813 }, MagicInfo { 0x007fffdd80140028ULL, 56693 },
        MagicInfo { 0x00020080200a0004ULL, 5818 }, MagicInfo { 0x0000101010100020ULL, 7098 },
        MagicInfo { 0x0007ffdfc1805000ULL, 4451 }, MagicInfo { 0x0003ffefe0c02200ULL, 4709 },
        MagicInfo { 0x0000000820806000ULL, 4794 }, MagicInfo { 0x0000000008403000ULL, 13364 },
        MagicInfo { 0x0000000100202000ULL, 4570 }, MagicInfo { 0x0000004040802000ULL, 4282 },
        MagicInfo { 0x0004010040100400ULL, 14964 }, MagicInfo { 0x00006020601803f4ULL, 4026 },
        MagicInfo { 0x0003ffdfdfc28048ULL, 4826 }, MagicInfo { 0x0000000820820020ULL, 7354 },
        MagicInfo { 0x0000000008208060ULL, 4848 }, MagicInfo { 0x0000000000808020ULL, 15946 },
        MagicInfo { 0x0000000001002020ULL, 14932 }, MagicInfo { 0x0000000401002008ULL, 16588 },
        MagicInfo { 0x0000004040404040ULL, 6905 }, MagicInfo { 0x007fff9fdf7ff813ULL, 16076 }
    };

    constexpr std::array rook_stuff {
        MagicInfo { 0x00280077ffebfffeULL, 26304 }, MagicInfo { 0x2004010201097fffULL, 35520 },
        MagicInfo { 0x0010020010053fffULL, 38592 }, MagicInfo { 0x0040040008004002ULL, 8026 },
        MagicInfo { 0x7fd00441ffffd003ULL, 22196 }, MagicInfo { 0x4020008887dffffeULL, 80870 },
        MagicInfo { 0x004000888847ffffULL, 76747 }, MagicInfo { 0x006800fbff75fffdULL, 30400 },
        MagicInfo { 0x000028010113ffffULL, 11115 }, MagicInfo { 0x0020040201fcffffULL, 18205 },
        MagicInfo { 0x007fe80042ffffe8ULL, 53577 }, MagicInfo { 0x00001800217fffe8ULL, 62724 },
        MagicInfo { 0x00001800073fffe8ULL, 34282 }, MagicInfo { 0x00001800e05fffe8ULL, 29196 },
        MagicInfo { 0x00001800602fffe8ULL, 23806 }, MagicInfo { 0x000030002fffffa0ULL, 49481 },
        MagicInfo { 0x00300018010bffffULL, 2410 }, MagicInfo { 0x0003000c0085fffbULL, 36498 },
        MagicInfo { 0x0004000802010008ULL, 24478 }, MagicInfo { 0x0004002020020004ULL, 10074 },
        MagicInfo { 0x0001002002002001ULL, 79315 }, MagicInfo { 0x0001001000801040ULL, 51779 },
        MagicInfo { 0x0000004040008001ULL, 13586 }, MagicInfo { 0x0000006800cdfff4ULL, 19323 },
        MagicInfo { 0x0040200010080010ULL, 70612 }, MagicInfo { 0x0000080010040010ULL, 83652 },
        MagicInfo { 0x0004010008020008ULL, 63110 }, MagicInfo { 0x0000040020200200ULL, 34496 },
        MagicInfo { 0x0002008010100100ULL, 84966 }, MagicInfo { 0x0000008020010020ULL, 54341 },
        MagicInfo { 0x0000008020200040ULL, 60421 }, MagicInfo { 0x0000820020004020ULL, 86402 },
        MagicInfo { 0x00fffd1800300030ULL, 50245 }, MagicInfo { 0x007fff7fbfd40020ULL, 76622 },
        MagicInfo { 0x003fffbd00180018ULL, 84676 }, MagicInfo { 0x001fffde80180018ULL, 78757 },
        MagicInfo { 0x000fffe0bfe80018ULL, 37346 }, MagicInfo { 0x0001000080202001ULL, 370 },
        MagicInfo { 0x0003fffbff980180ULL, 42182 }, MagicInfo { 0x0001fffdff9000e0ULL, 45385 },
        MagicInfo { 0x00fffefeebffd800ULL, 61659 }, MagicInfo { 0x007ffff7ffc01400ULL, 12790 },
        MagicInfo { 0x003fffbfe4ffe800ULL, 16762 }, MagicInfo { 0x001ffff01fc03000ULL, 0 },
        MagicInfo { 0x000fffe7f8bfe800ULL, 38380 }, MagicInfo { 0x0007ffdfdf3ff808ULL, 11098 },
        MagicInfo { 0x0003fff85fffa804ULL, 21803 }, MagicInfo { 0x0001fffd75ffa802ULL, 39189 },
        MagicInfo { 0x00ffffd7ffebffd8ULL, 58628 }, MagicInfo { 0x007fff75ff7fbfd8ULL, 44116 },
        MagicInfo { 0x003fff863fbf7fd8ULL, 78357 }, MagicInfo { 0x001fffbfdfd7ffd8ULL, 44481 },
        MagicInfo { 0x000ffff810280028ULL, 64134 }, MagicInfo { 0x0007ffd7f7feffd8ULL, 41759 },
        MagicInfo { 0x0003fffc0c480048ULL, 1394 }, MagicInfo { 0x0001ffffafd7ffd8ULL, 40910 },
        MagicInfo { 0x00ffffe4ffdfa3baULL, 66516 }, MagicInfo { 0x007fffef7ff3d3daULL, 3897 },
        MagicInfo { 0x003fffbfdfeff7faULL, 3930 }, MagicInfo { 0x001fffeff7fbfc22ULL, 72934 },
        MagicInfo { 0x0000020408001001ULL, 72662 }, MagicInfo { 0x0007fffeffff77fdULL, 56325 },
        MagicInfo { 0x0003ffffbf7dfeecULL, 66501 }, MagicInfo { 0x0001ffff9dffa333ULL, 14826 }
    };

    namespace masks = board::masks;

    constexpr auto NOT_PERIMETER = masks::PERIMETER.inverse();

    [[nodiscard]] constexpr std::array<Bitboard, 64uz> calculate_bishop_masks()
    {
        std::array<Bitboard, 64uz> result {};

        for (const auto square : masks::ALL.subboards())
            result.at(square.first()) = patterns::bishop(square) & NOT_PERIMETER;

        return result;
    }

    [[nodiscard]] constexpr std::array<Bitboard, 64uz> calculate_rook_masks()
    {
        std::array<Bitboard, 64uz> result {};

        for (auto i = 0; i < 64; ++i) {
            const auto square = Square::from_index(i);

            const int file = std::to_underlying(square.file);
            const int rank = std::to_underlying(square.rank);

            // Right
            for (auto r = rank + 1; r <= 6; ++r) {
                const auto nsq = Square::from_index(file + (r * 8));
                result[i] |= Bitboard::from_square(nsq);
            }

            // Left
            for (auto r = rank - 1; r >= 1; --r) {
                const auto nsq = Square::from_index(file + (r * 8));
                result[i] |= Bitboard::from_square(nsq);
            }

            // Up
            for (auto r = file + 1; r <= 6; ++r) {
                const auto nsq = Square::from_index(r + (rank * 8));
                result[i] |= Bitboard::from_square(nsq);
            }

            // Down
            for (auto r = file - 1; r >= 1; --r) {
                const auto nsq = Square::from_index(r + (rank * 8));
                result[i] |= Bitboard::from_square(nsq);
            }
        }

        return result;
    }

    constexpr auto bishop_masks = calculate_bishop_masks();
    constexpr auto rook_masks   = calculate_rook_masks();

    [[nodiscard]] constexpr Bitboard permute(
        const Bitboard set, const Bitboard subset)
    {
        return Bitboard { subset.to_int() - set.to_int() } & set;
    }

    [[nodiscard]] size_t calc_bishop_index(
        const BitboardIndex squareIdx, const Bitboard occupied)
    {
        const auto& info = bishop_stuff.at(squareIdx);
        const auto  mask = bishop_masks.at(squareIdx);

        return info.second + (((occupied & mask).to_int() * info.first) >> 55);
    }

    [[nodiscard]] size_t calc_rook_index(
        const BitboardIndex squareIdx, const Bitboard occupied)
    {
        const auto& info = rook_stuff.at(squareIdx);
        const auto  mask = rook_masks.at(squareIdx);

        return info.second + (((occupied & mask).to_int() * info.first) >> 52);
    }

    using MagicMoves = std::array<Bitboard, 88772uz>;

    [[nodiscard]] constexpr MagicMoves generate_magic_moves()
    {
        MagicMoves result {};

        for (auto i = 0; i < 64; ++i) {
            Bitboard perm;

            const auto square = Square::from_index(i);

            // Bishops
            perm.clear();
            do {
                auto& value = result.at(
                    calc_bishop_index(i, perm));

                value = pseudo_legal::bishop(
                    Bitboard::from_square(square),
                    perm.inverse(), {});

                perm = permute(bishop_masks.at(i), perm);
            } while (perm.any());

            // Rooks
            perm.clear();
            do {
                auto& value = result.at(
                    calc_rook_index(i, perm));

                value = pseudo_legal::rook(
                    Bitboard::from_square(square),
                    perm.inverse(), {});

                perm = permute(rook_masks.at(i), perm);
            } while (perm.any());
        }

        return result;
    }

    // NB. the MagicMoves isn't constexpr because we hit Clang's constexpr step limit
    [[nodiscard]] const MagicMoves& get_magic_moves()
    {
        static const auto moves = generate_magic_moves();

        return moves;
    }

    [[nodiscard]] Bitboard bishop_moves(const Square& bishopPos, const Bitboard occupied)
    {
        static const auto& magic_moves = get_magic_moves();

        return magic_moves.at(
            calc_bishop_index(bishopPos.index(), occupied));
    }

    [[nodiscard]] Bitboard rook_moves(const Square& rookPos, const Bitboard occupied)
    {
        static const auto& magic_moves = get_magic_moves();

        return magic_moves.at(
            calc_rook_index(rookPos.index(), occupied));
    }

} // namespace

Bitboard bishop(
    const Square& bishopPos, const Bitboard occupiedSquares, const Bitboard friendlyPieces)
{
    return bishop_moves(bishopPos, occupiedSquares) & friendlyPieces.inverse();
}

Bitboard rook(
    const Square& rookPos, const Bitboard occupiedSquares, const Bitboard friendlyPieces)
{
    return rook_moves(rookPos, occupiedSquares) & friendlyPieces.inverse();
}

Bitboard queen(
    const Square& queenPos, const Bitboard occupiedSquares, const Bitboard friendlyPieces)
{
    const auto moves = bishop_moves(queenPos, occupiedSquares)
                     | rook_moves(queenPos, occupiedSquares);

    return moves & friendlyPieces.inverse();
}

} // namespace chess::moves::magics
