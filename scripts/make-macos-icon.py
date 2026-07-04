#!/usr/bin/env python3
"""Build nebbie-qt/icons/nebbieedit.icns from nebbieedit-1024.png."""

from __future__ import annotations

import io
import struct
import sys
from pathlib import Path

try:
    from PIL import Image
except ImportError as exc:  # pragma: no cover
    raise SystemExit("Pillow is required: pip install pillow") from exc

ROOT = Path(__file__).resolve().parents[1]
SRC = ROOT / "nebbie-qt" / "icons" / "nebbieedit-1024.png"
OUT = ROOT / "nebbie-qt" / "icons" / "nebbieedit.icns"

# PNG payloads for common Retina icon slots.
ICON_SLOTS = (
    ("icp4", 16),
    ("icp5", 32),
    ("icp6", 64),
    ("ic07", 128),
    ("ic08", 256),
    ("ic09", 512),
    ("ic10", 1024),
)


def png_bytes(image: Image.Image, size: int) -> bytes:
    resized = image.resize((size, size), Image.Resampling.LANCZOS)
    buf = io.BytesIO()
    resized.save(buf, format="PNG")
    return buf.getvalue()


def build_icns(image: Image.Image) -> bytes:
    body = bytearray()
    for otype, size in ICON_SLOTS:
        data = png_bytes(image, size)
        chunk = otype.encode("ascii") + struct.pack(">I", len(data) + 8) + data
        while len(chunk) % 4:
            chunk += b"\x00"
        body.extend(chunk)
    return b"icns" + struct.pack(">I", 8 + len(body)) + bytes(body)


def main() -> int:
    src = SRC if len(sys.argv) < 2 else Path(sys.argv[1])
    out = OUT if len(sys.argv) < 3 else Path(sys.argv[2])
    if not src.exists():
        print(f"Source icon not found: {src}", file=sys.stderr)
        return 1
    image = Image.open(src).convert("RGBA")
    out.parent.mkdir(parents=True, exist_ok=True)
    out.write_bytes(build_icns(image))
    print(f"Wrote {out}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
