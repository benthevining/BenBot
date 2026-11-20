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
import json
import re
import subprocess
import sys
import pathlib
import os
import uuid

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


def wait_until_ready(engine) -> list:
    """Sends isready and returns all output lines received before the readyok."""
    engine.clear_output()
    engine.send_command("isready")
    engine.equals("readyok")
    return engine.get_output()[:-1]


def expect_no_errors(engine):
    """Asserts that the engine printed no info strings since the last command."""
    info_strings = [
        line for line in wait_until_ready(engine) if line.startswith("info string")
    ]
    assert not info_strings, f"Unexpected engine output: {info_strings}"


def get_engine_state(engine) -> dict:
    """Uses the writeconfig command to dump the engine's current state."""
    file_name = f"{uuid.uuid4().hex}.json"
    engine.send_command(f"writeconfig {file_name}")
    engine.expect("info string Wrote configuration file to: *")

    with open(file_name) as file:
        return json.load(file)


def get_option_values(engine) -> dict:
    return get_engine_state(engine)["uci_options"]


def write_json(file_name: str, data):
    with open(file_name, "w") as file:
        if isinstance(data, str):
            file.write(data)
        else:
            json.dump(data, file)


class TestInteractive(metaclass=OrderedClassMembers):
    @staticmethod
    def test_uci_command():
        with BenBot() as engine:
            engine.send_command("uci")
            engine.equals("uciok")

    @staticmethod
    def test_set_threads_option():
        with BenBot() as engine:
            engine.setoption("Threads", "1")
            expect_no_errors(engine)

    @staticmethod
    def test_ucinewgame_and_startpos_nodes_1000():
        with BenBot() as engine:
            engine.send_command("ucinewgame")
            engine.send_command("position startpos")
            engine.send_command("go nodes 1000")
            engine.starts_with("bestmove")

    @staticmethod
    def test_ucinewgame_and_startpos_moves():
        with BenBot() as engine:
            engine.send_command("ucinewgame")
            engine.send_command("position startpos moves e2e4 e7e6")
            engine.send_command("go nodes 1000")
            engine.starts_with("bestmove")

    @staticmethod
    def test_fen_position_1():
        with BenBot() as engine:
            engine.send_command("ucinewgame")
            engine.send_command("position fen 5rk1/1K4p1/8/8/3B4/8/8/8 b - - 0 1")
            engine.send_command("go nodes 1000")
            engine.starts_with("bestmove")

    @staticmethod
    def test_fen_position_2_flip():
        with BenBot() as engine:
            engine.send_command("ucinewgame")
            engine.send_command("position fen 5rk1/1K4p1/8/8/3B4/8/8/8 b - - 0 1")
            engine.send_command("flip")
            engine.send_command("go nodes 1000")
            engine.starts_with("bestmove")

    @staticmethod
    def test_depth_5_with_callback():
        with BenBot() as engine:
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

        with BenBot() as engine:
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
        with BenBot() as engine:
            engine.send_command("position startpos")
            engine.send_command("go depth 5")
            engine.starts_with("bestmove")

            engine.send_command("setoption name Clear Hash")
            expect_no_errors(engine)

            engine.send_command("showpos")
            engine.starts_with("Static eval:")
            assert not any(
                line.startswith("TT hit") for line in wait_until_ready(engine)
            )

    @staticmethod
    def test_fen_position_mate_1():
        with BenBot() as engine:
            engine.send_command("ucinewgame")
            engine.send_command("position fen 5K2/8/2qk4/2nPp3/3r4/6B1/B7/3R4 w - e6")
            engine.send_command("go depth 10")

            engine.expect("* score mate 1 * pv d5e6")
            engine.equals("bestmove d5e6")

    @staticmethod
    def test_fen_position_mate_minus_1():
        with BenBot() as engine:
            engine.send_command("ucinewgame")
            engine.send_command(
                "position fen 2brrb2/8/p7/Q7/1p1kpPp1/1P1pN1K1/3P4/8 b - -"
            )
            engine.send_command("go depth 10")
            engine.expect("* score mate -1 *")
            engine.starts_with("bestmove")

    @staticmethod
    def test_fen_position_fixed_node():
        with BenBot() as engine:
            engine.send_command("ucinewgame")
            engine.send_command(
                "position fen 5K2/8/2P1P1Pk/6pP/3p2P1/1P6/3P4/8 w - - 0 1"
            )
            engine.send_command("go nodes 10000")
            engine.starts_with("bestmove")

    @staticmethod
    def test_fen_position_with_mate_go_depth():
        with BenBot() as engine:
            engine.send_command("ucinewgame")
            engine.send_command("position fen 8/5R2/2K1P3/4k3/8/b1PPpp1B/5p2/8 w - -")
            engine.send_command("go depth 18 searchmoves c6d7")
            engine.expect("* score mate 2 * pv c6d7 * f7f5")

            engine.starts_with("bestmove")

    @staticmethod
    def test_fen_position_with_mate_go_mate():
        with BenBot() as engine:
            engine.send_command("ucinewgame")
            engine.send_command("position fen 8/5R2/2K1P3/4k3/8/b1PPpp1B/5p2/8 w - -")
            engine.send_command("go mate 2 searchmoves c6d7")
            engine.expect("* score mate 2 * pv c6d7 *")

            engine.starts_with("bestmove")

    @staticmethod
    def test_fen_position_with_mate_go_nodes():
        with BenBot() as engine:
            engine.send_command("ucinewgame")
            engine.send_command("position fen 8/5R2/2K1P3/4k3/8/b1PPpp1B/5p2/8 w - -")
            engine.send_command("go nodes 500000 searchmoves c6d7")
            engine.expect("* score mate 2 * pv c6d7 * f7f5")

            engine.starts_with("bestmove")

    @staticmethod
    def test_fen_position_depth_8():
        with BenBot() as engine:
            engine.send_command("ucinewgame")
            engine.send_command(
                "position fen r1b2r1k/pp1p2pp/2p5/2B1q3/8/8/P1PN2PP/R4RK1 w - - 0 18"
            )
            engine.send_command("go depth 8")
            engine.contains("score mate 1")

            engine.starts_with("bestmove")

    @staticmethod
    def test_fen_position_with_mate_go_depth_and_promotion():
        with BenBot() as engine:
            engine.send_command("ucinewgame")
            engine.send_command(
                "position fen 8/5R2/2K1P3/4k3/8/b1PPpp1B/5p2/8 w - - moves c6d7 f2f1q"
            )
            engine.send_command("go depth 13")
            engine.expect("* score mate 1 * pv f7f5")
            engine.starts_with("bestmove f7f5")

    @staticmethod
    def test_fen_position_with_mate_go_depth_and_searchmoves():
        with BenBot() as engine:
            engine.send_command("ucinewgame")
            engine.send_command("position fen 8/5R2/2K1P3/4k3/8/b1PPpp1B/5p2/8 w - -")
            engine.send_command("go depth 18 searchmoves c6d7")
            engine.expect("* score mate 2 * pv c6d7 * f7f5")

            engine.starts_with("bestmove c6d7")

    @staticmethod
    def test_fen_position_with_moves_with_mate_go_depth_and_searchmoves():
        with BenBot() as engine:
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
        with BenBot() as engine:
            engine.send_command(
                "position fen rnbqkbnr/ppp1p1pp/5p2/3pP3/8/8/PPPP1PPP/RNBQKBNR w kq d6 0 3"
            )
            engine.send_command("showpos")

            engine.expect_for_line_matching(
                "FEN*", "*rnbqkbnr/ppp1p1pp/5p2/3pP3/8/8/PPPP1PPP/RNBQKBNR w kq d6 0 3*"
            )

    @staticmethod
    def test_position_2():
        with BenBot() as engine:
            engine.send_command("position fen k7/8/8/1pP5/2K5/8/8/8 w - b6 0 1")
            engine.send_command("showpos")

            engine.expect_for_line_matching(
                "FEN*", "*k7/8/8/1pP5/2K5/8/8/8 w - b6 0 1*"
            )

    @staticmethod
    def test_position_3():
        with BenBot() as engine:
            engine.send_command("position fen k1r5/8/8/1pP5/2K5/8/8/8 w - b6 0 1")
            engine.send_command("showpos")

            engine.expect_for_line_matching(
                "XFEN*", "*k1r5/8/8/1pP5/2K5/8/8/8 w - - 0 1*"
            )

    @staticmethod
    def test_position_4():
        with BenBot() as engine:
            engine.send_command("position fen k1r5/8/8/1pP5/8/2K5/8/8 w - b6 0 1")
            engine.send_command("showpos")

            engine.expect_for_line_matching(
                "XFEN*", "*k1r5/8/8/1pP5/8/2K5/8/8 w - - 0 1*"
            )

    @staticmethod
    def test_position_5():
        with BenBot() as engine:
            engine.send_command("position fen k1r5/8/8/PpP5/8/2K5/8/8 w - b6 0 1")
            engine.send_command("showpos")

            engine.expect_for_line_matching(
                "FEN*", "*k1r5/8/8/PpP5/8/2K5/8/8 w - b6 0 1*"
            )

    @staticmethod
    def test_position_6():
        with BenBot() as engine:
            engine.send_command("position fen k1r5/8/8/PpP5/2K5/8/8/8 w - b6 0 1")
            engine.send_command("showpos")

            engine.expect_for_line_matching(
                "FEN*", "*k1r5/8/8/PpP5/2K5/8/8/8 w - b6 0 1*"
            )

    @staticmethod
    def test_position_7():
        with BenBot() as engine:
            engine.send_command("position fen k7/4b3/8/PpP5/1K6/8/8/8 w - b6 0 1")
            engine.send_command("showpos")

            engine.expect_for_line_matching(
                "FEN*", "*k7/4b3/8/PpP5/1K6/8/8/8 w - b6 0 1*"
            )

    @staticmethod
    def test_position_8():
        with BenBot() as engine:
            engine.send_command("position fen k7/b5b1/8/2PpP3/3K4/8/8/8 w - d6 0 1")
            engine.send_command("showpos")

            engine.expect_for_line_matching(
                "XFEN*", "*k7/b5b1/8/2PpP3/3K4/8/8/8 w - - 0 1*"
            )

    @staticmethod
    def test_position_9():
        with BenBot() as engine:
            engine.send_command("position fen k7/8/8/r2pPK2/8/8/8/8 w - d6 0 1")
            engine.send_command("showpos")

            engine.expect_for_line_matching(
                "XFEN*", "*k7/8/8/r2pPK2/8/8/8/8 w - - 0 1*"
            )

    @staticmethod
    def test_position_10():
        with BenBot() as engine:
            engine.send_command("position fen k7/8/8/r1PpPK2/8/8/8/8 w - d6 0 1")
            engine.send_command("showpos")

            engine.expect_for_line_matching(
                "FEN*", "*k7/8/8/r1PpPK2/8/8/8/8 w - d6 0 1*"
            )

    @staticmethod
    def test_position_11():
        with BenBot() as engine:
            engine.send_command("position fen kb6/8/8/3pP3/5K2/8/8/8 w - d6 0 1")
            engine.send_command("showpos")

            engine.expect_for_line_matching(
                "FEN*", "*kb6/8/8/3pP3/5K2/8/8/8 w - d6 0 1*"
            )


