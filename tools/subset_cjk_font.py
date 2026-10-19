#!/usr/bin/env python3
"""Build the embedded OFL CJK subset shipped in fonts/.

Source: Noto Sans SC (SIL Open Font License 1.1), which Google Fonts derives
from Adobe Source Han Sans. Because subsetting produces a Modified Version and
the upstream OFL declares the Reserved Font Name 'Source', the subset must be
renamed — this script rewrites the family names to "UniGUI Sans SC".

Pipeline:
  1. Instance the variable font at wght=400 (Regular).
  2. Subset to the GB2312 charset (standard simplified-Chinese UI coverage,
     including punctuation, kana and full-width forms) plus a few common
     symbol blocks.
  3. Drop hinting/layout tables that a glyph-fallback merge font never uses,
     to stay inside the 2-4 MB budget.
  4. Rename the family (OFL Reserved Font Name compliance) and write a
     deterministic output file.

Reproduce (fonttools >= 4.40):
  py -3 tools/subset_cjk_font.py <path-to-NotoSansSC[wght].ttf> \\
      fonts/UniGUISansSC-OFL-Subset-Regular.ttf

The resulting font is embedded into libunigui by cmake/FontEmbed.cmake
(see src/CMakeLists.txt) and is registered by fonts::Manager as the
"embedded OFL CJK fallback" that a no-system-font environment can rely on.
"""

import os
import sys

from fontTools import subset
from fontTools.ttLib import TTFont
from fontTools.varLib.instancer import instantiateVariableFont

#: GB2312 double-byte rows: 0xA1-0xF7 valid code points. Rows A1-A9 carry
#: punctuation/symbols/kana/Greek/Cyrillic; B0-F7 carry the 6763 level-1 and
#: level-2 simplified hanzi (99.7%+ of everyday Chinese text).
GB2312_ROWS = range(0xA1, 0xF8)
GB2312_TRAILS = range(0xA1, 0xFF)

#: Extra Unicode blocks a CJK UI merges for: CJK punctuation, kana phonetic
#: extensions, full-width forms, general punctuation, letterlike symbols,
#: arrows and geometric shapes.
EXTRA_RANGES = [
    (0x2000, 0x206F),  # General Punctuation
    (0x2100, 0x214F),  # Letterlike Symbols
    (0x2190, 0x21FF),  # Arrows
    (0x25A0, 0x25FF),  # Geometric Shapes
    (0x3000, 0x303F),  # CJK Symbols and Punctuation
    (0x3040, 0x30FF),  # Hiragana + Katakana
    (0x31F0, 0x31FF),  # Katakana Phonetic Extensions
    (0xFF00, 0xFFEF),  # Full-width and half-width forms
]

NEW_FAMILY = "UniGUI Sans SC"
NEW_POSTSCRIPT = "UniGUISansSC-Regular"


def gb2312_charset() -> set:
    chars = set()
    for lead in GB2312_ROWS:
        for trail in GB2312_TRAILS:
            try:
                chars.add(bytes([lead, trail]).decode("gb2312"))
            except UnicodeDecodeError:
                pass
    return chars


def rename(font: TTFont) -> None:
    """Rewrite the name table so the subset does not claim the upstream name."""
    ps_name = NEW_POSTSCRIPT
    records = {
        1: NEW_FAMILY,             # Family
        2: "Regular",              # Subfamily
        3: f"{NEW_POSTSCRIPT};subset",  # Unique ID
        4: f"{NEW_FAMILY} Regular",      # Full name
        6: ps_name,                     # PostScript name
        16: NEW_FAMILY,                 # Typographic family
        17: "Regular",                  # Typographic subfamily
    }
    name_table = font["name"]
    for name_id, value in records.items():
        name_table.setName(value, name_id, 3, 1, 0x409)  # Windows / Unicode BMP / en-US
        name_table.setName(value, name_id, 1, 0, 0)      # Macintosh / Roman / en


def main() -> int:
    if len(sys.argv) < 2:
        print(__doc__)
        return 2
    src = sys.argv[1]
    out = sys.argv[2] if len(sys.argv) > 2 else "fonts/UniGUISansSC-OFL-Subset-Regular.ttf"

    print(f"[subset] source: {src} ({os.path.getsize(src) / 1e6:.2f} MB)")
    font = TTFont(src, recalcTimestamp=False)

    if "fvar" in font:
        print("[subset] instancing variable font at wght=400 (Regular)")
        font = instantiateVariableFont(font, {"wght": 400}, inplace=False)
    else:
        print("[subset] static font: skipping instancing")

    chars = gb2312_charset()
    for lo, hi in EXTRA_RANGES:
        chars.update(chr(cp) for cp in range(lo, hi + 1))
    print(f"[subset] requested chars: {len(chars)}")

    opts = subset.Options()
    opts.flavor = None
    opts.hinting = False            # fallback-merge glyphs are rendered at one size
    opts.layout_features = []       # no shaping needed for a glyph-fallback merge
    opts.name_IDs = ["*"]
    opts.name_languages = ["*"]
    opts.notdef_outline = True
    opts.recalc_bounds = True
    opts.glyph_names = False
    opts.drop_tables += ["DSIG", "meta"]

    subsetter = subset.Subsetter(options=opts)
    subsetter.populate(text="".join(sorted(chars)))
    subsetter.subset(font)

    rename(font)
    font.save(out)
    size = os.path.getsize(out)
    print(f"[subset] wrote {out} ({size / 1e6:.2f} MB, glyphs={font['maxp'].numGlyphs})")
    if size > 4 * 1024 * 1024:
        print("[subset] WARNING: exceeds the 4 MB budget", file=sys.stderr)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
