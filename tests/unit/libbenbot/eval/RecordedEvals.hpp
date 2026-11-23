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
    ExpectedEval { "r3k2r/2pb1ppp/2pp1q2/p7/1nP1B3/1P2P3/P2N1PPP/R2QK2R w KQkq a6 0 1", 27, UINT64_C(0xe8035c354626f74f) },
    ExpectedEval { "4rrk1/2p1b1p1/p1p3q1/4p3/2P2n1p/1P1NR2P/PB3PP1/3R1QK1 b - - 0 1", -88, UINT64_C(0x1b0eef3cc5f4fd8a) },
    ExpectedEval { "r3qbrk/6p1/2b2pPp/p3pP1Q/PpPpP2P/3P1B2/2PB3K/R5R1 w - - 0 1", 169, UINT64_C(0xbc00cb61771e561b) },
    ExpectedEval { "6k1/1R3p2/6p1/2Bp3p/3P2q1/P7/1P2rQ1K/5R2 b - - 0 1", -963, UINT64_C(0xe631b494aeb8775e) },
    ExpectedEval { "8/8/1p2k1p1/3p3p/1p1P1P1P/1P2PK2/8/8 w - - 0 1", -38, UINT64_C(0xc759f629ef150289) },
    ExpectedEval { "7r/2p3k1/1p1p1qp1/1P1Bp3/p1P2r1P/P7/4R3/Q4RK1 w - - 0 1", 302, UINT64_C(0xbca99247ef7c0404) },
    ExpectedEval { "r1bq1rk1/pp2b1pp/n1pp1n2/3P1p2/2P1p3/2N1P2N/PP2BPPP/R1BQ1RK1 b - - 0 1", -30, UINT64_C(0xf28e015238094ff3) },
    ExpectedEval { "3r3k/2r4p/1p1b3q/p4P2/P2Pp3/1B2P3/3BQ1RP/6K1 w - - 0 1", -18, UINT64_C(0x34eda23a08d7b90a) },
    ExpectedEval { "2r4r/1p4k1/1Pnp4/3Qb1pq/8/4BpPp/5P2/2RR1BK1 w - - 0 1", -215, UINT64_C(0x8a07a8e00ca69804) },
    ExpectedEval { "4q1bk/6b1/7p/p1p4p/PNPpP2P/KN4P1/3Q4/4R3 b - - 0 1", -600, UINT64_C(0x2f32972275a1fd84) },
    ExpectedEval { "2q3r1/1r2pk2/pp3pp1/2pP3p/P1Pb1BbP/1P4Q1/R3NPP1/4R1K1 w - - 0 1", 103, UINT64_C(0xaca5d0903d68675f) },
    ExpectedEval { "1r2r2k/1b4q1/pp5p/2pPp1p1/P3Pn2/1P1B1Q1P/2R3P1/4BR1K b - - 0 1", -157, UINT64_C(0x0892112ea15e7658) },
    ExpectedEval { "r3kbbr/pp1n1p1P/3ppnp1/q5N1/1P1pP3/P1N1B3/2P1QP2/R3KB1R b KQkq b3 0 1", 220, UINT64_C(0xdb41de00f24a07dc) },
    ExpectedEval { "8/6pk/2b1Rp2/3r4/1R1B2PP/P5K1/8/2r5 b - - 0 1", -74, UINT64_C(0x853199f62deaa55c) },
    ExpectedEval { "1r4k1/4ppb1/2n1b1qp/pB4p1/1n1BP1P1/7P/2PNQPK1/3RN3 w - - 0 1", 16, UINT64_C(0x62bce43677636548) },
    ExpectedEval { "8/p2B4/PkP5/4p1pK/4Pb1p/5P2/8/8 w - - 0 1", 193, UINT64_C(0x32bc5c5c1fc098ba) },
    ExpectedEval { "3r4/ppq1ppkp/4bnp1/2pN4/2P1P3/1P4P1/PQ3PBP/R4K2 b - - 0 1", 103, UINT64_C(0xb3717d3562f82c23) },
    ExpectedEval { "5rr1/4n2k/4q2P/P1P2n2/3B1p2/4pP2/2N1P3/1RR1K2Q w - - 0 1", 140, UINT64_C(0x5d4d8cb2a8504e54) },
    ExpectedEval { "1r5k/2pq2p1/3p3p/p1pP4/4QP2/PP1R3P/6PK/8 w - - 0 1", 14, UINT64_C(0x6e7ca7c5dc243ebd) },
    ExpectedEval { "q5k1/5ppp/1r3bn1/1B6/P1N2P2/BQ2P1P1/5K1P/8 b - - 0 1", 72, UINT64_C(0x987798215749444c) },
    ExpectedEval { "r1b2k1r/5n2/p4q2/1ppn1Pp1/3pp1p1/NP2P3/P1PPBK2/1RQN2R1 w - - 0 1", -270, UINT64_C(0xfaa99e7b102f16a3) },
    ExpectedEval { "r1bqk2r/pppp1ppp/5n2/4b3/4P3/P1N5/1PP2PPP/R1BQKB1R w KQkq - 0 1", 93, UINT64_C(0xdb10c9136480dd5e) },
    ExpectedEval { "r1bqr1k1/pp1p1ppp/2p5/8/3N1Q2/P2BB3/1PP2PPP/R3K2n b Q - 0 1", 95, UINT64_C(0x56fc55b0ad922e6c) },
    ExpectedEval { "r1bq2k1/p4r1p/1pp2pp1/3p4/1P1B3Q/P2B1N2/2P3PP/4R1K1 b - - 0 1", -239, UINT64_C(0x9561e2a4a6c5c47b) },
    ExpectedEval { "r4qk1/6r1/1p4p1/2ppBbN1/1p5Q/P7/2P3PP/5RK1 w - - 0 1", -182, UINT64_C(0x19a31d5ab85978bf) },
    ExpectedEval { "r7/6k1/1p6/2pp1p2/7Q/8/p1P2K1P/8 w - - 0 1", 23, UINT64_C(0xfb582f88ed71cf40) },
    ExpectedEval { "r3k2r/ppp1pp1p/2nqb1pn/3p4/4P3/2PP4/PP1NBPPP/R2QK1NR w KQkq - 0 1", 28, UINT64_C(0xed657f0b57c21421) },
    ExpectedEval { "3r1rk1/1pp1pn1p/p1n1q1p1/3p4/Q3P3/2P5/PP1NBPPP/4RRK1 w - - 0 1", 21, UINT64_C(0xc5289c338b43f273) },
    ExpectedEval { "5rk1/1pp1pn1p/p3Brp1/8/1n6/5N2/PP3PPP/2R2RK1 w - - 0 1", -38, UINT64_C(0x2fe566820529ef68) },
    ExpectedEval { "8/1p2pk1p/p1p1r1p1/3n4/8/5R2/PP3PPP/4R1K1 b - - 0 1", -140, UINT64_C(0x1bc476f7ff81bbcd) },
    ExpectedEval { "8/4pk2/1p1r2p1/p1p4p/Pn5P/3R4/1P3PP1/4RK2 w - - 0 1", 172, UINT64_C(0x842eff43e89989f1) },
    ExpectedEval { "8/5k2/1pnrp1p1/p1p4p/P6P/4R1PK/1P3P2/4R3 b - - 0 1", -210, UINT64_C(0x7322ea81c1ae1981) },
    ExpectedEval { "8/8/1p1kp1p1/p1pr1n1p/P6P/1R4P1/1P3PK1/1R6 b - - 0 1", -156, UINT64_C(0xf93f95712f9c64a0) },
    ExpectedEval { "8/8/1p1k2p1/p1prp2p/P2n3P/6P1/1P1R1PK1/4R3 b - - 0 1", -285, UINT64_C(0x7eb431559170330b) },
    ExpectedEval { "8/8/1p4p1/p1p2k1p/P2npP1P/4K1P1/1P6/3R4 w - - 0 1", 72, UINT64_C(0x6816358085dc4b95) },
    ExpectedEval { "8/8/1p4p1/p1p2k1p/P2n1P1P/4K1P1/1P6/6R1 b - - 0 1", -259, UINT64_C(0x5b6eeaf876c1aa4a) },
    ExpectedEval { "8/5k2/1p4p1/p1pK3p/P2n1P1P/6P1/1P6/4R3 b - - 0 1", -307, UINT64_C(0x712478956533c7a0) },
    ExpectedEval { "8/1R6/1p1K1kp1/p6p/P1p2P1P/6P1/1Pn5/8 w - - 0 1", 327, UINT64_C(0x53d2ea1fd3a010c7) },
    ExpectedEval { "1rb1rn1k/p3q1bp/2p3p1/2p1p3/2P1P2N/PP1RQNP1/1B3P2/4R1K1 b - - 0 1", -191, UINT64_C(0x40b1a381fceadc09) },
    ExpectedEval { "4rrk1/pp1n1pp1/q5p1/P1pP4/2n3P1/7P/1P3PB1/R1BQ1RK1 w - - 0 1", 17, UINT64_C(0x40148f8250e1714d) },
    ExpectedEval { "r2qr1k1/pb1nbppp/1pn1p3/2ppP3/3P4/2PB1NN1/PP3PPP/R1BQR1K1 w - - 0 1", 114, UINT64_C(0x69e75b42f05e5862) },
    ExpectedEval { "2r2k2/8/4P1R1/1p6/8/P4K1N/7b/2B5 b - - 0 1", -719, UINT64_C(0x0d4e1ec2c254ea50) },
    ExpectedEval { "6k1/5pp1/8/2bKP2P/2P5/p4PNb/B7/8 b - - 0 1", 70, UINT64_C(0x260b0b1a59fe179b) },
    ExpectedEval { "2rqr1k1/1p3p1p/p2p2p1/P1nPb3/2B1P3/5P2/1PQ2NPP/R1R4K w - - 0 1", 78, UINT64_C(0x19f4bb3e7bb13eed) },
    ExpectedEval { "r1b2rk1/p1q1ppbp/6p1/2Q5/8/4BP2/PPP3PP/2KR1B1R b - - 0 1", -89, UINT64_C(0x4f2b23804a2c49a8) },
    ExpectedEval { "6r1/5k2/p1b1r2p/1pB1p1p1/1Pp3PP/2P1R1K1/2P2P2/3R4 w - - 0 1", 41, UINT64_C(0x7616bad0ce820c68) },
    ExpectedEval { "rnbqkb1r/pppppppp/5n2/8/2PP4/8/PP2PPPP/RNBQKBNR b KQkq c3 0 1", -36, UINT64_C(0x2e843079127c14fa) },
    ExpectedEval { "2rr2k1/1p4bp/p1q1p1p1/4Pp1n/2PB4/1PN3P1/P3Q2P/2RR2K1 w - f6 0 1", -5, UINT64_C(0x6f7eebeb3716f209) },
    ExpectedEval { "3br1k1/p1pn3p/1p3n2/5pNq/2P1p3/1PN3PP/P2Q1PB1/4R1K1 w - - 0 1", 247, UINT64_C(0x7a6d85b2e37fe9fe) },
    ExpectedEval { "2r2b2/5p2/5k2/p1r1pP2/P2pB3/1P3P2/K1P3R1/7R w - - 0 1", 153, UINT64_C(0x5d77ff3fba368779) },
    ExpectedEval { "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1", -22, UINT64_C(0x40b83f15c7ede7a1) },
    ExpectedEval { "r2qkb1r/ppp1p1pp/2n3p1/1b6/2BpP3/5Q2/PPPP1PPP/RNB1K1NR w KQkq - 0 1", 538, UINT64_C(0x2677f4b1de761020) },
    ExpectedEval { "8/8/8/8/8/8/3q4/2K1k3 w - - 0 1", -2045, UINT64_C(0x9d95b4693733af3e) },
    ExpectedEval { "8/2P5/8/8/k1K5/8/8/8 w - - 0 1", 510, UINT64_C(0x6133f17e11114160) },
    ExpectedEval { "r1b2rk1/pp1nqppp/2p1p3/3n4/2BP4/2N1PN2/PPQ2PPP/R4RK1 w - - 0 1", 147, UINT64_C(0xcc90a8c93d37c183) },
    ExpectedEval { "4k3/8/2r5/3P4/8/8/8/4K3 w - - 0 1", -1070, UINT64_C(0x5238560ddf1887ee) },
    ExpectedEval { "r2qk2r/ppp2ppp/3n1b2/4P3/8/8/PPP2PPP/R2QK2R b KQkq - 0 1", 882, UINT64_C(0xde9623504c4f78b1) },
    ExpectedEval { "2r1k3/3q4/1N6/8/8/8/8/4K3 w - - 0 1", -1803, UINT64_C(0x0df2a66bb7374f5e) },
    ExpectedEval { "6k1/8/4q3/8/8/8/4R3/6K1 w - - 0 1", -719, UINT64_C(0x7d5e84a338451456) },
    ExpectedEval { "6k1/8/8/4r3/3P4/5N2/8/6K1 w - - 0 1", -227, UINT64_C(0x74a759dd09d0d0f5) },
    ExpectedEval { "6k1/7p/4b3/8/8/8/4R3/6K1 w - - 0 1", 183, UINT64_C(0xa8d6396a56bc967f) },
    ExpectedEval { "6k1/7p/4b3/8/8/8/4R3/6K1 b - - 0 1", -183, UINT64_C(0x3b375392331d94e3) },
    ExpectedEval { "8/8/4k3/4n3/2N5/4NN2/6K1/8 b - - 0 1", -821, UINT64_C(0x4af7862dc583622c) },
    ExpectedEval { "6k1/8/8/4n3/8/2BB4/8/6K1 w - - 0 1", 553, UINT64_C(0x72c364c08c69b2ba) },
    ExpectedEval { "2b4k/1p1p4/8/8/8/8/1P1P4/K1B5 w - - 0 1", -6, UINT64_C(0x104d44658b15541f) },
    ExpectedEval { "2b4k/1p1p4/8/8/8/8/1P1P4/K4B2 w - - 0 1", 89, UINT64_C(0xdaedb06d490b6d59) },
    ExpectedEval { "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 0, UINT64_C(0x27fc9895123c4f28) },
    ExpectedEval { "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1", -5, UINT64_C(0xf9af09362471726b) },
    ExpectedEval { "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1", 306, UINT64_C(0x436028372c6c6962) },
    ExpectedEval { "6k1/5ppp/8/1P6/8/8/5PPP/1R4K1 b - - 0 1", -1466, UINT64_C(0xcfd92263e38baf59) },
    ExpectedEval { "6k1/5ppp/8/1P6/8/8/5PPP/1R4K1 w - - 0 1", 1538, UINT64_C(0x02b4dd9d84242eb4) },
    ExpectedEval { "1r4k1/5ppp/8/1P6/8/8/5PPP/6K1 w - - 0 1", -1061, UINT64_C(0x879116cd3ef6e95e) },
    ExpectedEval { "6k1/5ppp/8/1P6/8/8/5PPP/6KR w - - 0 1", 1398, UINT64_C(0xc0fe510995f15c5c) },
    ExpectedEval { "1k6/5ppp/8/1P6/8/8/5PPP/1R4K1 w - - 0 1", 1413, UINT64_C(0x0421d374c6229c86) },
    ExpectedEval { "6k1/5ppp/8/1P6/K7/8/5PPP/1R6 w - - 0 1", 1590, UINT64_C(0x43106e666d639e56) },
    ExpectedEval { "6k1/8/8/4n3/3P4/8/8/6K1 w - - 0 1", -744, UINT64_C(0xdea7f80686710fd9) },
    ExpectedEval { "6k1/8/8/3n4/3P4/8/8/6K1 w - - 0 1", -760, UINT64_C(0x79fa1c3d00101f03) },
    ExpectedEval { "6k1/8/8/4n3/3P4/8/8/6K1 b - - 0 1", 818, UINT64_C(0x88c4efe5e305b1b3) },
    ExpectedEval { "6k1/8/8/3n4/3P4/8/8/6K1 b - - 0 1", 835, UINT64_C(0x07de377b171df700) },
    ExpectedEval { "6k1/8/4r3/8/5N2/8/8/6K1 w - - 0 1", -344, UINT64_C(0x421b78b3d9b35259) },
    ExpectedEval { "6k1/8/3r4/8/5N2/8/8/6K1 w - - 0 1", -352, UINT64_C(0x51c32de0d444c17c) },
    ExpectedEval { "6k1/8/4r3/8/2B5/8/8/6K1 b - - 0 1", 306, UINT64_C(0xd369bd7e8c6fa8ce) },
    ExpectedEval { "6k1/8/3r4/8/2B5/8/8/6K1 b - - 0 1", 321, UINT64_C(0xfbfb6e88ae7ea0ae) },
    ExpectedEval { "6k1/8/4q3/6N1/8/8/PPP5/1K6 w - - 0 1", -600, UINT64_C(0x053d85641d8c96f6) },
    ExpectedEval { "6k1/8/3q4/6N1/8/8/PPP5/1K6 w - - 0 1", -579, UINT64_C(0xe088811c157f02d7) },
    ExpectedEval { "6k1/8/3q4/8/8/8/4R3/6K1 w - - 0 1", -687, UINT64_C(0xf4570c50f150cfd9) },
    ExpectedEval { "7k/8/3p4/3n4/8/4NN2/5K2/8 w - - 0 1", 431, UINT64_C(0xa63e2cf40d4bcd66) },
    ExpectedEval { "7k/8/3p4/4n3/8/4NN2/5K2/8 w - - 0 1", 413, UINT64_C(0x3a6dedd86bacf2a8) },
    ExpectedEval { "7k/8/2n5/3n4/8/4NN2/5K2/8 w - - 0 1", 53, UINT64_C(0xc13144c045fca5c1) },
    ExpectedEval { "7k/8/2n5/4n3/8/4NN2/5K2/8 w - - 0 1", 10, UINT64_C(0xb27975bef0ef7345) },
    ExpectedEval { "8/8/4k3/4n3/2N5/4NN2/6K1/8 w - - 0 1", 856, UINT64_C(0x2ba8e8f73600e6d9) },
    ExpectedEval { "8/8/4k3/3n4/2N5/4NN2/6K1/8 w - - 0 1", 828, UINT64_C(0x1cb37fa5f5476e60) },
    ExpectedEval { "8/8/8/4k3/8/8/8/KQ6 w - - 0 1", 1823, UINT64_C(0x53197d8b06f2d2e7) },
    ExpectedEval { "8/8/8/3k4/8/8/2K5/7R b - - 0 1", -1189, UINT64_C(0x453c308f58956f58) },
    ExpectedEval { "8/8/4k3/8/8/8/3PK3/8 w - - 0 1", 104, UINT64_C(0xaa29a82ff7a7fb1c) },
    ExpectedEval { "8/5k2/8/8/8/8/1N3B2/4K3 w - - 0 1", 1326, UINT64_C(0xa1819dc8506f3325) },
    ExpectedEval { "4k3/8/8/8/8/8/8/4K2R w K - 0 1", 1247, UINT64_C(0xa6734a110a0bbdaa) },
    ExpectedEval { "1k6/8/8/8/8/8/QQQQ4/K7 b - - 0 1", -6842, UINT64_C(0xb006fb4c87171a39) },
    ExpectedEval { "qqqqk3/8/8/8/8/8/8/4K3 w - - 0 1", -6836, UINT64_C(0x79a8361d44f94868) },
    ExpectedEval { "4k3/pppppppp/8/8/8/8/PPPPPPPP/4K3 w - - 0 1", 0, UINT64_C(0xe42950d5833b631a) },
    ExpectedEval { "8/1P4k1/8/8/8/8/6p1/K7 w - - 0 1", -86, UINT64_C(0x633d2029ffab0ab2) },
    ExpectedEval { "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b KQkq - 0 1", 0, UINT64_C(0x27fc9895123c4f28) },
};
// clang-format on

} // namespace recorded_evals
