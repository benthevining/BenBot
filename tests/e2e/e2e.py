# ======================================================================================
#
# ░▒▓███████▓▒░░▒▓████████▓▒░▒▓███████▓▒░       ░▒▓███████▓▒░ ░▒▓██████▓▒░▒▓████████▓▒░
# ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░
# ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░
# ░▒▓███████▓▒░░▒▓██████▓▒░ ░▒▓█▓▒░░▒▓█▓▒░      ░▒▓███████▓▒░░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░
# ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░
# ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░
# ░▒▓███████▓▒░░▒▓████████▓▒░▒▓█▓▒░░▒▓█▓▒░      ░▒▓███████▓▒░ ░▒▓██████▓▒░  ░▒▓█▓▒░
#
# ======================================================================================

import argparse
import re
import sys
import pathlib
import os

from testing import (
    BenBot as Engine,
    MiniTestFramework,
    OrderedClassMembers,
)

PATH = pathlib.Path(__file__).parent.resolve()
CWD = os.getcwd()


def get_path():
    return os.path.abspath(os.path.join(CWD, args.stockfish_path))


def BenBot(*args, **kwargs):
    return Engine(get_path(), *args, **kwargs)


class TestCLI(metaclass=OrderedClassMembers):
    def test_go_nodes_1000(self):
        engine = BenBot("go nodes 1000".split(" "), True)
        assert engine.process.returncode == 0

    def test_go_depth_10(self):
        engine = BenBot("go depth 10".split(" "), True)
        assert engine.process.returncode == 0

    def test_go_perft_4(self):
        engine = BenBot("perft 4 json".split(" "), True)
        assert engine.process.returncode == 0

    def test_go_movetime_1000(self):
        engine = BenBot("go movetime 1000".split(" "), True)
        assert engine.process.returncode == 0

    def test_go_wtime_8000_btime_8000_winc_500_binc_500(self):
        engine = BenBot(
            "go wtime 8000 btime 8000 winc 500 binc 500".split(" "),
            True,
        )
        assert engine.process.returncode == 0

    def test_go_wtime_1000_btime_1000_winc_0_binc_0(self):
        engine = BenBot(
            "go wtime 1000 btime 1000 winc 0 binc 0".split(" "),
            True,
        )
        assert engine.process.returncode == 0

    def test_go_wtime_1000_btime_1000_winc_0_binc_0_movestogo_5(self):
        engine = BenBot(
            "go wtime 1000 btime 1000 winc 0 binc 0 movestogo 5".split(" "),
            True,
        )
        assert engine.process.returncode == 0

    def test_go_movetime_200(self):
        engine = BenBot("go movetime 200".split(" "), True)
        assert engine.process.returncode == 0

    def test_go_nodes_20000_searchmoves_e2e4_d2d4(self):
        engine = BenBot("go nodes 20000 searchmoves e2e4 d2d4".split(" "), True)
        assert engine.process.returncode == 0

    def test_bench(self):
        engine = BenBot(
            f"bench".split(" "),
            True,
        )
        assert engine.process.returncode == 0

    def test_showpos(self):
        engine = BenBot("showpos".split(" "), True)
        assert engine.process.returncode == 0

    def test_compiler(self):
        engine = BenBot("compiler".split(" "), True)
        assert engine.process.returncode == 0

    def test_uci(self):
        engine = BenBot("uci".split(" "), True)
        assert engine.process.returncode == 0


