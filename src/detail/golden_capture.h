#pragma once

// ─────────────────────────────────────────────────────────────────────────────
// Golden-image capture (internal, CI/dev only)
//
// `UNIGUI_GOLDEN_CAPTURE=<path>` makes the app write the rendered back buffer as
// RAW RGBA after RenderDrawData (before present): an 8-byte header (two int32:
// width, height) followed by tightly-packed RGBA bytes. Raw is deliberate — the
// C++ side stays dependency-free; `scripts/golden.py` owns PNG encoding (stdlib
// zlib), diffing and the corpus. Header-only so both the app loop and the DX11
// renderer can share it without a new translation unit.
// ─────────────────────────────────────────────────────────────────────────────

#include <cstdint>
#include <cstdio>
#include <cstdlib>

namespace unigui::detail {

/// The capture path from the environment, or nullptr when disabled.
inline const char* GoldenCapturePath() {
    const char* e = std::getenv("UNIGUI_GOLDEN_CAPTURE");
    return e && e[0] ? e : nullptr;
}

/// Write one captured frame (tight RGBA, row-major, top-down) to @p path.
/// Overwrites any previous file (the caller re-captures every frame, so the file
/// holds the last frame of a --frames run). Returns false on I/O failure.
inline bool SaveGoldenRaw(const char* path, int w, int h, const std::uint8_t* rgba) {
    if (!path || w <= 0 || h <= 0 || !rgba)
        return false;
    FILE* f = std::fopen(path, "wb");
    if (!f)
        return false;
    const std::int32_t dims[2] = {w, h};
    const bool ok = std::fwrite(dims, sizeof(dims), 1, f) == 1 &&
                    std::fwrite(rgba, 1, (size_t) w * (size_t) h * 4u, f) ==
                            (size_t) w * (size_t) h * 4u;
    std::fclose(f);
    return ok;
}

} // namespace unigui::detail
