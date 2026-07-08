#include <unigui/widgets/commandpalette.h>

#include <imgui.h>

#include <algorithm>
#include <cctype>
#include <cstdio>

namespace unigui {

namespace detail {

namespace {
inline char lower(char c) {
    return static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
}

inline bool isBoundaryBefore(std::string_view text, std::size_t i) {
    if (i == 0)
        return true;
    const char prev = text[i - 1];
    const char cur = text[i];
    // Delimiter → next char starts a word.
    if (prev == ' ' || prev == '_' || prev == '-' || prev == '/' || prev == '.' || prev == '\\')
        return true;
    // camelCase hump: lower/digit → upper.
    const bool prevLowerish = (std::islower(static_cast<unsigned char>(prev)) ||
                               std::isdigit(static_cast<unsigned char>(prev)));
    const bool curUpper = std::isupper(static_cast<unsigned char>(cur)) != 0;
    return prevLowerish && curUpper;
}
} // namespace

bool FuzzyMatch(std::string_view pattern, std::string_view text, int& outScore) {
    outScore = 0;
    if (pattern.empty())
        return true; // everything matches the empty query
    if (text.empty())
        return false;

    std::size_t pi = 0;
    int score = 0;
    bool prevMatched = false;
    bool sawFirst = false;

    for (std::size_t i = 0; i < text.size() && pi < pattern.size(); ++i) {
        if (lower(text[i]) != lower(pattern[pi]))
            continue;

        int bonus = 1; // base point per matched char
        if (!sawFirst) {
            // Penalise how far into the string the first match begins.
            score -= static_cast<int>(std::min<std::size_t>(i, 8));
            sawFirst = true;
            if (i == 0)
                bonus += 12; // prefix match
        }
        if (isBoundaryBefore(text, i))
            bonus += 10; // word-boundary start
        if (prevMatched)
            bonus += 6; // contiguous run
        score += bonus;
        prevMatched = true;
        ++pi;
    }

    if (pi != pattern.size()) {
        outScore = 0;
        return false; // not all pattern chars consumed
    }
    // Whole-string equality (case-insensitive) is the strongest signal.
    if (pattern.size() == text.size())
        score += 8;
    outScore = score;
    return true;
}

} // namespace detail

CommandPalette::CommandPalette(std::string name)
        : FluentWidget<CommandPalette>(std::move(name)) {}

CommandPalette& CommandPalette::AddCommand(Command cmd) {
    if (cmd.title.empty())
        cmd.title = cmd.id;
    // Replace an existing command with the same id rather than duplicating.
    for (auto& c : commands_) {
        if (c.id == cmd.id) {
            c = std::move(cmd);
            return *this;
        }
    }
    commands_.push_back(std::move(cmd));
    return *this;
}

CommandPalette& CommandPalette::AddCommand(std::string id, std::string title,
                                           std::function<void()> action) {
    Command c;
    c.id = std::move(id);
    c.title = std::move(title);
    c.action = std::move(action);
    return AddCommand(std::move(c));
}

bool CommandPalette::RemoveCommand(const std::string& id) {
    const auto it = std::find_if(commands_.begin(), commands_.end(),
                                 [&](const Command& c) { return c.id == id; });
    if (it == commands_.end())
        return false;
    commands_.erase(it);
    return true;
}

void CommandPalette::ClearCommands() {
    commands_.clear();
}

bool CommandPalette::HasCommand(const std::string& id) const {
    return std::any_of(commands_.begin(), commands_.end(),
                       [&](const Command& c) { return c.id == id; });
}

void CommandPalette::Open() {
    open_ = true;
    openRequested_ = true;
    focusInput_ = true;
    selected_ = 0;
}

void CommandPalette::Close() {
    open_ = false;
    openRequested_ = false;
}

void CommandPalette::Toggle() {
    if (open_)
        Close();
    else
        Open();
}

void CommandPalette::SetQuery(const std::string& q) {
    query_ = q;
    std::snprintf(buf_, sizeof(buf_), "%s", q.c_str());
    selected_ = 0;
}

std::vector<CommandPalette::Scored> CommandPalette::ranked() const {
    std::vector<Scored> out;
    out.reserve(commands_.size());
    const bool emptyQuery = query_.empty();
    for (std::size_t i = 0; i < commands_.size(); ++i) {
        const Command& c = commands_[i];
        if (!c.enabled)
            continue;
        if (emptyQuery) {
            out.push_back({i, 0});
            continue;
        }
        int titleScore = 0;
        const bool titleHit = detail::FuzzyMatch(query_, c.title, titleScore);
        int catScore = 0;
        const bool catHit = !c.category.empty() && detail::FuzzyMatch(query_, c.category, catScore);
        if (titleHit) {
            out.push_back({i, titleScore});
        } else if (catHit) {
            // Category-only matches always rank below any title match.
            out.push_back({i, catScore - 100});
        }
    }
    // Stable sort by score desc → ties keep insertion order.
    std::stable_sort(out.begin(), out.end(),
                     [](const Scored& a, const Scored& b) { return a.score > b.score; });
    if (static_cast<int>(out.size()) > maxResults_)
        out.resize(static_cast<std::size_t>(maxResults_));
    return out;
}

std::vector<std::string> CommandPalette::Matches() const {
    std::vector<std::string> ids;
    for (const auto& s : ranked())
        ids.push_back(commands_[s.index].id);
    return ids;
}

bool CommandPalette::Execute(const std::string& id) {
    for (const auto& c : commands_) {
        if (c.id != id)
            continue;
        if (!c.enabled)
            return false;
        Close();
        if (c.action)
            c.action();
        return true;
    }
    return false;
}

void CommandPalette::Render() {
    if (!IsVisible())
        return;
    ImGui::PushID(GetName().c_str());

    if (openRequested_) {
        ImGui::OpenPopup("##cmdpalette");
        openRequested_ = false;
    }

    // Centre the palette near the top of the viewport.
    const ImGuiViewport* vp = ImGui::GetMainViewport();
    const ImVec2 size(std::min(560.0f, vp->WorkSize.x * 0.8f), 0.0f);
    ImGui::SetNextWindowPos(
        ImVec2(vp->WorkPos.x + vp->WorkSize.x * 0.5f, vp->WorkPos.y + vp->WorkSize.y * 0.18f),
        ImGuiCond_Always, ImVec2(0.5f, 0.0f));
    ImGui::SetNextWindowSize(size, ImGuiCond_Always);

    const ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    if (ImGui::BeginPopup("##cmdpalette", flags)) {
        const auto results = ranked();
        if (selected_ >= static_cast<int>(results.size()))
            selected_ = results.empty() ? 0 : static_cast<int>(results.size()) - 1;
        if (selected_ < 0)
            selected_ = 0;

        // ── Search input ──────────────────────────────────────────────────
        if (focusInput_) {
            ImGui::SetKeyboardFocusHere();
            focusInput_ = false;
        }
        ImGui::SetNextItemWidth(-1);
        if (ImGui::InputTextWithHint("##cmdq", placeholder_.c_str(), buf_, sizeof(buf_))) {
            query_ = buf_;
            selected_ = 0;
        }

        // ── Keyboard navigation ─────────────────────────────────────────────
        if (ImGui::IsKeyPressed(ImGuiKey_DownArrow))
            selected_ = results.empty() ? 0 : (selected_ + 1) % static_cast<int>(results.size());
        if (ImGui::IsKeyPressed(ImGuiKey_UpArrow) && !results.empty())
            selected_ = (selected_ - 1 + static_cast<int>(results.size())) %
                        static_cast<int>(results.size());
        const bool enter =
            ImGui::IsKeyPressed(ImGuiKey_Enter) || ImGui::IsKeyPressed(ImGuiKey_KeypadEnter);
        if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
            ImGui::CloseCurrentPopup();
            open_ = false;
        }

        ImGui::Separator();

        // ── Results list ────────────────────────────────────────────────────
        std::string toRun;
        ImGui::BeginChild("##cmdresults", ImVec2(0, 320), false);
        for (int r = 0; r < static_cast<int>(results.size()); ++r) {
            const Command& c = commands_[results[r].index];
            const bool isSel = (r == selected_);
            ImGui::PushID(c.id.c_str());
            if (ImGui::Selectable(c.title.c_str(), isSel, ImGuiSelectableFlags_AllowDoubleClick)) {
                toRun = c.id;
            }
            if (isSel &&
                (ImGui::IsKeyPressed(ImGuiKey_DownArrow) || ImGui::IsKeyPressed(ImGuiKey_UpArrow)))
                ImGui::SetScrollHereY();
            // Right-aligned shortcut / category hint.
            const char* hint = !c.shortcut.empty()   ? c.shortcut.c_str()
                               : !c.category.empty() ? c.category.c_str()
                                                     : nullptr;
            if (hint) {
                const float hw = ImGui::CalcTextSize(hint).x;
                ImGui::SameLine(ImGui::GetContentRegionAvail().x - hw);
                ImGui::TextDisabled("%s", hint);
            }
            ImGui::PopID();
        }
        ImGui::EndChild();

        if (enter && !results.empty())
            toRun = commands_[results[selected_].index].id;

        if (!toRun.empty()) {
            ImGui::CloseCurrentPopup();
            Execute(toRun); // sets open_ = false and runs the action
        }
        ImGui::EndPopup();
    } else if (open_) {
        // Popup was dismissed by a click outside / lost focus.
        open_ = false;
    }

    ImGui::PopID();
}

} // namespace unigui
