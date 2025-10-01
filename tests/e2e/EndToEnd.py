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
import collections
import concurrent.futures
import fnmatch
import io
import subprocess
import sys
import time
import traceback
from contextlib import redirect_stdout
from functools import wraps
from pathlib import Path
from typing import Callable

#

MAX_TIMEOUT = 60 * 5


class TimeoutException(Exception):
    def __init__(self, message: str, timeout: int):
        self.message = message
        self.timeout = timeout


def timeout_decorator(timeout: float):
    def decorator(func):
        @wraps(func)
        def wrapper(*args, **kwargs):
            with concurrent.futures.ThreadPoolExecutor() as executor:
                future = executor.submit(func, *args, **kwargs)
                try:
                    result = future.result(timeout=timeout)
                except concurrent.futures.TimeoutError:
                    raise TimeoutException(
                        f"Function {func.__name__} timed out after {timeout} seconds",
                        timeout,
                    )
            return result

        return wrapper

    return decorator


class BenBot:
    def __init__(self):
        self.process = None
        self.output = []

        self.process = subprocess.Popen(
            ENGINE_PATH,
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            universal_newlines=True,
            bufsize=1,
        )

    def _check_process_alive(self):
        if not self.process or self.process.poll() is not None:
            print("\n".join(self.output))
            raise RuntimeError("BenBot process has terminated")

    def send_command(self, command: str):
        if not self.process:
            raise RuntimeError("BenBot process is not started")

        self._check_process_alive()

        self.process.stdin.write(f"{command}\n")
        self.process.stdin.flush()

    def set_option(self, name: str, value: str):
        self.send_command(f"setoption name {name} value {value}")

    @timeout_decorator(MAX_TIMEOUT)
    def equals(self, expected_output: str):
        for line in self.readline():
            if line == expected_output:
                return

    @timeout_decorator(MAX_TIMEOUT)
    def starts_with(self, expected_output: str):
        for line in self.readline():
            if line.startswith(expected_output):
                return

    @timeout_decorator(MAX_TIMEOUT)
    def check_output(self, callback: Callable[[str], bool]):
        if not callback:
            raise ValueError("Callback function is required")

        for line in self.readline():
            if callback(line):
                return

    @timeout_decorator(MAX_TIMEOUT)
    def expect(self, expected_output: str):
        for line in self.readline():
            if fnmatch.fnmatch(line, expected_output):
                return

    def readline(self):
        if not self.process:
            raise RuntimeError("BenBot process is not started")

        while True:
            self._check_process_alive()
            line = self.process.stdout.readline().strip()
            self.output.append(line)

            yield line

    def clear_output(self):
        self.output = []

    def quit(self):
        self.send_command("quit")

    def close(self) -> int:
        if self.process:
            self.process.stdin.close()
            self.process.stdout.close()
            return self.process.wait()

        return 0


class OrderedClassMembers(type):
    @classmethod
    def __prepare__(self, name, bases):
        return collections.OrderedDict()

    def __new__(cls, name, bases, classdict):
        classdict["__ordered__"] = [
            key for key in classdict.keys() if key not in ("__module__", "__qualname__")
        ]
        return type.__new__(cls, name, bases, classdict)


