#include <unigui/core/accessibility.h>

namespace unigui::a11y {

namespace {
bool g_enabled = false;
bool g_hasFocus = false;
Node g_focused;
FocusFn g_onChanged;

bool Same(const Node& a, const Node& b) {
    return a.role == b.role && a.name == b.name && a.description == b.description &&
           a.value == b.value;
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

void SetFocused(const Node& node) {
    if (!g_enabled)
        return;
    if (g_hasFocus && Same(g_focused, node))
        return; // unchanged — don't re-announce
    g_focused = node;
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

} // namespace unigui::a11y
