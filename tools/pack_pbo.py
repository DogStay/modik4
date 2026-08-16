#!/usr/bin/env python3
"""Packs a mod folder into a .pbo, the way Mikero's tools do.

Only the parts DayZ actually needs are implemented, because a script mod needs
no more than that:

  * one header extension entry carrying the prefix, taken from $PBOPREFIX$;
  * one uncompressed entry per file, paths written with backslashes;
  * the terminating entry, then the file data, then the trailing SHA1.

Nothing is binarised. The engine reads a raw config.cpp out of a PBO perfectly
well, and leaving it as text keeps the packed mod diffable against the source.

Usage:
    python3 tools/pack_pbo.py <source-folder> <output.pbo>
"""

import hashlib
import os
import struct
import sys

# Files that describe the packing rather than belonging in the archive.
EXCLUDED_NAMES = {"$PBOPREFIX$", "$pboprefix$"}
EXCLUDED_SUFFIXES = (".pbo", ".bak", ".pyc")


def collect_files(root):
    """Every packable file, as (archive path with backslashes, absolute path)."""
    collected = []

    for directory, subdirectories, names in os.walk(root):
        # Deterministic order: the same source tree must produce the same PBO.
        subdirectories.sort()

        for name in sorted(names):
            if name in EXCLUDED_NAMES:
                continue

            if name.lower().endswith(EXCLUDED_SUFFIXES):
                continue

            absolute = os.path.join(directory, name)
            relative = os.path.relpath(absolute, root)
            collected.append((relative.replace(os.sep, "\\"), absolute))

    return collected


def read_prefix(root, fallback):
    """The addon prefix, which every script path in the config is resolved against."""
    path = os.path.join(root, "$PBOPREFIX$")

    if not os.path.isfile(path):
        return fallback

    with open(path, "rb") as handle:
        return handle.read().decode("utf-8").strip().strip("\x00")


def entry(name, packing, original_size, reserved, timestamp, data_size):
    packed = name.encode("utf-8") + b"\x00"
    packed += struct.pack("<5I", packing, original_size, reserved, timestamp, data_size)
    return packed


def pack(source, output):
    root = os.path.abspath(source)
    prefix = read_prefix(root, os.path.basename(root))
    files = collect_files(root)

    header = bytearray()

    # The version entry: an empty name, the "Vers" packing marker, and a list of
    # zero-terminated key/value pairs closed by an empty string. The prefix is
    # the only property that matters here.
    header += entry("", 0x56657273, 0, 0, 0, 0)
    header += b"prefix\x00" + prefix.encode("utf-8") + b"\x00"
    header += b"\x00"

    payload = bytearray()

    for archive_name, absolute in files:
        with open(absolute, "rb") as handle:
            content = handle.read()

        timestamp = int(os.path.getmtime(absolute))
        # Packing 0 means stored: size and data size are the same number.
        header += entry(archive_name, 0, len(content), 0, timestamp, len(content))
        payload += content

    # The terminator is an entry with an empty name and five zeroed fields.
    header += entry("", 0, 0, 0, 0, 0)

    body = bytes(header) + bytes(payload)
    digest = hashlib.sha1(body).digest()

    with open(output, "wb") as handle:
        handle.write(body)
        handle.write(b"\x00")
        handle.write(digest)

    return prefix, len(files), len(body) + 21


def main():
    if len(sys.argv) != 3:
        print(__doc__)
        return 1

    source, output = sys.argv[1], sys.argv[2]

    if not os.path.isdir(source):
        print("Not a folder: " + source)
        return 1

    os.makedirs(os.path.dirname(os.path.abspath(output)), exist_ok=True)
    prefix, count, size = pack(source, output)

    print("%s  prefix=%s  files=%d  bytes=%d" % (output, prefix, count, size))
    return 0


if __name__ == "__main__":
    sys.exit(main())