class TestInteractive(metaclass=OrderedClassMembers):
    def __init__(self):
        self.stockfish = BenBot()

    def afterEach(self):
        self.stockfish.clear_output()

    def test_uci_command(self):
        self.stockfish.send_command("uci")
        self.stockfish.equals("uciok")

    def test_set_threads_option(self):
        self.stockfish.send_command(f"setoption name Threads value 1")

    def test_ucinewgame_and_startpos_nodes_1000(self):
        self.stockfish.send_command("ucinewgame")
        self.stockfish.send_command("position startpos")
        self.stockfish.send_command("go nodes 1000")
        self.stockfish.starts_with("bestmove")

    def test_ucinewgame_and_startpos_moves(self):
        self.stockfish.send_command("ucinewgame")
        self.stockfish.send_command("position startpos moves e2e4 e7e6")
        self.stockfish.send_command("go nodes 1000")
        self.stockfish.starts_with("bestmove")

    def test_fen_position_1(self):
        self.stockfish.send_command("ucinewgame")
        self.stockfish.send_command("position fen 5rk1/1K4p1/8/8/3B4/8/8/8 b - - 0 1")
        self.stockfish.send_command("go nodes 1000")
        self.stockfish.starts_with("bestmove")

    def test_fen_position_2_flip(self):
        self.stockfish.send_command("ucinewgame")
        self.stockfish.send_command("position fen 5rk1/1K4p1/8/8/3B4/8/8/8 b - - 0 1")
        self.stockfish.send_command("flip")
        self.stockfish.send_command("go nodes 1000")
        self.stockfish.starts_with("bestmove")

    def test_depth_5_with_callback(self):
        self.stockfish.send_command("ucinewgame")
        self.stockfish.send_command("position startpos")
        self.stockfish.send_command("go depth 5")

        def callback(output):
            regex = r"info depth \d+ seldepth \d+ score cp -?\d+ time \d+ hashfull \d+ nodes \d+ nps \d+ tbhits \d+ pv"
            if output.startswith("info depth") and not re.match(regex, output):
                assert False
            if output.startswith("bestmove"):
                return True
            return False

        self.stockfish.check_output(callback)

    def test_ucinewgame_and_go_depth_4(self):
        total_depth = 4

        self.stockfish.send_command("ucinewgame")
        self.stockfish.send_command("position startpos")
        self.stockfish.send_command(f"go depth {total_depth}")

        depth = 0

        def callback(output):
            nonlocal depth
            nonlocal total_depth

            if output.startswith("info depth"):
                depth += 1

                regex = rf"info depth {depth} seldepth \d+ score cp -?\d+ time \d+ hashfull \d+ nodes \d+ nps \d+ tbhits \d+ pv"

                if not re.match(regex, output):
                    assert False

            if output.startswith("bestmove"):
                assert depth == total_depth
                return True

            return False

        self.stockfish.check_output(callback)

    def test_clear_hash(self):
        self.stockfish.send_command("setoption name Clear Hash")

    def test_fen_position_mate_1(self):
        self.stockfish.send_command("ucinewgame")
        self.stockfish.send_command(
            "position fen 5K2/8/2qk4/2nPp3/3r4/6B1/B7/3R4 w - e6"
        )
        self.stockfish.send_command("go depth 10")

        self.stockfish.expect("* score mate 1 * pv d5e6")
        self.stockfish.equals("bestmove d5e6")

    def test_fen_position_mate_minus_1(self):
        self.stockfish.send_command("ucinewgame")
        self.stockfish.send_command(
            "position fen 2brrb2/8/p7/Q7/1p1kpPp1/1P1pN1K1/3P4/8 b - -"
        )
        self.stockfish.send_command("go depth 10")
        self.stockfish.expect("* score mate -1 *")
        self.stockfish.starts_with("bestmove")

    def test_fen_position_fixed_node(self):
        self.stockfish.send_command("ucinewgame")
        self.stockfish.send_command(
            "position fen 5K2/8/2P1P1Pk/6pP/3p2P1/1P6/3P4/8 w - - 0 1"
        )
        self.stockfish.send_command("go nodes 10000")
        self.stockfish.starts_with("bestmove")

    def test_fen_position_with_mate_go_depth(self):
        self.stockfish.send_command("ucinewgame")
        self.stockfish.send_command(
            "position fen 8/5R2/2K1P3/4k3/8/b1PPpp1B/5p2/8 w - -"
        )
        self.stockfish.send_command("go depth 18 searchmoves c6d7")
        self.stockfish.expect("* score mate 2 * pv c6d7 * f7f5")

        self.stockfish.starts_with("bestmove")

    def test_fen_position_with_mate_go_mate(self):
        self.stockfish.send_command("ucinewgame")
        self.stockfish.send_command(
            "position fen 8/5R2/2K1P3/4k3/8/b1PPpp1B/5p2/8 w - -"
        )
        self.stockfish.send_command("go mate 2 searchmoves c6d7")
        self.stockfish.expect("* score mate 2 * pv c6d7 *")

        self.stockfish.starts_with("bestmove")

    def test_fen_position_with_mate_go_nodes(self):
        self.stockfish.send_command("ucinewgame")
        self.stockfish.send_command(
            "position fen 8/5R2/2K1P3/4k3/8/b1PPpp1B/5p2/8 w - -"
        )
        self.stockfish.send_command("go nodes 500000 searchmoves c6d7")
        self.stockfish.expect("* score mate 2 * pv c6d7 * f7f5")

        self.stockfish.starts_with("bestmove")

    def test_fen_position_depth_8(self):
        self.stockfish.send_command("ucinewgame")
        self.stockfish.send_command(
            "position fen r1b2r1k/pp1p2pp/2p5/2B1q3/8/8/P1PN2PP/R4RK1 w - - 0 18"
        )
        self.stockfish.send_command("go depth 8")
        self.stockfish.contains("score mate 1")

        self.stockfish.starts_with("bestmove")

    def test_fen_position_with_mate_go_depth_and_promotion(self):
        self.stockfish.send_command("ucinewgame")
        self.stockfish.send_command(
            "position fen 8/5R2/2K1P3/4k3/8/b1PPpp1B/5p2/8 w - - moves c6d7 f2f1q"
        )
        self.stockfish.send_command("go depth 13")
        self.stockfish.expect("* score mate 1 * pv f7f5")
        self.stockfish.starts_with("bestmove f7f5")

    def test_fen_position_with_mate_go_depth_and_searchmoves(self):
        self.stockfish.send_command("ucinewgame")
        self.stockfish.send_command(
            "position fen 8/5R2/2K1P3/4k3/8/b1PPpp1B/5p2/8 w - -"
        )
        self.stockfish.send_command("go depth 18 searchmoves c6d7")
        self.stockfish.expect("* score mate 2 * pv c6d7 * f7f5")

        self.stockfish.starts_with("bestmove c6d7")

    def test_fen_position_with_moves_with_mate_go_depth_and_searchmoves(self):
        self.stockfish.send_command("ucinewgame")
        self.stockfish.send_command(
            "position fen 8/5R2/2K1P3/4k3/8/b1PPpp1B/5p2/8 w - - moves c6d7"
        )
        self.stockfish.send_command("go depth 18 searchmoves e3e2")
        self.stockfish.expect("* score mate -1 * pv e3e2 f7f5")
        self.stockfish.starts_with("bestmove e3e2")


