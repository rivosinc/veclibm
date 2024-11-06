#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2023 Rivos Inc.
#
# SPDX-License-Identifier: Apache-2.0

import click
from pathlib import Path
import subprocess
import sys

def get_files(git_dir: Path, base_commit: str):
    if base_commit is None:
        git_command = [
            "git", "-C", str(git_dir.absolute()), "ls-tree", "-r", "--name-only", "HEAD"
        ]
    else:
        git_command = [
            "git", "-C", str(git_dir.absolute()), "diff", "--name-only", "--diff-filter=CAMT", base_commit
        ]
    git_files = subprocess.run(git_command, check=True, capture_output=True).\
            stdout.decode("utf-8").strip().split("\n")
    return git_files


def file_filter(fname: str):
    extensions = [".c", ".cpp", ".h", ".hpp"]
    for extension in extensions:
        if fname.endswith(extension):
            return True
    return False


def check_format_changes(git_dir: Path, base_commit: str, clang_format_exe: str):
    git_files = get_files(git_dir, base_commit)
    files_to_update = []
    for fname in filter(file_filter, git_files):
        # Do a dry run, and stop on the first error found
        clang_format_dry = [clang_format_exe, "-n", "--ferror-limit=1", str(git_dir.absolute() / fname)]
        clang_format_output = (
            subprocess.run(clang_format_dry, check=True, capture_output=True)
                      .stderr.decode("utf-8").strip()
        )
        needs_clang_format = (len(clang_format_output) > 0)
        if needs_clang_format:
            files_to_update.append(str(git_dir.absolute() / fname))
    if len(files_to_update) > 0:
        sep="\n\t"
        print(f"Found /usr/bin/clang-format changes in files\n{sep.join(files_to_update)}"
               "\nPlease address /usr/bin/clang-format errors and commit the changes")
        return 1
    print(f"No changes found with '{clang_format_exe}'")
    return 0


def apply_clang_format(git_dir: Path, base_commit: str, clang_format_exe: str):
    git_files = get_files(git_dir, base_commit)
    # Apply clang format to all of the files we are interested in
    for fname in filter(file_filter, git_files):
        clang_format_apply = [clang_format_exe, "-i", str(git_dir.absolute() / fname)]
        subprocess.run(clang_format_apply, check=True)
    return 0


@click.command()
@click.argument(
    "run_type",
    type=click.Choice(["check", "apply"]),
)
@click.option(
    "--base-commit",
    help="Base commit to find files with changes in. "
         "If not given, runs clang format on the whole repo"
)
@click.option(
    "--clang-format-exe",
    help="Command to use for clang-format. This needs to point to the same "
         "version of clang-format as run on the CI, otherwise you may end up "
         "with different corrections. At the time of writing (2024/11), this is "
         "version 14.0",
    default="clang-format"
)
def main(run_type, base_commit, clang_format_exe):
    git_dir = Path(__file__).absolute().parent.parent

    clang_format_func = check_format_changes if (run_type == "check") else apply_clang_format
    sys.exit(clang_format_func(git_dir, base_commit, clang_format_exe))


if __name__ == "__main__":
    main()
