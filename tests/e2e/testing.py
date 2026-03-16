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

import subprocess
from typing import List
import os
import collections
import time
import sys
import traceback
import fnmatch
from functools import wraps
from contextlib import redirect_stdout
import io
import concurrent.futures
import tempfile

CYAN_COLOR = "\033[36m"
GRAY_COLOR = "\033[2m"
RED_COLOR = "\033[31m"
GREEN_COLOR = "\033[32m"
RESET_COLOR = "\033[0m"
WHITE_BOLD = "\033[1m"

MAX_TIMEOUT = 60 * 5


class OrderedClassMembers(type):
    @classmethod
    def __prepare__(self, name, bases):
        return collections.OrderedDict()

    def __new__(self, name, bases, classdict):
        classdict["__ordered__"] = [
            key for key in classdict.keys() if key not in ("__module__", "__qualname__")
        ]
        return type.__new__(self, name, bases, classdict)


class TimeoutException(Exception):
    def __init__(self, message: str, timeout: int):
        self.message = message
        self.timeout = timeout


class UnexpectedOutputException(Exception):
    def __init__(self, actual: str, expected: str):
        self.actual = actual
        self.expected = expected


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
                        int(timeout),
                    )
            return result

        return wrapper

    return decorator


class MiniTestFramework:
    def __init__(self):
        self.passed_test_suites = 0
        self.failed_test_suites = 0
        self.passed_tests = 0
        self.failed_tests = 0
        self.start_time = None
        self.stop_on_failure = True

    def has_failed(self) -> bool:
        return self.failed_test_suites > 0

    def run(self, classes: List[type]) -> bool:
        self.start_time = time.time()

        for test_class in classes:
            with tempfile.TemporaryDirectory() as tmpdirname:
                original_cwd = os.getcwd()
                os.chdir(tmpdirname)

                try:
                    if self.__run(test_class):
                        self.failed_test_suites += 1
                    else:
                        self.passed_test_suites += 1
                except Exception as e:
                    self.failed_test_suites += 1
                    print(f"\n{RED_COLOR}Error: {e}{RESET_COLOR}")
                finally:
                    os.chdir(original_cwd)

        self.__print_summary(round(time.time() - self.start_time, 2))
        return self.has_failed()

    def __run(self, test_class) -> bool:
        test_instance = test_class()
        test_name = test_instance.__class__.__name__
        test_methods = [m for m in test_instance.__ordered__ if m.startswith("test_")]

        print(f"\nTest Suite: {test_name}")

        fails = 0

        for method in test_methods:
            fails += self.__run_test_method(test_instance, method)

        self.failed_tests += fails

        return fails > 0

    def __run_test_method(self, test_instance, method: str) -> int:
        print(f"    Running {method}... \r", end="", flush=True)

        buffer = io.StringIO()
        fails = 0

        try:
            t0 = time.time()

            with redirect_stdout(buffer):
                if hasattr(test_instance, "beforeEach"):
                    test_instance.beforeEach()

                getattr(test_instance, method)()

                if hasattr(test_instance, "afterEach"):
                    test_instance.afterEach()

            duration = time.time() - t0

            self.print_success(f" {method} ({duration * 1000:.2f}ms)")
            self.passed_tests += 1
        except Exception as e:
            if isinstance(e, TimeoutException):
                self.print_failure(
                    f" {method} (hit execution limit of {e.timeout} seconds)"
                )

            if isinstance(e, UnexpectedOutputException):
                self.print_failure(
                    f' {method} encountered unexpected output: "{e.actual}" when output matching "{e.expected}" was expected'
                )

            if isinstance(e, AssertionError):
                self.__handle_assertion_error(t0, method)

            if self.stop_on_failure:
                self.__print_buffer_output(buffer)
                raise e

            fails += 1
        finally:
            self.__print_buffer_output(buffer)

        return fails

    def __handle_assertion_error(self, start_time, method: str):
        duration = time.time() - start_time
        self.print_failure(f" {method} ({duration * 1000:.2f}ms)")
        traceback_output = "".join(traceback.format_tb(sys.exc_info()[2]))

        colored_traceback = "\n".join(
            f"  {CYAN_COLOR}{line}{RESET_COLOR}"
            for line in traceback_output.splitlines()
        )

        print(colored_traceback)

    @staticmethod
    def __print_buffer_output(buffer: io.StringIO):
        output = buffer.getvalue()
        if output:
            indented_output = "\n".join(f"    {line}" for line in output.splitlines())
            print(f"    {RED_COLOR}⎯⎯⎯⎯⎯OUTPUT⎯⎯⎯⎯⎯{RESET_COLOR}")
            print(f"{GRAY_COLOR}{indented_output}{RESET_COLOR}")
            print(f"    {RED_COLOR}⎯⎯⎯⎯⎯OUTPUT⎯⎯⎯⎯⎯{RESET_COLOR}")

    def __print_summary(self, duration: float):
        print(f"\n{WHITE_BOLD}Test Summary{RESET_COLOR}\n")
        print(
            f"    Test Suites: {GREEN_COLOR}{self.passed_test_suites} passed{RESET_COLOR}, {RED_COLOR}{self.failed_test_suites} failed{RESET_COLOR}, {self.passed_test_suites + self.failed_test_suites} total"
        )
        print(
            f"    Tests:       {GREEN_COLOR}{self.passed_tests} passed{RESET_COLOR}, {RED_COLOR}{self.failed_tests} failed{RESET_COLOR}, {self.passed_tests + self.failed_tests} total"
        )
        print(f"    Time:        {duration}s\n")

    @staticmethod
    def print_failure(add: str):
        print(f"    {RED_COLOR}✗{RESET_COLOR}{add}", flush=True)

    @staticmethod
    def print_success(add: str):
        print(f"    {GREEN_COLOR}✓{RESET_COLOR}{add}", flush=True)


