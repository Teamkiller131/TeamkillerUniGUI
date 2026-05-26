# UniGUI v1.1 — Embedded Fonts & Cross-Platform Polish

## TL;DR

> **Quick Summary**: Embed JetBrains Mono Nerd Font + Noto Sans CJK as binary resources. Eliminate system font dependency. Thai/Emoji rendering. CMake font embedding pipeline. Cross-platform parity.
> 
> **Test Target**: 186+ tests (no regression)
> **Tasks**: ~8 tasks

---

## Context

v1.0.0 uses `AddFontFromMemoryTTF` with system fonts (segoeui.ttf + msyh.ttc). This fails on:
- CI/sandboxed environments (no system font access)
- Linux/macOS (no Windows fonts)
- Containers/docker

Solution: embed open-source fonts directly in the binary via CMake code generation.

## Font Candidates

| Font | License | Size | Coverage | Use |
|------|---------|------|----------|-----|
| JetBrains Mono NL | OFL | ~200KB | ASCII + Powerline | Primary UI font |
| Noto Sans SC | OFL | ~5MB | Simplified Chinese | CJK merge |
| Noto Sans JP | OFL | ~5MB | Japanese | CJK merge |
| Noto Color Emoji | OFL | ~8MB | Emoji | Emoji merge |

**Approach**: Use a minimal approach — embed JetBrains Mono (~200KB) as primary. For CJK/Emoji, fall back to system fonts when available, otherwise skip. Full CJK embedding deferred to v1.2 (20MB+ per font is impractical for a header-only binary).

## Tasks

- [ ] 1. **CMake Font Embedding Pipeline**
  `cmake/FontEmbed.cmake` — custom function `embed_font(TARGET file)`. Uses `xxd -i` or PowerShell equivalent to convert TTF → C array header. Generated header goes to `${CMAKE_BINARY_DIR}/generated/fonts/`.

- [ ] 2. **Download JetBrains Mono Nerd Font**
  `fonts/JetBrainsMonoNLNerdFont-Regular.ttf` — committed to repo (~200KB). SIL OFL license included.

- [ ] 3. **Embed font + load via memory**
  `src/core/font_data.h` (generated) — `AddFontFromMemoryTTF(jb_mono_data, jb_mono_size, ...)`. Load as default font. No fallback needed since it's guaranteed.

- [ ] 4. **Linux/macOS platform defaults**
  - Linux: X11/Wayland → GLFW_GL3 backend
  - macOS: Cocoa → GLFW_GL3 backend
  - Verify AppConfig defaults per-platform

- [ ] 5. **Thai character display fix**
  JetBrains Mono lacks Thai glyphs. Merge a Thai-supporting system font (or embedded small Thai font).

- [ ] 6. **README Font Section**
  Document: embedded fonts (JetBrains Mono), CJK merge strategy, how to load custom fonts.

- [ ] 7. **CHANGELOG v1.1 + Version bump**
  1.0.0 → 1.1.0

- [ ] 8. **Build + Test (186+ tests)**
  Windows static+DLL, verify font loading, verify non-regression.

---

## Scope

### INCLUDE
- JetBrains Mono Nerd Font embedded via CMake code gen
- Thai character support investigation
- Linux/macOS default backends
- Version bump to 1.1.0

### EXCLUDE
- Full CJK/Emoji embedding (too large, deferred to v1.2)
- Runtime font downloading
- Font selection API (user-facing font config)

## v1.2 Preview
- Noto Sans CJK embedding (separate download on first run)
- Emoji font embedding
- Custom font API: `SetFont(ttf_path, size)`
- Font hot-reload
