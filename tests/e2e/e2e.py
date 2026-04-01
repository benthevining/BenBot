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
    return os.path.abspath(os.path.join(CWD, args.engine_path))


def BenBot(*args, **kwargs):
    return Engine(get_path(), *args, **kwargs)


class TestInteractive(metaclass=OrderedClassMembers):
    @staticmethod
    def test_uci_command():
        engine = BenBot()
        engine.send_command("uci")
        engine.equals("uciok")

    @staticmethod
    def test_set_threads_option():
        engine = BenBot()
        engine.send_command(f"setoption name Threads value 1")

    @staticmethod
    def test_ucinewgame_and_startpos_nodes_1000():
        engine = BenBot()
        engine.send_command("ucinewgame")
        engine.send_command("position startpos")
        engine.send_command("go nodes 1000")
        engine.starts_with("bestmove")

    @staticmethod
    def test_ucinewgame_and_startpos_moves():
        engine = BenBot()
        engine.send_command("ucinewgame")
        engine.send_command("position startpos moves e2e4 e7e6")
        engine.send_command("go nodes 1000")
        engine.starts_with("bestmove")

    @staticmethod
    def test_fen_position_1():
        engine = BenBot()
        engine.send_command("ucinewgame")
        engine.send_command("position fen 5rk1/1K4p1/8/8/3B4/8/8/8 b - - 0 1")
        engine.send_command("go nodes 1000")
        engine.starts_with("bestmove")

    @staticmethod
    def test_fen_position_2_flip():
        engine = BenBot()
        engine.send_command("ucinewgame")
        engine.send_command("position fen 5rk1/1K4p1/8/8/3B4/8/8/8 b - - 0 1")
        engine.send_command("flip")
        engine.send_command("go nodes 1000")
        engine.starts_with("bestmove")

    @staticmethod
    def test_depth_5_with_callback():
        engine = BenBot()
        engine.send_command("ucinewgame")
        engine.send_command("position startpos")
        engine.send_command("go depth 5")

        def callback(output):
            regex = r"info depth \d+ seldepth \d+ score cp -?\d+ time \d+ hashfull \d+ nodes \d+ nps \d+ tbhits \d+ pv"
            if output.startswith("info depth") and not re.match(regex, output):
                assert False
            if output.startswith("bestmove"):
                return True
            return False

        engine.check_output(callback)

    @staticmethod
    def test_ucinewgame_and_go_depth_4():
        total_depth = 4

        engine = BenBot()
        engine.send_command("ucinewgame")
        engine.send_command("position startpos")
        engine.send_command(f"go depth {total_depth}")

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

        engine.check_output(callback)

    @staticmethod
    def test_clear_hash():
        engine = BenBot()
        engine.send_command("setoption name Clear Hash")

    @staticmethod
    def test_fen_position_mate_1():
        engine = BenBot()
        engine.send_command("ucinewgame")
        engine.send_command("position fen 5K2/8/2qk4/2nPp3/3r4/6B1/B7/3R4 w - e6")
        engine.send_command("go depth 10")

        engine.expect("* score mate 1 * pv d5e6")
        engine.equals("bestmove d5e6")

    @staticmethod
    def test_fen_position_mate_minus_1():
        engine = BenBot()
        engine.send_command("ucinewgame")
        engine.send_command("position fen 2brrb2/8/p7/Q7/1p1kpPp1/1P1pN1K1/3P4/8 b - -")
        engine.send_command("go depth 10")
        engine.expect("* score mate -1 *")
        engine.starts_with("bestmove")

    @staticmethod
    def test_fen_position_fixed_node():
        engine = BenBot()
        engine.send_command("ucinewgame")
        engine.send_command("position fen 5K2/8/2P1P1Pk/6pP/3p2P1/1P6/3P4/8 w - - 0 1")
        engine.send_command("go nodes 10000")
        engine.starts_with("bestmove")

    @staticmethod
    def test_fen_position_with_mate_go_depth():
        engine = BenBot()
        engine.send_command("ucinewgame")
        engine.send_command("position fen 8/5R2/2K1P3/4k3/8/b1PPpp1B/5p2/8 w - -")
        engine.send_command("go depth 18 searchmoves c6d7")
        engine.expect("* score mate 2 * pv c6d7 * f7f5")

        engine.starts_with("bestmove")

    @staticmethod
    def test_fen_position_with_mate_go_mate():
        engine = BenBot()
        engine.send_command("ucinewgame")
        engine.send_command("position fen 8/5R2/2K1P3/4k3/8/b1PPpp1B/5p2/8 w - -")
        engine.send_command("go mate 2 searchmoves c6d7")
        engine.expect("* score mate 2 * pv c6d7 *")

        engine.starts_with("bestmove")

    @staticmethod
    def test_fen_position_with_mate_go_nodes():
        engine = BenBot()
        engine.send_command("ucinewgame")
        engine.send_command("position fen 8/5R2/2K1P3/4k3/8/b1PPpp1B/5p2/8 w - -")
        engine.send_command("go nodes 500000 searchmoves c6d7")
        engine.expect("* score mate 2 * pv c6d7 * f7f5")

        engine.starts_with("bestmove")

    @staticmethod
    def test_fen_position_depth_8():
        engine = BenBot()
        engine.send_command("ucinewgame")
        engine.send_command(
            "position fen r1b2r1k/pp1p2pp/2p5/2B1q3/8/8/P1PN2PP/R4RK1 w - - 0 18"
        )
        engine.send_command("go depth 8")
        engine.contains("score mate 1")

        engine.starts_with("bestmove")

    @staticmethod
    def test_fen_position_with_mate_go_depth_and_promotion():
        engine = BenBot()
        engine.send_command("ucinewgame")
        engine.send_command(
            "position fen 8/5R2/2K1P3/4k3/8/b1PPpp1B/5p2/8 w - - moves c6d7 f2f1q"
        )
        engine.send_command("go depth 13")
        engine.expect("* score mate 1 * pv f7f5")
        engine.starts_with("bestmove f7f5")

    @staticmethod
    def test_fen_position_with_mate_go_depth_and_searchmoves():
        engine = BenBot()
        engine.send_command("ucinewgame")
        engine.send_command("position fen 8/5R2/2K1P3/4k3/8/b1PPpp1B/5p2/8 w - -")
        engine.send_command("go depth 18 searchmoves c6d7")
        engine.expect("* score mate 2 * pv c6d7 * f7f5")

        engine.starts_with("bestmove c6d7")

    @staticmethod
    def test_fen_position_with_moves_with_mate_go_depth_and_searchmoves():
        engine = BenBot()
        engine.send_command("ucinewgame")
        engine.send_command(
            "position fen 8/5R2/2K1P3/4k3/8/b1PPpp1B/5p2/8 w - - moves c6d7"
        )
        engine.send_command("go depth 18 searchmoves e3e2")
        engine.expect("* score mate -1 * pv e3e2 f7f5")
        engine.starts_with("bestmove e3e2")