class BenBot:
    def __init__(
        self,
        path: str,
        args: List[str] = [],
        cli: bool = False,
    ):
        self.path = path
        self.process = None
        self.args = args
        self.cli = cli
        self.output = []

        self.start()

    def __del__(self):
        self.quit()
        assert self.close() == 0

    def _check_process_alive(self):
        if not self.process or self.process.poll() is not None:
            print("\n".join(self.output))
            raise RuntimeError("BenBot process has terminated")

    def start(self):
        if self.cli:
            self.process = subprocess.run(
                [self.path] + self.args + ["--no-loop"],
                capture_output=True,
                text=True,
            )

            if self.process.returncode != 0:
                print(self.process.stdout)
                print(self.process.stderr)
                print(f"Process failed with return code {self.process.returncode}")

            self.process = None

            return

        self.process = subprocess.Popen(
            [self.path] + self.args,
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            universal_newlines=True,
            bufsize=1,
        )

    def setoption(self, name: str, value: str):
        self.send_command(f"setoption name {name} value {value}")

    def send_command(self, command: str):
        if not self.process:
            raise RuntimeError("BenBot process is completed or not started")

        self._check_process_alive()

        self.process.stdin.write(command + "\n")
        self.process.stdin.flush()

    @timeout_decorator(MAX_TIMEOUT)
    def equals(self, expected_output: str):
        for line in self.readline():
            if line == expected_output:
                return

    @timeout_decorator(MAX_TIMEOUT)
    def expect(self, expected_output: str):
        for line in self.readline():
            if fnmatch.fnmatch(line, expected_output):
                return

    @timeout_decorator(MAX_TIMEOUT)
    def contains(self, expected_output: str):
        for line in self.readline():
            if expected_output in line:
                return

    @timeout_decorator(MAX_TIMEOUT)
    def starts_with(self, expected_output: str):
        for line in self.readline():
            if line.startswith(expected_output):
                return

    @timeout_decorator(MAX_TIMEOUT)
    def check_output(self, callback):
        if not callback:
            raise ValueError("Callback function is required")

        for line in self.readline():
            if callback(line):
                return

    @timeout_decorator(MAX_TIMEOUT)
    def expect_for_line_matching(self, line_match: str, expected: str):
        for line in self.readline():
            if fnmatch.fnmatch(line, line_match):
                if fnmatch.fnmatch(line, expected):
                    break
                else:
                    raise UnexpectedOutputException(line, expected)

    def readline(self):
        if not self.process:
            raise RuntimeError("BenBot process is completed or not started")

        while True:
            self._check_process_alive()
            line = self.process.stdout.readline().strip()
            self.output.append(line)

            yield line

    def clear_output(self):
        self.output = []

    def get_output(self) -> List[str]:
        return self.output

    def quit(self):
        if self.process:
            self.send_command("quit")

    def close(self):
        if self.process:
            self.process.stdin.close()
            self.process.stdout.close()
            return self.process.wait()

        return 0
