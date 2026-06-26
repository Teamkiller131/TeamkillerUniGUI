# Immediate-Mode API Reference — `unigui::im`

> Reference for TeamkillerUniGUI **3.16.0** · header: [`include/unigui/im/im.h`](../include/unigui/im/im.h)

The `unigui::im` namespace is TeamkillerUniGUI's **immediate-mode layer**: a
thin, themed, allocation-light set of free functions that wrap Dear ImGui so the
common "just draw a control" case stays a one-liner. There are ~210 functions
covering buttons, text, every flavour of input/slider/drag, selection widgets,
windows and child regions, layout and cursor control, popups/menus/tabs/trees,
color editors, plots, item and input queries, and direct draw-list access.

```cpp
#include <unigui/im/im.h>

if (unigui::im::Button("Save")) save();
unigui::im::Checkbox("Enabled", &enabled);
unigui::im::SliderFloat("Gain", &gain, 0.0f, 1.0f);
```

---

## What the immediate layer is (and is not)

TeamkillerUniGUI has **two UI layers that coexist**:

| Layer | Style | State | Use it for |
|-------|-------|-------|------------|
| Retained widgets (`unigui::Button`, `unigui::Table`, …) | classes, fluent `With*` setters | persistent — validation, undo/redo, serialization | long-lived UI you configure once and reuse |
| Immediate (`unigui::im::Button`, …) | stateless free functions | none — you own all state | one-off controls, custom drawing, throwaway UI |

The immediate functions live in `unigui::im` **on purpose**: the retained-mode
widgets are *classes* in the plain `unigui` namespace, so same-named free
functions would collide with them. The immediate layer **complements** — it does
not replace — the retained widgets.

**Pick `unigui::im` when:**

- You want a control without inventing a unique name, owning a `shared_ptr`, or
  spelling out `ImGui::`.
- You are doing **custom drawing** and need cursor/clip/draw-list access.
- You are inside a **`dsl::Custom`** node — the immediate layer is the escape
  hatch that lets arbitrary immediate-mode drawing (or a hosted `Component`)
  live inside a declarative DSL tree:

  ```cpp
  using namespace unigui;
  auto node = dsl::Custom([&] {
      im::Text("Drawn verbatim each frame, at the current cursor:");
      if (im::Button("Reset")) value = 0;
      im::SliderInt("Value", &value, 0, 100);
  });
  ```

