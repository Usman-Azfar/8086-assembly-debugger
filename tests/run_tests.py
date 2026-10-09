"""Build CPROJ.EXE and run the scripted test sessions in DOSBox-X.

Each test is a pair of files in tests/cases/:
    NAME.in   keystrokes fed to the debugger through DOS input redirection
    NAME.exp  lines that must appear in the output, in this order

In .exp files, blank lines and lines starting with '#' are ignored, and
'xxxx' matches any four hex digits (used for segment values, which depend
on where DOS loads the program).

Usage:
    python tests/run_tests.py --jwasm path/to/JWasm.exe --dosbox path/to/dosbox-x.exe
The paths can also be given in the JWASM and DOSBOX environment variables.
"""

import argparse
import os
import re
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SRC = ROOT / "src"
CASES = ROOT / "tests" / "cases"


def build(jwasm, workdir):
    for f in SRC.iterdir():
        shutil.copy(f, workdir)
    result = subprocess.run(
        [jwasm, "-mz", "-Fo", "CPROJ.EXE", "CPROJ.ASM"],
        cwd=workdir, capture_output=True, text=True)
    print(result.stdout.strip())
    if result.returncode != 0 or not (workdir / "CPROJ.EXE").exists():
        print(result.stderr)
        sys.exit("Build failed")


def run_in_dosbox(dosbox, workdir, names):
    commands = ["mount c \"%s\"" % workdir, "c:"]
    for name in names:
        commands.append("CPROJ.EXE < %s.IN > %s.OUT" % (name, name))
    commands.append("exit")
    args = [dosbox, "-silent", "-nopromptfolder", "-defaultconf", "-fastlaunch"]
    for c in commands:
        args += ["-c", c]
    subprocess.run(args, cwd=workdir, stdout=subprocess.DEVNULL,
                   stderr=subprocess.DEVNULL, timeout=120)


def to_regex(line):
    return re.compile(re.escape(line).replace("xxxx", "[0-9A-F]{4}"))


def check(name, output):
    """Return None if every expected line is found in order, else a message."""
    out_lines = output.replace("\r", "").split("\n")
    pos = 0
    exp_file = CASES / (name.lower() + ".exp")
    for raw in exp_file.read_text().splitlines():
        if not raw.strip() or raw.startswith("#"):
            continue
        pattern = to_regex(raw.rstrip())
        while pos < len(out_lines) and not pattern.search(out_lines[pos]):
            pos += 1
        if pos == len(out_lines):
            return "expected line not found (in order):\n    " + raw
        pos += 1
    return None


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--jwasm", default=os.environ.get("JWASM", "jwasm"))
    parser.add_argument("--dosbox", default=os.environ.get("DOSBOX", "dosbox-x"))
    parser.add_argument("--keep", action="store_true",
                        help="keep the work directory and print its path")
    opts = parser.parse_args()
    # The tools run from a temporary directory, so make paths absolute
    for name in ("jwasm", "dosbox"):
        path = Path(getattr(opts, name))
        if path.exists():
            setattr(opts, name, str(path.resolve()))

    names = sorted(p.stem.upper() for p in CASES.glob("*.in"))
    for n in names:
        if len(n) > 8:
            sys.exit("Test name %r is longer than 8 characters (DOS limit)" % n)

    workdir = Path(tempfile.mkdtemp(prefix="cproj-"))
    try:
        build(opts.jwasm, workdir)
        for n in names:
            # DOS input uses CR line endings
            text = (CASES / (n.lower() + ".in")).read_text()
            (workdir / (n + ".IN")).write_bytes(text.replace("\n", "\r").encode("ascii"))
        run_in_dosbox(opts.dosbox, workdir, names)

        failures = 0
        for n in names:
            out_file = workdir / (n + ".OUT")
            output = out_file.read_text(errors="replace") if out_file.exists() else ""
            problem = check(n, output) if output else "no output was produced"
            if problem:
                failures += 1
                print("FAIL  %s: %s" % (n.lower(), problem))
                print("----- output -----\n" + output.replace("\r", "") + "------------------")
            else:
                print("ok    %s" % n.lower())
        print("\n%d of %d tests passed" % (len(names) - failures, len(names)))
        return 1 if failures else 0
    finally:
        if opts.keep:
            print("work directory:", workdir)
        else:
            shutil.rmtree(workdir, ignore_errors=True)


if __name__ == "__main__":
    sys.exit(main())
