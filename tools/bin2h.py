#!/usr/bin/env python3

import argparse
import contextlib
from pathlib import Path
import sys


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("-w", "--width", type=int, default=12)
    parser.add_argument("input", type=Path)
    parser.add_argument("output", type=Path)
    parser.add_argument("name")
    args = parser.parse_args()

    try:
        with args.input.open("rb") as f:
            data = f.read()
    except OSError as e:
        print(e, file=sys.stderr)
        return 1

    stream = open(args.output, "w")
    try:
        with contextlib.redirect_stdout(stream):
            print("#pragma once\n"
                  "\n"
                  f"constexpr std::uint8_t {args.name}[] = {{")
            for offset in range(0, len(data), args.width):
                chunk = data[offset:offset + args.width]
                values = ", ".join(f"0x{byte:02X}" for byte in chunk)
                print(f"  {values},")
            print("};")
    finally:
        stream.close()


if __name__ == "__main__":
    main()
