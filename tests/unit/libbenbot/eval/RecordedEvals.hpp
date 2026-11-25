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
    ExpectedEval { "r3k2r/2pb1ppp/2pp1q2/p7/1nP1B3/1P2P3/P2N1PPP/R2QK2R w KQkq a6 0 1", 51, UINT64_C(0xb5163fe1358d3659) },
    ExpectedEval { "4rrk1/2p1b1p1/p1p3q1/4p3/2P2n1p/1P1NR2P/PB3PP1/3R1QK1 b - - 0 1", -38, UINT64_C(0x93e1191145b479ca) },
    ExpectedEval { "r3qbrk/6p1/2b2pPp/p3pP1Q/PpPpP2P/3P1B2/2PB3K/R5R1 w - - 0 1", 184, UINT64_C(0x8c6de460ddf666dd) },
    ExpectedEval { "6k1/1R3p2/6p1/2Bp3p/3P2q1/P7/1P2rQ1K/5R2 b - - 0 1", -965, UINT64_C(0x71f796713db93171) },
    ExpectedEval { "8/8/1p2k1p1/3p3p/1p1P1P1P/1P2PK2/8/8 w - - 0 1", -39, UINT64_C(0x5436e76bd35a7a7b) },
    ExpectedEval { "7r/2p3k1/1p1p1qp1/1P1Bp3/p1P2r1P/P7/4R3/Q4RK1 w - - 0 1", 301, UINT64_C(0x1b2a3dccc504ed9f) },
    ExpectedEval { "r1bq1rk1/pp2b1pp/n1pp1n2/3P1p2/2P1p3/2N1P2N/PP2BPPP/R1BQ1RK1 b - - 0 1", -33, UINT64_C(0x3861e631f13d66c7) },
    ExpectedEval { "3r3k/2r4p/1p1b3q/p4P2/P2Pp3/1B2P3/3BQ1RP/6K1 w - - 0 1", -28, UINT64_C(0x666a3cc5e2e06eed) },
    ExpectedEval { "2r4r/1p4k1/1Pnp4/3Qb1pq/8/4BpPp/5P2/2RR1BK1 w - - 0 1", -227, UINT64_C(0x5da75d430b3f07c9) },
    ExpectedEval { "4q1bk/6b1/7p/p1p4p/PNPpP2P/KN4P1/3Q4/4R3 b - - 0 1", -600, UINT64_C(0xa08d097cec582c76) },
    ExpectedEval { "2q3r1/1r2pk2/pp3pp1/2pP3p/P1Pb1BbP/1P4Q1/R3NPP1/4R1K1 w - - 0 1", 92, UINT64_C(0x65b0d365ef48359b) },
    ExpectedEval { "1r2r2k/1b4q1/pp5p/2pPp1p1/P3Pn2/1P1B1Q1P/2R3P1/4BR1K b - - 0 1", -181, UINT64_C(0x4af937134d381616) },
    ExpectedEval { "r3kbbr/pp1n1p1P/3ppnp1/q5N1/1P1pP3/P1N1B3/2P1QP2/R3KB1R b KQkq b3 0 1", 228, UINT64_C(0xabe900b603a0ea07) },
    ExpectedEval { "8/6pk/2b1Rp2/3r4/1R1B2PP/P5K1/8/2r5 b - - 0 1", -75, UINT64_C(0xad139b7bf4da689e) },
    ExpectedEval { "1r4k1/4ppb1/2n1b1qp/pB4p1/1n1BP1P1/7P/2PNQPK1/3RN3 w - - 0 1", 53, UINT64_C(0x9ac5df3f8a17f91f) },
    ExpectedEval { "8/p2B4/PkP5/4p1pK/4Pb1p/5P2/8/8 w - - 0 1", 197, UINT64_C(0x03834c544a063b45) },
    ExpectedEval { "3r4/ppq1ppkp/4bnp1/2pN4/2P1P3/1P4P1/PQ3PBP/R4K2 b - - 0 1", 77, UINT64_C(0x0ab4d6aba68050b1) },
    ExpectedEval { "5rr1/4n2k/4q2P/P1P2n2/3B1p2/4pP2/2N1P3/1RR1K2Q w - - 0 1", 122, UINT64_C(0xb1bbfb99416b76ce) },
    ExpectedEval { "1r5k/2pq2p1/3p3p/p1pP4/4QP2/PP1R3P/6PK/8 w - - 0 1", 17, UINT64_C(0x08859227af5cff31) },
    ExpectedEval { "q5k1/5ppp/1r3bn1/1B6/P1N2P2/BQ2P1P1/5K1P/8 b - - 0 1", 10, UINT64_C(0x10d5b0ab64236589) },
    ExpectedEval { "r1b2k1r/5n2/p4q2/1ppn1Pp1/3pp1p1/NP2P3/P1PPBK2/1RQN2R1 w - - 0 1", -305, UINT64_C(0xf33b64947369265e) },
    ExpectedEval { "r1bqk2r/pppp1ppp/5n2/4b3/4P3/P1N5/1PP2PPP/R1BQKB1R w KQkq - 0 1", 101, UINT64_C(0x675f451a6807b1a9) },
    ExpectedEval { "r1bqr1k1/pp1p1ppp/2p5/8/3N1Q2/P2BB3/1PP2PPP/R3K2n b Q - 0 1", 128, UINT64_C(0xde7b142fd323a11d) },
    ExpectedEval { "r1bq2k1/p4r1p/1pp2pp1/3p4/1P1B3Q/P2B1N2/2P3PP/4R1K1 b - - 0 1", -238, UINT64_C(0x71ade89956d08466) },
    ExpectedEval { "r4qk1/6r1/1p4p1/2ppBbN1/1p5Q/P7/2P3PP/5RK1 w - - 0 1", -140, UINT64_C(0xc359c53513fed015) },
    ExpectedEval { "r7/6k1/1p6/2pp1p2/7Q/8/p1P2K1P/8 w - - 0 1", 21, UINT64_C(0x9363a6ce1b83b5b7) },
    ExpectedEval { "r3k2r/ppp1pp1p/2nqb1pn/3p4/4P3/2PP4/PP1NBPPP/R2QK1NR w KQkq - 0 1", 17, UINT64_C(0x99cd97b10702096a) },
    ExpectedEval { "3r1rk1/1pp1pn1p/p1n1q1p1/3p4/Q3P3/2P5/PP1NBPPP/4RRK1 w - - 0 1", -19, UINT64_C(0x446b8731cfe3ab30) },
    ExpectedEval { "5rk1/1pp1pn1p/p3Brp1/8/1n6/5N2/PP3PPP/2R2RK1 w - - 0 1", -17, UINT64_C(0xa37db3208a962bdf) },
    ExpectedEval { "8/1p2pk1p/p1p1r1p1/3n4/8/5R2/PP3PPP/4R1K1 b - - 0 1", -141, UINT64_C(0x8b5e48d75f9c67a2) },
    ExpectedEval { "8/4pk2/1p1r2p1/p1p4p/Pn5P/3R4/1P3PP1/4RK2 w - - 0 1", 155, UINT64_C(0x36389e9105a9e36c) },
    ExpectedEval { "8/5k2/1pnrp1p1/p1p4p/P6P/4R1PK/1P3P2/4R3 b - - 0 1", -213, UINT64_C(0x74683496efc8c384) },
    ExpectedEval { "8/8/1p1kp1p1/p1pr1n1p/P6P/1R4P1/1P3PK1/1R6 b - - 0 1", -134, UINT64_C(0x9abb7f3ef7b8da8b) },
    ExpectedEval { "8/8/1p1k2p1/p1prp2p/P2n3P/6P1/1P1R1PK1/4R3 b - - 0 1", -282, UINT64_C(0xda7e69fe076ee110) },
    ExpectedEval { "8/8/1p4p1/p1p2k1p/P2npP1P/4K1P1/1P6/3R4 w - - 0 1", 73, UINT64_C(0xd104d4be1d0ddffb) },
    ExpectedEval { "8/8/1p4p1/p1p2k1p/P2n1P1P/4K1P1/1P6/6R1 b - - 0 1", -259, UINT64_C(0xac2e060291d67bf3) },
    ExpectedEval { "8/5k2/1p4p1/p1pK3p/P2n1P1P/6P1/1P6/4R3 b - - 0 1", -308, UINT64_C(0x9df7d9fe2f6c5a3a) },
    ExpectedEval { "8/1R6/1p1K1kp1/p6p/P1p2P1P/6P1/1Pn5/8 w - - 0 1", 332, UINT64_C(0xe346f3d4015d4649) },
    ExpectedEval { "1rb1rn1k/p3q1bp/2p3p1/2p1p3/2P1P2N/PP1RQNP1/1B3P2/4R1K1 b - - 0 1", -181, UINT64_C(0xd8aa3899c89fc3cc) },
    ExpectedEval { "4rrk1/pp1n1pp1/q5p1/P1pP4/2n3P1/7P/1P3PB1/R1BQ1RK1 w - - 0 1", 31, UINT64_C(0x704e26f951871392) },
    ExpectedEval { "r2qr1k1/pb1nbppp/1pn1p3/2ppP3/3P4/2PB1NN1/PP3PPP/R1BQR1K1 w - - 0 1", 87, UINT64_C(0xeef2a24b3f1fcfad) },
    ExpectedEval { "2r2k2/8/4P1R1/1p6/8/P4K1N/7b/2B5 b - - 0 1", -724, UINT64_C(0x2505853c5341ff27) },
    ExpectedEval { "6k1/5pp1/8/2bKP2P/2P5/p4PNb/B7/8 b - - 0 1", 74, UINT64_C(0x6634313bc1094a40) },
    ExpectedEval { "2rqr1k1/1p3p1p/p2p2p1/P1nPb3/2B1P3/5P2/1PQ2NPP/R1R4K w - - 0 1", 52, UINT64_C(0xa6bc63e3145d0549) },
    ExpectedEval { "r1b2rk1/p1q1ppbp/6p1/2Q5/8/4BP2/PPP3PP/2KR1B1R b - - 0 1", -98, UINT64_C(0x7e2ddb27692d4eaf) },
    ExpectedEval { "6r1/5k2/p1b1r2p/1pB1p1p1/1Pp3PP/2P1R1K1/2P2P2/3R4 w - - 0 1", 40, UINT64_C(0x6d4905a744d3fbc2) },
    ExpectedEval { "rnbqkb1r/pppppppp/5n2/8/2PP4/8/PP2PPPP/RNBQKBNR b KQkq c3 0 1", -26, UINT64_C(0x637825236d5d473f) },
    ExpectedEval { "2rr2k1/1p4bp/p1q1p1p1/4Pp1n/2PB4/1PN3P1/P3Q2P/2RR2K1 w - f6 0 1", -7, UINT64_C(0x62a50de732ad31d4) },
    ExpectedEval { "3br1k1/p1pn3p/1p3n2/5pNq/2P1p3/1PN3PP/P2Q1PB1/4R1K1 w - - 0 1", 224, UINT64_C(0xe87c3f4201918690) },
    ExpectedEval { "2r2b2/5p2/5k2/p1r1pP2/P2pB3/1P3P2/K1P3R1/7R w - - 0 1", 158, UINT64_C(0xddf6b22edc4f7db3) },
    ExpectedEval { "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1", -67, UINT64_C(0x851ea7ae6d8c131e) },
    ExpectedEval { "r2qkb1r/ppp1p1pp/2n3p1/1b6/2BpP3/5Q2/PPPP1PPP/RNB1K1NR w KQkq - 0 1", 546, UINT64_C(0xbfc01d22c641c606) },
    ExpectedEval { "8/8/8/8/8/8/3q4/2K1k3 w - - 0 1", -2037, UINT64_C(0xea4f4e4705d71eea) },
    ExpectedEval { "8/2P5/8/8/k1K5/8/8/8 w - - 0 1", 511, UINT64_C(0xf661253719498686) },
    ExpectedEval { "r1b2rk1/pp1nqppp/2p1p3/3n4/2BP4/2N1PN2/PPQ2PPP/R4RK1 w - - 0 1", 149, UINT64_C(0xa8f5397fb54016d2) },
    ExpectedEval { "4k3/8/2r5/3P4/8/8/8/4K3 w - - 0 1", -1069, UINT64_C(0x41ec66522ddd33f4) },
    ExpectedEval { "r2qk2r/ppp2ppp/3n1b2/4P3/8/8/PPP2PPP/R2QK2R b KQkq - 0 1", 880, UINT64_C(0x62315d502f6dd80f) },
    ExpectedEval { "2r1k3/3q4/1N6/8/8/8/8/4K3 w - - 0 1", -1807, UINT64_C(0x91634577a9ddea60) },
    ExpectedEval { "6k1/8/4q3/8/8/8/4R3/6K1 w - - 0 1", -710, UINT64_C(0xc92537f9a94dda16) },
    ExpectedEval { "6k1/8/8/4r3/3P4/5N2/8/6K1 w - - 0 1", -228, UINT64_C(0x3bb982984fd1ada7) },
    ExpectedEval { "6k1/7p/4b3/8/8/8/4R3/6K1 w - - 0 1", 185, UINT64_C(0x64d7c5e36e9a2de3) },
    ExpectedEval { "6k1/7p/4b3/8/8/8/4R3/6K1 b - - 0 1", -185, UINT64_C(0x8d7bbd0eff0b0d10) },
    ExpectedEval { "8/8/4k3/4n3/2N5/4NN2/6K1/8 b - - 0 1", -816, UINT64_C(0x5536e639b2dd59c7) },
    ExpectedEval { "6k1/8/8/4n3/8/2BB4/8/6K1 w - - 0 1", 552, UINT64_C(0xe396fb1c2a397498) },
    ExpectedEval { "2b4k/1p1p4/8/8/8/8/1P1P4/K1B5 w - - 0 1", -5, UINT64_C(0x92a45248ea681873) },
    ExpectedEval { "2b4k/1p1p4/8/8/8/8/1P1P4/K4B2 w - - 0 1", 92, UINT64_C(0xa631ab38eb35513b) },
    ExpectedEval { "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 0, UINT64_C(0x31cddfa5d68cd4ae) },
    ExpectedEval { "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1", -20, UINT64_C(0x94993e1b1945cc1d) },
    ExpectedEval { "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1", 316, UINT64_C(0x1a87ce0fdc238388) },
    ExpectedEval { "6k1/5ppp/8/1P6/8/8/5PPP/1R4K1 b - - 0 1", -1463, UINT64_C(0x1105e09eec83c7f5) },
    ExpectedEval { "6k1/5ppp/8/1P6/8/8/5PPP/1R4K1 w - - 0 1", 1535, UINT64_C(0xe16f691da3c2b560) },
    ExpectedEval { "1r4k1/5ppp/8/1P6/8/8/5PPP/6K1 w - - 0 1", -1059, UINT64_C(0x138a0f2c728896f3) },
    ExpectedEval { "6k1/5ppp/8/1P6/8/8/5PPP/6KR w - - 0 1", 1395, UINT64_C(0xbf3721119cc98ec3) },
    ExpectedEval { "1k6/5ppp/8/1P6/8/8/5PPP/1R4K1 w - - 0 1", 1411, UINT64_C(0xee6218df812cfcc3) },
    ExpectedEval { "6k1/5ppp/8/1P6/K7/8/5PPP/1R6 w - - 0 1", 1588, UINT64_C(0x1a70349a9d6f7fc3) },
    ExpectedEval { "6k1/8/8/4n3/3P4/8/8/6K1 w - - 0 1", -740, UINT64_C(0xc43ac5aabd55c9b5) },
    ExpectedEval { "6k1/8/8/3n4/3P4/8/8/6K1 w - - 0 1", -752, UINT64_C(0xd97efc89aa235860) },
    ExpectedEval { "6k1/8/8/4n3/3P4/8/8/6K1 b - - 0 1", 814, UINT64_C(0x26ff846435ff0a75) },
    ExpectedEval { "6k1/8/8/3n4/3P4/8/8/6K1 b - - 0 1", 826, UINT64_C(0x9d216ba366e17d51) },
    ExpectedEval { "6k1/8/4r3/8/5N2/8/8/6K1 w - - 0 1", -346, UINT64_C(0xe80a21d034e2275c) },
    ExpectedEval { "6k1/8/3r4/8/5N2/8/8/6K1 w - - 0 1", -352, UINT64_C(0xab0f1b28db4b7836) },
    ExpectedEval { "6k1/8/4r3/8/2B5/8/8/6K1 b - - 0 1", 306, UINT64_C(0xb1e3c0a2cdfc7b38) },
    ExpectedEval { "6k1/8/3r4/8/2B5/8/8/6K1 b - - 0 1", 319, UINT64_C(0x500ba5eac5a33d07) },
    ExpectedEval { "6k1/8/4q3/6N1/8/8/PPP5/1K6 w - - 0 1", -593, UINT64_C(0x3b86f5aa0854d5d9) },
    ExpectedEval { "6k1/8/3q4/6N1/8/8/PPP5/1K6 w - - 0 1", -570, UINT64_C(0xd171e50ac949be37) },
    ExpectedEval { "6k1/8/3q4/8/8/8/4R3/6K1 w - - 0 1", -680, UINT64_C(0x8da609f8cd70ad21) },
    ExpectedEval { "7k/8/3p4/3n4/8/4NN2/5K2/8 w - - 0 1", 431, UINT64_C(0x4f9fb458bf4a6896) },
    ExpectedEval { "7k/8/3p4/4n3/8/4NN2/5K2/8 w - - 0 1", 410, UINT64_C(0xdd260b2cd535f480) },
    ExpectedEval { "7k/8/2n5/3n4/8/4NN2/5K2/8 w - - 0 1", 57, UINT64_C(0x8deeb0005ac7e46b) },
    ExpectedEval { "7k/8/2n5/4n3/8/4NN2/5K2/8 w - - 0 1", 10, UINT64_C(0xefcab6f116aac794) },
    ExpectedEval { "8/8/4k3/4n3/2N5/4NN2/6K1/8 w - - 0 1", 851, UINT64_C(0xb1c06f04a496fb84) },
    ExpectedEval { "8/8/4k3/3n4/2N5/4NN2/6K1/8 w - - 0 1", 827, UINT64_C(0x10118dc734a1def3) },
    ExpectedEval { "8/8/8/4k3/8/8/8/KQ6 w - - 0 1", 1814, UINT64_C(0xf5e2b59fb262185c) },
    ExpectedEval { "8/8/8/3k4/8/8/2K5/7R b - - 0 1", -1189, UINT64_C(0x6e272aaf7956b48c) },
    ExpectedEval { "8/8/4k3/8/8/8/3PK3/8 w - - 0 1", 105, UINT64_C(0x2737287ff36b95b9) },
    ExpectedEval { "8/5k2/8/8/8/8/1N3B2/4K3 w - - 0 1", 1323, UINT64_C(0x37a959cfced5f46c) },
    ExpectedEval { "4k3/8/8/8/8/8/8/4K2R w K - 0 1", 1234, UINT64_C(0x9cccfb4a05e22980) },
    ExpectedEval { "1k6/8/8/8/8/8/QQQQ4/K7 b - - 0 1", -6946, UINT64_C(0xb008934c8719cf81) },
    ExpectedEval { "qqqqk3/8/8/8/8/8/8/4K3 w - - 0 1", -6944, UINT64_C(0x78601812dbc0c13b) },
    ExpectedEval { "4k3/pppppppp/8/8/8/8/PPPPPPPP/4K3 w - - 0 1", 0, UINT64_C(0x9ec1406e6b338682) },
    ExpectedEval { "8/1P4k1/8/8/8/8/6p1/K7 w - - 0 1", -87, UINT64_C(0xa69de0c52dd5bcce) },
    ExpectedEval { "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b KQkq - 0 1", 0, UINT64_C(0x31cddfa5d68cd4ae) },
};
// clang-format on

} // namespace recorded_evals
