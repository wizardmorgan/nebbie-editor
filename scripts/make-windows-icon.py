#!/usr/bin/env python3
"""Build nebbie-qt/icons/nebbieedit.ico from nebbieedit-1024.png."""

from __future__ import annotations

import struct
import sys
from pathlib import Path

try:
    from PIL import Image
except ImportError as exc:  # pragma: no cover
    raise SystemExit("Pillow is required: pip install pillow") from exc

ROOT = Path(__file__).resolve().parents[1]
SRC = ROOT / "nebbie-qt" / "icons" / "nebbieedit-1024.png"
OUT = ROOT / "nebbie-qt" / "icons" / "nebbieedit.ico"

SIZES = (16, 32, 48, 64, 128, 256)


def build_ico(image: Image.Image) -> bytes:
    entries = []
    for size in SIZES:
        resized = image.resize((size, size), Image.Resampling.LANCZOS)
        entries.append((size, resized))

    offset = 6 + 16 * len(entries)
    header = struct.pack("<HHH", 0, 1, len(entries))
    directory = bytearray()
    blobs = bytearray()

    for size, img in entries:
        from io import BytesIO

        buf = BytesIO()
        img.save(buf, format="PNG")
        data = buf.getvalue()
        width = 0 if size >= 256 else size
        height = 0 if size >= 256 else size
        directory.extend(struct.pack("<BBBBHHII", width, height, 0, 0, 1, 32, len(data), offset))
        blobs.extend(data)
        offset += len(data)

    return header + bytes(directory) + bytes(blobs)


def main() -> int:
    src = SRC if len(sys.argv) < 2 else Path(sys.argv[1])
    out = OUT if len(sys.argv) < 3 else Path(sys.argv[2])
    if not src.exists():
        print(f"Source icon not found: {src}", file=sys.stderr)
        return 1
    image = Image.open(src).convert("RGBA")
    out.parent.mkdir(parents=True, exist_ok=True)
    out.write_bytes(build_ico(image))
    print(f"Wrote {out}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