class BenBotTests(metaclass=OrderedClassMembers):
    def before_all(self):
        self.engine = BenBot()

    def after_all(self):
        self.engine.quit()
        assert self.engine.close() == 0

    def after_each(self):
        self.engine.clear_output()

    def test_uci_command(self):
        self.engine.send_command("uci")
        self.engine.equals("uciok")

    def test_ucinewgame_and_startpos_nodes_1000(self):
        self.engine.send_command("ucinewgame")
        self.engine.send_command("position startpos")
        self.engine.send_command("go nodes 1000")
        self.engine.starts_with("bestmove")

    def test_ucinewgame_and_startpos_moves(self):
        self.engine.send_command("ucinewgame")
        self.engine.send_command("position startpos moves e2e4 e7e6")
        self.engine.send_command("go nodes 1000")
        self.engine.starts_with("bestmove")

    def test_fen_position_1(self):
        self.engine.send_command("ucinewgame")
        self.engine.send_command("position fen 5rk1/1K4p1/8/8/3B4/8/8/8 b - - 0 1")
        self.engine.send_command("go nodes 1000")
        self.engine.starts_with("bestmove")

    def test_fen_position_2_flip(self):
        self.engine.send_command("ucinewgame")
        self.engine.send_command("position fen 5rk1/1K4p1/8/8/3B4/8/8/8 b - - 0 1")
        self.engine.send_command("flip")
        self.engine.send_command("go nodes 1000")
        self.engine.starts_with("bestmove")

    def test_clear_hash(self):
        self.engine.send_command("setoption name Clear Hash")

    def test_fen_position_mate_1(self):
        self.engine.send_command("ucinewgame")
        self.engine.send_command("position fen 5K2/8/2qk4/2nPp3/3r4/6B1/B7/3R4 w - e6")
        self.engine.send_command("go depth 7")

        self.engine.expect("* score mate 1 *")
        self.engine.equals("bestmove d5e6")

    def test_fen_position_mate_minus_1(self):
        self.engine.send_command("ucinewgame")
        self.engine.send_command(
            "position fen 2brrb2/8/p7/Q7/1p1kpPp1/1P1pN1K1/3P4/8 b - -"
        )
        self.engine.send_command("go depth 7")
        self.engine.expect("* score mate -1 *")
        self.engine.starts_with("bestmove")

    def test_fen_position_fixed_node(self):
        self.engine.send_command("ucinewgame")
        self.engine.send_command(
            "position fen 5K2/8/2P1P1Pk/6pP/3p2P1/1P6/3P4/8 w - - 0 1"
        )
        self.engine.send_command("go nodes 500000")
        self.engine.starts_with("bestmove")

    def test_fen_position_with_mate_go_depth(self):
        self.engine.send_command("ucinewgame")
        self.engine.send_command("position fen 8/5R2/2K1P3/4k3/8/b1PPpp1B/5p2/8 w - -")
        self.engine.send_command("go depth 18 searchmoves c6d7")
        self.engine.expect("* score mate 2 *")

        self.engine.starts_with("bestmove")

    def test_fen_position_with_mate_go_mate(self):
        self.engine.send_command("ucinewgame")
        self.engine.send_command("position fen 8/5R2/2K1P3/4k3/8/b1PPpp1B/5p2/8 w - -")
        self.engine.send_command("go mate 2 searchmoves c6d7")
        self.engine.expect("* score mate 2 *")

        self.engine.starts_with("bestmove")

    def test_fen_position_with_mate_go_nodes(self):
        self.engine.send_command("ucinewgame")
        self.engine.send_command("position fen 8/5R2/2K1P3/4k3/8/b1PPpp1B/5p2/8 w - -")
        self.engine.send_command("go nodes 500000 searchmoves c6d7")
        self.engine.expect("* score mate 2 *")

        self.engine.starts_with("bestmove")

    def test_fen_position_with_mate_go_depth_and_promotion(self):
        self.engine.send_command("ucinewgame")
        self.engine.send_command(
            "position fen 8/5R2/2K1P3/4k3/8/b1PPpp1B/5p2/8 w - - moves c6d7 f2f1q"
        )
        self.engine.send_command("go depth 10")
        self.engine.expect("* score mate 1 *")
        self.engine.starts_with("bestmove f7f5")

    def test_fen_position_with_mate_go_depth_and_searchmoves(self):
        self.engine.send_command("ucinewgame")
        self.engine.send_command("position fen 8/5R2/2K1P3/4k3/8/b1PPpp1B/5p2/8 w - -")
        self.engine.send_command("go depth 10 searchmoves c6d7")
        self.engine.expect("* score mate 2 *")

        self.engine.starts_with("bestmove c6d7")

    def test_fen_position_with_moves_with_mate_go_depth_and_searchmoves(self):
        self.engine.send_command("ucinewgame")
        self.engine.send_command(
            "position fen 8/5R2/2K1P3/4k3/8/b1PPpp1B/5p2/8 w - - moves c6d7"
        )
        self.engine.send_command("go depth 10 searchmoves e3e2")
        self.engine.expect("* score mate -1 *")
        self.engine.starts_with("bestmove e3e2")


