"""Build the static live-demo site.

Packs CPROJ.EXE into a js-dos bundle (a zip containing the program and
.jsdos/dosbox.conf) and copies the web page next to it.

Usage:
    python web/make_site.py path/to/CPROJ.EXE output_dir
"""

import shutil
import sys
import zipfile
from pathlib import Path

WEB = Path(__file__).resolve().parent


def main():
    if len(sys.argv) != 3:
        sys.exit(__doc__)
    exe = Path(sys.argv[1])
    out = Path(sys.argv[2])
    out.mkdir(parents=True, exist_ok=True)

    with zipfile.ZipFile(out / "cproj.jsdos", "w", zipfile.ZIP_DEFLATED) as z:
        z.write(WEB / "dosbox.conf", ".jsdos/dosbox.conf")
        z.write(exe, "CPROJ.EXE")

    shutil.copy(WEB / "index.html", out / "index.html")
    print("site written to", out)


if __name__ == "__main__":
    main()