class TestEnPassantSanitization(metaclass=OrderedClassMembers):
    @staticmethod
    def test_position_1():
        engine = BenBot()
        engine.send_command(
            "position fen rnbqkbnr/ppp1p1pp/5p2/3pP3/8/8/PPPP1PPP/RNBQKBNR w kq d6 0 3"
        )
        engine.send_command("showpos")

        engine.expect_for_line_matching(
            "FEN*", "*rnbqkbnr/ppp1p1pp/5p2/3pP3/8/8/PPPP1PPP/RNBQKBNR w kq d6 0 3*"
        )

    @staticmethod
    def test_position_2():
        engine = BenBot()
        engine.send_command("position fen k7/8/8/1pP5/2K5/8/8/8 w - b6 0 1")
        engine.send_command("showpos")

        engine.expect_for_line_matching("FEN*", "*k7/8/8/1pP5/2K5/8/8/8 w - b6 0 1*")

    # def test_position_3(self):
    #     self.engine.send_command("position fen k1r5/8/8/1pP5/2K5/8/8/8 w - b6 0 1")
    #     self.engine.send_command("showpos")
    #
    #     self.engine.expect_for_line_matching(
    #         "XFEN*", "*k1r5/8/8/1pP5/2K5/8/8/8 w - - 0 1*"
    #     )

    # def test_position_4(self):
    #     self.engine.send_command("position fen k1r5/8/8/1pP5/8/2K5/8/8 w - b6 0 1")
    #     self.engine.send_command("showpos")
    #
    #     self.engine.expect_for_line_matching(
    #         "XFEN*", "*k1r5/8/8/1pP5/8/2K5/8/8 w - - 0 1*"
    #     )

    @staticmethod
    def test_position_5():
        engine = BenBot()
        engine.send_command("position fen k1r5/8/8/PpP5/8/2K5/8/8 w - b6 0 1")
        engine.send_command("showpos")

        engine.expect_for_line_matching("FEN*", "*k1r5/8/8/PpP5/8/2K5/8/8 w - b6 0 1*")

    @staticmethod
    def test_position_6():
        engine = BenBot()
        engine.send_command("position fen k1r5/8/8/PpP5/2K5/8/8/8 w - b6 0 1")
        engine.send_command("showpos")

        engine.expect_for_line_matching("FEN*", "*k1r5/8/8/PpP5/2K5/8/8/8 w - b6 0 1*")

    @staticmethod
    def test_position_7():
        engine = BenBot()
        engine.send_command("position fen k7/4b3/8/PpP5/1K6/8/8/8 w - b6 0 1")
        engine.send_command("showpos")

        engine.expect_for_line_matching("FEN*", "*k7/4b3/8/PpP5/1K6/8/8/8 w - b6 0 1*")

    # def test_position_8(self):
    #     self.engine.send_command("position fen k7/b5b1/8/2PpP3/3K4/8/8/8 w - d6 0 1")
    #     self.engine.send_command("showpos")
    #
    #     self.engine.expect_for_line_matching(
    #         "XFEN*", "*k7/b5b1/8/2PpP3/3K4/8/8/8 w - - 0 1*"
    #     )

    # def test_position_9(self):
    #     self.engine.send_command("position fen k7/8/8/r2pPK2/8/8/8/8 w - d6 0 1")
    #     self.engine.send_command("showpos")
    #
    #     self.engine.expect_for_line_matching(
    #         "XFEN*", "*k7/8/8/r2pPK2/8/8/8/8 w - - 0 1*"
    #     )

    @staticmethod
    def test_position_10():
        engine = BenBot()
        engine.send_command("position fen k7/8/8/r1PpPK2/8/8/8/8 w - d6 0 1")
        engine.send_command("showpos")

        engine.expect_for_line_matching("FEN*", "*k7/8/8/r1PpPK2/8/8/8/8 w - d6 0 1*")

    @staticmethod
    def test_position_11():
        engine = BenBot()
        engine.send_command("position fen kb6/8/8/3pP3/5K2/8/8/8 w - d6 0 1")
        engine.send_command("showpos")

        engine.expect_for_line_matching("FEN*", "*kb6/8/8/3pP3/5K2/8/8/8 w - d6 0 1*")


def parse_args():
    parser = argparse.ArgumentParser(description="Run BenBot end-to-end tests")

    parser.add_argument("engine_path", type=str, help="Path to BenBot binary")

    return parser.parse_args()


if __name__ == "__main__":
    args = parse_args()

    framework = MiniTestFramework()

    # Each test suite will be run inside a temporary directory
    framework.run([TestInteractive, TestEnPassantSanitization])

    if framework.has_failed():
        sys.exit(1)

    sys.exit(0)
