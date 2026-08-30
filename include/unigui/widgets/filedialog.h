#pragma once
#include <unigui/widgets/widget_base.h>

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace unigui {

namespace detail {

/// One entry in a directory listing.
struct DirEntry {
    std::string name;        ///< file / sub-directory name (no path)
    bool isDir = false;      ///< true for sub-directories (and "..")
    std::uintmax_t size = 0; ///< file size in bytes (0 for directories)
};

/// Case-insensitive test of whether `filename`'s extension is in `exts`.
/// `exts` entries include the leading dot (e.g. ".csv"); an empty `exts`
/// accepts everything. Pure — no filesystem access.
bool ExtensionMatches(const std::string& filename, const std::vector<std::string>& exts);

/// List `dir` into `out`, sorted directories-first then files, each group
/// alphabetical (case-insensitive). When `exts` is non-empty only files whose
/// extension matches are included; directories are always included. Dotfiles
/// are included only when `showHidden`. Non-throwing: returns false (and clears
/// `out`) when `dir` cannot be read. Does not prepend a ".." entry — the
/// FileDialog adds that itself.
bool ListDirectory(const std::string& dir, const std::vector<std::string>& exts, bool showHidden,
                   std::vector<DirEntry>& out);

} // namespace detail

/// FileDialog — an in-ImGui file / folder picker. Dear ImGui ships no native
/// file dialog, so apps either pull in a platform dialog (loses theming, blocks
/// the loop) or hand-roll one; this is the hand-rolled one, themed and modal.
///
/// Three modes: open an existing file, choose a save path (with a filename
/// field), or select a folder. Navigation (enter sub-dir, go up, extension
/// filtering, resolved-path computation) is exposed as plain methods so the
/// behaviour is unit-testable against a real temp directory without a GL
/// context; only the actual drawing needs ImGui.
class FileDialog : public FluentWidget<FileDialog> {
public:
    enum class Mode { OpenFile, SaveFile, SelectFolder };

    explicit FileDialog(std::string name = "file_dialog");

    void Render() override;

    // ── Configuration (chainable) ───────────────────────────────────────────
    FileDialog& SetMode(Mode m) {
        mode_ = m;
        return *this;
    }
    Mode GetMode() const { return mode_; }
    /// Set the current directory. Ignored (returns *this unchanged state) if the
    /// path does not name an existing directory.
    FileDialog& SetDirectory(const std::string& dir);
    const std::string& GetDirectory() const { return dir_; }
    /// Accepted file extensions, each with a leading dot (e.g. {".csv", ".txt"}).
    /// Empty = all files. Ignored in SelectFolder mode.
    FileDialog& SetFilters(std::vector<std::string> exts) {
        filters_ = std::move(exts);
        return *this;
    }
    /// Initial / current filename for SaveFile mode.
    FileDialog& SetFilename(const std::string& name);
    FileDialog& SetTitle(std::string t) {
        title_ = std::move(t);
        return *this;
    }
    FileDialog& SetShowHidden(bool on) {
        showHidden_ = on;
        return *this;
    }
    bool GetShowHidden() const { return showHidden_; }

    // ── Open / close ──────────────────────────────────────────────────────
    void Open();
    void Close();
    bool IsOpen() const { return open_; }

    // ── Callbacks ───────────────────────────────────────────────────────────
    void SetOnConfirm(std::function<void(const std::string& path)> fn) {
        onConfirm_ = std::move(fn);
    }
    void SetOnCancel(std::function<void()> fn) { onCancel_ = std::move(fn); }
    /// The path produced by the most recent confirm (empty until confirmed).
    const std::string& GetSelectedPath() const { return confirmedPath_; }

    // ── Fluent (chainable) helpers — return FileDialog& via CRTP base ──────
    FileDialog& WithOnConfirm(std::function<void(const std::string& path)> fn) {
        SetOnConfirm(std::move(fn));
        return *this;
    }
    FileDialog& WithOnCancel(std::function<void()> fn) {
        SetOnCancel(std::move(fn));
        return *this;
    }

    // ── Navigation / state (headless-testable) ───────────────────────────────
    /// The current directory's filtered, sorted listing (no ".." entry).
    std::vector<detail::DirEntry> Entries() const;
    /// Enter a sub-directory of the current directory. Returns false if `dirName`
    /// is not an existing sub-directory.
    bool NavigateInto(const std::string& dirName);
    /// Move to the parent directory. Returns false if already at a filesystem
    /// root (no parent).
    bool NavigateUp();
    /// Select a file in the current directory (OpenFile/SaveFile). No filesystem
    /// check — just records the highlighted name.
    void SelectFile(const std::string& name);
    const std::string& GetSelectedFile() const { return selected_; }
    /// Does `filename` pass the current extension filter?
    bool MatchesFilter(const std::string& filename) const;
    /// The path Confirm() would produce right now for the current mode +
    /// selection + filename. Empty when the selection is incomplete (e.g.
    /// OpenFile with nothing selected, or SaveFile with an empty filename).
    std::string ResolvedPath() const;
    /// Apply the current selection: computes ResolvedPath(), records it as the
    /// selected path, fires onConfirm, and closes. Returns the resolved path
    /// (empty + no-op if the selection is incomplete).
    std::string Confirm();

private:
    Mode mode_ = Mode::OpenFile;
    std::string dir_;
    std::string title_ = "Select a file";
    std::string selected_;      ///< highlighted file name (no path)
    std::string confirmedPath_; ///< last confirmed full path
    std::vector<std::string> filters_;
    char nameBuf_[256] = {}; ///< SaveFile filename field
    bool showHidden_ = false;
    bool open_ = false;
    bool openRequested_ = false;
    std::function<void(const std::string&)> onConfirm_;
    std::function<void()> onCancel_;
};

} // namespace unigui
