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
# This script parses the warnings log produced by Doxygen
# and outputs check info in JSON format.

import argparse
import json
import sys
from pathlib import Path

parser = argparse.ArgumentParser(
    prog="ReportDoxygenWarnings",
    description="Parse Doxygen warnings log and report in JSON format for GitHub Actions check run",
)

parser.add_argument("--summary", action="store_true", help="Produce summary")

parser.add_argument(
    "--doxygen-log",
    required=True,
    help="Path to Doxygen warnings log",
    dest="doxygen_log",
)

parser.add_argument(
    "--repo-root", help="Path to the root of the BenBot repo", dest="repo_root"
)

args = parser.parse_args()

with open(args.doxygen_log) as file:
    log_lines = file.readlines()

# filter empty log lines
log_lines = [line for line in log_lines if line]

if args.summary:
    num_warnings = len(log_lines)

    if num_warnings == 0:
        summary = "No warnings produced."
    else:
        summary = f"Doxygen produced {num_warnings} warnings"

    data = {
        "title": "Docs build",
        "summary": summary,
    }

    print(json.dumps(data))

    sys.exit(0)

annotations = []

repo_root = Path(args.repo_root)

for line in log_lines:
    before, _, after = line.partition(": warning: ")

    pathStr, _, lineStr = before.partition(":")

    line = int(lineStr)
    fullPath = Path(pathStr)

    data = {
        "path": str(fullPath.relative_to(repo_root)),
        "start_line": line,
        "end_line": line,
        "annotation_level": "warning",
        "message": after.strip(),
    }

    annotations.append(data)

print(json.dumps(annotations))
