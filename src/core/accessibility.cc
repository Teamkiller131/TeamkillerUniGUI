#include <unigui/core/accessibility.h>

#include <unigui/core/log.h>

#include <imgui.h>

#include <utility>

namespace unigui::a11y {

namespace {
bool g_enabled = false;

// Focus
bool g_hasFocus = false;
Node g_focused;
FocusFn g_onChanged;

// Per-frame tree
std::vector<Node> g_tree;
bool g_focusSeenThisFrame = false; ///< set when a focused element is reported in a frame

// Announcements
std::vector<Announcement> g_queue;  ///< drainable by pollers
std::vector<Announcement> g_recent; ///< capped ring for the inspector
AnnounceFn g_onAnnounce;
constexpr size_t kRecentMax = 12;
constexpr size_t kQueueMax = 256; ///< bound g_queue so a subscribe-only app can't grow it forever

bool Same(const Node& a, const Node& b) {
    return a.role == b.role && a.name == b.name && a.description == b.description &&
           a.value == b.value && a.disabled == b.disabled;
}
} // namespace

const char* RoleName(Role r) {
    switch (r) {
    case Role::Button:
        return "button";
    case Role::CheckBox:
        return "checkbox";
    case Role::Radio:
        return "radio";
    case Role::Text:
        return "text";
    case Role::Input:
        return "input";
    case Role::Slider:
        return "slider";
    case Role::Combo:
        return "combobox";
    case Role::Tab:
        return "tab";
    case Role::ListItem:
        return "listitem";
    case Role::MenuItem:
        return "menuitem";
    case Role::Link:
        return "link";
    case Role::Table:
        return "table";
    case Role::Tree:
        return "tree";
    case Role::Toggle:
        return "switch";
    case Role::Progress:
        return "progressbar";
    case Role::Window:
        return "window";
    case Role::Group:
        return "group";
    case Role::Status:
        return "status";
    case Role::Dialog:
        return "dialog";
    case Role::Menu:
        return "menu";
    case Role::Unknown:
    default:
        return "unknown";
    }
}

void SetEnabled(bool on) {
    g_enabled = on;
}
bool IsEnabled() {
    return g_enabled;
}

// ── Focus ────────────────────────────────────────────────────────────────────

void SetFocused(const Node& node) {
    if (!g_enabled)
        return;
    g_focusSeenThisFrame = true; // a focused element was reported this frame (even if unchanged)
    if (g_hasFocus && Same(g_focused, node))
        return; // unchanged — don't re-announce
    g_focused = node;
    g_focused.focused = true;
    g_hasFocus = true;
    if (g_onChanged)
        g_onChanged(g_focused);
}

void ClearFocus() {
    if (!g_hasFocus)
        return;
    g_hasFocus = false;
    g_focused = Node{};
    if (g_enabled && g_onChanged)
        g_onChanged(g_focused);
}

bool HasFocus() {
    return g_hasFocus;
}

const Node& Focused() {
    return g_focused;
}

void SetOnFocusChanged(FocusFn fn) {
    g_onChanged = std::move(fn);
}

// ── Per-frame tree ─────────────────────────────────────────────────────────────

void BeginFrame() {
    if (!g_enabled)
        return;
    // If the frame that just finished reported no focused element, focus has left the UI —
    // clear it (fires the focus-changed callback once; a no-op thereafter) so Focused()
    // doesn't go stale. ClearFocus() self-guards on g_hasFocus.
    if (!g_focusSeenThisFrame)
        ClearFocus();
    g_focusSeenThisFrame = false;
    g_tree.clear();
}

void AddNode(const Node& node) {
    if (!g_enabled)
        return;
    g_tree.push_back(node);
    if (node.focused)
        SetFocused(node); // drive the focus-changed callback off the focused element
}

const std::vector<Node>& Tree() {
    return g_tree;
}

// ── Announcements ──────────────────────────────────────────────────────────────

void Announce(std::string message, Live politeness) {
    if (!g_enabled || message.empty())
        return;
    Announcement a{std::move(message), politeness};
    g_queue.push_back(a);
    // Bound the drainable queue: nothing in the app loop or the bridges calls
    // DrainAnnouncements(), so a subscribe-only (or no-consumer) app would otherwise grow
    // it without limit. Pollers still see the most recent kQueueMax.
    if (g_queue.size() > kQueueMax)
        g_queue.erase(g_queue.begin(), g_queue.begin() + (g_queue.size() - kQueueMax));
    g_recent.push_back(a);
    if (g_recent.size() > kRecentMax)
        g_recent.erase(g_recent.begin(), g_recent.begin() + (g_recent.size() - kRecentMax));
    if (g_onAnnounce)
        g_onAnnounce(a);
}

void SetOnAnnounce(AnnounceFn fn) {
    g_onAnnounce = std::move(fn);
}

std::vector<Announcement> DrainAnnouncements() {
    std::vector<Announcement> out;
    out.swap(g_queue);
    return out;
}

// ── Inspector ──────────────────────────────────────────────────────────────────

void DrawInspector(bool* open) {
    if (open && !*open)
        return;
    if (!ImGui::Begin("Accessibility Inspector", open)) {
        ImGui::End();
        return;
    }

    bool enabled = g_enabled;
    if (ImGui::Checkbox("a11y enabled", &enabled))
        SetEnabled(enabled);
    ImGui::SameLine();
    ImGui::TextDisabled("(%d nodes this frame)", (int) g_tree.size());

    ImGui::SeparatorText("Focused");
    if (g_hasFocus) {
        ImGui::Text("%s: \"%s\"", RoleName(g_focused.role), g_focused.name.c_str());
        if (!g_focused.value.empty())
            ImGui::Text("  value: %s", g_focused.value.c_str());
        if (!g_focused.description.empty())
            ImGui::TextDisabled("  %s", g_focused.description.c_str());
    } else {
        ImGui::TextDisabled("(nothing focused)");
    }

    ImGui::SeparatorText("Tree");
    if (ImGui::BeginTable("a11y_tree", 4,
                          ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
                              ImGuiTableFlags_ScrollY,
                          ImVec2(0, 180))) {
        ImGui::TableSetupColumn("Role");
        ImGui::TableSetupColumn("Name");
        ImGui::TableSetupColumn("Value");
        ImGui::TableSetupColumn("State");
        ImGui::TableHeadersRow();
        for (const Node& n : g_tree) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::TextUnformatted(RoleName(n.role));
            ImGui::TableNextColumn();
            ImGui::TextUnformatted(n.name.c_str());
            ImGui::TableNextColumn();
            ImGui::TextUnformatted(n.value.c_str());
            ImGui::TableNextColumn();
            if (n.focused)
                ImGui::TextUnformatted("focused");
            else if (n.disabled)
                ImGui::TextDisabled("disabled");
        }
        ImGui::EndTable();
    }