class TestCommands(metaclass=OrderedClassMembers):
    @staticmethod
    def _search(go_command: str, *setup_commands: str):
        with BenBot() as engine:
            for command in setup_commands:
                engine.send_command(command)

            engine.send_command(go_command)
            engine.starts_with("bestmove")

    @staticmethod
    def test_go_nodes():
        TestCommands._search("go nodes 1000")

    @staticmethod
    def test_go_depth():
        TestCommands._search("go depth 6")

    @staticmethod
    def test_go_movetime():
        TestCommands._search("go movetime 1000")

    @staticmethod
    def test_go_short_movetime():
        TestCommands._search("go movetime 200")

    @staticmethod
    def test_go_clock_with_increment():
        TestCommands._search("go wtime 8000 btime 8000 winc 500 binc 500")

    @staticmethod
    def test_go_clock_without_increment():
        TestCommands._search("go wtime 1000 btime 1000 winc 0 binc 0")

    @staticmethod
    def test_go_clock_with_movestogo():
        TestCommands._search("go wtime 1000 btime 1000 winc 0 binc 0 movestogo 5")

    @staticmethod
    def test_go_searchmoves():
        with BenBot() as engine:
            engine.send_command("go nodes 20000 searchmoves e2e4 d2d4")
            engine.check_output(
                lambda line: line in ("bestmove e2e4", "bestmove d2d4")
                or line.startswith(("bestmove e2e4 ", "bestmove d2d4 "))
            )

    @staticmethod
    def test_go_without_ucinewgame_after_position():
        TestCommands._search("go depth 5", "position startpos moves e2e4")

    @staticmethod
    def test_stop():
        with BenBot() as engine:
            engine.send_command("go infinite")
            engine.send_command("stop")
            engine.starts_with("bestmove")

    @staticmethod
    def test_perft_json():
        with BenBot() as engine:
            engine.send_command("perft 4 json")

            def callback(line):
                if not line.startswith("{"):
                    return False

                assert json.loads(line)["totalNodes"] == 197281
                return True

            engine.check_output(callback)

    @staticmethod
    def test_showpos():
        with BenBot() as engine:
            engine.send_command("showpos")
            engine.expect_for_line_matching(
                "FEN*", "*rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
            )

    @staticmethod
    def test_compiler():
        with BenBot() as engine:
            engine.send_command("compiler")
            engine.expect("info string Compiled by *, version *, for *")
            engine.expect("info string Build configuration: *")

    @staticmethod
    def test_unknown_command():
        with BenBot() as engine:
            engine.send_command("got depth 5")
            engine.equals("info string Unknown UCI command: 'got'")
            engine.equals("info string The closest known command is: go")

            # the engine should still be responsive afterwards
            wait_until_ready(engine)

    @staticmethod
    def test_invalid_position_is_ignored():
        with BenBot() as engine:
            engine.send_command("position fen not a fen")
            engine.starts_with("info string Error setting position:")
            engine.equals(
                "info string Retained previous position: rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
            )


