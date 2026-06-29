# Accessibility (`unigui::a11y`)

Dear ImGui draws its own widgets, so assistive technology (screen readers) has no
native control tree to introspect. UniGUI fills that gap with a small, pure,
headless-testable accessibility layer in `<unigui/core/accessibility.h>` that a
platform bridge can drive. It is **disabled by default** (zero per-frame cost) — call
`unigui::a11y::SetEnabled(true)` (or `InstallLoggingBridge()`) to activate it.

## What it provides

| Part | API | Purpose |
|------|-----|---------|
| **Focus** | `SetFocused` / `Focused` / `SetOnFocusChanged` | The element that holds keyboard focus; a change fires a callback an AT bridge announces. |
| **Tree** | `BeginFrame` / `AddNode` / `Tree` | Every visible focusable element registers per frame, so a bridge / inspector / test can enumerate the whole UI. |
| **Announce** | `Announce` / `SetOnAnnounce` / `DrainAnnouncements` | Focus-independent live-region messages (status, validation, toasts) with `Live::Polite` / `Live::Assertive` politeness. |
| **Inspector** | `DrawInspector` | An ImGui window listing the tree, the focused element, and recent announcements — a dev tool + manual-audit aid. |
| **Reference bridge** | `InstallLoggingBridge` | Routes focus + announcements to the logger — a minimal "screen reader" you can hear in the console. |

A `Node` carries `name`, `description`, `value`, `role` (`Role::Button`, `CheckBox`,
`Input`, `Toggle`, …), and `focused` / `disabled` state.

## Keyboard navigation

The app loop enables `ImGuiConfigFlags_NavEnableKeyboard`, so **Tab / Shift-Tab / arrow
keys** move focus and **Space / Enter** activate — the foundation for keyboard-only
operation, and what the focus tracker rides on. It resets the per-frame tree
(`a11y::BeginFrame()`) at the start of every frame, before your widgets render.

## Wiring widgets

Retained-mode widgets report themselves with one call right after submitting their ImGui
item, passing `ImGui::IsItemFocused()`:

```cpp
ImGui::Checkbox(label_.c_str(), &value_);
ReportAccessible(a11y::Role::CheckBox, ImGui::IsItemFocused(), value_ ? "checked" : "unchecked");
```

`Widget::ReportAccessible(role, focused, value, disabled)` registers the element in the
tree **and**, when focused, drives the focus-changed announcement. The accessible name
comes from `WithAccessibleName(...)` (falling back to the widget id); the description from
`WithAccessibleDescription(...)`. Button, CheckBox, ToggleSwitch, and LineEdit are wired
today; more follow. Raw `unigui::im` / `ImGui::` calls can register directly with
`a11y::AddNode(...)`.

## Announcements

For status that isn't tied to focus — a save confirmation, a validation error, a toast:

```cpp
a11y::Announce("Saved", a11y::Live::Polite);
a11y::Announce("Form has errors", a11y::Live::Assertive); // interrupts
```

Consumers either subscribe (`SetOnAnnounce`) or poll (`DrainAnnouncements`, e.g. in a
headless test).

## Writing a platform bridge

A real screen-reader integration implements the same two callbacks the logging bridge
does, forwarding to the OS AT runtime:

```cpp
a11y::SetEnabled(true);
a11y::SetOnFocusChanged([](const a11y::Node& n) { /* UIA / NSAccessibility / AT-SPI / ARIA */ });
a11y::SetOnAnnounce([](const a11y::Announcement& a) { /* live region */ });
```

`InstallLoggingBridge()` is the reference implementation (logger output). On the web,
the same hooks can mirror focus/announcements into ARIA live regions in the page.

## Try it

The `web_demo` gallery has an **Accessibility** tab: toggle a11y on, open the inspector,
Tab through the *Widgets* tab, and watch the tree + focus update live; the *Announce*
buttons exercise the live region.
