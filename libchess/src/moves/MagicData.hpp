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

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <libchess/board/BitboardIndex.hpp>
#include <utility>

namespace chess::moves::magics::data {

// see https://github.com/kz04px/libchess/blob/master/src/movegen.cpp

using board::BitboardIndex;
using std::size_t;

// pair of multiplier, offset
using MagicInfo = std::pair<std::uint64_t, int>;

inline constexpr auto BISHOP_SHIFT = 55uz;

inline constexpr std::array BISHOP_MAGICS {
    MagicInfo { UINT64_C(0x007fbfbfbfbfbfff), 5378 },
    MagicInfo { UINT64_C(0x0000a060401007fc), 4093 },
    MagicInfo { UINT64_C(0x0001004008020000), 4314 },
    MagicInfo { UINT64_C(0x0000806004000000), 6587 },
    MagicInfo { UINT64_C(0x0000100400000000), 6491 },
    MagicInfo { UINT64_C(0x000021c100b20000), 6330 },
    MagicInfo { UINT64_C(0x0000040041008000), 5609 },
    MagicInfo { UINT64_C(0x00000fb0203fff80), 22236 },
    MagicInfo { UINT64_C(0x0000040100401004), 6106 },
    MagicInfo { UINT64_C(0x0000020080200802), 5625 },
    MagicInfo { UINT64_C(0x0000004010202000), 16785 },
    MagicInfo { UINT64_C(0x0000008060040000), 16817 },
    MagicInfo { UINT64_C(0x0000004402000000), 6842 },
    MagicInfo { UINT64_C(0x0000000801008000), 7003 },
    MagicInfo { UINT64_C(0x000007efe0bfff80), 4197 },
    MagicInfo { UINT64_C(0x0000000820820020), 7356 },
    MagicInfo { UINT64_C(0x0000400080808080), 4602 },
    MagicInfo { UINT64_C(0x00021f0100400808), 4538 },
    MagicInfo { UINT64_C(0x00018000c06f3fff), 29531 },
    MagicInfo { UINT64_C(0x0000258200801000), 45393 },
    MagicInfo { UINT64_C(0x0000240080840000), 12420 },
    MagicInfo { UINT64_C(0x000018000c03fff8), 15763 },
    MagicInfo { UINT64_C(0x00000a5840208020), 5050 },
    MagicInfo { UINT64_C(0x0000020008208020), 4346 },
    MagicInfo { UINT64_C(0x0000804000810100), 6074 },
    MagicInfo { UINT64_C(0x0001011900802008), 7866 },
    MagicInfo { UINT64_C(0x0000804000810100), 32139 },
    MagicInfo { UINT64_C(0x000100403c0403ff), 57673 },
    MagicInfo { UINT64_C(0x00078402a8802000), 55365 },
    MagicInfo { UINT64_C(0x0000101000804400), 15818 },
    MagicInfo { UINT64_C(0x0000080800104100), 5562 },
    MagicInfo { UINT64_C(0x00004004c0082008), 6390 },
    MagicInfo { UINT64_C(0x0001010120008020), 7930 },
    MagicInfo { UINT64_C(0x000080809a004010), 13329 },
    MagicInfo { UINT64_C(0x0007fefe08810010), 7170 },
    MagicInfo { UINT64_C(0x0003ff0f833fc080), 27267 },
    MagicInfo { UINT64_C(0x007fe08019003042), 53787 },
    MagicInfo { UINT64_C(0x003fffefea003000), 5097 },
    MagicInfo { UINT64_C(0x0000101010002080), 6643 },
    MagicInfo { UINT64_C(0x0000802005080804), 6138 },
    MagicInfo { UINT64_C(0x0000808080a80040), 7418 },
    MagicInfo { UINT64_C(0x0000104100200040), 7898 },
    MagicInfo { UINT64_C(0x0003ffdf7f833fc0), 42012 },
    MagicInfo { UINT64_C(0x0000008840450020), 57350 },
    MagicInfo { UINT64_C(0x00007ffc80180030), 22813 },
    MagicInfo { UINT64_C(0x007fffdd80140028), 56693 },
    MagicInfo { UINT64_C(0x00020080200a0004), 5818 },
    MagicInfo { UINT64_C(0x0000101010100020), 7098 },
    MagicInfo { UINT64_C(0x0007ffdfc1805000), 4451 },
    MagicInfo { UINT64_C(0x0003ffefe0c02200), 4709 },
    MagicInfo { UINT64_C(0x0000000820806000), 4794 },
    MagicInfo { UINT64_C(0x0000000008403000), 13364 },
    MagicInfo { UINT64_C(0x0000000100202000), 4570 },
    MagicInfo { UINT64_C(0x0000004040802000), 4282 },
    MagicInfo { UINT64_C(0x0004010040100400), 14964 },
    MagicInfo { UINT64_C(0x00006020601803f4), 4026 },
    MagicInfo { UINT64_C(0x0003ffdfdfc28048), 4826 },
    MagicInfo { UINT64_C(0x0000000820820020), 7354 },
    MagicInfo { UINT64_C(0x0000000008208060), 4848 },
    MagicInfo { UINT64_C(0x0000000000808020), 15946 },
    MagicInfo { UINT64_C(0x0000000001002020), 14932 },
    MagicInfo { UINT64_C(0x0000000401002008), 16588 },
    MagicInfo { UINT64_C(0x0000004040404040), 6905 },
    MagicInfo { UINT64_C(0x007fff9fdf7ff813), 16076 }
};

inline constexpr auto ROOK_SHIFT = 52uz;

inline constexpr std::array ROOK_MAGICS {
    MagicInfo { UINT64_C(0x00280077ffebfffe), 26304 },
    MagicInfo { UINT64_C(0x2004010201097fff), 35520 },
    MagicInfo { UINT64_C(0x0010020010053fff), 38592 },
    MagicInfo { UINT64_C(0x0040040008004002), 8026 },
    MagicInfo { UINT64_C(0x7fd00441ffffd003), 22196 },
    MagicInfo { UINT64_C(0x4020008887dffffe), 80870 },
    MagicInfo { UINT64_C(0x004000888847ffff), 76747 },
    MagicInfo { UINT64_C(0x006800fbff75fffd), 30400 },
    MagicInfo { UINT64_C(0x000028010113ffff), 11115 },
    MagicInfo { UINT64_C(0x0020040201fcffff), 18205 },
    MagicInfo { UINT64_C(0x007fe80042ffffe8), 53577 },
    MagicInfo { UINT64_C(0x00001800217fffe8), 62724 },
    MagicInfo { UINT64_C(0x00001800073fffe8), 34282 },
    MagicInfo { UINT64_C(0x00001800e05fffe8), 29196 },
    MagicInfo { UINT64_C(0x00001800602fffe8), 23806 },
    MagicInfo { UINT64_C(0x000030002fffffa0), 49481 },
    MagicInfo { UINT64_C(0x00300018010bffff), 2410 },
    MagicInfo { UINT64_C(0x0003000c0085fffb), 36498 },
    MagicInfo { UINT64_C(0x0004000802010008), 24478 },
    MagicInfo { UINT64_C(0x0004002020020004), 10074 },
    MagicInfo { UINT64_C(0x0001002002002001), 79315 },
    MagicInfo { UINT64_C(0x0001001000801040), 51779 },
    MagicInfo { UINT64_C(0x0000004040008001), 13586 },
    MagicInfo { UINT64_C(0x0000006800cdfff4), 19323 },
    MagicInfo { UINT64_C(0x0040200010080010), 70612 },
    MagicInfo { UINT64_C(0x0000080010040010), 83652 },
    MagicInfo { UINT64_C(0x0004010008020008), 63110 },
    MagicInfo { UINT64_C(0x0000040020200200), 34496 },
    MagicInfo { UINT64_C(0x0002008010100100), 84966 },
    MagicInfo { UINT64_C(0x0000008020010020), 54341 },
    MagicInfo { UINT64_C(0x0000008020200040), 60421 },
    MagicInfo { UINT64_C(0x0000820020004020), 86402 },
    MagicInfo { UINT64_C(0x00fffd1800300030), 50245 },
    MagicInfo { UINT64_C(0x007fff7fbfd40020), 76622 },
    MagicInfo { UINT64_C(0x003fffbd00180018), 84676 },
    MagicInfo { UINT64_C(0x001fffde80180018), 78757 },
    MagicInfo { UINT64_C(0x000fffe0bfe80018), 37346 },
    MagicInfo { UINT64_C(0x0001000080202001), 370 },
    MagicInfo { UINT64_C(0x0003fffbff980180), 42182 },
    MagicInfo { UINT64_C(0x0001fffdff9000e0), 45385 },
    MagicInfo { UINT64_C(0x00fffefeebffd800), 61659 },
    MagicInfo { UINT64_C(0x007ffff7ffc01400), 12790 },
    MagicInfo { UINT64_C(0x003fffbfe4ffe800), 16762 },
    MagicInfo { UINT64_C(0x001ffff01fc03000), 0 },
    MagicInfo { UINT64_C(0x000fffe7f8bfe800), 38380 },
    MagicInfo { UINT64_C(0x0007ffdfdf3ff808), 11098 },
    MagicInfo { UINT64_C(0x0003fff85fffa804), 21803 },
    MagicInfo { UINT64_C(0x0001fffd75ffa802), 39189 },
    MagicInfo { UINT64_C(0x00ffffd7ffebffd8), 58628 },
    MagicInfo { UINT64_C(0x007fff75ff7fbfd8), 44116 },
    MagicInfo { UINT64_C(0x003fff863fbf7fd8), 78357 },
    MagicInfo { UINT64_C(0x001fffbfdfd7ffd8), 44481 },
    MagicInfo { UINT64_C(0x000ffff810280028), 64134 },
    MagicInfo { UINT64_C(0x0007ffd7f7feffd8), 41759 },
    MagicInfo { UINT64_C(0x0003fffc0c480048), 1394 },
    MagicInfo { UINT64_C(0x0001ffffafd7ffd8), 40910 },
    MagicInfo { UINT64_C(0x00ffffe4ffdfa3ba), 66516 },
    MagicInfo { UINT64_C(0x007fffef7ff3d3da), 3897 },
    MagicInfo { UINT64_C(0x003fffbfdfeff7fa), 3930 },
    MagicInfo { UINT64_C(0x001fffeff7fbfc22), 72934 },
    MagicInfo { UINT64_C(0x0000020408001001), 72662 },
    MagicInfo { UINT64_C(0x0007fffeffff77fd), 56325 },
    MagicInfo { UINT64_C(0x0003ffffbf7dfeec), 66501 },
    MagicInfo { UINT64_C(0x0001ffff9dffa333), 14826 }
};

} // namespace chess::moves::magics::data
