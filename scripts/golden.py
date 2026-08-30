#!/usr/bin/env python3
"""Golden-image pipeline for UniGUI (CI/dev).

The C++ side writes RAW RGBA (16-byte header: two int32 width/height, then tightly
packed RGBA) when UNIGUI_GOLDEN_CAPTURE=<path> is set — no dependencies there. This
script owns PNG encoding (stdlib zlib), running an example to capture a frame, and
diffing two images with a per-pixel threshold + changed-region summary.

Usage:
  py scripts/golden.py raw2png <capture.raw> <out.png>
  py scripts/golden.py capture <exe> <out.png> [exe args...]
  py scripts/golden.py diff <a.png> <b.png> [--threshold N] [--max-pixels P]
"""

import argparse
import os
import struct
import subprocess
import sys
import tempfile
import zlib


# ── Minimal PNG codec (8-bit RGB/RGBA, non-interlaced) ─────────────────────────

def _chunk(tag: bytes, data: bytes) -> bytes:
    return (struct.pack(">I", len(data)) + tag + data +
            struct.pack(">I", zlib.crc32(tag + data) & 0xFFFFFFFF))


def load_png(path: str) -> tuple[bytes, int, int, int]:
    """Return (pixels, width, height, channels) for an 8-bit RGB/RGBA PNG."""
    raw = open(path, "rb").read()
    assert raw[:8] == b"\x89PNG\r\n\x1a\n", f"{path}: not a PNG"
    pos, w, h, ctype, idat = 8, 0, 0, 0, bytearray()
    while pos < len(raw):
        (length,) = struct.unpack(">I", raw[pos:pos + 4])
        tag = raw[pos + 4:pos + 8]
        data = raw[pos + 8:pos + 8 + length]
        if tag == b"IHDR":
            w, h, depth, ctype = struct.unpack(">IIBB", data[:10])
            assert depth == 8 and ctype in (2, 6), f"{path}: only 8-bit RGB/RGBA supported"
        elif tag == b"IDAT":
            idat += data
        elif tag == b"IEND":
            break
        pos += 12 + length
    assert w and h, f"{path}: no IHDR"
    channels = 4 if ctype == 6 else 3
    stride = w * channels
    decompressed = zlib.decompress(bytes(idat))
    prev = bytearray(stride)
    out = bytearray(stride * h)
    p = 0
    for y in range(h):
        f = decompressed[p]
        p += 1
        row = bytearray(decompressed[p:p + stride])
        p += stride
        if f == 1:  # Sub
            for i in range(channels, stride):
                row[i] = (row[i] + row[i - channels]) & 0xFF
        elif f == 2:  # Up
            for i in range(stride):
                row[i] = (row[i] + prev[i]) & 0xFF
        elif f == 3:  # Average
            for i in range(stride):
                left = row[i - channels] if i >= channels else 0
                row[i] = (row[i] + ((left + prev[i]) >> 1)) & 0xFF
        elif f == 4:  # Paeth
            for i in range(stride):
                a = row[i - channels] if i >= channels else 0
                b = prev[i]
                c = prev[i - channels] if i >= channels else 0
                pa, pb, pc = abs(b - c), abs(a - c), abs(a + b - 2 * c)
                pred = a if (pa <= pb and pa <= pc) else (b if pb <= pc else c)
                row[i] = (row[i] + pred) & 0xFF
        out[y * stride:(y + 1) * stride] = row
        prev = row
    return bytes(out), w, h, channels


def save_png(path: str, pixels: bytes, w: int, h: int, channels: int) -> None:
    stride = w * channels
    raw = bytearray()
    for y in range(h):
        raw.append(0)  # filter: None
        raw += pixels[y * stride:(y + 1) * stride]
    ihdr = struct.pack(">IIBBBBB", w, h, 8, 6 if channels == 4 else 2, 0, 0, 0)
    png = (b"\x89PNG\r\n\x1a\n" + _chunk(b"IHDR", ihdr) +
           _chunk(b"IDAT", zlib.compress(bytes(raw), 6)) + _chunk(b"IEND", b""))
    with open(path, "wb") as f:
        f.write(png)


