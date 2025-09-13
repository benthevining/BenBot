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

# The download-artifact action will unzip the artifacts by default (and this can't be disabled).
# However, we want to upload a zip for each platform because on SO platforms, this zip will include
# any shared libraries that the ben_bot executable relies on, and we wouldn't want each of those
# libraries to appear as a separate artifact on the release page.
# So this script finds each top-level directory created when the downloaded artifacts were unzipped,
# and re-zips them into a .zip file with the same filename as the artifact. This zip file is created
# in the specified output directory:
# python3 ReZipArtifacts.py <downloadsRoot> <outputDirectory>

from pathlib import Path
import sys
import os
import shutil

DOWNLOADS_ROOT = Path(sys.argv[1])
OUTPUT_DIR = Path(sys.argv[2])

for dirpath, dirnames, filenames in os.walk(DOWNLOADS_ROOT):
    for subdir_name in dirnames:
        # the docs artifact is a special case
        if subdir_name == 'BenBot-docs':
            os.rename(
                DOWNLOADS_ROOT / subdir_name / 'artifact.tar',
                OUTPUT_DIR / 'BenBot-docs.tar'
            )
        else:
            shutil.make_archive(
                OUTPUT_DIR / subdir_name,
                'zip',
                root_dir=DOWNLOADS_ROOT,
                base_dir=subdir_name
            )

    # we only want the outer loop to run once, to process just the top-level subdirectories
    break
