# UniGUI — Fix CJK Font Loading

## Root Cause

`LoadDefaultFont()` in `src/theme/theme.cc` tries to load TTF fonts with backslash paths:
```cpp
"C:\\Windows\\Fonts\\segoeui.ttf"  // backslashes — may fail in ImGui's ImFileOpen
```

Despite font files existing on disk, `AddFontFromFileTTF` returns NULL for all paths, falling back to built-in 13px font. CJK merge also fails for the same reason.

## Tasks

- [ ] 1. Fix font path format + add diagnostics
  **File**: `src/theme/theme.cc` — `LoadDefaultFont()` function
  **Change**: Replace backslash paths with forward slashes:
  ```cpp
  "C:/Windows/Fonts/segoeui.ttf"
  "C:/Windows/Fonts/arial.ttf"
  "C:/Windows/Fonts/calibri.ttf"
  ```
  Also log which fonts failed: `UNIGUI_LOG_DEBUG("Font not loaded: {}", path)`
  And log which font succeeded: `UNIGUI_LOG_INFO("Font: {} ({}px)", path, (int)size_pixels)`
  **Same change** for CJK merge paths (msyh.ttc, simhei.ttf) — use forward slashes.

- [ ] 2. Build + test
  **Command**: `$env:VCPKG_ROOT="D:\vcpkg"; cmake --build --preset windows-msvc-debug; ctest --preset windows-msvc-debug`
  **Expected**: 186/186 pass, `hello_unigui --frames 2` logs show "Font: C:/Windows/Fonts/arial.ttf (24px)" instead of "Built-in font"

- [ ] 3. Verify CJK rendering
  **Command**: `hello_unigui.exe --frames 3`
  **Expected**: i18n panel shows Chinese/Japanese/Korean/Arabic/Emoji text correctly

## Notes
- Forward slashes work in `fopen()` on Windows and are preferred by ImGui's `ImFileOpen`
- calibri.ttf added as additional fallback (always available on Windows 10+)
