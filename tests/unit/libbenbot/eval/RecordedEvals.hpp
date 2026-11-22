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
#include <cstdint>
#include <string_view>

// The positions & evals recorded for the RecordedEvals test: the bench.epd positions & the
// positions in the other eval tests, plus a few endgames. The trace tests use them too.

namespace recorded_evals {

struct ExpectedEval final {
    std::string_view fen;

    // the eval of the position itself
    int eval { 0 };

    // a hash of the evals of every position in the move tree, to depth 2
    std::uint64_t treeHash { 0 };
};

// clang-format off
inline constexpr std::array EXPECTED_EVALS {
    ExpectedEval { "r3k2r/2pb1ppp/2pp1q2/p7/1nP1B3/1P2P3/P2N1PPP/R2QK2R w KQkq a6 0 1", 16, UINT64_C(0x18b85851cee219f9) },
    ExpectedEval { "4rrk1/2p1b1p1/p1p3q1/4p3/2P2n1p/1P1NR2P/PB3PP1/3R1QK1 b - - 0 1", -62, UINT64_C(0x4e933a27bc0aaf9f) },
    ExpectedEval { "r3qbrk/6p1/2b2pPp/p3pP1Q/PpPpP2P/3P1B2/2PB3K/R5R1 w - - 0 1", 164, UINT64_C(0x04373a35ed620837) },
    ExpectedEval { "6k1/1R3p2/6p1/2Bp3p/3P2q1/P7/1P2rQ1K/5R2 b - - 0 1", -987, UINT64_C(0x45e5df519ae1a6a0) },
    ExpectedEval { "8/8/1p2k1p1/3p3p/1p1P1P1P/1P2PK2/8/8 w - - 0 1", -4, UINT64_C(0x3e8a01bc53d25aa0) },
    ExpectedEval { "7r/2p3k1/1p1p1qp1/1P1Bp3/p1P2r1P/P7/4R3/Q4RK1 w - - 0 1", 283, UINT64_C(0x9cd2fd9ffd6c46d4) },
    ExpectedEval { "r1bq1rk1/pp2b1pp/n1pp1n2/3P1p2/2P1p3/2N1P2N/PP2BPPP/R1BQ1RK1 b - - 0 1", 0, UINT64_C(0xe93c2dd0bf0902be) },
    ExpectedEval { "3r3k/2r4p/1p1b3q/p4P2/P2Pp3/1B2P3/3BQ1RP/6K1 w - - 0 1", -9, UINT64_C(0x63c033f661ba00b4) },
    ExpectedEval { "2r4r/1p4k1/1Pnp4/3Qb1pq/8/4BpPp/5P2/2RR1BK1 w - - 0 1", -199, UINT64_C(0xc8de16a252e780e7) },
    ExpectedEval { "4q1bk/6b1/7p/p1p4p/PNPpP2P/KN4P1/3Q4/4R3 b - - 0 1", -638, UINT64_C(0xf6cdb7a5bfd85f00) },
    ExpectedEval { "2q3r1/1r2pk2/pp3pp1/2pP3p/P1Pb1BbP/1P4Q1/R3NPP1/4R1K1 w - - 0 1", 120, UINT64_C(0xecba7d4b5ea30eff) },
    ExpectedEval { "1r2r2k/1b4q1/pp5p/2pPp1p1/P3Pn2/1P1B1Q1P/2R3P1/4BR1K b - - 0 1", -159, UINT64_C(0xafa7194eb2335ace) },
    ExpectedEval { "r3kbbr/pp1n1p1P/3ppnp1/q5N1/1P1pP3/P1N1B3/2P1QP2/R3KB1R b KQkq b3 0 1", 176, UINT64_C(0xa64f3e94020decde) },
    ExpectedEval { "8/6pk/2b1Rp2/3r4/1R1B2PP/P5K1/8/2r5 b - - 0 1", -67, UINT64_C(0x4b15a52efd8c4fc5) },
    ExpectedEval { "1r4k1/4ppb1/2n1b1qp/pB4p1/1n1BP1P1/7P/2PNQPK1/3RN3 w - - 0 1", -1, UINT64_C(0x60f05913298137bb) },
    ExpectedEval { "8/p2B4/PkP5/4p1pK/4Pb1p/5P2/8/8 w - - 0 1", 157, UINT64_C(0x4f78be7ecd36cb9e) },
    ExpectedEval { "3r4/ppq1ppkp/4bnp1/2pN4/2P1P3/1P4P1/PQ3PBP/R4K2 b - - 0 1", 103, UINT64_C(0x7401b9f0b899cbc2) },
    ExpectedEval { "5rr1/4n2k/4q2P/P1P2n2/3B1p2/4pP2/2N1P3/1RR1K2Q w - - 0 1", 167, UINT64_C(0xacb585b2c1ca4c83) },
    ExpectedEval { "1r5k/2pq2p1/3p3p/p1pP4/4QP2/PP1R3P/6PK/8 w - - 0 1", 9, UINT64_C(0x4f5e81eba312212e) },
    ExpectedEval { "q5k1/5ppp/1r3bn1/1B6/P1N2P2/BQ2P1P1/5K1P/8 b - - 0 1", 71, UINT64_C(0x104b3485dddad785) },
    ExpectedEval { "r1b2k1r/5n2/p4q2/1ppn1Pp1/3pp1p1/NP2P3/P1PPBK2/1RQN2R1 w - - 0 1", -297, UINT64_C(0xa41e9effa155398e) },
    ExpectedEval { "r1bqk2r/pppp1ppp/5n2/4b3/4P3/P1N5/1PP2PPP/R1BQKB1R w KQkq - 0 1", 80, UINT64_C(0xef56239c5c36371a) },
    ExpectedEval { "r1bqr1k1/pp1p1ppp/2p5/8/3N1Q2/P2BB3/1PP2PPP/R3K2n b Q - 0 1", 104, UINT64_C(0xa4dc88aa5a3f97af) },
    ExpectedEval { "r1bq2k1/p4r1p/1pp2pp1/3p4/1P1B3Q/P2B1N2/2P3PP/4R1K1 b - - 0 1", -223, UINT64_C(0x8f46e383d76b92c2) },
    ExpectedEval { "r4qk1/6r1/1p4p1/2ppBbN1/1p5Q/P7/2P3PP/5RK1 w - - 0 1", -176, UINT64_C(0x869bb501a6b302c7) },
    ExpectedEval { "r7/6k1/1p6/2pp1p2/7Q/8/p1P2K1P/8 w - - 0 1", 10, UINT64_C(0x35941a46ec9b234f) },
    ExpectedEval { "r3k2r/ppp1pp1p/2nqb1pn/3p4/4P3/2PP4/PP1NBPPP/R2QK1NR w KQkq - 0 1", 14, UINT64_C(0x31efdda64127a7de) },
    ExpectedEval { "3r1rk1/1pp1pn1p/p1n1q1p1/3p4/Q3P3/2P5/PP1NBPPP/4RRK1 w - - 0 1", 40, UINT64_C(0x6c3c14916ffee71e) },
    ExpectedEval { "5rk1/1pp1pn1p/p3Brp1/8/1n6/5N2/PP3PPP/2R2RK1 w - - 0 1", -19, UINT64_C(0xc0d36f5f76fd1698) },
    ExpectedEval { "8/1p2pk1p/p1p1r1p1/3n4/8/5R2/PP3PPP/4R1K1 b - - 0 1", -155, UINT64_C(0x351a2436a1b653c5) },
    ExpectedEval { "8/4pk2/1p1r2p1/p1p4p/Pn5P/3R4/1P3PP1/4RK2 w - - 0 1", 184, UINT64_C(0x822073c50c0607f3) },
    ExpectedEval { "8/5k2/1pnrp1p1/p1p4p/P6P/4R1PK/1P3P2/4R3 b - - 0 1", -202, UINT64_C(0x1f7e9c5fa3e5ac62) },
    ExpectedEval { "8/8/1p1kp1p1/p1pr1n1p/P6P/1R4P1/1P3PK1/1R6 b - - 0 1", -161, UINT64_C(0x7e86b741695f1792) },
    ExpectedEval { "8/8/1p1k2p1/p1prp2p/P2n3P/6P1/1P1R1PK1/4R3 b - - 0 1", -270, UINT64_C(0x62ae41326117bf12) },
    ExpectedEval { "8/8/1p4p1/p1p2k1p/P2npP1P/4K1P1/1P6/3R4 w - - 0 1", 83, UINT64_C(0x2595bc615f65481b) },
    ExpectedEval { "8/8/1p4p1/p1p2k1p/P2n1P1P/4K1P1/1P6/6R1 b - - 0 1", -282, UINT64_C(0x8bd9a35ac60cd67f) },
    ExpectedEval { "8/5k2/1p4p1/p1pK3p/P2n1P1P/6P1/1P6/4R3 b - - 0 1", -317, UINT64_C(0x14b222754b380f1e) },
    ExpectedEval { "8/1R6/1p1K1kp1/p6p/P1p2P1P/6P1/1Pn5/8 w - - 0 1", 335, UINT64_C(0x7335f6a7ad2d0c53) },
    ExpectedEval { "1rb1rn1k/p3q1bp/2p3p1/2p1p3/2P1P2N/PP1RQNP1/1B3P2/4R1K1 b - - 0 1", -167, UINT64_C(0xf6249e1efa2f25ea) },
    ExpectedEval { "4rrk1/pp1n1pp1/q5p1/P1pP4/2n3P1/7P/1P3PB1/R1BQ1RK1 w - - 0 1", 2, UINT64_C(0x615a0298b2cf604e) },
    ExpectedEval { "r2qr1k1/pb1nbppp/1pn1p3/2ppP3/3P4/2PB1NN1/PP3PPP/R1BQR1K1 w - - 0 1", 121, UINT64_C(0x667ae2d88d2d3f1f) },
    ExpectedEval { "2r2k2/8/4P1R1/1p6/8/P4K1N/7b/2B5 b - - 0 1", -696, UINT64_C(0x320660aa29b42fff) },
    ExpectedEval { "6k1/5pp1/8/2bKP2P/2P5/p4PNb/B7/8 b - - 0 1", 75, UINT64_C(0x48c3ce1b74043c15) },
    ExpectedEval { "2rqr1k1/1p3p1p/p2p2p1/P1nPb3/2B1P3/5P2/1PQ2NPP/R1R4K w - - 0 1", 82, UINT64_C(0xd4128cdac4e0750b) },
    ExpectedEval { "r1b2rk1/p1q1ppbp/6p1/2Q5/8/4BP2/PPP3PP/2KR1B1R b - - 0 1", -86, UINT64_C(0x7e19a42edaae9d9f) },
    ExpectedEval { "6r1/5k2/p1b1r2p/1pB1p1p1/1Pp3PP/2P1R1K1/2P2P2/3R4 w - - 0 1", 31, UINT64_C(0x6d3726b26c1a72db) },
    ExpectedEval { "rnbqkb1r/pppppppp/5n2/8/2PP4/8/PP2PPPP/RNBQKBNR b KQkq c3 0 1", -46, UINT64_C(0x680cc21553dee41b) },
    ExpectedEval { "2rr2k1/1p4bp/p1q1p1p1/4Pp1n/2PB4/1PN3P1/P3Q2P/2RR2K1 w - f6 0 1", 11, UINT64_C(0x78bdd7d9e8d35db2) },
    ExpectedEval { "3br1k1/p1pn3p/1p3n2/5pNq/2P1p3/1PN3PP/P2Q1PB1/4R1K1 w - - 0 1", 212, UINT64_C(0x8d5cffea3fedcac2) },
    ExpectedEval { "2r2b2/5p2/5k2/p1r1pP2/P2pB3/1P3P2/K1P3R1/7R w - - 0 1", 172, UINT64_C(0xad8d708cd451e6ec) },
    ExpectedEval { "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1", -26, UINT64_C(0xd2a1d828270421c2) },
    ExpectedEval { "r2qkb1r/ppp1p1pp/2n3p1/1b6/2BpP3/5Q2/PPPP1PPP/RNB1K1NR w KQkq - 0 1", 484, UINT64_C(0x3b8c5cf7ca6205f9) },
    ExpectedEval { "8/8/8/8/8/8/3q4/2K1k3 w - - 0 1", -2002, UINT64_C(0x9c6b73c4790f43dd) },
    ExpectedEval { "8/2P5/8/8/k1K5/8/8/8 w - - 0 1", 494, UINT64_C(0x1932ac145965dd3e) },
    ExpectedEval { "r1b2rk1/pp1nqppp/2p1p3/3n4/2BP4/2N1PN2/PPQ2PPP/R4RK1 w - - 0 1", 137, UINT64_C(0x3a734fc18f54375e) },
    ExpectedEval { "4k3/8/2r5/3P4/8/8/8/4K3 w - - 0 1", -1069, UINT64_C(0x79ca13dac2ec93b5) },
    ExpectedEval { "r2qk2r/ppp2ppp/3n1b2/4P3/8/8/PPP2PPP/R2QK2R b KQkq - 0 1", 867, UINT64_C(0x0d3e9d6b6b28d0ae) },
    ExpectedEval { "2r1k3/3q4/1N6/8/8/8/8/4K3 w - - 0 1", -1800, UINT64_C(0xeb57d5ee277b1af4) },
    ExpectedEval { "6k1/8/4q3/8/8/8/4R3/6K1 w - - 0 1", -735, UINT64_C(0x648fc73832ac8b5b) },
    ExpectedEval { "6k1/8/8/4r3/3P4/5N2/8/6K1 w - - 0 1", -212, UINT64_C(0xba5d2e8085058b10) },
    ExpectedEval { "6k1/7p/4b3/8/8/8/4R3/6K1 w - - 0 1", 188, UINT64_C(0xd5f567bd8585ae03) },
    ExpectedEval { "6k1/7p/4b3/8/8/8/4R3/6K1 b - - 0 1", -188, UINT64_C(0xca3beefa5d99b2e1) },
    ExpectedEval { "8/8/4k3/4n3/2N5/4NN2/6K1/8 b - - 0 1", -816, UINT64_C(0xebabdc82efb94ffb) },
    ExpectedEval { "6k1/8/8/4n3/8/2BB4/8/6K1 w - - 0 1", 558, UINT64_C(0xd3cc3f122454da89) },
    ExpectedEval { "2b4k/1p1p4/8/8/8/8/1P1P4/K1B5 w - - 0 1", 0, UINT64_C(0xb9b3a8e0838bf660) },
    ExpectedEval { "2b4k/1p1p4/8/8/8/8/1P1P4/K4B2 w - - 0 1", 94, UINT64_C(0x4e8d628d723a257d) },
    ExpectedEval { "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 0, UINT64_C(0x772512c3ee3173cc) },
    ExpectedEval { "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1", 3, UINT64_C(0x94719f3d55589f1c) },
    ExpectedEval { "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1", 271, UINT64_C(0x60353eb83170be73) },
    ExpectedEval { "6k1/5ppp/8/1P6/8/8/5PPP/1R4K1 b - - 0 1", -1464, UINT64_C(0xf31e07a5ff5aafc0) },
    ExpectedEval { "6k1/5ppp/8/1P6/8/8/5PPP/1R4K1 w - - 0 1", 1535, UINT64_C(0xde2edacbe39b6bcc) },
    ExpectedEval { "1r4k1/5ppp/8/1P6/8/8/5PPP/6K1 w - - 0 1", -1051, UINT64_C(0xa74c682251598150) },
    ExpectedEval { "6k1/5ppp/8/1P6/8/8/5PPP/6KR w - - 0 1", 1387, UINT64_C(0x06ad8583ecdb737c) },
    ExpectedEval { "1k6/5ppp/8/1P6/8/8/5PPP/1R4K1 w - - 0 1", 1427, UINT64_C(0xe53ee9e8f159d961) },
    ExpectedEval { "6k1/5ppp/8/1P6/K7/8/5PPP/1R6 w - - 0 1", 1584, UINT64_C(0x788b58c223967c4b) },
    ExpectedEval { "6k1/8/8/4n3/3P4/8/8/6K1 w - - 0 1", -736, UINT64_C(0x71b109674d5b404d) },
    ExpectedEval { "6k1/8/8/3n4/3P4/8/8/6K1 w - - 0 1", -766, UINT64_C(0xace66c6571c69e65) },
    ExpectedEval { "6k1/8/8/4n3/3P4/8/8/6K1 b - - 0 1", 811, UINT64_C(0xfacc90ff72717426) },
    ExpectedEval { "6k1/8/8/3n4/3P4/8/8/6K1 b - - 0 1", 841, UINT64_C(0x3d02d6da97698d7e) },
    ExpectedEval { "6k1/8/4r3/8/5N2/8/8/6K1 w - - 0 1", -336, UINT64_C(0x7ae29b24356ac348) },
    ExpectedEval { "6k1/8/3r4/8/5N2/8/8/6K1 w - - 0 1", -346, UINT64_C(0x0a70ace88bb31df3) },
    ExpectedEval { "6k1/8/4r3/8/2B5/8/8/6K1 b - - 0 1", 302, UINT64_C(0x94d14ba5dac9d036) },
    ExpectedEval { "6k1/8/3r4/8/2B5/8/8/6K1 b - - 0 1", 318, UINT64_C(0x68a1006cdae10495) },
    ExpectedEval { "6k1/8/4q3/6N1/8/8/PPP5/1K6 w - - 0 1", -629, UINT64_C(0xc19a9b8689998f34) },
    ExpectedEval { "6k1/8/3q4/6N1/8/8/PPP5/1K6 w - - 0 1", -612, UINT64_C(0xdd675626076c3df4) },
    ExpectedEval { "6k1/8/3q4/8/8/8/4R3/6K1 w - - 0 1", -701, UINT64_C(0x6934a6e0a6564c50) },
    ExpectedEval { "7k/8/3p4/3n4/8/4NN2/5K2/8 w - - 0 1", 449, UINT64_C(0xb8a6f90c825e6366) },
    ExpectedEval { "7k/8/3p4/4n3/8/4NN2/5K2/8 w - - 0 1", 441, UINT64_C(0x48b74a7e38fe628f) },
    ExpectedEval { "7k/8/2n5/3n4/8/4NN2/5K2/8 w - - 0 1", 67, UINT64_C(0x3bf473424c4d0306) },
    ExpectedEval { "7k/8/2n5/4n3/8/4NN2/5K2/8 w - - 0 1", 33, UINT64_C(0x2e83e4128c7932e4) },
    ExpectedEval { "8/8/4k3/4n3/2N5/4NN2/6K1/8 w - - 0 1", 851, UINT64_C(0x72ba2eea53b62eb9) },
    ExpectedEval { "8/8/4k3/3n4/2N5/4NN2/6K1/8 w - - 0 1", 834, UINT64_C(0xea264091f9045a7e) },
    ExpectedEval { "8/8/8/4k3/8/8/8/KQ6 w - - 0 1", 1817, UINT64_C(0xefd5487fe53f8dfc) },
    ExpectedEval { "8/8/8/3k4/8/8/2K5/7R b - - 0 1", -1190, UINT64_C(0xd4796fe960a0afa6) },
    ExpectedEval { "8/8/4k3/8/8/8/3PK3/8 w - - 0 1", 99, UINT64_C(0x91f176980bf081b1) },
    ExpectedEval { "8/5k2/8/8/8/8/1N3B2/4K3 w - - 0 1", 1295, UINT64_C(0x4a6a35322b4b357f) },
    ExpectedEval { "4k3/8/8/8/8/8/8/4K2R w K - 0 1", 1271, UINT64_C(0xcd851dc1cdc06583) },
    ExpectedEval { "1k6/8/8/8/8/8/QQQQ4/K7 b - - 0 1", -6485, UINT64_C(0xb006264c8715b04a) },
    ExpectedEval { "qqqqk3/8/8/8/8/8/8/4K3 w - - 0 1", -6551, UINT64_C(0xaf875be4269ac4d9) },
    ExpectedEval { "4k3/pppppppp/8/8/8/8/PPPPPPPP/4K3 w - - 0 1", 0, UINT64_C(0x410d4f41de511e66) },
    ExpectedEval { "8/1P4k1/8/8/8/8/6p1/K7 w - - 0 1", -76, UINT64_C(0x262c794b5bf01de6) },
    ExpectedEval { "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b KQkq - 0 1", 0, UINT64_C(0x772512c3ee3173cc) },
};
// clang-format on

} // namespace recorded_evals
