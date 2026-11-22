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

#include <array>
#include <catch2/catch_test_macros.hpp>
#include <cstddef>
#include <cstdint>
#include <libbenbot/eval/Evaluation.hpp>
#include <libbenbot/eval/PieceSquareTables.hpp>
#include <libchess/game/Position.hpp>
#include <libchess/moves/MoveGen.hpp>
#include <libchess/notation/FEN.hpp>
#include <string_view>

// This checks that the eval gives exactly the same results as when these values were recorded,
// for the bench.epd positions & the positions in the other eval tests, plus a few endgames.
// It's meant for refactors that mustn't change the eval. If you change the eval on purpose,
// record the new values instead.

namespace {

struct ExpectedEval final {
    std::string_view fen;

    // the eval of the position itself
    int eval { 0 };

    // a hash of the evals of every position in the move tree, to depth 2
    std::uint64_t treeHash { 0 };
};

// clang-format off
constexpr std::array EXPECTED_EVALS {
    ExpectedEval { "r3k2r/2pb1ppp/2pp1q2/p7/1nP1B3/1P2P3/P2N1PPP/R2QK2R w KQkq a6 0 1", 57, UINT64_C(0xda2c4464dee0a9ec) },
    ExpectedEval { "4rrk1/2p1b1p1/p1p3q1/4p3/2P2n1p/1P1NR2P/PB3PP1/3R1QK1 b - - 0 1", -232, UINT64_C(0x446331ff5b68ab98) },
    ExpectedEval { "r3qbrk/6p1/2b2pPp/p3pP1Q/PpPpP2P/3P1B2/2PB3K/R5R1 w - - 0 1", 158, UINT64_C(0x1d99dadde01ef2dc) },
    ExpectedEval { "6k1/1R3p2/6p1/2Bp3p/3P2q1/P7/1P2rQ1K/5R2 b - - 0 1", -565, UINT64_C(0xffd3d53ff0c616c4) },
    ExpectedEval { "8/8/1p2k1p1/3p3p/1p1P1P1P/1P2PK2/8/8 w - - 0 1", 31, UINT64_C(0xca4816879db19fdd) },
    ExpectedEval { "7r/2p3k1/1p1p1qp1/1P1Bp3/p1P2r1P/P7/4R3/Q4RK1 w - - 0 1", -34, UINT64_C(0x9c4abb7b23a27e77) },
    ExpectedEval { "r1bq1rk1/pp2b1pp/n1pp1n2/3P1p2/2P1p3/2N1P2N/PP2BPPP/R1BQ1RK1 b - - 0 1", -20, UINT64_C(0x6a84aec822a1015f) },
    ExpectedEval { "3r3k/2r4p/1p1b3q/p4P2/P2Pp3/1B2P3/3BQ1RP/6K1 w - - 0 1", 69, UINT64_C(0x6179efda249e55ee) },
    ExpectedEval { "2r4r/1p4k1/1Pnp4/3Qb1pq/8/4BpPp/5P2/2RR1BK1 w - - 0 1", -371, UINT64_C(0x86affe1621b1dbb2) },
    ExpectedEval { "4q1bk/6b1/7p/p1p4p/PNPpP2P/KN4P1/3Q4/4R3 b - - 0 1", -502, UINT64_C(0x58f2e5593510ba46) },
    ExpectedEval { "2q3r1/1r2pk2/pp3pp1/2pP3p/P1Pb1BbP/1P4Q1/R3NPP1/4R1K1 w - - 0 1", 150, UINT64_C(0x7edbb48a1c908f1f) },
    ExpectedEval { "1r2r2k/1b4q1/pp5p/2pPp1p1/P3Pn2/1P1B1Q1P/2R3P1/4BR1K b - - 0 1", -199, UINT64_C(0x4152b93063294bb9) },
    ExpectedEval { "r3kbbr/pp1n1p1P/3ppnp1/q5N1/1P1pP3/P1N1B3/2P1QP2/R3KB1R b KQkq b3 0 1", -157, UINT64_C(0x9680a23959a330bb) },
    ExpectedEval { "8/6pk/2b1Rp2/3r4/1R1B2PP/P5K1/8/2r5 b - - 0 1", -112, UINT64_C(0x5a1e2730943b2265) },
    ExpectedEval { "1r4k1/4ppb1/2n1b1qp/pB4p1/1n1BP1P1/7P/2PNQPK1/3RN3 w - - 0 1", -54, UINT64_C(0xc241eae57214900d) },
    ExpectedEval { "8/p2B4/PkP5/4p1pK/4Pb1p/5P2/8/8 w - - 0 1", 1, UINT64_C(0x2bc90965b3ac763f) },
    ExpectedEval { "3r4/ppq1ppkp/4bnp1/2pN4/2P1P3/1P4P1/PQ3PBP/R4K2 b - - 0 1", -21, UINT64_C(0xfa3040c523a400da) },
    ExpectedEval { "5rr1/4n2k/4q2P/P1P2n2/3B1p2/4pP2/2N1P3/1RR1K2Q w - - 0 1", 432, UINT64_C(0xa85cc68df597b442) },
    ExpectedEval { "1r5k/2pq2p1/3p3p/p1pP4/4QP2/PP1R3P/6PK/8 w - - 0 1", -4, UINT64_C(0xaa0844644446d5f8) },
    ExpectedEval { "q5k1/5ppp/1r3bn1/1B6/P1N2P2/BQ2P1P1/5K1P/8 b - - 0 1", -88, UINT64_C(0xcf90741c3ccc4381) },
    ExpectedEval { "r1b2k1r/5n2/p4q2/1ppn1Pp1/3pp1p1/NP2P3/P1PPBK2/1RQN2R1 w - - 0 1", -407, UINT64_C(0xc21dbb23558ee0c2) },
    ExpectedEval { "r1bqk2r/pppp1ppp/5n2/4b3/4P3/P1N5/1PP2PPP/R1BQKB1R w KQkq - 0 1", 70, UINT64_C(0x7d6e3861a4b17eee) },
    ExpectedEval { "r1bqr1k1/pp1p1ppp/2p5/8/3N1Q2/P2BB3/1PP2PPP/R3K2n b Q - 0 1", 185, UINT64_C(0x33c8a55b6d4e56d0) },
    ExpectedEval { "r1bq2k1/p4r1p/1pp2pp1/3p4/1P1B3Q/P2B1N2/2P3PP/4R1K1 b - - 0 1", -26, UINT64_C(0xc8a3dd8e61cf032e) },
    ExpectedEval { "r4qk1/6r1/1p4p1/2ppBbN1/1p5Q/P7/2P3PP/5RK1 w - - 0 1", -265, UINT64_C(0xe908199825696b0c) },
    ExpectedEval { "r7/6k1/1p6/2pp1p2/7Q/8/p1P2K1P/8 w - - 0 1", -209, UINT64_C(0xfe21d91fd1cc9d8d) },
    ExpectedEval { "r3k2r/ppp1pp1p/2nqb1pn/3p4/4P3/2PP4/PP1NBPPP/R2QK1NR w KQkq - 0 1", -35, UINT64_C(0x48f692c052a9fdd0) },
    ExpectedEval { "3r1rk1/1pp1pn1p/p1n1q1p1/3p4/Q3P3/2P5/PP1NBPPP/4RRK1 w - - 0 1", -41, UINT64_C(0x10d86b4e180c0929) },
    ExpectedEval { "5rk1/1pp1pn1p/p3Brp1/8/1n6/5N2/PP3PPP/2R2RK1 w - - 0 1", -4, UINT64_C(0xb4932d188a2f96f0) },
    ExpectedEval { "8/1p2pk1p/p1p1r1p1/3n4/8/5R2/PP3PPP/4R1K1 b - - 0 1", -147, UINT64_C(0x1a831d8c2fefb895) },
    ExpectedEval { "8/4pk2/1p1r2p1/p1p4p/Pn5P/3R4/1P3PP1/4RK2 w - - 0 1", 129, UINT64_C(0x245fb82faeb44fad) },
    ExpectedEval { "8/5k2/1pnrp1p1/p1p4p/P6P/4R1PK/1P3P2/4R3 b - - 0 1", -118, UINT64_C(0x5e9e5cddc99d68bd) },
    ExpectedEval { "8/8/1p1kp1p1/p1pr1n1p/P6P/1R4P1/1P3PK1/1R6 b - - 0 1", -93, UINT64_C(0x583f1f299299f797) },
    ExpectedEval { "8/8/1p1k2p1/p1prp2p/P2n3P/6P1/1P1R1PK1/4R3 b - - 0 1", -168, UINT64_C(0x07c3ba0d552a891f) },
    ExpectedEval { "8/8/1p4p1/p1p2k1p/P2npP1P/4K1P1/1P6/3R4 w - - 0 1", 55, UINT64_C(0x9ba4aa61703fa3e9) },
    ExpectedEval { "8/8/1p4p1/p1p2k1p/P2n1P1P/4K1P1/1P6/6R1 b - - 0 1", -164, UINT64_C(0x896a2a3fa2311f29) },
    ExpectedEval { "8/5k2/1p4p1/p1pK3p/P2n1P1P/6P1/1P6/4R3 b - - 0 1", -277, UINT64_C(0xf22f82599a2377d9) },
    ExpectedEval { "8/1R6/1p1K1kp1/p6p/P1p2P1P/6P1/1Pn5/8 w - - 0 1", 231, UINT64_C(0x3cf4d5233c84571a) },
    ExpectedEval { "1rb1rn1k/p3q1bp/2p3p1/2p1p3/2P1P2N/PP1RQNP1/1B3P2/4R1K1 b - - 0 1", -293, UINT64_C(0xe09d14fecd05567a) },
    ExpectedEval { "4rrk1/pp1n1pp1/q5p1/P1pP4/2n3P1/7P/1P3PB1/R1BQ1RK1 w - - 0 1", -31, UINT64_C(0x7b907b6caa83e2e5) },
    ExpectedEval { "r2qr1k1/pb1nbppp/1pn1p3/2ppP3/3P4/2PB1NN1/PP3PPP/R1BQR1K1 w - - 0 1", 62, UINT64_C(0x1cecec267a518e4a) },
    ExpectedEval { "2r2k2/8/4P1R1/1p6/8/P4K1N/7b/2B5 b - - 0 1", -475, UINT64_C(0x64fe232745b51a16) },
    ExpectedEval { "6k1/5pp1/8/2bKP2P/2P5/p4PNb/B7/8 b - - 0 1", -19, UINT64_C(0x0aec1b605ddc1e57) },
    ExpectedEval { "2rqr1k1/1p3p1p/p2p2p1/P1nPb3/2B1P3/5P2/1PQ2NPP/R1R4K w - - 0 1", 122, UINT64_C(0x95cd61bc7409699d) },
    ExpectedEval { "r1b2rk1/p1q1ppbp/6p1/2Q5/8/4BP2/PPP3PP/2KR1B1R b - - 0 1", -321, UINT64_C(0xd70de517003f280c) },
    ExpectedEval { "6r1/5k2/p1b1r2p/1pB1p1p1/1Pp3PP/2P1R1K1/2P2P2/3R4 w - - 0 1", -29, UINT64_C(0x1aec48215501c5f5) },
    ExpectedEval { "rnbqkb1r/pppppppp/5n2/8/2PP4/8/PP2PPPP/RNBQKBNR b KQkq c3 0 1", 52, UINT64_C(0xec93d4ad2b1f9f6c) },
    ExpectedEval { "2rr2k1/1p4bp/p1q1p1p1/4Pp1n/2PB4/1PN3P1/P3Q2P/2RR2K1 w - f6 0 1", 25, UINT64_C(0xf2711822b6d349e1) },
    ExpectedEval { "3br1k1/p1pn3p/1p3n2/5pNq/2P1p3/1PN3PP/P2Q1PB1/4R1K1 w - - 0 1", 78, UINT64_C(0x10f5e5fc01d60e30) },
    ExpectedEval { "2r2b2/5p2/5k2/p1r1pP2/P2pB3/1P3P2/K1P3R1/7R w - - 0 1", 172, UINT64_C(0x4e7170bdfad89e94) },
    ExpectedEval { "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1", 67, UINT64_C(0xd06a2808c1ba1b4d) },
    ExpectedEval { "r2qkb1r/ppp1p1pp/2n3p1/1b6/2BpP3/5Q2/PPPP1PPP/RNB1K1NR w KQkq - 0 1", 340, UINT64_C(0x82a87e77a7c40e22) },
    ExpectedEval { "8/8/8/8/8/8/3q4/2K1k3 w - - 0 1", -1418, UINT64_C(0x22eff878b8ebcc5a) },
    ExpectedEval { "8/2P5/8/8/k1K5/8/8/8 w - - 0 1", 302, UINT64_C(0x31bcf625728de262) },
    ExpectedEval { "r1b2rk1/pp1nqppp/2p1p3/3n4/2BP4/2N1PN2/PPQ2PPP/R4RK1 w - - 0 1", 102, UINT64_C(0x9ffdafbbd73ee58e) },
    ExpectedEval { "4k3/8/2r5/3P4/8/8/8/4K3 w - - 0 1", -854, UINT64_C(0x485de25013641c13) },
    ExpectedEval { "r2qk2r/ppp2ppp/3n1b2/4P3/8/8/PPP2PPP/R2QK2R b KQkq - 0 1", 457, UINT64_C(0xecf07da7108191df) },
    ExpectedEval { "2r1k3/3q4/1N6/8/8/8/8/4K3 w - - 0 1", -1098, UINT64_C(0x5413164620191f9f) },
    ExpectedEval { "6k1/8/4q3/8/8/8/4R3/6K1 w - - 0 1", -306, UINT64_C(0x7fed6565dcea0b23) },
    ExpectedEval { "6k1/8/8/4r3/3P4/5N2/8/6K1 w - - 0 1", 13, UINT64_C(0x1c46241eada1b7b0) },
    ExpectedEval { "6k1/7p/4b3/8/8/8/4R3/6K1 w - - 0 1", 116, UINT64_C(0x8a1c884a30f1ebaa) },
    ExpectedEval { "6k1/7p/4b3/8/8/8/4R3/6K1 b - - 0 1", -116, UINT64_C(0x9fbefb8540213fd9) },
    ExpectedEval { "8/8/4k3/4n3/2N5/4NN2/6K1/8 b - - 0 1", -692, UINT64_C(0x4272eb8aad4e72ec) },
    ExpectedEval { "6k1/8/8/4n3/8/2BB4/8/6K1 w - - 0 1", 418, UINT64_C(0x89f37223da923d79) },
    ExpectedEval { "2b4k/1p1p4/8/8/8/8/1P1P4/K1B5 w - - 0 1", 4, UINT64_C(0xf7df6acf82d61c3f) },
    ExpectedEval { "2b4k/1p1p4/8/8/8/8/1P1P4/K4B2 w - - 0 1", 34, UINT64_C(0x8ecf290f2ea84610) },
    ExpectedEval { "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 0, UINT64_C(0xa480a7ed89724e0e) },
    ExpectedEval { "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1", -108, UINT64_C(0xdb1b00933df3f2e7) },
    ExpectedEval { "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1", -8, UINT64_C(0xcedeb1c71bf1d1b5) },
    ExpectedEval { "6k1/5ppp/8/1P6/8/8/5PPP/1R4K1 b - - 0 1", -1225, UINT64_C(0xacb914be0f69008c) },
    ExpectedEval { "6k1/5ppp/8/1P6/8/8/5PPP/1R4K1 w - - 0 1", 1291, UINT64_C(0x50bfe00985042765) },
    ExpectedEval { "1r4k1/5ppp/8/1P6/8/8/5PPP/6K1 w - - 0 1", -896, UINT64_C(0x2ed36809a6691601) },
    ExpectedEval { "6k1/5ppp/8/1P6/8/8/5PPP/6KR w - - 0 1", 1199, UINT64_C(0x4f268486fa23f207) },
    ExpectedEval { "1k6/5ppp/8/1P6/8/8/5PPP/1R4K1 w - - 0 1", 1234, UINT64_C(0xbadf176466562858) },
    ExpectedEval { "6k1/5ppp/8/1P6/K7/8/5PPP/1R6 w - - 0 1", 1289, UINT64_C(0xa10f788d06dcd35a) },
    ExpectedEval { "6k1/8/8/4n3/3P4/8/8/6K1 w - - 0 1", -641, UINT64_C(0x48b3e764b4cfac4a) },
    ExpectedEval { "6k1/8/8/3n4/3P4/8/8/6K1 w - - 0 1", -687, UINT64_C(0x450c14a9eac2053c) },
    ExpectedEval { "6k1/8/8/4n3/3P4/8/8/6K1 b - - 0 1", 711, UINT64_C(0x3d17c1a299040612) },
    ExpectedEval { "6k1/8/8/3n4/3P4/8/8/6K1 b - - 0 1", 757, UINT64_C(0x803d2ccfd86a584c) },
    ExpectedEval { "6k1/8/4r3/8/5N2/8/8/6K1 w - - 0 1", -187, UINT64_C(0x5adc8d6f2fc35e9e) },
    ExpectedEval { "6k1/8/3r4/8/5N2/8/8/6K1 w - - 0 1", -242, UINT64_C(0xda3d97325d6046d1) },
    ExpectedEval { "6k1/8/4r3/8/2B5/8/8/6K1 b - - 0 1", 183, UINT64_C(0x870bdd81cfd15d8d) },
    ExpectedEval { "6k1/8/3r4/8/2B5/8/8/6K1 b - - 0 1", 226, UINT64_C(0xaf0418edd7a07d98) },
    ExpectedEval { "6k1/8/4q3/6N1/8/8/PPP5/1K6 w - - 0 1", -64, UINT64_C(0x94ac42a6daa8d75e) },
    ExpectedEval { "6k1/8/3q4/6N1/8/8/PPP5/1K6 w - - 0 1", -112, UINT64_C(0xa01a3a092240877a) },
    ExpectedEval { "6k1/8/3q4/8/8/8/4R3/6K1 w - - 0 1", -330, UINT64_C(0x6f3c1ad388776081) },
    ExpectedEval { "7k/8/3p4/3n4/8/4NN2/5K2/8 w - - 0 1", 319, UINT64_C(0x4dcf799ff0b014cf) },
    ExpectedEval { "7k/8/3p4/4n3/8/4NN2/5K2/8 w - - 0 1", 296, UINT64_C(0x1cba3f5abe88cad4) },
    ExpectedEval { "7k/8/2n5/3n4/8/4NN2/5K2/8 w - - 0 1", 47, UINT64_C(0x5539632a8fc42d74) },
    ExpectedEval { "7k/8/2n5/4n3/8/4NN2/5K2/8 w - - 0 1", 32, UINT64_C(0x21cf9d04a580eac9) },
    ExpectedEval { "8/8/4k3/4n3/2N5/4NN2/6K1/8 w - - 0 1", 718, UINT64_C(0xaa5b5951af0cd466) },
    ExpectedEval { "8/8/4k3/3n4/2N5/4NN2/6K1/8 w - - 0 1", 697, UINT64_C(0x6cd6c39a667060c4) },
    ExpectedEval { "8/8/8/4k3/8/8/8/KQ6 w - - 0 1", 1379, UINT64_C(0xd2e6dd852a87d839) },
    ExpectedEval { "8/8/8/3k4/8/8/2K5/7R b - - 0 1", -1074, UINT64_C(0x52b518bb1de6f073) },
    ExpectedEval { "8/8/4k3/8/8/8/3PK3/8 w - - 0 1", 83, UINT64_C(0x22c377f0299a9559) },
    ExpectedEval { "8/5k2/8/8/8/8/1N3B2/4K3 w - - 0 1", 1165, UINT64_C(0x6c1f7b498cc74567) },
    ExpectedEval { "4k3/8/8/8/8/8/8/4K2R w K - 0 1", 1128, UINT64_C(0x1842c988eac0e807) },
    ExpectedEval { "1k6/8/8/8/8/8/QQQQ4/K7 b - - 0 1", -4191, UINT64_C(0xb00d1c4c8721844c) },
    ExpectedEval { "qqqqk3/8/8/8/8/8/8/4K3 w - - 0 1", -4196, UINT64_C(0x863990c2830bc25f) },
    ExpectedEval { "4k3/pppppppp/8/8/8/8/PPPPPPPP/4K3 w - - 0 1", 0, UINT64_C(0xd75d85b470619af7) },
    ExpectedEval { "8/1P4k1/8/8/8/8/6p1/K7 w - - 0 1", -22, UINT64_C(0x67a8236ddc17348b) },
    ExpectedEval { "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b KQkq - 0 1", 0, UINT64_C(0xa480a7ed89724e0e) },
};
// clang-format on

inline constexpr auto TREE_DEPTH = 2uz;

// FNV-1a over the evals, in move generation order
void hash_tree( // NOLINT(misc-no-recursion)
    const chess::game::Position& position, const std::size_t depth,
    const ben_bot::eval::PieceSquareTables& pst, std::uint64_t& hash)
{
    const auto eval = static_cast<std::int16_t>(ben_bot::eval::evaluate(position, pst));

    hash ^= static_cast<std::uint16_t>(eval);
    hash *= UINT64_C(1099511628211);

    if (depth == 0uz)
        return;

    for (const auto& move : chess::moves::generate(position))
        hash_tree(after_move(position, move), depth - 1uz, pst, hash);
}

} // namespace

TEST_CASE("Evaluation - unchanged from recorded values", "[eval][evaluation]")
{
    const auto pst = ben_bot::eval::PieceSquareTables::get_default();

    for (const auto& expected : EXPECTED_EVALS) {
        INFO(expected.fen);

        const auto position = chess::notation::from_fen(expected.fen).value();

        REQUIRE(static_cast<int>(ben_bot::eval::evaluate(position, pst)) == expected.eval);

        std::uint64_t hash { UINT64_C(14695981039346656037) };

        hash_tree(position, TREE_DEPTH, pst, hash);

        REQUIRE(hash == expected.treeHash);
    }
}
