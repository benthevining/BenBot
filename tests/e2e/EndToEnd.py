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
import subprocess
from functools import wraps
from pathlib import Path

#


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
    def __init__(self, path: str):
        self.path = path
        self.process = None
        self.output = []

        self.start()

    def _check_process_alive(self):
        if not self.process or self.process.poll() is not None:
            print("\n".join(self.output))
            raise RuntimeError("BenBot process has terminated")

    def start(self):
        self.process = subprocess.Popen(
            [self.path],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            universal_newlines=True,
            bufsize=1,
        )

    def send_command(self, command: str):
        if not self.process:
            raise RuntimeError("BenBot process is not started")

        self._check_process_alive()

        self.process.stdin.write(f"{command}\n")
        self.process.stdin.flush()

    def setoption(self, name: str, value: str):
        self.send_command(f"setoption name {name} value {value}")


class OrderedClassMembers(type):
    @classmethod
    def __prepare__(self, name, bases):
        return collections.OrderedDict()

    def __new__(cls, name, bases, classdict):
        classdict["__ordered__"] = [
            key for key in classdict.keys() if key not in ("__module__", "__qualname__")
        ]
        return type.__new__(cls, name, bases, classdict)


#

parser = argparse.ArgumentParser(
    prog="EndToEnd",
    description="Run end-to-end tests",
    epilog="This script is intended to be invoked by CTest",
)

parser.add_argument("-e", "--engine", required=True, help="Path to engine executable")

args = parser.parse_args()

ENGINE_PATH = Path(args.engine).resolve()

# test UCI response


exit(0)