def load_raw(path: str) -> tuple[bytes, int, int, int]:
    raw = open(path, "rb").read()
    assert len(raw) >= 16, f"{path}: not a UniGUI raw capture"
    w, h = struct.unpack("<ii", raw[:8])
    pixels = raw[8:]
    assert len(pixels) == w * h * 4, f"{path}: size mismatch ({len(pixels)} != {w * h * 4})"
    return pixels, w, h, 4


# ── Commands ──────────────────────────────────────────────────────────────────

def cmd_raw2png(a: argparse.Namespace) -> int:
    pixels, w, h, c = load_raw(a.raw)
    save_png(a.out, pixels, w, h, c)
    print(f"wrote {a.out} ({w}x{h}, {c} channels)")
    return 0


def cmd_capture(a: argparse.Namespace) -> int:
    with tempfile.NamedTemporaryFile(suffix=".raw", delete=False) as tmp:
        raw_path = tmp.name
    env = dict(os.environ)
    env["UNIGUI_GOLDEN_CAPTURE"] = raw_path
    try:
        r = subprocess.run([a.exe, *a.args], env=env)
        if r.returncode != 0:
            print(f"{a.exe} exited {r.returncode}", file=sys.stderr)
            return r.returncode or 1
        pixels, w, h, c = load_raw(raw_path)
        save_png(a.out, pixels, w, h, c)
        print(f"wrote {a.out} ({w}x{h})")
        return 0
    finally:
        os.unlink(raw_path)


def cmd_diff(a: argparse.Namespace) -> int:
    pa, wa, ha, ca = load_png(a.a)
    pb, wb, hb, cb = load_png(a.b)
    if (wa, ha) != (wb, hb):
        print(f"size mismatch: {a.a} {wa}x{ha} vs {a.b} {wb}x{hb}")
        return 1
    changed = 0
    minx, miny, maxx, maxy = wa, ha, -1, -1
    for y in range(ha):
        for x in range(wa):
            i = (y * wa + x) * 3
            if (abs(pa[i] - pb[i]) > a.threshold or
                    abs(pa[i + 1] - pb[i + 1]) > a.threshold or
                    abs(pa[i + 2] - pb[i + 2]) > a.threshold):
                changed += 1
                minx, miny = min(minx, x), min(miny, y)
                maxx, maxy = max(maxx, x), max(maxx, y)
    print(f"{a.a} vs {a.b}: {wa}x{ha}, {changed} pixels differ "
          f"(threshold {a.threshold})" +
          (f", changed region ({minx},{miny})-({maxx},{maxy})" if changed else ""))
    return 0 if changed <= a.max_pixels else 1


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = ap.add_subparsers(dest="cmd", required=True)

    p = sub.add_parser("raw2png", help="convert a raw capture to PNG")
    p.add_argument("raw")
    p.add_argument("out")
    p.set_defaults(fn=cmd_raw2png)

    p = sub.add_parser("capture", help="run an example with capture and keep the PNG")
    p.add_argument("exe")
    p.add_argument("out")
    p.add_argument("args", nargs="*")
    p.set_defaults(fn=cmd_capture)

    p = sub.add_parser("diff", help="compare two images")
    p.add_argument("a")
    p.add_argument("b")
    p.add_argument("--threshold", type=int, default=8,
                   help="per-channel difference that counts as changed (default 8)")
    p.add_argument("--max-pixels", type=int, default=0,
                   help="fail if more than this many pixels differ (default 0 = any)")
    p.set_defaults(fn=cmd_diff)

    a = ap.parse_args()
    return a.fn(a)


if __name__ == "__main__":
    sys.exit(main())
