#!/usr/bin/env python3
"""Reads the trackpad's QMK debug console and timestamps each line.

Use it with the macos_debug firmware to capture a gesture log for a bug
report. Needs the `hidapi` package (pip install hidapi).

    python3 tools/console.py [logfile]

Each line is prefixed with milliseconds since start. Stop with Ctrl-C.
"""
import sys
import time

import hid

VID = 0xFEED
CONSOLE_USAGE_PAGE = 0xFF31


def open_console():
    for d in hid.enumerate(VID):
        if d["usage_page"] == CONSOLE_USAGE_PAGE:
            dev = hid.device()
            dev.open_path(d["path"])
            return dev
    return None


def main():
    out = open(sys.argv[1], "w") if len(sys.argv) > 1 else None
    dev = None
    t0 = time.monotonic()
    buf = ""
    print("waiting for trackpad console...", file=sys.stderr)
    while True:
        if dev is None:
            dev = open_console()
            if dev is None:
                time.sleep(0.5)
                continue
            print("connected", file=sys.stderr)
        try:
            data = dev.read(64, 200)
        except OSError:
            dev = None
            continue
        if not data:
            continue
        buf += bytes(data).split(b"\0", 1)[0].decode("ascii", "replace")
        while "\n" in buf:
            line, buf = buf.split("\n", 1)
            stamped = f"{(time.monotonic() - t0) * 1000:9.1f} {line}"
            print(stamped)
            if out:
                out.write(stamped + "\n")
                out.flush()


if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        pass