class TestEngineInfo(metaclass=OrderedClassMembers):
    @staticmethod
    def _expected_name() -> str:
        return f"BenBot {args.version}" if args.version else "BenBot *"

    @staticmethod
    def test_uci_id():
        with BenBot() as engine:
            engine.send_command("uci")
            engine.expect_for_line_matching(
                "id name *", f"id name {TestEngineInfo._expected_name()}"
            )
            engine.equals("id author Ben Vining")
            engine.equals("uciok")

    @staticmethod
    def test_help_version():
        with BenBot() as engine:
            engine.send_command("help")
            engine.expect_for_line_matching(
                "*BenBot *, by *", f"*{TestEngineInfo._expected_name()}, by Ben Vining*"
            )


class TestOptions(metaclass=OrderedClassMembers):
    @staticmethod
    def test_set_valid_values():
        with BenBot() as engine:
            engine.setoption("MultiPV", "3")
            engine.setoption("Move Format", "UCI")
            engine.setoption("Pretty Print", "true")
            engine.setoption("Move Overhead", "50")
            expect_no_errors(engine)

            values = get_option_values(engine)
            assert values["MultiPV"] == 3
            assert values["Move Format"] == "UCI"
            assert values["Pretty Print"] is True
            assert values["Move Overhead"] == 50

    @staticmethod
    def test_unknown_option_name():
        with BenBot() as engine:
            before = get_option_values(engine)

            engine.setoption("NotAnOption", "3")
            engine.equals("info string Attempted to set unknown option 'NotAnOption'")
            engine.starts_with("info string The closest known option is:")

            assert get_option_values(engine) == before

    @staticmethod
    def test_misspelled_option_name():
        with BenBot() as engine:
            engine.setoption("MultPV", "3")
            engine.equals("info string Attempted to set unknown option 'MultPV'")
            engine.equals("info string The closest known option is: MultiPV")

            assert get_option_values(engine)["MultiPV"] == 1

    @staticmethod
    def test_option_names_are_case_sensitive():
        with BenBot() as engine:
            engine.setoption("multipv", "3")
            engine.equals("info string Attempted to set unknown option 'multipv'")

    @staticmethod
    def test_int_option_out_of_range_is_clamped():
        with BenBot() as engine:
            engine.setoption("MultiPV", "1000")
            assert get_option_values(engine)["MultiPV"] == 256

            engine.setoption("MultiPV", "-5")
            assert get_option_values(engine)["MultiPV"] == 1

    @staticmethod
    def test_int_option_non_numeric_value_is_ignored():
        with BenBot() as engine:
            engine.setoption("MultiPV", "4")
            engine.setoption("MultiPV", "lots")
            assert get_option_values(engine)["MultiPV"] == 4

    @staticmethod
    def test_combo_option_invalid_value_resets_to_default():
        with BenBot() as engine:
            engine.setoption("Move Format", "UCI")
            engine.setoption("Move Format", "Hieroglyphs")
            assert get_option_values(engine)["Move Format"] == "Algebraic"

    @staticmethod
    def test_bool_option_invalid_value_resets_to_default():
        with BenBot() as engine:
            engine.setoption("Pretty Print", "true")
            engine.setoption("Pretty Print", "yes")
            assert get_option_values(engine)["Pretty Print"] is False

    @staticmethod
    def test_option_without_value_is_ignored():
        with BenBot() as engine:
            engine.setoption("MultiPV", "4")
            engine.send_command("setoption name MultiPV")
            assert get_option_values(engine)["MultiPV"] == 4

    @staticmethod
    def test_malformed_setoption_is_ignored():
        with BenBot() as engine:
            before = get_option_values(engine)

            engine.send_command("setoption")
            engine.send_command("setoption MultiPV value 4")
            expect_no_errors(engine)

            engine.send_command("setoption name")
            engine.equals("info string Attempted to set unknown option ''")

            assert get_option_values(engine) == before

    @staticmethod
    def test_search_after_invalid_options():
        with BenBot() as engine:
            engine.setoption("Hash", "0")
            engine.setoption("MultiPV", "1000")
            engine.setoption("Move Overhead", "-1")
            # MultiPV gets clamped to the number of legal moves, so this searches
            # every root move as its own line; keep the depth low for the Debug builds
            engine.send_command("go depth 3")
            engine.starts_with("bestmove")