class TestFramework:
    def __init__(self):
        self.passed_tests = 0
        self.failed_tests = 0

    def num_failed(self) -> int:
        return self.failed_tests

    def run(self, classes: list[type]):
        self.start_time = time.time()

        for test_class in classes:
            try:
                self.__run(test_class)
            except Exception as e:
                print(f"\nError: {e}")

        self.__print_summary(round(time.time() - self.start_time, 2))

    def __run(self, test_class: type) -> bool:
        test_instance = test_class()
        test_name = test_instance.__class__.__name__
        test_methods = [m for m in test_instance.__ordered__ if m.startswith("test_")]

        print(f"\nTest Suite: {test_name}")

        if hasattr(test_instance, "before_all"):
            test_instance.before_all()

        fails = 0

        for method in test_methods:
            fails += self.__run_test_method(test_instance, method)

        if hasattr(test_instance, "after_all"):
            test_instance.after_all()

        self.failed_tests += fails

        return fails > 0

    def __run_test_method(self, test_instance, method: str) -> int:
        print(f"    Running {method}... \r", end="", flush=True)

        buffer = io.StringIO()
        fails = 0

        try:
            t0 = time.time()

            with redirect_stdout(buffer):
                if hasattr(test_instance, "before_each"):
                    test_instance.before_each()

                getattr(test_instance, method)()

                if hasattr(test_instance, "after_each"):
                    test_instance.after_each()

            duration = time.time() - t0

            self.print_success(f" {method} ({duration * 1000:.2f}ms)")
            self.passed_tests += 1
        except Exception as e:
            if isinstance(e, TimeoutException):
                self.print_failure(
                    f" {method} (hit execution limit of {e.timeout} seconds)"
                )

            if isinstance(e, AssertionError):
                self.__handle_assertion_error(t0, method)

            fails += 1
        finally:
            self.__print_buffer_output(buffer)

        return fails

    def __handle_assertion_error(self, start_time, method: str):
        duration = time.time() - start_time
        self.print_failure(f" {method} ({duration * 1000:.2f}ms)")
        traceback_output = "".join(traceback.format_tb(sys.exc_info()[2]))

        print(traceback_output)

    def __print_buffer_output(self, buffer: io.StringIO):
        output = buffer.getvalue()
        if output:
            indented_output = "\n".join(f"    {line}" for line in output.splitlines())
            print(f"    ⎯⎯⎯⎯⎯OUTPUT⎯⎯⎯⎯⎯")
            print(f"{indented_output}")
            print(f"    ⎯⎯⎯⎯⎯OUTPUT⎯⎯⎯⎯⎯")

    def __print_summary(self, duration: float):
        print(f"\nTest Summary\n")
        print(
            f"    Tests:       {self.passed_tests} passed, {self.failed_tests} failed, {self.passed_tests + self.failed_tests} total"
        )
        print(f"    Time:        {duration}s\n")

    def print_failure(self, add: str):
        print(f"    ✗{add}", flush=True)

    def print_success(self, add: str):
        print(f"    ✓{add}", flush=True)


#

parser = argparse.ArgumentParser(
    prog="EndToEnd",
    description="Run end-to-end tests",
    epilog="This script is intended to be invoked by CTest",
)

parser.add_argument("-e", "--engine", required=True, help="Path to engine executable")

args = parser.parse_args()

ENGINE_PATH = Path(args.engine).resolve()

framework = TestFramework()

framework.run([BenBotTests])

exit(framework.num_failed())
