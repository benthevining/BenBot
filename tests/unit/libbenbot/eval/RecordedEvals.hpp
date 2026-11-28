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
    ExpectedEval { "r3k2r/2pb1ppp/2pp1q2/p7/1nP1B3/1P2P3/P2N1PPP/R2QK2R w KQkq a6 0 1", 78, UINT64_C(0xd0a93908841251b9) },
    ExpectedEval { "4rrk1/2p1b1p1/p1p3q1/4p3/2P2n1p/1P1NR2P/PB3PP1/3R1QK1 b - - 0 1", -14, UINT64_C(0x0a39b0f1fed9780f) },
    ExpectedEval { "r3qbrk/6p1/2b2pPp/p3pP1Q/PpPpP2P/3P1B2/2PB3K/R5R1 w - - 0 1", 155, UINT64_C(0x36344cde9cdbafa1) },
    ExpectedEval { "6k1/1R3p2/6p1/2Bp3p/3P2q1/P7/1P2rQ1K/5R2 b - - 0 1", -1137, UINT64_C(0x0c5aeb60423d7c7e) },
    ExpectedEval { "8/8/1p2k1p1/3p3p/1p1P1P1P/1P2PK2/8/8 w - - 0 1", -79, UINT64_C(0x9f18a86a727906bc) },
    ExpectedEval { "7r/2p3k1/1p1p1qp1/1P1Bp3/p1P2r1P/P7/4R3/Q4RK1 w - - 0 1", 310, UINT64_C(0x0fc31ccf574d88bf) },
    ExpectedEval { "r1bq1rk1/pp2b1pp/n1pp1n2/3P1p2/2P1p3/2N1P2N/PP2BPPP/R1BQ1RK1 b - - 0 1", -28, UINT64_C(0x8bdff601a536e21a) },
    ExpectedEval { "3r3k/2r4p/1p1b3q/p4P2/P2Pp3/1B2P3/3BQ1RP/6K1 w - - 0 1", 2, UINT64_C(0x7e554aea3db611af) },
    ExpectedEval { "2r4r/1p4k1/1Pnp4/3Qb1pq/8/4BpPp/5P2/2RR1BK1 w - - 0 1", -294, UINT64_C(0x5653a5c9e6125aa5) },
    ExpectedEval { "4q1bk/6b1/7p/p1p4p/PNPpP2P/KN4P1/3Q4/4R3 b - - 0 1", -618, UINT64_C(0x8d4ea5216f785dbd) },
    ExpectedEval { "2q3r1/1r2pk2/pp3pp1/2pP3p/P1Pb1BbP/1P4Q1/R3NPP1/4R1K1 w - - 0 1", 41, UINT64_C(0xd77f6b87f8db76a9) },
    ExpectedEval { "1r2r2k/1b4q1/pp5p/2pPp1p1/P3Pn2/1P1B1Q1P/2R3P1/4BR1K b - - 0 1", -175, UINT64_C(0x3b5be8d100611a13) },
    ExpectedEval { "r3kbbr/pp1n1p1P/3ppnp1/q5N1/1P1pP3/P1N1B3/2P1QP2/R3KB1R b KQkq b3 0 1", 287, UINT64_C(0xfff849f00745d155) },
    ExpectedEval { "8/6pk/2b1Rp2/3r4/1R1B2PP/P5K1/8/2r5 b - - 0 1", -88, UINT64_C(0x0447e1ed52bdd60a) },
    ExpectedEval { "1r4k1/4ppb1/2n1b1qp/pB4p1/1n1BP1P1/7P/2PNQPK1/3RN3 w - - 0 1", -6, UINT64_C(0x01d829039b36fb25) },
    ExpectedEval { "8/p2B4/PkP5/4p1pK/4Pb1p/5P2/8/8 w - - 0 1", 209, UINT64_C(0x2c21077bcc42b92d) },
    ExpectedEval { "3r4/ppq1ppkp/4bnp1/2pN4/2P1P3/1P4P1/PQ3PBP/R4K2 b - - 0 1", 27, UINT64_C(0xd8d56cdd9f390e74) },
    ExpectedEval { "5rr1/4n2k/4q2P/P1P2n2/3B1p2/4pP2/2N1P3/1RR1K2Q w - - 0 1", 356, UINT64_C(0x7f2cbc30c3fee78c) },
    ExpectedEval { "1r5k/2pq2p1/3p3p/p1pP4/4QP2/PP1R3P/6PK/8 w - - 0 1", 36, UINT64_C(0x942e38d006cd7a81) },
    ExpectedEval { "q5k1/5ppp/1r3bn1/1B6/P1N2P2/BQ2P1P1/5K1P/8 b - - 0 1", -47, UINT64_C(0x3b8dd2507cbfc225) },
    ExpectedEval { "r1b2k1r/5n2/p4q2/1ppn1Pp1/3pp1p1/NP2P3/P1PPBK2/1RQN2R1 w - - 0 1", -171, UINT64_C(0xf021eb6c807f7e40) },
    ExpectedEval { "r1bqk2r/pppp1ppp/5n2/4b3/4P3/P1N5/1PP2PPP/R1BQKB1R w KQkq - 0 1", 110, UINT64_C(0x19dad4f19091e186) },
    ExpectedEval { "r1bqr1k1/pp1p1ppp/2p5/8/3N1Q2/P2BB3/1PP2PPP/R3K2n b Q - 0 1", 124, UINT64_C(0x9a8ebe8b086b016e) },
    ExpectedEval { "r1bq2k1/p4r1p/1pp2pp1/3p4/1P1B3Q/P2B1N2/2P3PP/4R1K1 b - - 0 1", -170, UINT64_C(0xd5a7dea047bc64a8) },
    ExpectedEval { "r4qk1/6r1/1p4p1/2ppBbN1/1p5Q/P7/2P3PP/5RK1 w - - 0 1", -274, UINT64_C(0xdef4ddb467d446ad) },
    ExpectedEval { "r7/6k1/1p6/2pp1p2/7Q/8/p1P2K1P/8 w - - 0 1", 30, UINT64_C(0x19f305cc7bae1f10) },
    ExpectedEval { "r3k2r/ppp1pp1p/2nqb1pn/3p4/4P3/2PP4/PP1NBPPP/R2QK1NR w KQkq - 0 1", 5, UINT64_C(0xe92c47cc53376ac1) },
    ExpectedEval { "3r1rk1/1pp1pn1p/p1n1q1p1/3p4/Q3P3/2P5/PP1NBPPP/4RRK1 w - - 0 1", 31, UINT64_C(0x143884a0398562bd) },
    ExpectedEval { "5rk1/1pp1pn1p/p3Brp1/8/1n6/5N2/PP3PPP/2R2RK1 w - - 0 1", -17, UINT64_C(0xd3d7a53868d33ac6) },
    ExpectedEval { "8/1p2pk1p/p1p1r1p1/3n4/8/5R2/PP3PPP/4R1K1 b - - 0 1", -124, UINT64_C(0xabfb92b45b221f97) },
    ExpectedEval { "8/4pk2/1p1r2p1/p1p4p/Pn5P/3R4/1P3PP1/4RK2 w - - 0 1", 138, UINT64_C(0x0a0bb0296e7e1d6c) },
    ExpectedEval { "8/5k2/1pnrp1p1/p1p4p/P6P/4R1PK/1P3P2/4R3 b - - 0 1", -207, UINT64_C(0x18f63fb547e68cf5) },
    ExpectedEval { "8/8/1p1kp1p1/p1pr1n1p/P6P/1R4P1/1P3PK1/1R6 b - - 0 1", -149, UINT64_C(0xc63b4b570a0a2767) },
    ExpectedEval { "8/8/1p1k2p1/p1prp2p/P2n3P/6P1/1P1R1PK1/4R3 b - - 0 1", -257, UINT64_C(0xb008eb68980324f7) },
    ExpectedEval { "8/8/1p4p1/p1p2k1p/P2npP1P/4K1P1/1P6/3R4 w - - 0 1", 53, UINT64_C(0xbde99fa4eaf0c012) },
    ExpectedEval { "8/8/1p4p1/p1p2k1p/P2n1P1P/4K1P1/1P6/6R1 b - - 0 1", -256, UINT64_C(0xd795b89f14891d68) },
    ExpectedEval { "8/5k2/1p4p1/p1pK3p/P2n1P1P/6P1/1P6/4R3 b - - 0 1", -310, UINT64_C(0x5f6149aca0e29a9e) },
    ExpectedEval { "8/1R6/1p1K1kp1/p6p/P1p2P1P/6P1/1Pn5/8 w - - 0 1", 389, UINT64_C(0x244811123d6c6213) },
    ExpectedEval { "1rb1rn1k/p3q1bp/2p3p1/2p1p3/2P1P2N/PP1RQNP1/1B3P2/4R1K1 b - - 0 1", -110, UINT64_C(0x14b6f1814dc07787) },
    ExpectedEval { "4rrk1/pp1n1pp1/q5p1/P1pP4/2n3P1/7P/1P3PB1/R1BQ1RK1 w - - 0 1", 38, UINT64_C(0x99c536cc6a38d37c) },
    ExpectedEval { "r2qr1k1/pb1nbppp/1pn1p3/2ppP3/3P4/2PB1NN1/PP3PPP/R1BQR1K1 w - - 0 1", 43, UINT64_C(0x8ebcd4ef8c4cd7da) },
    ExpectedEval { "2r2k2/8/4P1R1/1p6/8/P4K1N/7b/2B5 b - - 0 1", -813, UINT64_C(0xb6a5282b5c01ab53) },
    ExpectedEval { "6k1/5pp1/8/2bKP2P/2P5/p4PNb/B7/8 b - - 0 1", 70, UINT64_C(0x5ce925e2de1bb6ec) },
    ExpectedEval { "2rqr1k1/1p3p1p/p2p2p1/P1nPb3/2B1P3/5P2/1PQ2NPP/R1R4K w - - 0 1", 15, UINT64_C(0x2bc971e7680d6670) },
    ExpectedEval { "r1b2rk1/p1q1ppbp/6p1/2Q5/8/4BP2/PPP3PP/2KR1B1R b - - 0 1", -66, UINT64_C(0x31788d79f670f9c1) },
    ExpectedEval { "6r1/5k2/p1b1r2p/1pB1p1p1/1Pp3PP/2P1R1K1/2P2P2/3R4 w - - 0 1", -4, UINT64_C(0x96e10e7816881064) },
    ExpectedEval { "rnbqkb1r/pppppppp/5n2/8/2PP4/8/PP2PPPP/RNBQKBNR b KQkq c3 0 1", -21, UINT64_C(0x68a847c2898c32bd) },
    ExpectedEval { "2rr2k1/1p4bp/p1q1p1p1/4Pp1n/2PB4/1PN3P1/P3Q2P/2RR2K1 w - f6 0 1", -1, UINT64_C(0xa2115fa820e35409) },
    ExpectedEval { "3br1k1/p1pn3p/1p3n2/5pNq/2P1p3/1PN3PP/P2Q1PB1/4R1K1 w - - 0 1", 234, UINT64_C(0x15cff68c7ed27b0e) },
    ExpectedEval { "2r2b2/5p2/5k2/p1r1pP2/P2pB3/1P3P2/K1P3R1/7R w - - 0 1", 176, UINT64_C(0x5335e7e11549eaa4) },
    ExpectedEval { "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1", -70, UINT64_C(0x65eb945265bcddb7) },
    ExpectedEval { "r2qkb1r/ppp1p1pp/2n3p1/1b6/2BpP3/5Q2/PPPP1PPP/RNB1K1NR w KQkq - 0 1", 578, UINT64_C(0xcf8fb4d0ec35edf8) },
    ExpectedEval { "8/8/8/8/8/8/3q4/2K1k3 w - - 0 1", -2258, UINT64_C(0x5e58b2b41d0a4d1a) },
    ExpectedEval { "8/2P5/8/8/k1K5/8/8/8 w - - 0 1", 590, UINT64_C(0x4ddc5b901eb45825) },
    ExpectedEval { "r1b2rk1/pp1nqppp/2p1p3/3n4/2BP4/2N1PN2/PPQ2PPP/R4RK1 w - - 0 1", 104, UINT64_C(0x83e42b1503e73aa7) },
    ExpectedEval { "4k3/8/2r5/3P4/8/8/8/4K3 w - - 0 1", -1108, UINT64_C(0x8e063051f7798c5a) },
    ExpectedEval { "r2qk2r/ppp2ppp/3n1b2/4P3/8/8/PPP2PPP/R2QK2R b KQkq - 0 1", 973, UINT64_C(0xdae6ad06357f9d6b) },
    ExpectedEval { "2r1k3/3q4/1N6/8/8/8/8/4K3 w - - 0 1", -2014, UINT64_C(0x801dc61afabb22bf) },
    ExpectedEval { "6k1/8/4q3/8/8/8/4R3/6K1 w - - 0 1", -795, UINT64_C(0xc2fbd88730cc1db4) },
    ExpectedEval { "6k1/8/8/4r3/3P4/5N2/8/6K1 w - - 0 1", -221, UINT64_C(0x1affa8fe5c6b9a07) },
    ExpectedEval { "6k1/7p/4b3/8/8/8/4R3/6K1 w - - 0 1", 159, UINT64_C(0xec2bc57feee66e5d) },
    ExpectedEval { "6k1/7p/4b3/8/8/8/4R3/6K1 b - - 0 1", -159, UINT64_C(0x0c1b3a7e35fde397) },
    ExpectedEval { "8/8/4k3/4n3/2N5/4NN2/6K1/8 b - - 0 1", -941, UINT64_C(0x5be195136cceecb3) },
    ExpectedEval { "6k1/8/8/4n3/8/2BB4/8/6K1 w - - 0 1", 594, UINT64_C(0xbfad4c9c7dbb795e) },
    ExpectedEval { "2b4k/1p1p4/8/8/8/8/1P1P4/K1B5 w - - 0 1", 48, UINT64_C(0x4d214f83f4e70d0e) },
    ExpectedEval { "2b4k/1p1p4/8/8/8/8/1P1P4/K4B2 w - - 0 1", 187, UINT64_C(0x056b07b1f4c2e79c) },
    ExpectedEval { "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 0, UINT64_C(0xc929aeca0f04ec46) },
    ExpectedEval { "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1", -43, UINT64_C(0x0b0965f879095d4a) },
    ExpectedEval { "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1", 239, UINT64_C(0xc03545df51c01e15) },
    ExpectedEval { "6k1/5ppp/8/1P6/8/8/5PPP/1R4K1 b - - 0 1", -1624, UINT64_C(0xa1281d8c6cbf59b5) },
    ExpectedEval { "6k1/5ppp/8/1P6/8/8/5PPP/1R4K1 w - - 0 1", 1695, UINT64_C(0x4ff5ac8db4eded7a) },
    ExpectedEval { "1r4k1/5ppp/8/1P6/8/8/5PPP/6K1 w - - 0 1", -1095, UINT64_C(0x6cb9c4e3eac99367) },
    ExpectedEval { "6k1/5ppp/8/1P6/8/8/5PPP/6KR w - - 0 1", 1569, UINT64_C(0x28c370c690680496) },
    ExpectedEval { "1k6/5ppp/8/1P6/8/8/5PPP/1R4K1 w - - 0 1", 1520, UINT64_C(0x4049ae5f16164189) },
    ExpectedEval { "6k1/5ppp/8/1P6/K7/8/5PPP/1R6 w - - 0 1", 1749, UINT64_C(0x78d10d960ffdaa65) },
    ExpectedEval { "6k1/8/8/4n3/3P4/8/8/6K1 w - - 0 1", -778, UINT64_C(0xf26aa0d8595491a0) },
    ExpectedEval { "6k1/8/8/3n4/3P4/8/8/6K1 w - - 0 1", -786, UINT64_C(0x6f6785509c121ecf) },
    ExpectedEval { "6k1/8/8/4n3/3P4/8/8/6K1 b - - 0 1", 853, UINT64_C(0x01e18e0b592db4f5) },
    ExpectedEval { "6k1/8/8/3n4/3P4/8/8/6K1 b - - 0 1", 860, UINT64_C(0xde18d31178eee6f0) },
    ExpectedEval { "6k1/8/4r3/8/5N2/8/8/6K1 w - - 0 1", -352, UINT64_C(0xf832e1f956f7e95f) },
    ExpectedEval { "6k1/8/3r4/8/5N2/8/8/6K1 w - - 0 1", -355, UINT64_C(0xb62597c5ab8f1b7e) },
    ExpectedEval { "6k1/8/4r3/8/2B5/8/8/6K1 b - - 0 1", 317, UINT64_C(0x060495819c8a0194) },
    ExpectedEval { "6k1/8/3r4/8/2B5/8/8/6K1 b - - 0 1", 331, UINT64_C(0x87ae925ea9929df4) },
    ExpectedEval { "6k1/8/4q3/6N1/8/8/PPP5/1K6 w - - 0 1", -542, UINT64_C(0x8515ffd15f34c7eb) },
    ExpectedEval { "6k1/8/3q4/6N1/8/8/PPP5/1K6 w - - 0 1", -542, UINT64_C(0xb10b9612bd670c70) },
    ExpectedEval { "6k1/8/3q4/8/8/8/4R3/6K1 w - - 0 1", -787, UINT64_C(0x1f8b03aadcc93d90) },
    ExpectedEval { "7k/8/3p4/3n4/8/4NN2/5K2/8 w - - 0 1", 501, UINT64_C(0x9372f18317475034) },
    ExpectedEval { "7k/8/3p4/4n3/8/4NN2/5K2/8 w - - 0 1", 446, UINT64_C(0x63b5ccdf708939b2) },
    ExpectedEval { "7k/8/2n5/3n4/8/4NN2/5K2/8 w - - 0 1", 89, UINT64_C(0x4e7479c666eaa832) },
    ExpectedEval { "7k/8/2n5/4n3/8/4NN2/5K2/8 w - - 0 1", 24, UINT64_C(0xa8a81828c9519872) },
    ExpectedEval { "8/8/4k3/4n3/2N5/4NN2/6K1/8 w - - 0 1", 976, UINT64_C(0x686f59af4a13a01d) },
    ExpectedEval { "8/8/4k3/3n4/2N5/4NN2/6K1/8 w - - 0 1", 962, UINT64_C(0x1c0f97f7d3340e39) },
    ExpectedEval { "8/8/8/4k3/8/8/8/KQ6 w - - 0 1", 2047, UINT64_C(0xdb3a475083025438) },
    ExpectedEval { "8/8/8/3k4/8/8/2K5/7R b - - 0 1", -1267, UINT64_C(0x715863d165e633cc) },
    ExpectedEval { "8/8/4k3/8/8/8/3PK3/8 w - - 0 1", 145, UINT64_C(0x8ba218ba22a5d462) },
    ExpectedEval { "8/5k2/8/8/8/8/1N3B2/4K3 w - - 0 1", 1465, UINT64_C(0xa62b37afbf55f60b) },
    ExpectedEval { "4k3/8/8/8/8/8/8/4K2R w K - 0 1", 1322, UINT64_C(0x57e13e9832c08e58) },
    ExpectedEval { "1k6/8/8/8/8/8/QQQQ4/K7 b - - 0 1", -7570, UINT64_C(0xb001e34c870e7271) },
    ExpectedEval { "qqqqk3/8/8/8/8/8/8/4K3 w - - 0 1", -7629, UINT64_C(0x4fe19b09dd854302) },
    ExpectedEval { "4k3/pppppppp/8/8/8/8/PPPPPPPP/4K3 w - - 0 1", 0, UINT64_C(0xa05df782b9920400) },
    ExpectedEval { "8/1P4k1/8/8/8/8/6p1/K7 w - - 0 1", -48, UINT64_C(0xa67c3543c382432e) },
    ExpectedEval { "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b KQkq - 0 1", 0, UINT64_C(0xc929aeca0f04ec46) },
};
// clang-format on

} // namespace recorded_evals