class TestConfigFile(metaclass=OrderedClassMembers):
    @staticmethod
    def _non_default_state(engine):
        engine.setoption("MultiPV", "7")
        engine.setoption("Move Format", "UCI")
        engine.setoption("Sanitize Positions", "true")
        engine.setoption("Move Overhead", "123")
        engine.send_command("debug on")

    @staticmethod
    def test_round_trip():
        with BenBot() as engine:
            TestConfigFile._non_default_state(engine)
            engine.send_command("writeconfig saved.json")
            engine.expect("info string Wrote configuration file to: *saved.json")
            saved = get_engine_state(engine)

        assert saved["debug"] is True
        assert saved["uci_options"]["MultiPV"] == 7

        with BenBot() as engine:
            assert get_engine_state(engine) != saved

            engine.send_command("readconfig saved.json")
            engine.expect("info string Read configuration from file: *saved.json")

            assert get_engine_state(engine) == saved

    @staticmethod
    def test_config_file_environment_variable():
        with BenBot() as engine:
            TestConfigFile._non_default_state(engine)
            saved = get_engine_state(engine)

        write_json("env.json", saved)

        with BenBot(env={"BENBOT_CONFIG": os.path.abspath("env.json")}) as engine:
            engine.expect("info string Read configuration from file: *env.json")
            assert get_engine_state(engine) == saved

    @staticmethod
    def test_config_affects_engine_behavior():
        write_json("multipv.json", {"debug": False, "uci_options": {"MultiPV": 3}})

        with BenBot() as engine:
            engine.send_command("readconfig multipv.json")
            engine.expect("info string Read configuration from file: *")

            engine.send_command("go depth 5")
            engine.contains(" multipv 3 ")
            engine.starts_with("bestmove")

    @staticmethod
    def test_partial_config_keeps_other_values():
        write_json("partial.json", {"uci_options": {"MultiPV": 4}})

        with BenBot() as engine:
            engine.setoption("Move Format", "UCI")
            engine.send_command("readconfig partial.json")
            engine.expect("info string Read configuration from file: *")

            values = get_option_values(engine)
            assert values["MultiPV"] == 4
            assert values["Move Format"] == "UCI"

    @staticmethod
    def test_config_with_invalid_values():
        write_json(
            "invalid.json",
            {
                "debug": "yes",
                "uci_options": {
                    "MultiPV": "lots",
                    "Move Format": "Hieroglyphs",
                    "Move Overhead": 99999,
                    "NotAnOption": 5,
                },
            },
        )

        with BenBot() as engine:
            engine.send_command("readconfig invalid.json")
            engine.equals(
                "info string Ignoring invalid value for option 'MultiPV': \"lots\""
            )
            engine.expect("info string Read configuration from file: *")

            state = get_engine_state(engine)
            assert state["debug"] is False
            assert state["uci_options"]["MultiPV"] == 1
            assert state["uci_options"]["Move Format"] == "Algebraic"
            assert state["uci_options"]["Move Overhead"] == 5000

    @staticmethod
    def test_malformed_config_file():
        write_json("malformed.json", "this is not json {")

        with BenBot() as engine:
            before = get_engine_state(engine)

            engine.send_command("readconfig malformed.json")
            engine.equals("info string Error parsing engine state: invalid JSON")

            assert get_engine_state(engine) == before

    @staticmethod
    def test_malformed_config_file_environment_variable():
        write_json("malformed.json", "[1, 2")

        with BenBot(env={"BENBOT_CONFIG": os.path.abspath("malformed.json")}) as engine:
            engine.equals("info string Error parsing engine state: invalid JSON")
            engine.send_command("go depth 3")
            engine.starts_with("bestmove")

    @staticmethod
    def test_missing_config_file():
        with BenBot() as engine:
            engine.send_command("readconfig does_not_exist.json")
            engine.expect("info string Could not open file for reading at path *")

    @staticmethod
    def test_missing_config_path():
        with BenBot() as engine:
            engine.send_command("readconfig")
            engine.equals("info string No filepath provided for readconfig")

            engine.send_command("writeconfig")
            engine.equals("info string No filepath provided for writeconfig")


