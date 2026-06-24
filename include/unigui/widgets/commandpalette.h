#pragma once
#include <unigui/widgets/widget_base.h>

#include <functional>
#include <string>
#include <string_view>
#include <vector>

namespace unigui {

namespace detail {

/// Subsequence fuzzy matcher with relevance scoring (fzf-lite).
///
/// Returns true when every character of `pattern` appears in `text`, in order,
/// case-insensitively. `outScore` is set to a relevance score (higher = better):
/// matches at the start of `text`, at word boundaries (after a space / `_` / `-`
/// / `/` / `.` or a camelCase hump), and in contiguous runs score higher; gaps
/// before the first match and between matches are penalised. An empty pattern
/// matches everything with score 0. On a non-match the function returns false
/// and sets `outScore` to 0.
///
/// Pure and allocation-free — the unit-testable core behind CommandPalette
/// ranking; reusable anywhere a "type to filter" list is wanted.
bool FuzzyMatch(std::string_view pattern, std::string_view text, int& outScore);

} // namespace detail

/// CommandPalette — a VS-Code-style (Ctrl+P / Ctrl+Shift+P) fuzzy-searchable
/// command launcher rendered as a centred modal popup. Register commands once;
/// the palette filters and ranks them as the user types, runs the chosen
/// command's `action` on Enter / click, and closes. Up/Down navigate the
/// result list and Esc dismisses.
///
/// The query → ranked-results → execute pipeline is exposed directly
/// (`SetQuery`, `Matches`, `Execute`) so the matching/ordering behaviour is
/// testable without a GL context.
class CommandPalette : public Widget {
public:
    struct Command {
        std::string id;               ///< stable unique identifier
        std::string title;            ///< display label (primary match target)
        std::string category;         ///< optional group/context (secondary, lower weight)
        std::string shortcut;         ///< optional accelerator hint, shown right-aligned
        std::function<void()> action; ///< invoked when the command is chosen
        bool enabled = true;          ///< disabled commands are hidden from results
    };

    explicit CommandPalette(std::string name = "command_palette");

    void Render() override;

    // ── Command registry ──────────────────────────────────────────────────
    CommandPalette& AddCommand(Command cmd);
    /// Convenience overload: id doubles as the title.
    CommandPalette& AddCommand(std::string id, std::string title, std::function<void()> action);
    bool RemoveCommand(const std::string& id);
    void ClearCommands();
    std::size_t CommandCount() const { return commands_.size(); }
    bool HasCommand(const std::string& id) const;

    // ── Open / close ──────────────────────────────────────────────────────
    void Open();
    void Close();
    void Toggle();
    bool IsOpen() const { return open_; }

    // ── Configuration (chainable) ───────────────────────────────────────────
    CommandPalette& SetPlaceholder(std::string s) {
        placeholder_ = std::move(s);
        return *this;
    }
    /// Cap the number of results shown / returned by Matches() (default 50).
    CommandPalette& SetMaxResults(int n) {
        maxResults_ = n < 1 ? 1 : n;
        return *this;
    }

    // ── Query + results (headless-testable) ─────────────────────────────────
    void SetQuery(const std::string& q);
    const std::string& GetQuery() const { return query_; }
    /// Filtered + ranked command ids for the current query, best first, capped
    /// to SetMaxResults(). An empty query returns all enabled commands in
    /// insertion order.
    std::vector<std::string> Matches() const;
    /// Run the command with the given id if it exists and is enabled; invokes
    /// its action, then closes the palette. Returns true if a command ran.
    bool Execute(const std::string& id);

private:
    struct Scored {
        std::size_t index; ///< into commands_
        int score;
    };
    /// Ranked indices into commands_ for the current query (best first, capped).
    std::vector<Scored> ranked() const;

    std::vector<Command> commands_;
    std::string query_;
    std::string placeholder_ = "Type a command…";
    char buf_[256] = {};
    int maxResults_ = 50;
    int selected_ = 0; ///< highlighted row in the ranked list
    bool open_ = false;
    bool focusInput_ = false;    ///< request input focus on the frame after Open()
    bool openRequested_ = false; ///< pending ImGui::OpenPopup on next Render
};

} // namespace unigui
