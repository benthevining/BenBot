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
    ExpectedEval { "r3k2r/2pb1ppp/2pp1q2/p7/1nP1B3/1P2P3/P2N1PPP/R2QK2R w KQkq a6 0 1", 55, UINT64_C(0x4a2752017c6bd349) },
    ExpectedEval { "4rrk1/2p1b1p1/p1p3q1/4p3/2P2n1p/1P1NR2P/PB3PP1/3R1QK1 b - - 0 1", -231, UINT64_C(0xa51af6372660730a) },
    ExpectedEval { "r3qbrk/6p1/2b2pPp/p3pP1Q/PpPpP2P/3P1B2/2PB3K/R5R1 w - - 0 1", 164, UINT64_C(0x74eae74c025c7f87) },
    ExpectedEval { "6k1/1R3p2/6p1/2Bp3p/3P2q1/P7/1P2rQ1K/5R2 b - - 0 1", -577, UINT64_C(0x4d3f8f38f030948f) },
    ExpectedEval { "8/8/1p2k1p1/3p3p/1p1P1P1P/1P2PK2/8/8 w - - 0 1", 31, UINT64_C(0xca4816879db19fdd) },
    ExpectedEval { "7r/2p3k1/1p1p1qp1/1P1Bp3/p1P2r1P/P7/4R3/Q4RK1 w - - 0 1", -33, UINT64_C(0x2f261a67481b66e2) },
    ExpectedEval { "r1bq1rk1/pp2b1pp/n1pp1n2/3P1p2/2P1p3/2N1P2N/PP2BPPP/R1BQ1RK1 b - - 0 1", -20, UINT64_C(0x3e8f715c27a71cb3) },
    ExpectedEval { "3r3k/2r4p/1p1b3q/p4P2/P2Pp3/1B2P3/3BQ1RP/6K1 w - - 0 1", 68, UINT64_C(0x6ce0c099774bac1c) },
    ExpectedEval { "2r4r/1p4k1/1Pnp4/3Qb1pq/8/4BpPp/5P2/2RR1BK1 w - - 0 1", -372, UINT64_C(0x1603bc1ea8501fb5) },
    ExpectedEval { "4q1bk/6b1/7p/p1p4p/PNPpP2P/KN4P1/3Q4/4R3 b - - 0 1", -535, UINT64_C(0xa458e5a89c06fd4d) },
    ExpectedEval { "2q3r1/1r2pk2/pp3pp1/2pP3p/P1Pb1BbP/1P4Q1/R3NPP1/4R1K1 w - - 0 1", 140, UINT64_C(0xe760790b28ffc7cb) },
    ExpectedEval { "1r2r2k/1b4q1/pp5p/2pPp1p1/P3Pn2/1P1B1Q1P/2R3P1/4BR1K b - - 0 1", -199, UINT64_C(0x2e34cd720faedf37) },
    ExpectedEval { "r3kbbr/pp1n1p1P/3ppnp1/q5N1/1P1pP3/P1N1B3/2P1QP2/R3KB1R b KQkq b3 0 1", -157, UINT64_C(0x49b4e5501f09bd2f) },
    ExpectedEval { "8/6pk/2b1Rp2/3r4/1R1B2PP/P5K1/8/2r5 b - - 0 1", -134, UINT64_C(0x5fc26a3ffaefc6f4) },
    ExpectedEval { "1r4k1/4ppb1/2n1b1qp/pB4p1/1n1BP1P1/7P/2PNQPK1/3RN3 w - - 0 1", -50, UINT64_C(0x21dfd01bd341ce95) },
    ExpectedEval { "8/p2B4/PkP5/4p1pK/4Pb1p/5P2/8/8 w - - 0 1", 2, UINT64_C(0xb3ca21a42f69e14c) },
    ExpectedEval { "3r4/ppq1ppkp/4bnp1/2pN4/2P1P3/1P4P1/PQ3PBP/R4K2 b - - 0 1", -6, UINT64_C(0x60d55241cba8eaae) },
    ExpectedEval { "5rr1/4n2k/4q2P/P1P2n2/3B1p2/4pP2/2N1P3/1RR1K2Q w - - 0 1", 444, UINT64_C(0xd966b8f376565408) },
    ExpectedEval { "1r5k/2pq2p1/3p3p/p1pP4/4QP2/PP1R3P/6PK/8 w - - 0 1", 1, UINT64_C(0x9a381817c8e30a79) },
    ExpectedEval { "q5k1/5ppp/1r3bn1/1B6/P1N2P2/BQ2P1P1/5K1P/8 b - - 0 1", -93, UINT64_C(0x2e515d924b05562a) },
    ExpectedEval { "r1b2k1r/5n2/p4q2/1ppn1Pp1/3pp1p1/NP2P3/P1PPBK2/1RQN2R1 w - - 0 1", -404, UINT64_C(0x5acd5949e8e68eca) },
    ExpectedEval { "r1bqk2r/pppp1ppp/5n2/4b3/4P3/P1N5/1PP2PPP/R1BQKB1R w KQkq - 0 1", 70, UINT64_C(0x475005ce5ec832ab) },
    ExpectedEval { "r1bqr1k1/pp1p1ppp/2p5/8/3N1Q2/P2BB3/1PP2PPP/R3K2n b Q - 0 1", 186, UINT64_C(0xffb5d98a4799e5d3) },
    ExpectedEval { "r1bq2k1/p4r1p/1pp2pp1/3p4/1P1B3Q/P2B1N2/2P3PP/4R1K1 b - - 0 1", -31, UINT64_C(0x2ed21fc715c748bb) },
    ExpectedEval { "r4qk1/6r1/1p4p1/2ppBbN1/1p5Q/P7/2P3PP/5RK1 w - - 0 1", -259, UINT64_C(0xab4f0661c17cd36d) },
    ExpectedEval { "r7/6k1/1p6/2pp1p2/7Q/8/p1P2K1P/8 w - - 0 1", -202, UINT64_C(0x3fb20069f972ecdb) },
    ExpectedEval { "r3k2r/ppp1pp1p/2nqb1pn/3p4/4P3/2PP4/PP1NBPPP/R2QK1NR w KQkq - 0 1", -35, UINT64_C(0x520e02216df8119c) },
    ExpectedEval { "3r1rk1/1pp1pn1p/p1n1q1p1/3p4/Q3P3/2P5/PP1NBPPP/4RRK1 w - - 0 1", -42, UINT64_C(0x200b66697cc8322f) },
    ExpectedEval { "5rk1/1pp1pn1p/p3Brp1/8/1n6/5N2/PP3PPP/2R2RK1 w - - 0 1", -2, UINT64_C(0x9c5fe7e7b9541ed4) },
    ExpectedEval { "8/1p2pk1p/p1p1r1p1/3n4/8/5R2/PP3PPP/4R1K1 b - - 0 1", -140, UINT64_C(0xcab32c03f8ece2ce) },
    ExpectedEval { "8/4pk2/1p1r2p1/p1p4p/Pn5P/3R4/1P3PP1/4RK2 w - - 0 1", 132, UINT64_C(0x6beb4f0efe87cac8) },
    ExpectedEval { "8/5k2/1pnrp1p1/p1p4p/P6P/4R1PK/1P3P2/4R3 b - - 0 1", -115, UINT64_C(0xa4b5c6626500aae6) },
    ExpectedEval { "8/8/1p1kp1p1/p1pr1n1p/P6P/1R4P1/1P3PK1/1R6 b - - 0 1", -63, UINT64_C(0x9a0d8fdcd13b02ab) },
    ExpectedEval { "8/8/1p1k2p1/p1prp2p/P2n3P/6P1/1P1R1PK1/4R3 b - - 0 1", -146, UINT64_C(0x08dd1c9df7dbea8b) },
    ExpectedEval { "8/8/1p4p1/p1p2k1p/P2npP1P/4K1P1/1P6/3R4 w - - 0 1", 55, UINT64_C(0x8af6cb4bf8268fe1) },
    ExpectedEval { "8/8/1p4p1/p1p2k1p/P2n1P1P/4K1P1/1P6/6R1 b - - 0 1", -164, UINT64_C(0xd6d79486854010a7) },
    ExpectedEval { "8/5k2/1p4p1/p1pK3p/P2n1P1P/6P1/1P6/4R3 b - - 0 1", -290, UINT64_C(0xae93ffba47359c6c) },
    ExpectedEval { "8/1R6/1p1K1kp1/p6p/P1p2P1P/6P1/1Pn5/8 w - - 0 1", 234, UINT64_C(0x50468a0c6435c3fa) },
    ExpectedEval { "1rb1rn1k/p3q1bp/2p3p1/2p1p3/2P1P2N/PP1RQNP1/1B3P2/4R1K1 b - - 0 1", -294, UINT64_C(0xba427bc134873859) },
    ExpectedEval { "4rrk1/pp1n1pp1/q5p1/P1pP4/2n3P1/7P/1P3PB1/R1BQ1RK1 w - - 0 1", -31, UINT64_C(0xfb59bdf32604c1ec) },
    ExpectedEval { "r2qr1k1/pb1nbppp/1pn1p3/2ppP3/3P4/2PB1NN1/PP3PPP/R1BQR1K1 w - - 0 1", 62, UINT64_C(0x93f136c774c167e3) },
    ExpectedEval { "2r2k2/8/4P1R1/1p6/8/P4K1N/7b/2B5 b - - 0 1", -505, UINT64_C(0x62bbae0906160b7c) },
    ExpectedEval { "6k1/5pp1/8/2bKP2P/2P5/p4PNb/B7/8 b - - 0 1", -49, UINT64_C(0xef6a6f29cc0888e3) },
    ExpectedEval { "2rqr1k1/1p3p1p/p2p2p1/P1nPb3/2B1P3/5P2/1PQ2NPP/R1R4K w - - 0 1", 121, UINT64_C(0x455c9e0f89215ec4) },
    ExpectedEval { "r1b2rk1/p1q1ppbp/6p1/2Q5/8/4BP2/PPP3PP/2KR1B1R b - - 0 1", -323, UINT64_C(0xafc8723ec98b1587) },
    ExpectedEval { "6r1/5k2/p1b1r2p/1pB1p1p1/1Pp3PP/2P1R1K1/2P2P2/3R4 w - - 0 1", -23, UINT64_C(0xded15ca774331b15) },
    ExpectedEval { "rnbqkb1r/pppppppp/5n2/8/2PP4/8/PP2PPPP/RNBQKBNR b KQkq c3 0 1", 52, UINT64_C(0xec93d4ad2b1f9f6c) },
    ExpectedEval { "2rr2k1/1p4bp/p1q1p1p1/4Pp1n/2PB4/1PN3P1/P3Q2P/2RR2K1 w - f6 0 1", 25, UINT64_C(0x8d307bd024688dca) },
    ExpectedEval { "3br1k1/p1pn3p/1p3n2/5pNq/2P1p3/1PN3PP/P2Q1PB1/4R1K1 w - - 0 1", 80, UINT64_C(0xc71812b31c482b49) },
    ExpectedEval { "2r2b2/5p2/5k2/p1r1pP2/P2pB3/1P3P2/K1P3R1/7R w - - 0 1", 147, UINT64_C(0x817367444685eed0) },
    ExpectedEval { "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1", 67, UINT64_C(0x33fdd93f2c6dfa1a) },
    ExpectedEval { "r2qkb1r/ppp1p1pp/2n3p1/1b6/2BpP3/5Q2/PPPP1PPP/RNB1K1NR w KQkq - 0 1", 340, UINT64_C(0xb2c6b5bb7940e702) },
    ExpectedEval { "8/8/8/8/8/8/3q4/2K1k3 w - - 0 1", -1426, UINT64_C(0xc023179970d07bff) },
    ExpectedEval { "8/2P5/8/8/k1K5/8/8/8 w - - 0 1", 302, UINT64_C(0x94d63cb87cab564f) },
    ExpectedEval { "r1b2rk1/pp1nqppp/2p1p3/3n4/2BP4/2N1PN2/PPQ2PPP/R4RK1 w - - 0 1", 103, UINT64_C(0xbaecd6142178062d) },
    ExpectedEval { "4k3/8/2r5/3P4/8/8/8/4K3 w - - 0 1", -858, UINT64_C(0xdea6ba4d7756b4a5) },
    ExpectedEval { "r2qk2r/ppp2ppp/3n1b2/4P3/8/8/PPP2PPP/R2QK2R b KQkq - 0 1", 480, UINT64_C(0xe457a00750255a01) },
    ExpectedEval { "2r1k3/3q4/1N6/8/8/8/8/4K3 w - - 0 1", -1111, UINT64_C(0xbaa185b66b97395d) },
    ExpectedEval { "6k1/8/4q3/8/8/8/4R3/6K1 w - - 0 1", -310, UINT64_C(0xd80132c9031ef189) },
    ExpectedEval { "6k1/8/8/4r3/3P4/5N2/8/6K1 w - - 0 1", 3, UINT64_C(0x3418fcbcb3b84de4) },
    ExpectedEval { "6k1/7p/4b3/8/8/8/4R3/6K1 w - - 0 1", 115, UINT64_C(0x9a0a1feb9e14778e) },
    ExpectedEval { "6k1/7p/4b3/8/8/8/4R3/6K1 b - - 0 1", -115, UINT64_C(0x5337348d4fa6085e) },
    ExpectedEval { "8/8/4k3/4n3/2N5/4NN2/6K1/8 b - - 0 1", -666, UINT64_C(0x7fe2ad12d8290ec2) },
    ExpectedEval { "6k1/8/8/4n3/8/2BB4/8/6K1 w - - 0 1", 432, UINT64_C(0xccac336e1a1b5809) },
    ExpectedEval { "2b4k/1p1p4/8/8/8/8/1P1P4/K1B5 w - - 0 1", 4, UINT64_C(0x904489abd9b1ae55) },
    ExpectedEval { "2b4k/1p1p4/8/8/8/8/1P1P4/K4B2 w - - 0 1", 34, UINT64_C(0xc009990bcb29350b) },
    ExpectedEval { "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 0, UINT64_C(0xa480a7ed89724e0e) },
    ExpectedEval { "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1", -111, UINT64_C(0x82cb42594f342709) },
    ExpectedEval { "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1", -8, UINT64_C(0x231d3d0bd969ca38) },
    ExpectedEval { "6k1/5ppp/8/1P6/8/8/5PPP/1R4K1 b - - 0 1", -1225, UINT64_C(0x067afff477ea6c25) },
    ExpectedEval { "6k1/5ppp/8/1P6/8/8/5PPP/1R4K1 w - - 0 1", 1297, UINT64_C(0x4581814f05a2bbb4) },
    ExpectedEval { "1r4k1/5ppp/8/1P6/8/8/5PPP/6K1 w - - 0 1", -895, UINT64_C(0x5045f8cc31030773) },
    ExpectedEval { "6k1/5ppp/8/1P6/8/8/5PPP/6KR w - - 0 1", 1203, UINT64_C(0xe83bcfad92cb453e) },
    ExpectedEval { "1k6/5ppp/8/1P6/8/8/5PPP/1R4K1 w - - 0 1", 1239, UINT64_C(0x956d941c76cbe190) },
    ExpectedEval { "6k1/5ppp/8/1P6/K7/8/5PPP/1R6 w - - 0 1", 1297, UINT64_C(0xcf54fc51789c4854) },
    ExpectedEval { "6k1/8/8/4n3/3P4/8/8/6K1 w - - 0 1", -643, UINT64_C(0xe3ce7f78ca294864) },
    ExpectedEval { "6k1/8/8/3n4/3P4/8/8/6K1 w - - 0 1", -687, UINT64_C(0x94614757b1d0a548) },
    ExpectedEval { "6k1/8/8/4n3/3P4/8/8/6K1 b - - 0 1", 718, UINT64_C(0x9555766bee32f434) },
    ExpectedEval { "6k1/8/8/3n4/3P4/8/8/6K1 b - - 0 1", 761, UINT64_C(0x433bfe68ae9cdbd6) },
    ExpectedEval { "6k1/8/4r3/8/5N2/8/8/6K1 w - - 0 1", -194, UINT64_C(0x6fc42c018c6e5f3f) },
    ExpectedEval { "6k1/8/3r4/8/5N2/8/8/6K1 w - - 0 1", -244, UINT64_C(0x5fb3519782f7d1f4) },
    ExpectedEval { "6k1/8/4r3/8/2B5/8/8/6K1 b - - 0 1", 190, UINT64_C(0x5b9576f3bd9ee195) },
    ExpectedEval { "6k1/8/3r4/8/2B5/8/8/6K1 b - - 0 1", 228, UINT64_C(0x4ab40193a4e7e052) },
    ExpectedEval { "6k1/8/4q3/6N1/8/8/PPP5/1K6 w - - 0 1", -73, UINT64_C(0x2ba72d568a232cba) },
    ExpectedEval { "6k1/8/3q4/6N1/8/8/PPP5/1K6 w - - 0 1", -113, UINT64_C(0x78be2904f9ab4706) },
    ExpectedEval { "6k1/8/3q4/8/8/8/4R3/6K1 w - - 0 1", -330, UINT64_C(0x3b310d5db874ccc4) },
    ExpectedEval { "7k/8/3p4/3n4/8/4NN2/5K2/8 w - - 0 1", 341, UINT64_C(0x7faeb2b0aa507421) },
    ExpectedEval { "7k/8/3p4/4n3/8/4NN2/5K2/8 w - - 0 1", 321, UINT64_C(0xdcf82f7ea31ae4d1) },
    ExpectedEval { "7k/8/2n5/3n4/8/4NN2/5K2/8 w - - 0 1", 58, UINT64_C(0x7c52ebd0cb3630f9) },
    ExpectedEval { "7k/8/2n5/4n3/8/4NN2/5K2/8 w - - 0 1", 48, UINT64_C(0xefe9f91bc10d2bf9) },
    ExpectedEval { "8/8/4k3/4n3/2N5/4NN2/6K1/8 w - - 0 1", 701, UINT64_C(0xea425c9c4d2fc18c) },
    ExpectedEval { "8/8/4k3/3n4/2N5/4NN2/6K1/8 w - - 0 1", 684, UINT64_C(0xf510767e07a25e40) },
    ExpectedEval { "8/8/8/4k3/8/8/8/KQ6 w - - 0 1", 1366, UINT64_C(0x5ae9538d71415524) },
    ExpectedEval { "8/8/8/3k4/8/8/2K5/7R b - - 0 1", -1069, UINT64_C(0x903a4f87f9cbd6d8) },
    ExpectedEval { "8/8/4k3/8/8/8/3PK3/8 w - - 0 1", 83, UINT64_C(0x22c377f0299a9559) },
    ExpectedEval { "8/5k2/8/8/8/8/1N3B2/4K3 w - - 0 1", 1168, UINT64_C(0x4c10eb60bf6ba3ec) },
    ExpectedEval { "4k3/8/8/8/8/8/8/4K2R w K - 0 1", 1132, UINT64_C(0x44789ebdcc725a0a) },
    ExpectedEval { "1k6/8/8/8/8/8/QQQQ4/K7 b - - 0 1", -4175, UINT64_C(0xb00d2c4c87219f7c) },
    ExpectedEval { "qqqqk3/8/8/8/8/8/8/4K3 w - - 0 1", -4155, UINT64_C(0x805ddad2a406da31) },
    ExpectedEval { "4k3/pppppppp/8/8/8/8/PPPPPPPP/4K3 w - - 0 1", 0, UINT64_C(0xd75d85b470619af7) },
    ExpectedEval { "8/1P4k1/8/8/8/8/6p1/K7 w - - 0 1", -22, UINT64_C(0x5720c0a405649c55) },
    ExpectedEval { "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b KQkq - 0 1", 0, UINT64_C(0xa480a7ed89724e0e) },
};
// clang-format on

} // namespace recorded_evals