class TestCLI(metaclass=OrderedClassMembers):
    @staticmethod
    def test_bench():
        # the default bench depth takes too long in Debug & sanitizer builds,
        # so this uses the bench UCI command, which takes a depth
        process = subprocess.run(
            [get_path()], input="bench 3\n", capture_output=True, text=True, timeout=600
        )

        output = process.stdout + process.stderr

        assert process.returncode == 0, output
        assert re.search(r"^info string Total nodes: \d+$", process.stdout, re.M)
        assert re.search(r"^info string NPS: \d+$", process.stdout, re.M)

    @staticmethod
    def test_eof_on_stdin_exits_cleanly():
        process = subprocess.run(
            [get_path()], input="isready\n", capture_output=True, text=True, timeout=60
        )

        assert process.returncode == 0
        assert "readyok" in process.stdout


def parse_args():
    parser = argparse.ArgumentParser(description="Run BenBot end-to-end tests")

    parser.add_argument("engine_path", type=str, help="Path to BenBot binary")
    parser.add_argument(
        "--version",
        type=str,
        default=None,
        help="Expected engine version string (if not given, the version isn't checked)",
    )

    return parser.parse_args()


if __name__ == "__main__":
    args = parse_args()

    framework = MiniTestFramework()

    # Each test suite will be run inside a temporary directory
    framework.run(
        [
            TestInteractive,
            TestEnPassantSanitization,
            TestCommands,
            TestEngineInfo,
            TestOptions,
            TestConfigFile,
            TestCLI,
        ]
    )

    if framework.has_failed():
        sys.exit(1)

    sys.exit(0)
