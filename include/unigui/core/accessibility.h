#pragma once

#include <functional>
#include <string>

namespace unigui::a11y {

// ─────────────────────────────────────────────────────────────────────────────
// Accessibility foundation (namespace unigui::a11y)
//
// Dear ImGui renders its own widgets, so assistive technology (screen readers)
// has nothing to introspect — there is no platform a11y tree. This module is the
// missing seam: widgets report a small semantic descriptor (name / role / value)
// when they gain focus, and a process-wide tracker fires a change event. A
// platform bridge (e.g. Windows UI Automation) subscribes to that event to
// announce the focused element; the *model + event stream here* is pure and
// headless-testable, independent of any AT runtime.
//
// Disabled by default (zero overhead): call SetEnabled(true) to activate.
// ─────────────────────────────────────────────────────────────────────────────

/// Semantic role of a focusable element (maps onto platform a11y control types).
enum class Role {
    Unknown,
    Button,
    CheckBox,
    Radio,
    Text,
    Input,
    Slider,
    Combo,
    Tab,
    ListItem,
    MenuItem,
    Link,
    Table,
    Tree,
};

/// A focusable element's accessible descriptor.
struct Node {
    std::string name;        ///< accessible name (label / fallback to widget id)
    std::string description; ///< extended description / tooltip
    std::string value;       ///< current value text (for inputs/sliders), if any
    Role role = Role::Unknown;
};

/// Human-readable role name (for the AT bridge / logging / tests).
const char* RoleName(Role r);

/// Master switch. When disabled (the default), SetFocused() is a no-op so there
/// is no per-frame cost in apps that don't opt in.
void SetEnabled(bool on);
bool IsEnabled();

/// Report the element that now has focus. Fires the focus-changed callback only
/// when the descriptor actually changes (so widgets may call it every frame
/// while focused without spamming). No-op when disabled.
void SetFocused(const Node& node);
/// Clear the focused element (fires the callback with an empty node if changed).
void ClearFocus();
bool HasFocus();
const Node& Focused();

/// Subscribe to focus changes — this is what a platform AT bridge implements.
using FocusFn = std::function<void(const Node&)>;
void SetOnFocusChanged(FocusFn fn);

} // namespace unigui::a11y
