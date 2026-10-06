# SPDX-License-Identifier: MIT-0
"""Read DataMatrix codes with the dmr Python package.

Install the wheel for your platform from GitHub Releases first:

    pip install dmr-1.0.0-py3-none-win_amd64.whl

Usage:

    python example.py frame.jpg [more images...]
"""

import sys

import dmr
import dmr.license as license


def main(paths):
    print("dmr", dmr.version())

    # Trial period (30 days from the first run) or a license is required.
    status = license.status()
    if not status["can_scan"]:
        print(status["message"])
        return 3

    rc = 0
    # One scanner for the whole run: the symbol size hint accumulates across frames.
    with dmr.Scanner(dmr.Settings.single()) as scanner:
        for path in paths:
            # GRAY is the cheapest input for the scanner; the result is the same as for COLOR.
            with dmr.Image(path, dmr.LoadAs.GRAY) as img:
                if not img:
                    print(f"{path}: {img.status.name}")  # NOT_FOUND or UNREADABLE
                    rc = 1
                    continue
                result = scanner.scan(img.view())

            if result.ok:
                for code in result.codes:
                    # code.text is bytes: the GS1 separator stays as byte 0x1D.
                    text = code.text.decode("utf-8", errors="replace").replace("\x1d", "<GS>")
                    print(f"{path}: {text}  ({code.module_count}x{code.module_count}, {result.elapsed_ms:.1f} ms)")
            elif result.status == dmr.Status.NOT_FOUND:
                print(f"{path}: no symbol in the frame")
                rc = 1
            else:
                print(f"{path}: symbol found but could not be read")
                rc = 1
    return rc


if __name__ == "__main__":
    if len(sys.argv) < 2:
        print(__doc__)
        sys.exit(2)
    sys.exit(main(sys.argv[1:]))