Everything here is **themed**: buttons, sliders and inputs pick up the active
dark/light theme automatically. The `im::InputText*` helpers additionally
overlay a DPI-correct text caret (working around imgui #7031) via
`DrawActiveInputCaret()`.

### String conventions

- Every string parameter is a `std::string_view`.
- For optional `const char*` parameters (overlays, hints, formats), passing an
  **empty view (`{}` or `""`)** means *"use ImGui's default (`nullptr`)"*.
- Text functions take literal text, **not** a printf format string — there is no
  variadic formatting and no `%` parsing (`Text` / `TextUnformatted` render the
  bytes you pass). Format your strings with `std::format` first.

### What is intentionally absent

The immediate layer deliberately omits a curated set of raw ImGui calls, because
those concerns are handled elsewhere in TeamkillerUniGUI:

- **Style-var / style-color push-pop** (`PushStyleVar`, `PushStyleColor`, …) —
  styling is **theme-driven**; use the theme engine and CSS-like style layer
  instead of hand-pushing colors.
- **ID / font push-pop** (`PushID`, `PushFont`, …) — ID safety is handled by the
  RAII **scope guards** in `<unigui/core/scope.h>` (`IDScope`, `WindowScope`, …),
  and fonts are managed by the font module.
- **Key-chord / shortcut routing** helpers — handled at a higher level.

If you genuinely need one of these, drop down to raw `ImGui::` for that one call.

---

## `ButtonVariant` enum

Color variants for `im::Button`, mirroring `unigui::Button::ColorVariant`:

```cpp
enum class ButtonVariant { Default, Primary, Danger, Success, Warning };
```

---

## Buttons

| Signature | Note |
|-----------|------|
| `bool Button(std::string_view label, const ImVec2& size = ImVec2(0,0))` | Themed button; returns true on the frame it is clicked. |
| `bool Button(std::string_view label, ButtonVariant variant, const ImVec2& size = ImVec2(0,0))` | Button with an explicit color variant. |
| `bool SmallButton(std::string_view label)` | Small single-line button for inline placement. |
| `bool InvisibleButton(std::string_view strId, const ImVec2& size, ImGuiButtonFlags flags = 0)` | Clickable invisible region for custom hit-testing (use with `IsItem*`). |
| `bool ArrowButton(std::string_view strId, ImGuiDir dir)` | Small square button with a directional arrow glyph. |
| `bool ColorButton(std::string_view descId, const ImVec4& col, ImGuiColorEditFlags flags = 0, const ImVec2& size = ImVec2(0,0))` | Clickable color swatch; returns true when clicked. |

---

## Text

| Signature | Note |
|-----------|------|
| `void Text(std::string_view text)` | Plain text label. |
| `void TextWrapped(std::string_view text)` | Text that wraps to the content region width. |
| `void TextDisabled(std::string_view text)` | Greyed-out text. |
| `void TextColored(const ImVec4& color, std::string_view text)` | Text in an explicit color. |
| `void BulletText(std::string_view text)` | Bullet glyph followed by text. |
| `void LabelText(std::string_view label, std::string_view text)` | Displays the `text` value in a framed area with the `label` shown to its right. |
| `void TextUnformatted(std::string_view text)` | Raw text, no parsing — fastest path for long/literal strings. |
| `bool TextLink(std::string_view label)` | Clickable text hyperlink; returns true on the frame it is clicked. |
| `void TextLinkOpenURL(std::string_view label, std::string_view url = {})` | Hyperlink that opens `url` (defaults to the label) on click. |

---

## Boolean & selection inputs (checkbox / radio)

| Signature | Note |
|-----------|------|
| `bool Checkbox(std::string_view label, bool* value)` | Checkbox bound to a `bool`. |
| `bool CheckboxFlags(std::string_view label, int* flags, int flagsValue)` | Checkbox toggling bits in an `int` flags field. |
| `bool CheckboxFlags(std::string_view label, unsigned int* flags, unsigned int flagsValue)` | Same for an `unsigned int` flags field. |
| `bool RadioButton(std::string_view label, int* current, int buttonValue)` | Radio bound to an index; selects `buttonValue`. |
| `bool RadioButton(std::string_view label, bool active)` | Stateless radio; `active` controls the filled state. |

---

## Numeric inputs — Sliders / Drags / Input fields

### Float sliders

| Signature | Note |
|-----------|------|
| `bool SliderFloat(std::string_view label, float* value, float min, float max, std::string_view format = "%.3f")` | Single float slider. |
| `bool SliderFloat2(std::string_view label, float v[2], float min, float max, std::string_view format = "%.3f")` | 2-component float slider. |
| `bool SliderFloat3(std::string_view label, float v[3], float min, float max, std::string_view format = "%.3f")` | 3-component float slider. |
| `bool SliderFloat4(std::string_view label, float v[4], float min, float max, std::string_view format = "%.3f")` | 4-component float slider. |
| `bool SliderAngle(std::string_view label, float* v_rad, float degreesMin = -360.0f, float degreesMax = 360.0f, std::string_view format = "%.0f deg")` | Angle slider; `v_rad` is radians, displayed as degrees. |
| `bool VSliderFloat(std::string_view label, const ImVec2& size, float* value, float min, float max, std::string_view format = "%.3f")` | Vertical float slider; `size` is the widget dimensions. |

### Int sliders

| Signature | Note |
|-----------|------|
| `bool SliderInt(std::string_view label, int* value, int min, int max, std::string_view format = "%d")` | Single int slider. |
| `bool SliderInt2(std::string_view label, int v[2], int min, int max, std::string_view format = "%d")` | 2-component int slider. |
| `bool SliderInt3(std::string_view label, int v[3], int min, int max, std::string_view format = "%d")` | 3-component int slider. |
| `bool SliderInt4(std::string_view label, int v[4], int min, int max, std::string_view format = "%d")` | 4-component int slider. |
| `bool VSliderInt(std::string_view label, const ImVec2& size, int* value, int min, int max, std::string_view format = "%d")` | Vertical int slider. |

### Float drags

| Signature | Note |
|-----------|------|
| `bool DragFloat(std::string_view label, float* value, float speed = 1.0f, float min = 0.0f, float max = 0.0f, std::string_view format = "%.3f")` | Drag-to-change float. `min==max` means unbounded. |
| `bool DragFloat2(std::string_view label, float v[2], float speed = 1.0f, float min = 0.0f, float max = 0.0f, std::string_view format = "%.3f")` | 2-component float drag. |
| `bool DragFloat3(std::string_view label, float v[3], float speed = 1.0f, float min = 0.0f, float max = 0.0f, std::string_view format = "%.3f")` | 3-component float drag. |
| `bool DragFloat4(std::string_view label, float v[4], float speed = 1.0f, float min = 0.0f, float max = 0.0f, std::string_view format = "%.3f")` | 4-component float drag. |
| `bool DragFloatRange2(std::string_view label, float* currentMin, float* currentMax, float speed = 1.0f, float min = 0.0f, float max = 0.0f, std::string_view format = "%.3f", std::string_view formatMax = "")` | Range drag with separate min/max pointers — both editable. |

### Int drags

| Signature | Note |
|-----------|------|
| `bool DragInt(std::string_view label, int* value, float speed = 1.0f, int min = 0, int max = 0, std::string_view format = "%d")` | Drag-to-change int. |
| `bool DragInt2(std::string_view label, int v[2], float speed = 1.0f, int min = 0, int max = 0, std::string_view format = "%d")` | 2-component int drag. |
| `bool DragInt3(std::string_view label, int v[3], float speed = 1.0f, int min = 0, int max = 0, std::string_view format = "%d")` | 3-component int drag. |
| `bool DragInt4(std::string_view label, int v[4], float speed = 1.0f, int min = 0, int max = 0, std::string_view format = "%d")` | 4-component int drag. |
| `bool DragIntRange2(std::string_view label, int* currentMin, int* currentMax, float speed = 1.0f, int min = 0, int max = 0, std::string_view format = "%d", std::string_view formatMax = "")` | Integer range drag with separate min/max pointers. |

### Int / float / double input fields (with steppers)

| Signature | Note |
|-----------|------|
| `bool InputInt(std::string_view label, int* value, int step = 1, int stepFast = 100)` | Int input with +/- step buttons. |
| `bool InputInt2(std::string_view label, int v[2])` | 2-component int input. |
| `bool InputInt3(std::string_view label, int v[3])` | 3-component int input. |
| `bool InputInt4(std::string_view label, int v[4])` | 4-component int input. |
| `bool InputFloat(std::string_view label, float* value, float step = 0.0f, float stepFast = 0.0f, std::string_view format = "%.3f")` | Float input with optional step buttons. |
| `bool InputFloat2(std::string_view label, float v[2], std::string_view format = "%.3f")` | 2-component float input. |
| `bool InputFloat3(std::string_view label, float v[3], std::string_view format = "%.3f")` | 3-component float input. |
| `bool InputFloat4(std::string_view label, float v[4], std::string_view format = "%.3f")` | 4-component float input. |
| `bool InputDouble(std::string_view label, double* value, double step = 0.0, double stepFast = 0.0, std::string_view format = "%.6f")` | Double input with optional step buttons. |

---

## Text inputs

| Signature | Note |
|-----------|------|
| `bool InputText(std::string_view label, std::string* value, std::size_t maxLength = 256, ImGuiInputTextFlags flags = 0)` | Single-line text bound to a `std::string`. |
| `bool InputTextWithHint(std::string_view label, std::string_view hint, std::string* value, std::size_t maxLength = 256, ImGuiInputTextFlags flags = 0)` | Single-line input with a greyed-out hint shown when empty. |
| `bool InputTextMultiline(std::string_view label, std::string* value, const ImVec2& size = ImVec2(0,0), std::size_t maxLength = 4096, ImGuiInputTextFlags flags = 0)` | Multi-line text bound to a `std::string`. |
| `void DrawActiveInputCaret()` | Overlay a DPI-correct, font-scaled caret on the active input. The `im::InputText*` helpers call this automatically; call it yourself after a raw `ImGui::InputText*` on the same item. |

The `im::InputText*` functions take a `std::string*` directly — you do **not**
manage a fixed `char[]` buffer. `maxLength` caps the resulting string length.

---

## Combo / ListBox / Selectable

| Signature | Note |
|-----------|------|
| `bool Combo(std::string_view label, int* current, const std::vector<std::string>& items)` | High-level dropdown bound to an index into `items`; true when the selection changes. |
| `bool BeginCombo(std::string_view label, std::string_view preview, ImGuiComboFlags flags = 0)` | Low-level combo; emit `Selectable()` items inside, then `EndCombo()` when true. |
| `void EndCombo()` | Close a `BeginCombo`. |
| `bool BeginListBox(std::string_view label, const ImVec2& size = ImVec2(0,0))` | Scrolling list region; emit `Selectable()` items, then `EndListBox()` when true. |
| `void EndListBox()` | Close a `BeginListBox`. |
| `bool Selectable(std::string_view label, bool selected = false, ImGuiSelectableFlags flags = 0, const ImVec2& size = ImVec2(0,0))` | Selectable row; true on click, `selected` controls the highlight. |
| `bool Selectable(std::string_view label, bool* pSelected, ImGuiSelectableFlags flags = 0, const ImVec2& size = ImVec2(0,0))` | Selectable row bound to a `bool` toggled on click. |

---

## Windows & child regions

### Child windows

| Signature | Note |
|-----------|------|
| `bool BeginChild(std::string_view strId, const ImVec2& size = ImVec2(0,0), ImGuiChildFlags childFlags = 0, ImGuiWindowFlags windowFlags = 0)` | Begin a scrollable sub-region by string ID. Always call `EndChild()` regardless of return. |
| `bool BeginChild(ImGuiID id, const ImVec2& size = ImVec2(0,0), ImGuiChildFlags childFlags = 0, ImGuiWindowFlags windowFlags = 0)` | Overload by numeric `ImGuiID` (useful for dynamic lists). |
| `void EndChild()` | Close a `BeginChild`. |

### Next-window hints (call before `Begin` / `BeginChild`)

| Signature | Note |
|-----------|------|
| `void SetNextWindowPos(const ImVec2& pos, ImGuiCond cond = 0, const ImVec2& pivot = ImVec2(0,0))` | Position the next window; `pivot` is the anchor within the window. |
| `void SetNextWindowSize(const ImVec2& size, ImGuiCond cond = 0)` | Set the next window's size. |
| `void SetNextWindowSizeConstraints(const ImVec2& sizeMin, const ImVec2& sizeMax)` | Clamp the next window's size to a range. |
| `void SetNextWindowContentSize(const ImVec2& size)` | Set the next window's content (scroll) size. |
| `void SetNextWindowCollapsed(bool collapsed, ImGuiCond cond = 0)` | Set collapsed state of the next window. |
| `void SetNextWindowFocus()` | Focus the next window. |
| `void SetNextWindowScroll(const ImVec2& scroll)` | Set the next window's scroll offset. |
| `void SetNextWindowBgAlpha(float alpha)` | Override the next window's background alpha. |

### Window-state queries

| Signature | Note |
|-----------|------|
| `bool IsWindowAppearing()` | True on the frame the current window first appears. |
| `bool IsWindowCollapsed()` | True if the current window is collapsed. |
| `bool IsWindowFocused(ImGuiFocusedFlags flags = 0)` | Focus query for the current window. |
| `bool IsWindowHovered(ImGuiHoveredFlags flags = 0)` | Hover query for the current window. |
| `ImVec2 GetWindowPos()` | Current window position in screen space. |
| `ImVec2 GetWindowSize()` | Current window size. |
| `float GetWindowWidth()` | Current window width. |
| `float GetWindowHeight()` | Current window height. |

---

## Layout helpers

| Signature | Note |
|-----------|------|
| `void SameLine(float offsetFromStart = 0.0f, float spacing = -1.0f)` | Place the next item on the same line. |
| `void NewLine()` | Move to the next line. |
| `void Spacing()` | Add vertical spacing. |
| `void Separator()` | Horizontal separator line. |
| `void SeparatorText(std::string_view text)` | Separator with a centered label. |
| `void Dummy(float width, float height)` | Reserve an empty rectangle of the given size. |
| `void Indent(float width = 0.0f)` | Increase the left indent. |
| `void Unindent(float width = 0.0f)` | Decrease the left indent. |
| `void Bullet()` | Render a bullet glyph (advance, no text). |

### Alignment & line metrics

| Signature | Note |
|-----------|------|
| `void AlignTextToFramePadding()` | Vertically align the next text baseline to frame padding. |
| `float GetTextLineHeight()` | Text line height. |
| `float GetTextLineHeightWithSpacing()` | Line height including item spacing. |
| `float GetFrameHeight()` | Height of a framed widget. |
| `float GetFrameHeightWithSpacing()` | Frame height including item spacing. |
| `void SetItemDefaultFocus()` | Make the last item the default-focused element of a newly appearing window. |

---

## Cursor & content region

| Signature | Note |
|-----------|------|
| `ImVec2 GetCursorScreenPos()` | Absolute screen position of the cursor (use with `ImDrawList`). |
| `void SetCursorScreenPos(const ImVec2& pos)` | Set the cursor to an absolute screen position. |
| `ImVec2 GetCursorPos()` | Window-local cursor position. |
| `float GetCursorPosX()` | Window-local cursor X. |
| `float GetCursorPosY()` | Window-local cursor Y. |
| `void SetCursorPos(const ImVec2& localPos)` | Set the window-local cursor position. |
| `void SetCursorPosX(float localX)` | Set the window-local cursor X. |
| `void SetCursorPosY(float localY)` | Set the window-local cursor Y. |
| `ImVec2 GetCursorStartPos()` | Window-local cursor at the top of the content area. |
| `ImVec2 GetContentRegionAvail()` | Remaining space from the cursor to the window's bottom-right. |

---

## Scrolling

| Signature | Note |
|-----------|------|
| `float GetScrollX()` | Current horizontal scroll. |
| `float GetScrollY()` | Current vertical scroll. |
| `float GetScrollMaxX()` | Maximum horizontal scroll. |
| `float GetScrollMaxY()` | Maximum vertical scroll. |
| `void SetScrollX(float scrollX)` | Set horizontal scroll. |
| `void SetScrollY(float scrollY)` | Set vertical scroll. |
| `void SetScrollHereX(float centerXRatio = 0.5f)` | Scroll so the cursor X is visible (0=left, 0.5=center, 1=right). |
| `void SetScrollHereY(float centerYRatio = 0.5f)` | Scroll so the cursor Y is visible (0=top, 0.5=center, 1=bottom). |
| `void SetScrollFromPosX(float localX, float centerXRatio = 0.5f)` | Scroll to make a local X position visible. |
| `void SetScrollFromPosY(float localY, float centerYRatio = 0.5f)` | Scroll to make a local Y position visible. |

---

## Groups & clip rect

### Group

| Signature | Note |
|-----------|------|
| `void BeginGroup()` | Lock the horizontal start so the whole group is treated as one logical item. |
| `void EndGroup()` | Close a `BeginGroup`. |

### Clip rect

| Signature | Note |
|-----------|------|
| `void PushClipRect(const ImVec2& clipMin, const ImVec2& clipMax, bool intersectWithCurrent)` | Push a scissor rect for rendering and hit-testing; `intersectWithCurrent` narrows the existing clip rather than replacing it. |
| `void PopClipRect()` | Pop the last clip rect. |

---

## Item width

| Signature | Note |
|-----------|------|
| `void PushItemWidth(float itemWidth)` | Push a fixed item width. `>0` = pixels; `<0` = align N px from the right; `-FLT_MIN` always aligns to the right. |
| `void PopItemWidth()` | Pop the last pushed item width. |
| `void SetNextItemWidth(float itemWidth)` | Set the width of the *next* single item only. |
| `float CalcItemWidth()` | Computed width of the next item (honoring any pushed width). |

---

## Popups & modals

| Signature | Note |
|-----------|------|
| `void OpenPopup(std::string_view strId, ImGuiPopupFlags popupFlags = 0)` | Mark a popup open. Call once (not every frame). |
| `void OpenPopup(ImGuiID id, ImGuiPopupFlags popupFlags = 0)` | Open a popup by numeric ID. |
| `void OpenPopupOnItemClick(std::string_view strId = "", ImGuiPopupFlags popupFlags = 1)` | Open a popup when the last item is right-clicked. |
| `bool BeginPopup(std::string_view strId, ImGuiWindowFlags flags = 0)` | Begin a generic popup; call `EndPopup()` only when true. |
| `bool BeginPopupModal(std::string_view name, bool* pOpen = nullptr, ImGuiWindowFlags flags = 0)` | Begin a blocking modal; `pOpen` drives the ✕ close button. |
| `void EndPopup()` | End a popup/modal — only when the matching `Begin*` returned true. |
| `void CloseCurrentPopup()` | Close the innermost open popup (call from inside `Begin/EndPopup`). |
| `bool IsPopupOpen(std::string_view strId, ImGuiPopupFlags flags = 0)` | True if the named popup is open. |
| `bool BeginPopupContextItem(std::string_view strId = "", ImGuiPopupFlags popupFlags = 1)` | Open + begin a context popup on right-click of the last item. |
| `bool BeginPopupContextWindow(std::string_view strId = "", ImGuiPopupFlags popupFlags = 1)` | Open + begin a context popup on right-click of the current window. |
| `bool BeginPopupContextVoid(std::string_view strId = "", ImGuiPopupFlags popupFlags = 1)` | Open + begin a context popup on right-click of empty space. |

---

## Menus

| Signature | Note |
|-----------|------|
| `bool BeginMenuBar()` | Append to the current window's menu bar (needs `ImGuiWindowFlags_MenuBar`). |
| `void EndMenuBar()` | Close a `BeginMenuBar`. |
| `bool BeginMainMenuBar()` | Create/append to the full-screen main menu bar. |
| `void EndMainMenuBar()` | Close a `BeginMainMenuBar`. |
| `bool BeginMenu(std::string_view label, bool enabled = true)` | Add a sub-menu; only call `EndMenu()` when true. |
| `void EndMenu()` | Close a `BeginMenu`. |
| `bool MenuItem(std::string_view label, std::string_view shortcut = "", bool selected = false, bool enabled = true)` | Menu item; true on activation. |
| `bool MenuItem(std::string_view label, std::string_view shortcut, bool* pSelected, bool enabled = true)` | Toggleable menu item — activation flips `*pSelected`. |

---

## Tab bars

| Signature | Note |
|-----------|------|
| `bool BeginTabBar(std::string_view strId, ImGuiTabBarFlags flags = 0)` | Begin a tab bar. |
| `void EndTabBar()` | Close a `BeginTabBar`. |
| `bool BeginTabItem(std::string_view label, bool* pOpen = nullptr, ImGuiTabItemFlags flags = 0)` | Begin a tab; `pOpen` drives a close button. Emit content when true. |
| `void EndTabItem()` | Close a `BeginTabItem`. |

---

## Trees & collapsing headers

| Signature | Note |
|-----------|------|
| `bool TreeNode(std::string_view label)` | Tree node; emit children and call `TreePop()` when true. |
| `bool TreeNodeEx(std::string_view label, ImGuiTreeNodeFlags flags = 0)` | Tree node with flags. |
| `void TreePop()` | Close a `TreeNode` / `TreeNodeEx`. |
| `void SetNextItemOpen(bool isOpen, ImGuiCond cond = 0)` | Force the next tree node / header open or closed. |
| `bool CollapsingHeader(std::string_view label, ImGuiTreeNodeFlags flags = 0)` | Full-width collapsing header. |
| `bool CollapsingHeader(std::string_view label, bool* pVisible, ImGuiTreeNodeFlags flags = 0)` | Collapsing header with a close ✕ bound to `pVisible`. |

---

## Tooltips

| Signature | Note |
|-----------|------|
| `bool BeginTooltip()` | Begin a tooltip window; pair with `EndTooltip()` when true. |
| `void EndTooltip()` | Close a `BeginTooltip`. |
| `void SetTooltip(std::string_view text)` | One-shot plain-text tooltip. |
| `bool BeginItemTooltip()` | Begin a tooltip only when the last item is hovered (delay-aware). |
| `void SetItemTooltip(std::string_view text)` | One-shot tooltip shown only when the last item is hovered. |

---

## Disabled block

| Signature | Note |
|-----------|------|
| `void BeginDisabled(bool disabled = true)` | Push a disabled scope — controls grey out and stop responding. Pass `false` for a no-op that keeps the stack balanced. |
| `void EndDisabled()` | Close a `BeginDisabled`. |

---

## Color editors, pickers & conversion

| Signature | Note |
|-----------|------|
| `bool ColorEdit3(std::string_view label, float col[3], ImGuiColorEditFlags flags = 0)` | RGB color editor. |
| `bool ColorEdit4(std::string_view label, float col[4], ImGuiColorEditFlags flags = 0)` | RGBA color editor. |
| `bool ColorPicker3(std::string_view label, float col[3], ImGuiColorEditFlags flags = 0)` | RGB color picker. |
| `bool ColorPicker4(std::string_view label, float col[4], ImGuiColorEditFlags flags = 0, const float* ref = nullptr)` | RGBA color picker with an optional reference swatch. |
| `void ColorConvertRGBtoHSV(float r, float g, float b, float& outH, float& outS, float& outV)` | RGB → HSV conversion. |
| `void ColorConvertHSVtoRGB(float h, float s, float v, float& outR, float& outG, float& outB)` | HSV → RGB conversion. |
| `ImU32 ColorConvertFloat4ToU32(const ImVec4& in)` | Pack a float4 color into a packed `ImU32`. |
| `ImVec4 ColorConvertU32ToFloat4(ImU32 in)` | Unpack a packed `ImU32` into a float4 color. |

---

## Progress bar & plots

| Signature | Note |
|-----------|------|
| `void ProgressBar(float fraction, const ImVec2& size = ImVec2(-FLT_MIN, 0), std::string_view overlay = {})` | Progress bar in [0,1]; empty overlay shows the default percentage. |
| `void PlotLines(std::string_view label, const float* values, int count, int offset = 0, std::string_view overlay = {}, float scaleMin = FLT_MAX, float scaleMax = FLT_MAX, const ImVec2& size = ImVec2(0,0))` | Line plot of a float array. |
| `void PlotHistogram(std::string_view label, const float* values, int count, int offset = 0, std::string_view overlay = {}, float scaleMin = FLT_MAX, float scaleMax = FLT_MAX, const ImVec2& size = ImVec2(0,0))` | Histogram plot of a float array. |

> The immediate layer exposes no `Table*` functions — use the retained-mode
> `unigui::Table` widget (or raw `ImGui::BeginTable`) for tabular layouts.

---

## Item & input queries

| Signature | Note |
|-----------|------|
| `bool IsItemHovered(ImGuiHoveredFlags flags = 0)` | Last item is hovered. |
| `bool IsItemActive()` | Last item is being interacted with. |
| `bool IsItemFocused()` | Last item is focused. |
| `bool IsItemClicked(ImGuiMouseButton mouseButton = 0)` | Last item was clicked with the given button. |
| `bool IsItemVisible()` | Last item is visible (not clipped). |
| `bool IsItemEdited()` | Last item's value was edited this frame. |
| `bool IsItemActivated()` | Last item was activated this frame. |
| `bool IsItemDeactivated()` | Last item was deactivated this frame. |
| `bool IsItemDeactivatedAfterEdit()` | Last item was deactivated after an edit (good for commit-on-blur). |
| `bool IsItemToggledOpen()` | Last tree node / header was toggled open this frame. |
| `bool IsAnyItemHovered()` | Any item is hovered. |
| `bool IsAnyItemActive()` | Any item is active. |
| `bool IsAnyItemFocused()` | Any item is focused. |
| `ImVec2 GetItemRectMin()` | Top-left of the last item (screen space). |
| `ImVec2 GetItemRectMax()` | Bottom-right of the last item (screen space). |
| `ImVec2 GetItemRectSize()` | Size of the last item. |

---

## Keyboard queries

| Signature | Note |
|-----------|------|
| `bool IsKeyDown(ImGuiKey key)` | Key is currently held. |
| `bool IsKeyPressed(ImGuiKey key, bool repeat = true)` | Key was pressed this frame (with optional auto-repeat). |
| `bool IsKeyReleased(ImGuiKey key)` | Key was released this frame. |

---

## Mouse queries

| Signature | Note |
|-----------|------|
| `bool IsMouseDown(ImGuiMouseButton button)` | Button is currently held. |
| `bool IsMouseClicked(ImGuiMouseButton button, bool repeat = false)` | Button was clicked this frame. |
| `bool IsMouseReleased(ImGuiMouseButton button)` | Button was released this frame. |
| `bool IsMouseDoubleClicked(ImGuiMouseButton button)` | Button was double-clicked this frame. |
| `bool IsMouseDragging(ImGuiMouseButton button, float lockThreshold = -1.0f)` | Button is being dragged past the lock threshold. |
| `bool IsMouseHoveringRect(const ImVec2& rMin, const ImVec2& rMax, bool clip = true)` | Mouse is over a screen-space rectangle. |
| `ImVec2 GetMousePos()` | Current mouse position (screen space). |
| `ImVec2 GetMouseDragDelta(ImGuiMouseButton button = 0, float lockThreshold = -1.0f)` | Accumulated drag delta for a button. |
| `void ResetMouseDragDelta(ImGuiMouseButton button = 0)` | Reset the accumulated drag delta. |

---

## Misc utilities

| Signature | Note |
|-----------|------|
| `ImVec2 CalcTextSize(std::string_view text, bool hideAfterDoubleHash = false, float wrapWidth = -1.0f)` | Measure rendered text size (respects an optional wrap width). |
| `void SetKeyboardFocusHere(int offset = 0)` | Focus the next (or `offset`-th following) widget for keyboard input. |
| `double GetTime()` | Seconds since the ImGui context was created. |
| `int GetFrameCount()` | Number of frames submitted so far. |
| `void SetMouseCursor(ImGuiMouseCursor type)` | Set the mouse cursor shape for this frame. |
| `ImGuiMouseCursor GetMouseCursor()` | Current mouse cursor shape. |

---

## Debug / tool windows

| Signature | Note |
|-----------|------|
| `void ShowDemoWindow(bool* pOpen = nullptr)` | Open/render the built-in Dear ImGui demo window. |
| `void ShowMetricsWindow(bool* pOpen = nullptr)` | Open/render the Metrics/Debugger window (draw calls, windows, internal state). |
| `void ShowStyleEditor(ImGuiStyle* ref = nullptr)` | Render the Style Editor block (not a standalone window). |

---

## Draw-list access

| Signature | Note |
|-----------|------|
| `ImDrawList* GetWindowDrawList()` | Draw list of the current window — append custom primitives here. |
| `ImDrawList* GetBackgroundDrawList()` | Background draw list (rendered before all windows). |
| `ImDrawList* GetForegroundDrawList()` | Foreground draw list (rendered on top of everything). |

---

## Usage examples

### A small settings panel

```cpp
#include <unigui/im/im.h>

namespace im = unigui::im;

struct Settings {
    bool        enabled   = true;
    float       gain      = 0.5f;
    int         quality   = 2;
    std::string name      = "untitled";
};

void DrawSettings(Settings& s) {
    im::SeparatorText("Audio");

    im::Checkbox("Enabled", &s.enabled);

    im::BeginDisabled(!s.enabled);          // grey out the rest when disabled
    im::SliderFloat("Gain", &s.gain, 0.0f, 1.0f);
    im::Combo("Quality", &s.quality, {"Low", "Medium", "High"});
    im::InputText("Name", &s.name);
    im::EndDisabled();

    if (im::Button("Apply", im::ButtonVariant::Primary)) {
        // commit settings...
    }
    im::SameLine();
    if (im::Button("Reset", im::ButtonVariant::Danger)) {
        s = Settings{};
    }
}
```

### A right-click context menu and a confirmation modal

```cpp
namespace im = unigui::im;

void DrawRow(const char* label, bool& askDelete) {
    im::Selectable(label);
    if (im::BeginPopupContextItem()) {       // right-click the row
        if (im::MenuItem("Delete")) {
            im::OpenPopup("Confirm delete");
        }
        im::EndPopup();
    }

    if (im::BeginPopupModal("Confirm delete")) {
        im::Text("Really delete this item?");
        if (im::Button("Yes", im::ButtonVariant::Danger)) {
            askDelete = true;
            im::CloseCurrentPopup();
        }
        im::SameLine();
        if (im::Button("Cancel")) im::CloseCurrentPopup();
        im::EndPopup();
    }
}
```

### Custom drawing with the draw list

```cpp
namespace im = unigui::im;

void DrawSpark(const std::vector<float>& data) {
    ImVec2 origin = im::GetCursorScreenPos();
    ImVec2 avail  = im::GetContentRegionAvail();
    ImDrawList* dl = im::GetWindowDrawList();

    const float w = avail.x, h = 40.0f;
    const ImU32 col = im::ColorConvertFloat4ToU32(ImVec4(0.2f, 0.8f, 0.4f, 1.0f));

    for (size_t i = 1; i < data.size(); ++i) {
        ImVec2 a{origin.x + w * (i - 1) / (data.size() - 1), origin.y + h * (1 - data[i - 1])};
        ImVec2 b{origin.x + w * i       / (data.size() - 1), origin.y + h * (1 - data[i])};
        dl->AddLine(a, b, col, 1.5f);
    }
    im::Dummy(w, h);   // reserve the space we drew into
}
```

### As the escape hatch under `dsl::Custom`

```cpp
using namespace unigui;
namespace im = unigui::im;

dsl::NodePtr BuildPanel(int& counter) {
    return dsl::VBox({
        dsl::Label("Mixed DSL + immediate-mode"),
        dsl::Custom([&counter] {
            if (im::Button("Increment")) ++counter;
            im::SameLine();
            im::Text(std::format("Count: {}", counter));
        }),
    });
}
```

---

## See also

- [`docs/WIDGET_API.md`](WIDGET_API.md) — retained-mode widget reference.
- [`docs/API_INDEX.md`](API_INDEX.md) — master index (widgets + `im` + DSL + core).
- [`include/unigui/dsl/dsl.h`](../include/unigui/dsl/dsl.h) — `dsl::Custom` and the declarative builders.
- [`include/unigui/core/scope.h`](../include/unigui/core/scope.h) — RAII `IDScope` / `WindowScope` guards (use instead of `PushID`/`PopID`).
