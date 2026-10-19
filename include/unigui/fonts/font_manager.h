#pragma once
#include <imgui.h>

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace unigui::fonts {

/// Synthetic path of the compiled-in OFL CJK subset. It appears in probe plans,
/// diagnostics and logs so a resolution is always attributable.
inline constexpr const char* kEmbeddedCJKAssetId =
    "embedded:UniGUISansSC-OFL-Subset-Regular.ttf";

/// Where a resolved font file came from. Probe order: Explicit → Embedded → System.
enum class FontSource {
    None,     // not resolved
    Explicit, // caller-supplied path (CJK override / search path / Load* argument)
    Embedded, // compiled-in OFL asset (fonts/UniGUISansSC-OFL-Subset-Regular.ttf)
    System,   // OS font-directory probe (%WINDIR% etc.)
};

/// Human-readable source name for logs and diagnostics ("explicit"/"embedded"/
/// "system"/"none").
const char* FontSourceName(FontSource source);

/// One probe result: the candidates tried in order, which one won, and — when
/// nothing matched — an explicit reason. A failed probe is never silent.
struct FontProbe {
    bool found = false;
    FontSource source = FontSource::None;
    std::string path;                ///< winning file path; "embedded:<asset>" for the built-in font
    std::vector<std::string> probed; ///< candidates tried, in probe order (winner last)
    std::string error;               ///< non-empty when !found: what was missing and where we looked

    /// One-line report for logs/tests.
    std::string ToString() const;
};

/// Result of one font load attempt — carries an explicit error instead of an
/// opaque nullptr.
struct FontLoadResult {
    bool ok = false;
    FontSource source = FontSource::None;
    std::string path;
    std::string error; ///< non-empty when !ok
    ImFont* font = nullptr;
};

/// Snapshot of font-chain health — the "no font environment" fallback report.
/// Build with Manager::Diagnostics() or theme::GetFontFallbackReport().
struct FontDiagnostics {
    bool embeddedCJKAvailable = false; ///< this build ships the OFL CJK subset
    bool systemProbeEnabled = true;    ///< %WINDIR%/distro probing allowed
    bool cjkFound = false;
    FontSource cjkSource = FontSource::None;
    std::string cjkPath;
    std::string cjkError; ///< explicit reason when !cjkFound
    std::vector<std::string> cjkProbed;
    bool emojiFound = false;
    std::string emojiPath;
    std::string emojiError;
    bool bodyLoaded = false;   ///< at least one font is in the current atlas
    bool fallbackOnly = false; ///< body loaded but no CJK fallback: CJK renders as tofu

    /// One-line report suitable for logs and bug reports; never empty.
    std::string Summary() const;
};

/// One registered font. `data` is only set for file/memory loads the atlas owns;
/// it is never freed by Unload()/Reset() (the atlas owns it).
struct FontEntry {
    std::string name;
    ImFont* font = nullptr;
    void* data = nullptr;
    int dataSize = 0;
    std::vector<std::string> fallbacks;
};

class Manager {
public:
    static Manager& Instance();

    // ── Atlas lifecycle ────────────────────────────────────────────────────
    /// Forget every registry entry. Call right after ImGui::GetIO().Fonts->Clear():
    /// the atlas is about to be rebuilt, so all cached ImFont* pointers are stale.
    /// Does NOT free font data — the atlas owns it (same contract as Unload()).
    void Reset();

    // ── Injected probe inputs (checked before embedded/system fonts) ────────
    /// Ordered search paths: CJK font files, or directories containing a known
    /// CJK font file. Tried after the explicit CJK override, before
    /// embedded/system fonts.
    void SetSearchPaths(std::vector<std::string> paths);
    void AddSearchPath(const std::string& path);
    std::vector<std::string> SearchPaths() const;
    void ClearSearchPaths();

    /// Explicit CJK font file ("" = auto: env → search paths → embedded → system).
    void SetCJKPath(const std::string& path);
    std::string CJKPath() const;

    /// Explicit emoji font file ("" = auto: env → system probe).
    void SetEmojiPath(const std::string& path);
    std::string EmojiPath() const;

    /// Toggle OS font-directory probing (tests / deterministic fontless runs).
    void SetSystemProbeEnabled(bool enabled);
    bool SystemProbeEnabled() const;
    /// Toggle the compiled-in OFL CJK asset as a probe candidate.
    void SetEmbeddedFallbackEnabled(bool enabled);
    bool EmbeddedFallbackEnabled() const;
    /// True when this build embeds the OFL CJK subset (UNIGUI_HAS_EMBEDDED_CJK).
    bool HasEmbeddedCJKFont() const;

    /// Full ordered CJK probe plan: UNIGUI_CJK_FONT env → explicit override →
    /// search paths → embedded asset → system fonts. Does not touch the atlas.
    std::vector<std::string> CJKProbePlan() const;
    /// Resolve the CJK candidate without loading it into the atlas.
    FontProbe ProbeCJK() const;
    /// Resolve the emoji candidate without loading it into the atlas.
    FontProbe ProbeEmoji() const;
    /// Current font-chain health, including the explicit no-font-environment
    /// fallback report (see FontDiagnostics::Summary()).
    FontDiagnostics Diagnostics() const;

    // ── Loading ────────────────────────────────────────────────────────────
    /// Load a font from a TTF file. Returns ImFont* or nullptr (LoadChecked() for
    /// the explicit error).
    ImFont* Load(const std::string& name, const std::string& path, float size);

    /// Load a font from a TTF file, returning an explicit result: ok=false with a
    /// human-readable error and an ERROR log entry when the file is missing,
    /// unreadable or rejected by ImGui.
    FontLoadResult LoadChecked(const std::string& name, const std::string& path, float size);

    /// Load a font from memory. Returns ImFont* or nullptr.
    ImFont* LoadFromMemory(const std::string& name, const void* data, int size, float fontSize);

    /// Get a loaded font by name. Returns nullptr if not found.
    ImFont* Get(const std::string& name) const;

    /// Unload a font by name. Returns true if found.
    bool Unload(const std::string& name);

    /// Set the default font for ImGui.
    void SetDefault(const std::string& name);

    /// Add a fallback chain: if glyph missing in 'name', try 'fallbackName'.
    void SetFallback(const std::string& name, const std::string& fallbackName);

    /// List all loaded font names.
    std::vector<std::string> List() const;

    /// Push/Pop font for current ImGui scope.
    void Push(const std::string& name);
    void Pop();

    /// Build the font atlas. Must be called after all fonts are loaded and
    /// BEFORE any text rendering.  **WARNING**: Rebuilding the atlas
    /// invalidates ALL previously obtained ImFont* pointers.  To preserve
    /// them, call Reset() then Get()/Load* again after Build() — or load ALL
    /// fonts first, then Build() once.
    void Build();

    /// Merge the best available CJK font into the default font over `ranges`
    /// (nullptr = built-in BMP CJK ranges). Explicit result: on failure it logs
    /// an ERROR naming every probed candidate and returns found=false + error.
    /// Probe order: UNIGUI_CJK_FONT env → SetCJKPath() → search paths → embedded
    /// OFL subset → system fonts. Idempotent ("cjk" registry entry).
    FontProbe LoadSystemCJKEx(float size = 0, const ImWchar* ranges = nullptr);

    /// Legacy wrapper around LoadSystemCJKEx(): returns the merged font or
    /// nullptr. The failure reason is logged and available via Diagnostics().
    ImFont* LoadSystemCJK(float size = 0);

    /// Load the system emoji font and merge it into the default font as a glyph
    /// fallback. size=0 uses the current default font size. Platform-aware: probes
    /// "Segoe UI Emoji" under %WINDIR% on Windows, "Apple Color Emoji" on macOS, and
    /// the common "Noto Color Emoji" install locations across Linux distros, plus
    /// SetEmojiPath()/UNIGUI_EMOJI_FONT. Registered as "emoji" on success;
    /// idempotent. Non-fatal when absent (WARN + Diagnostics().emojiError).
    void LoadSystemEmoji(float size = 0);

    /// Explicit-result variant of LoadSystemEmoji().
    FontProbe LoadSystemEmojiEx(float size = 0);

    /// Public for the context registry (src/detail/context_registry.h) — prefer
    /// Instance(); direct construction bypasses the per-context lifetime.
    Manager() = default;

private:
    FontProbe resolveCJK() const;
    FontProbe resolveEmoji() const;
    /// Ordered candidates as (source, path/asset-id) pairs.
    std::vector<std::pair<FontSource, std::string>> cjkCandidates() const;
    std::vector<std::pair<FontSource, std::string>> emojiCandidates() const;

    std::unordered_map<std::string, FontEntry> fonts_;
    std::string cjkPath_;             // explicit CJK override ("" = auto)
    std::string emojiPath_;           // explicit emoji override ("" = auto)
    std::vector<std::string> searchPaths_; // caller-injected probe inputs
    bool systemProbe_ = true;         // OS font-dir probing enabled
    bool embeddedFallback_ = true;    // compiled-in OFL asset allowed as fallback
    FontProbe lastCJK_;               // last CJK resolution (for Diagnostics)
    FontProbe lastEmoji_;             // last emoji resolution (for Diagnostics)
};

} // namespace unigui::fonts