    ImGui::SeparatorText("Announcements (recent)");
    if (g_recent.empty()) {
        ImGui::TextDisabled("(none)");
    } else {
        for (auto it = g_recent.rbegin(); it != g_recent.rend(); ++it)
            ImGui::BulletText("[%s] %s",
                              it->politeness == Live::Assertive ? "assertive" : "polite",
                              it->message.c_str());
    }

    ImGui::End();
}

// ── Reference logging bridge ────────────────────────────────────────────────────

void InstallLoggingBridge() {
    SetEnabled(true);
    SetOnFocusChanged([](const Node& n) {
        if (n.role == Role::Unknown && n.name.empty()) {
            UNIGUI_LOG_INFO("[a11y] focus cleared");
            return;
        }
        if (n.value.empty())
            UNIGUI_LOG_INFO("[a11y] {} \"{}\"", RoleName(n.role), n.name);
        else
            UNIGUI_LOG_INFO("[a11y] {} \"{}\" = {}", RoleName(n.role), n.name, n.value);
    });
    SetOnAnnounce([](const Announcement& a) {
        UNIGUI_LOG_INFO("[a11y] announce ({}): {}",
                        a.politeness == Live::Assertive ? "assertive" : "polite", a.message);
    });
}

} // namespace unigui::a11y
