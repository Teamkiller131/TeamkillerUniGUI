#pragma once

#include <functional>
#include <string>
#include <vector>

namespace unigui::a11y {

// ─────────────────────────────────────────────────────────────────────────────
// Accessibility foundation (namespace unigui::a11y)
//
// Dear ImGui renders its own widgets, so assistive technology (screen readers)
// has nothing to introspect — there is no platform a11y tree. This module is the
// missing seam, in three parts, all pure + headless-testable and independent of
// any AT runtime:
//
//   • Focus      — widgets report the focused element's semantic descriptor; a
//                  change fires a callback an AT bridge announces.
//   • Tree       — every visible focusable element registers per frame, so a
//                  bridge / inspector / test can enumerate the whole UI.
//   • Announce   — focus-independent live-region messages (status, validation,
//                  toasts) with ARIA-style politeness.
//
// A platform bridge (Windows UI Automation, macOS NSAccessibility, AT-SPI, or a
// web ARIA shim) subscribes to the focus + announce callbacks; InstallLoggingBridge()
// is a minimal reference bridge that speaks to the logger.
//
// Disabled by default (zero overhead): call SetEnabled(true) to activate.
// ─────────────────────────────────────────────────────────────────────────────

/// Semantic role of an accessible element (maps onto platform a11y control types).
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
    Toggle,   ///< on/off switch
    Progress, ///< progress bar
    Window,   ///< top-level window / panel
    Group,    ///< logical grouping (groupbox, form section)
    Status,   ///< status bar / live status text
    Dialog,   ///< modal / dialog
    Menu,     ///< menu / menu bar
};

/// An accessible element's descriptor.
struct Node {
    std::string name;        ///< accessible name (label / fallback to widget id)
    std::string description; ///< extended description / tooltip
    std::string value;       ///< current value text (for inputs/sliders), if any
    Role role = Role::Unknown;
    bool focused = false;  ///< does this element currently hold keyboard focus
    bool disabled = false; ///< is the element disabled (non-interactive)
};

/// Human-readable role name (for the AT bridge / logging / tests).
const char* RoleName(Role r);

/// Master switch. When disabled (the default), all of the report calls below are
/// no-ops so there is no per-frame cost in apps that don't opt in.
void SetEnabled(bool on);
bool IsEnabled();

// ── Focus ────────────────────────────────────────────────────────────────────

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

// ── Per-frame accessibility tree ───────────────────────────────────────────────

/// Reset the tree for a new frame. The app loop calls this once per frame (in
/// NewFrame) before any widget renders. No-op when disabled.
void BeginFrame();
/// Register a visible/focusable element for this frame. Widgets call this after
/// laying out their item (with `node.focused = ImGui::IsItemFocused()`), which
/// also forwards the focused node to SetFocused(). No-op when disabled.
void AddNode(const Node& node);
/// The elements registered since the last BeginFrame(), in render (≈ nav) order.
const std::vector<Node>& Tree();

// ── Live announcements (ARIA-style live regions) ───────────────────────────────

/// Politeness: Polite waits for a pause in speech; Assertive interrupts.
enum class Live { Polite, Assertive };
struct Announcement {
    std::string message;
    Live politeness = Live::Polite;
};
/// Queue a focus-independent announcement (status change, validation, toast).
/// Fires the announce callback immediately (if set) AND enqueues it for pollers.
/// No-op when disabled.
void Announce(std::string message, Live politeness = Live::Polite);
/// Subscribe to announcements — a platform bridge routes these to the AT runtime.
using AnnounceFn = std::function<void(const Announcement&)>;
void SetOnAnnounce(AnnounceFn fn);
/// Return and clear the queued announcements (for pollers / headless tests).
std::vector<Announcement> DrainAnnouncements();

// ── Inspector + reference bridge ───────────────────────────────────────────────

/// Render an ImGui window listing the current accessibility tree, the focused
/// element, and recent announcements — a dev tool + manual a11y-audit aid.
/// Pass an optional open flag for a closable window.
void DrawInspector(bool* open = nullptr);

/// Wire focus changes + announcements to the logger — a minimal reference
/// "screen reader" you can hear in the console. A real platform bridge
/// (UIA / NSAccessibility / AT-SPI / ARIA) implements FocusFn/AnnounceFn the same
/// way. Also calls SetEnabled(true).
void InstallLoggingBridge();

/// Install the platform screen-reader bridge and enable a11y. On Windows this raises
/// UI Automation notification events (spoken by Narrator / NVDA / JAWS) for focus
/// changes + announcements; on other platforms it currently falls back to
/// InstallLoggingBridge(). Pass the native window handle (HWND on Windows, from
/// `unigui::GetNativeWindowHandle()`); pass nullptr to force the logging fallback.
void InstallSystemBridge(void* nativeWindowHandle);

} // namespace unigui::a11y
