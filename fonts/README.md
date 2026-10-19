# fonts/

Font assets embedded into `libunigui` at build time (`cmake/FontEmbed.cmake`).

This directory is the **only** place binary font files live in this repo.

## Assets

| File | Size | Purpose | License |
|---|---|---|---|
| `JetBrainsMonoNerdFont-Regular.ttf` | 2.47 MB | Default body/mono font, always available | SIL OFL 1.1 (`LICENSE`) |
| `UniGUISansSC-OFL-Subset-Regular.ttf` | 2.25 MB | Embedded CJK glyph fallback (S0/D37) | SIL OFL 1.1, see `OFL-NotoSansSC.txt` |
| `OFL-NotoSansSC.txt` | 4.4 KB | OFL license text for the CJK subset | — |

## Embedded OFL CJK subset (`UniGUISansSC-OFL-Subset-Regular.ttf`)

Guarantees CJK glyph coverage on a host that has **no system CJK font at all**
(servers, CI containers, minimal Windows installs), so CJK text is never
rendered as tofu boxes. It is a *fallback*, not a dependency: a system CJK font
(the host's Microsoft YaHei / Source Han / Noto) is still preferred when
present, because it has full-coverage glyphs.

- **Upstream:** Noto Sans SC, SIL Open Font License 1.1 — Google Fonts:
  `https://raw.githubusercontent.com/google/fonts/main/ofl/notosanssc/NotoSansSC%5Bwght%5D.ttf`
  (Noto Sans SC is derived from Adobe Source Han Sans; both are OFL 1.1.)
- **Modified Version renaming (OFL §Reserved Font Name):** the upstream fonts
  declare the reserved name `Source`; the subset is therefore renamed to family
  **`UniGUI Sans SC`** / PostScript `UniGUISansSC-Regular`. It is *not* the
  upstream font and must not be redistributed under the upstream name.
- **Charset:** GB2312 (8043 requested chars: 6763 simplified hanzi of levels 1–2,
  punctuation, kana, Greek/Cyrillic letterforms) plus General Punctuation,
  Letterlike Symbols, Arrows, Geometric Shapes, CJK Symbols & Punctuation,
  Hiragana/Katakana, Katakana Phonetic Extensions and Full-width Forms.
- **Variable font:** instanced at `wght=400` (Regular); hinting and OpenType
  layout features are dropped because this is a merge-only glyph fallback.
- **Fingerprint:** 2,251,756 bytes, 7735 glyphs,
  SHA256 `9744F83D2A5DC10BDBD054AF20F4D1C39E0068303CD9756AE4D23FC8443A0244`.

### Reproduce

Requires Python 3 + `fonttools` (>= 4.40; built with 4.63.0):

```bash
py -3 tools/subset_cjk_font.py <path-to-NotoSansSC[wght].ttf> \
    fonts/UniGUISansSC-OFL-Subset-Regular.ttf
```

`src/CMakeLists.txt` embeds the file when it exists and defines
`UNIGUI_HAS_EMBEDDED_CJK=1`; otherwise the build prints a `message(WARNING)`
and ships **system-font-only** CJK support (a documented gap — no fake font
data is ever generated). `fonts::Manager::Diagnostics().embeddedCJKAvailable`
reports which mode the build has.

## Runtime resolution (font manager)

`fonts::Manager` probes in this order and never fails silently:

1. `UNIGUI_CJK_FONT` environment variable (explicit absolute path).
2. `Manager::SetCJKPath()` / `ThemeConfig::font_path` (app config).
3. `Manager::SetSearchPaths()` / `UNIGUI_FONT_DIRS` (`;`-separated): font files
   or directories containing a known CJK file name.
4. The embedded OFL subset above.
5. System fonts derived from `%WINDIR%\Fonts` on Windows (msyh.ttc, simsun.ttc,
   simhei.ttf, Noto Sans SC / Source Han Sans SC if installed), the standard
   macOS/Linux paths otherwise. `Manager::SetSystemProbeEnabled(false)` turns
   this step off (deterministic runs / tests).

`UNIGUI_EMOJI_FONT` / `Manager::SetEmojiPath()` override the emoji fallback
probe the same way.

On failure the manager returns an explicit `FontProbe` / `FontLoadResult` with
the probe list and reason, logs at `ERROR`/`WARN`, and exposes the whole chain
through `Manager::Diagnostics()` and `theme::GetFontFallbackReport()` — the
"no font environment" fallback report.