class TestEnPassantSanitization(metaclass=OrderedClassMembers):
    def __init__(self):
        self.stockfish = BenBot()

    def afterEach(self):
        self.stockfish.clear_output()

    def test_position_1(self):
        self.stockfish.send_command(
            "position fen rnbqkbnr/ppp1p1pp/5p2/3pP3/8/8/PPPP1PPP/RNBQKBNR w kq d6 0 3"
        )
        self.stockfish.send_command("showpos")

        self.stockfish.expect_for_line_matching(
            "FEN*", "*rnbqkbnr/ppp1p1pp/5p2/3pP3/8/8/PPPP1PPP/RNBQKBNR w kq d6 0 3*"
        )

    def test_position_2(self):
        self.stockfish.send_command("position fen k7/8/8/1pP5/2K5/8/8/8 w - b6 0 1")
        self.stockfish.send_command("showpos")

        self.stockfish.expect_for_line_matching(
            "FEN*", "*k7/8/8/1pP5/2K5/8/8/8 w - b6 0 1*"
        )

    # def test_position_3(self):
    #     self.stockfish.send_command("position fen k1r5/8/8/1pP5/2K5/8/8/8 w - b6 0 1")
    #     self.stockfish.send_command("showpos")
    #
    #     self.stockfish.expect_for_line_matching(
    #         "XFEN*", "*k1r5/8/8/1pP5/2K5/8/8/8 w - - 0 1*"
    #     )

    # def test_position_4(self):
    #     self.stockfish.send_command("position fen k1r5/8/8/1pP5/8/2K5/8/8 w - b6 0 1")
    #     self.stockfish.send_command("showpos")
    #
    #     self.stockfish.expect_for_line_matching(
    #         "XFEN*", "*k1r5/8/8/1pP5/8/2K5/8/8 w - - 0 1*"
    #     )

    def test_position_5(self):
        self.stockfish.send_command("position fen k1r5/8/8/PpP5/8/2K5/8/8 w - b6 0 1")
        self.stockfish.send_command("showpos")

        self.stockfish.expect_for_line_matching(
            "FEN*", "*k1r5/8/8/PpP5/8/2K5/8/8 w - b6 0 1*"
        )

    def test_position_6(self):
        self.stockfish.send_command("position fen k1r5/8/8/PpP5/2K5/8/8/8 w - b6 0 1")
        self.stockfish.send_command("showpos")

        self.stockfish.expect_for_line_matching(
            "FEN*", "*k1r5/8/8/PpP5/2K5/8/8/8 w - b6 0 1*"
        )

    def test_position_7(self):
        self.stockfish.send_command("position fen k7/4b3/8/PpP5/1K6/8/8/8 w - b6 0 1")
        self.stockfish.send_command("showpos")

        self.stockfish.expect_for_line_matching(
            "FEN*", "*k7/4b3/8/PpP5/1K6/8/8/8 w - b6 0 1*"
        )

    # def test_position_8(self):
    #     self.stockfish.send_command("position fen k7/b5b1/8/2PpP3/3K4/8/8/8 w - d6 0 1")
    #     self.stockfish.send_command("showpos")
    #
    #     self.stockfish.expect_for_line_matching(
    #         "XFEN*", "*k7/b5b1/8/2PpP3/3K4/8/8/8 w - - 0 1*"
    #     )

    # def test_position_9(self):
    #     self.stockfish.send_command("position fen k7/8/8/r2pPK2/8/8/8/8 w - d6 0 1")
    #     self.stockfish.send_command("showpos")
    #
    #     self.stockfish.expect_for_line_matching(
    #         "XFEN*", "*k7/8/8/r2pPK2/8/8/8/8 w - - 0 1*"
    #     )

    def test_position_10(self):
        self.stockfish.send_command("position fen k7/8/8/r1PpPK2/8/8/8/8 w - d6 0 1")
        self.stockfish.send_command("showpos")

        self.stockfish.expect_for_line_matching(
            "FEN*", "*k7/8/8/r1PpPK2/8/8/8/8 w - d6 0 1*"
        )

    def test_position_11(self):
        self.stockfish.send_command("position fen kb6/8/8/3pP3/5K2/8/8/8 w - d6 0 1")
        self.stockfish.send_command("showpos")

        self.stockfish.expect_for_line_matching(
            "FEN*", "*kb6/8/8/3pP3/5K2/8/8/8 w - d6 0 1*"
        )


def parse_args():
    parser = argparse.ArgumentParser(description="Run Stockfish with testing options")

    parser.add_argument("stockfish_path", type=str, help="Path to Stockfish binary")

    return parser.parse_args()


if __name__ == "__main__":
    args = parse_args()

    framework = MiniTestFramework()

    # Each test suite will be run inside a temporary directory
    framework.run([TestCLI, TestInteractive, TestEnPassantSanitization])

    if framework.has_failed():
        sys.exit(1)

    sys.exit(0)
