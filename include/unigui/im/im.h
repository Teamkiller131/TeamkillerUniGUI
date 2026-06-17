#pragma once

// ─────────────────────────────────────────────────────────────────────────────
// UniGUI Immediate-Mode API  (namespace unigui::im)
//
// A thin, allocation-light, immediate-mode layer over Dear ImGui that lets the
// common "just draw a control" case stay a one-liner — without spelling out
// `ImGui::`, managing `shared_ptr`s or inventing unique names:
//
//     if (unigui::im::Button("Save")) save();
//     unigui::im::Checkbox("Enabled", &enabled);
//     unigui::im::SliderFloat("Gain", &gain, 0.f, 1.f);
//
// These functions live in `unigui::im` rather than plain `unigui` on purpose:
// the retained-mode widgets (`unigui::Button`, `unigui::Separator`, ...) are
// *classes* in the `unigui` namespace, so same-named free functions would
// collide with them. The immediate layer complements — it does not replace —
// the retained-mode widgets, which remain the right tool when you need
// persistent state, validation, undo/redo or serialization.
// ─────────────────────────────────────────────────────────────────────────────

#include <imgui.h>

#include <cfloat>
#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace unigui::im {

// ── Button color variants (mirror unigui::Button::ColorVariant) ──────────────
enum class ButtonVariant { Default, Primary, Danger, Success, Warning };

// ── Buttons ──────────────────────────────────────────────────────────────────
/// Themed button. Returns true on the frame it is clicked.
bool Button(std::string_view label, const ImVec2& size = ImVec2(0, 0));
/// Themed button with an explicit color variant (Primary/Danger/Success).
bool Button(std::string_view label, ButtonVariant variant, const ImVec2& size = ImVec2(0, 0));
/// Small (single-line) button — convenience for inline placement.
bool SmallButton(std::string_view label);

// ── Text ──────────────────────────────────────────────────────────────────────
void Text(std::string_view text);
void TextWrapped(std::string_view text);
void TextDisabled(std::string_view text);
void TextColored(const ImVec4& color, std::string_view text);
void BulletText(std::string_view text);
void LabelText(std::string_view label, std::string_view text);

// ── Boolean / selection inputs ────────────────────────────────────────────────
bool Checkbox(std::string_view label, bool* value);
bool RadioButton(std::string_view label, int* current, int buttonValue);
bool RadioButton(std::string_view label, bool active);

// ── Numeric inputs ────────────────────────────────────────────────────────────
bool SliderFloat(std::string_view label, float* value, float min, float max,
                 std::string_view format = "%.3f");
bool SliderFloat2(std::string_view label, float v[2], float min, float max,
                  std::string_view format = "%.3f");
bool SliderFloat3(std::string_view label, float v[3], float min, float max,
                  std::string_view format = "%.3f");
bool SliderFloat4(std::string_view label, float v[4], float min, float max,
                  std::string_view format = "%.3f");
/// Angle slider — `v_rad` is in radians; the widget displays degrees.
bool SliderAngle(std::string_view label, float* v_rad, float degreesMin = -360.0f,
                 float degreesMax = 360.0f, std::string_view format = "%.0f deg");
bool SliderInt(std::string_view label, int* value, int min, int max,
               std::string_view format = "%d");
bool SliderInt2(std::string_view label, int v[2], int min, int max, std::string_view format = "%d");
bool SliderInt3(std::string_view label, int v[3], int min, int max, std::string_view format = "%d");
bool SliderInt4(std::string_view label, int v[4], int min, int max, std::string_view format = "%d");
/// Vertical float slider. `size` is the widget dimensions.
bool VSliderFloat(std::string_view label, const ImVec2& size, float* value, float min, float max,
                  std::string_view format = "%.3f");
/// Vertical int slider.
bool VSliderInt(std::string_view label, const ImVec2& size, int* value, int min, int max,
                std::string_view format = "%d");

bool DragFloat(std::string_view label, float* value, float speed = 1.0f, float min = 0.0f,
               float max = 0.0f, std::string_view format = "%.3f");
bool DragFloat2(std::string_view label, float v[2], float speed = 1.0f, float min = 0.0f,
                float max = 0.0f, std::string_view format = "%.3f");
bool DragFloat3(std::string_view label, float v[3], float speed = 1.0f, float min = 0.0f,
                float max = 0.0f, std::string_view format = "%.3f");
bool DragFloat4(std::string_view label, float v[4], float speed = 1.0f, float min = 0.0f,
                float max = 0.0f, std::string_view format = "%.3f");
/// Drag widget with separate min/max pointers for a range — both values are editable.
bool DragFloatRange2(std::string_view label, float* currentMin, float* currentMax,
                     float speed = 1.0f, float min = 0.0f, float max = 0.0f,
                     std::string_view format = "%.3f", std::string_view formatMax = "");

bool DragInt(std::string_view label, int* value, float speed = 1.0f, int min = 0, int max = 0,
             std::string_view format = "%d");
bool DragInt2(std::string_view label, int v[2], float speed = 1.0f, int min = 0, int max = 0,
              std::string_view format = "%d");
bool DragInt3(std::string_view label, int v[3], float speed = 1.0f, int min = 0, int max = 0,
              std::string_view format = "%d");
bool DragInt4(std::string_view label, int v[4], float speed = 1.0f, int min = 0, int max = 0,
              std::string_view format = "%d");
/// Drag widget with separate min/max pointers for an integer range.
bool DragIntRange2(std::string_view label, int* currentMin, int* currentMax, float speed = 1.0f,
                   int min = 0, int max = 0, std::string_view format = "%d",
                   std::string_view formatMax = "");

bool InputInt(std::string_view label, int* value, int step = 1, int stepFast = 100);
bool InputInt2(std::string_view label, int v[2]);
bool InputInt3(std::string_view label, int v[3]);
bool InputInt4(std::string_view label, int v[4]);
bool InputFloat(std::string_view label, float* value, float step = 0.0f, float stepFast = 0.0f,
                std::string_view format = "%.3f");
bool InputFloat2(std::string_view label, float v[2], std::string_view format = "%.3f");
bool InputFloat3(std::string_view label, float v[3], std::string_view format = "%.3f");
bool InputFloat4(std::string_view label, float v[4], std::string_view format = "%.3f");
bool InputDouble(std::string_view label, double* value, double step = 0.0, double stepFast = 0.0,
                 std::string_view format = "%.6f");

// ── Text inputs ───────────────────────────────────────────────────────────────
/// Single-line text input bound to a std::string. Returns true when edited.
bool InputText(std::string_view label, std::string* value, std::size_t maxLength = 256,
               ImGuiInputTextFlags flags = 0);
/// Single-line text input with a greyed-out `hint` shown when the field is empty.
bool InputTextWithHint(std::string_view label, std::string_view hint, std::string* value,
                       std::size_t maxLength = 256, ImGuiInputTextFlags flags = 0);
/// Multi-line text input bound to a std::string.
bool InputTextMultiline(std::string_view label, std::string* value,
                        const ImVec2& size = ImVec2(0, 0), std::size_t maxLength = 4096,
                        ImGuiInputTextFlags flags = 0);
/// Overlay a thicker, font-scaled text caret on the *currently active* input, matching
/// ImGui's blink. Works around imgui #7031 (the built-in 1px caret truncates to 1px at
/// fractional DPI and is nearly invisible). The `im::InputText*` helpers call this
/// automatically; call it yourself right after a raw `ImGui::InputText*` on the same item.
void DrawActiveInputCaret();

// ── Combo ─────────────────────────────────────────────────────────────────────
/// Dropdown bound to an index into @p items. Returns true when the selection
/// changes.
bool Combo(std::string_view label, int* current, const std::vector<std::string>& items);

// ── Layout helpers (thin wrappers — keep user code free of raw ImGui::) ───────
void SameLine(float offsetFromStart = 0.0f, float spacing = -1.0f);
void NewLine();
void Spacing();
void Separator();
void SeparatorText(std::string_view text);
void Dummy(float width, float height);
void Indent(float width = 0.0f);
void Unindent(float width = 0.0f);
void Bullet();

// ── Item width ────────────────────────────────────────────────────────────────
/// Push a fixed item width onto the stack for "item+label" widgets.
/// >0: width in pixels. <0: align N pixels from the right edge. -FLT_MIN
/// always aligns to the right.
void PushItemWidth(float itemWidth);
void PopItemWidth();
/// Set width for the *next* single item only.
void SetNextItemWidth(float itemWidth);
/// Return the computed width of the next item (takes pushed width into account).
float CalcItemWidth();

// ── Group ─────────────────────────────────────────────────────────────────────
/// Lock the horizontal start position of subsequent items so that SameLine()
/// and layout queries treat the whole group as one logical item.
void BeginGroup();
void EndGroup();

// ── Child windows ─────────────────────────────────────────────────────────────
/// Begin a scrollable sub-region identified by a string ID. Returns false when
/// the child is collapsed/clipped; always call EndChild() regardless.
bool BeginChild(std::string_view strId, const ImVec2& size = ImVec2(0, 0),
                ImGuiChildFlags childFlags = 0, ImGuiWindowFlags windowFlags = 0);
/// Overload identified by a numeric ImGuiID (useful for dynamic lists).
bool BeginChild(ImGuiID id, const ImVec2& size = ImVec2(0, 0), ImGuiChildFlags childFlags = 0,
                ImGuiWindowFlags windowFlags = 0);
void EndChild();

// ── Next-window hints (call before Begin / BeginChild) ────────────────────────
void SetNextWindowPos(const ImVec2& pos, ImGuiCond cond = 0, const ImVec2& pivot = ImVec2(0, 0));
void SetNextWindowSize(const ImVec2& size, ImGuiCond cond = 0);
void SetNextWindowSizeConstraints(const ImVec2& sizeMin, const ImVec2& sizeMax);
void SetNextWindowContentSize(const ImVec2& size);
void SetNextWindowCollapsed(bool collapsed, ImGuiCond cond = 0);
void SetNextWindowFocus();
void SetNextWindowScroll(const ImVec2& scroll);
void SetNextWindowBgAlpha(float alpha);

// ── Scrolling ─────────────────────────────────────────────────────────────────
float GetScrollX();
float GetScrollY();
float GetScrollMaxX();
float GetScrollMaxY();
void SetScrollX(float scrollX);
void SetScrollY(float scrollY);
/// Scroll so the current cursor position is visible. ratio 0=top, 0.5=center, 1=bottom.
void SetScrollHereX(float centerXRatio = 0.5f);
void SetScrollHereY(float centerYRatio = 0.5f);
void SetScrollFromPosX(float localX, float centerXRatio = 0.5f);
void SetScrollFromPosY(float localY, float centerYRatio = 0.5f);

// ── Cursor & content region ───────────────────────────────────────────────────
/// Best-friend: absolute screen position of the current cursor (use with ImDrawList).
ImVec2 GetCursorScreenPos();
/// Set cursor to an absolute screen position.
void SetCursorScreenPos(const ImVec2& pos);
/// Window-local cursor position.
ImVec2 GetCursorPos();
float GetCursorPosX();
float GetCursorPosY();
void SetCursorPos(const ImVec2& localPos);
void SetCursorPosX(float localX);
void SetCursorPosY(float localY);
/// Window-local cursor position at the top of the current window (after title bar etc.).
ImVec2 GetCursorStartPos();
/// Best-friend: remaining space from the cursor to the bottom-right of the current window.
ImVec2 GetContentRegionAvail();
/// Current window position in screen space.
ImVec2 GetWindowPos();
/// Current window size.
ImVec2 GetWindowSize();
float GetWindowWidth();
float GetWindowHeight();

// ── Clip rect ─────────────────────────────────────────────────────────────────
/// Push a scissor rectangle for rendering and hit-testing. Set
/// intersectWithCurrent to narrow an existing clip rather than replace it.
void PushClipRect(const ImVec2& clipMin, const ImVec2& clipMax, bool intersectWithCurrent);
void PopClipRect();

// ── Alignment & line metrics ──────────────────────────────────────────────────
/// Vertically align the next text baseline to FramePadding so it lines up
/// with neighbouring framed widgets.
void AlignTextToFramePadding();
float GetTextLineHeight();
float GetTextLineHeightWithSpacing();
float GetFrameHeight();
float GetFrameHeightWithSpacing();

/// Make the last item the default-focused element of a newly appearing window
/// (e.g. the first text field in a modal).
void SetItemDefaultFocus();

// ── Popups ────────────────────────────────────────────────────────────────────
/// Mark a popup as open. Call before the frame that renders it; don't call
/// every frame. The popup is rendered by the corresponding BeginPopup* call.
void OpenPopup(std::string_view strId, ImGuiPopupFlags popupFlags = 0);
/// Open a popup identified by a numeric ID.
void OpenPopup(ImGuiID id, ImGuiPopupFlags popupFlags = 0);
/// Convenience: open a popup when the last item is right-clicked.
void OpenPopupOnItemClick(std::string_view strId = "", ImGuiPopupFlags popupFlags = 1);

/// Begin rendering a generic popup. Returns true (and you must call EndPopup)
/// only when the popup is open.
bool BeginPopup(std::string_view strId, ImGuiWindowFlags flags = 0);
/// Begin rendering a blocking modal dialog. Pass a bool* for the ✕ close button.
bool BeginPopupModal(std::string_view name, bool* pOpen = nullptr, ImGuiWindowFlags flags = 0);
/// End the popup / modal — call only when the corresponding Begin* returned true.
void EndPopup();

/// Close the innermost open popup (call from inside Begin/EndPopup).
void CloseCurrentPopup();
/// Returns true if the popup identified by strId is currently open.
bool IsPopupOpen(std::string_view strId, ImGuiPopupFlags flags = 0);

/// Open + begin a context popup when the last item is right-clicked.
bool BeginPopupContextItem(std::string_view strId = "", ImGuiPopupFlags popupFlags = 1);
/// Open + begin a context popup when the current window is right-clicked.
bool BeginPopupContextWindow(std::string_view strId = "", ImGuiPopupFlags popupFlags = 1);
/// Open + begin a context popup when clicking in void (no window under cursor).
bool BeginPopupContextVoid(std::string_view strId = "", ImGuiPopupFlags popupFlags = 1);

// ── Menus ─────────────────────────────────────────────────────────────────────
/// Append to the menu bar of the current window (requires
/// ImGuiWindowFlags_MenuBar on the parent window).
bool BeginMenuBar();
void EndMenuBar();
/// Create and append to a full-screen menu bar (useful for the main app menu).
bool BeginMainMenuBar();
void EndMainMenuBar();
/// Add a sub-menu entry. Only call EndMenu() when this returns true.
bool BeginMenu(std::string_view label, bool enabled = true);
void EndMenu();
/// Add a menu item. Returns true on the frame it is activated.
bool MenuItem(std::string_view label, std::string_view shortcut = "", bool selected = false,
              bool enabled = true);
/// Toggleable menu item — activating flips *pSelected.
bool MenuItem(std::string_view label, std::string_view shortcut, bool* pSelected,
              bool enabled = true);

// ── Item queries ──────────────────────────────────────────────────────────────
bool IsItemHovered(ImGuiHoveredFlags flags = 0);
bool IsItemActive();
bool IsItemFocused();
bool IsItemClicked(ImGuiMouseButton mouseButton = 0);
bool IsItemVisible();
bool IsItemEdited();
bool IsItemActivated();
bool IsItemDeactivated();
bool IsItemDeactivatedAfterEdit();
bool IsItemToggledOpen();
bool IsAnyItemHovered();
bool IsAnyItemActive();
bool IsAnyItemFocused();
ImVec2 GetItemRectMin();
ImVec2 GetItemRectMax();
ImVec2 GetItemRectSize();

// ── Keyboard queries ──────────────────────────────────────────────────────────
bool IsKeyDown(ImGuiKey key);
bool IsKeyPressed(ImGuiKey key, bool repeat = true);
bool IsKeyReleased(ImGuiKey key);

// ── Mouse queries ─────────────────────────────────────────────────────────────
bool IsMouseDown(ImGuiMouseButton button);
bool IsMouseClicked(ImGuiMouseButton button, bool repeat = false);
bool IsMouseReleased(ImGuiMouseButton button);
bool IsMouseDoubleClicked(ImGuiMouseButton button);
bool IsMouseDragging(ImGuiMouseButton button, float lockThreshold = -1.0f);
bool IsMouseHoveringRect(const ImVec2& rMin, const ImVec2& rMax, bool clip = true);
ImVec2 GetMousePos();
ImVec2 GetMouseDragDelta(ImGuiMouseButton button = 0, float lockThreshold = -1.0f);
void ResetMouseDragDelta(ImGuiMouseButton button = 0);

// ── Misc widgets ──────────────────────────────────────────────────────────────
/// Clickable invisible region — use with IsItemActive/Hovered to build custom
/// hit-testing logic. Always call SetNextItemAllowOverlap() before if another
/// item needs to overlap.
bool InvisibleButton(std::string_view strId, const ImVec2& size, ImGuiButtonFlags flags = 0);
/// Small square button with a directional arrow glyph.
bool ArrowButton(std::string_view strId, ImGuiDir dir);
/// Checkbox that toggles individual bits in an int flags field.
bool CheckboxFlags(std::string_view label, int* flags, int flagsValue);
/// Checkbox that toggles individual bits in an unsigned int flags field.
bool CheckboxFlags(std::string_view label, unsigned int* flags, unsigned int flagsValue);
/// Clickable color swatch. Returns true when clicked.
bool ColorButton(std::string_view descId, const ImVec4& col, ImGuiColorEditFlags flags = 0,
                 const ImVec2& size = ImVec2(0, 0));

// ── Debug / tool windows ──────────────────────────────────────────────────────
/// Open/render the built-in Dear ImGui demo window. Pass a bool* for a close ✕.
void ShowDemoWindow(bool* pOpen = nullptr);
/// Open/render the Metrics/Debugger window (draw calls, windows, internal state).
void ShowMetricsWindow(bool* pOpen = nullptr);
/// Render the Style Editor block (not a standalone window).
void ShowStyleEditor(ImGuiStyle* ref = nullptr);

// ── Draw-list access ──────────────────────────────────────────────────────────
/// The draw list of the current window — append custom primitives here.
ImDrawList* GetWindowDrawList();
/// The background draw list (rendered before all windows).
ImDrawList* GetBackgroundDrawList();
/// The foreground draw list (rendered on top of everything).
ImDrawList* GetForegroundDrawList();

// ─────────────────────────────────────────────────────────────────────────────
// A6 — remaining practical surface (text/tooltips/disabled/combo/listbox/
// selectable/trees/tabs/plots/color/window-queries/misc). Each is a thin,
// allocation-light pass-through; string params are std::string_view and optional
// const char* params accept an empty view as "use ImGui's default (nullptr)".
// ─────────────────────────────────────────────────────────────────────────────

// ── Text (extras) ─────────────────────────────────────────────────────────────
/// Raw text, no printf parsing — the fastest path for long/literal strings.
void TextUnformatted(std::string_view text);
/// Clickable text hyperlink. Returns true on the frame it is clicked.
bool TextLink(std::string_view label);
/// Clickable text hyperlink that opens `url` (defaults to the label) on click.
void TextLinkOpenURL(std::string_view label, std::string_view url = {});

// ── Tooltips ──────────────────────────────────────────────────────────────────
/// Begin a tooltip window; pair with EndTooltip() when it returns true.
bool BeginTooltip();
void EndTooltip();
/// One-shot tooltip with plain text (begins + sets text + ends).
void SetTooltip(std::string_view text);
/// Begin a tooltip only when the last item is hovered (delay-aware).
bool BeginItemTooltip();
/// One-shot tooltip shown only when the last item is hovered.
void SetItemTooltip(std::string_view text);

// ── Disabled block ────────────────────────────────────────────────────────────
/// Push a disabled scope — controls grey out and stop responding. Pair with
/// EndDisabled(). Pass false to push a no-op (keeps the stack balanced).
void BeginDisabled(bool disabled = true);
void EndDisabled();

// ── Combo (low-level) ─────────────────────────────────────────────────────────
/// Open a custom combo; emit Selectable() items inside, then EndCombo() when true.
bool BeginCombo(std::string_view label, std::string_view preview, ImGuiComboFlags flags = 0);
void EndCombo();

// ── ListBox (low-level) ───────────────────────────────────────────────────────
/// Open a scrolling list region; emit Selectable() items, then EndListBox() when true.
bool BeginListBox(std::string_view label, const ImVec2& size = ImVec2(0, 0));
void EndListBox();

// ── Selectable ────────────────────────────────────────────────────────────────
/// Selectable row. Returns true on click; `selected` controls the highlight.
bool Selectable(std::string_view label, bool selected = false, ImGuiSelectableFlags flags = 0,
                const ImVec2& size = ImVec2(0, 0));
/// Selectable row bound to a bool toggled on click.
bool Selectable(std::string_view label, bool* pSelected, ImGuiSelectableFlags flags = 0,
                const ImVec2& size = ImVec2(0, 0));

// ── Trees & headers ───────────────────────────────────────────────────────────
/// Tree node; when it returns true, emit children and call TreePop().
bool TreeNode(std::string_view label);
bool TreeNodeEx(std::string_view label, ImGuiTreeNodeFlags flags = 0);
void TreePop();
/// Force the next tree node / collapsing header open/closed state.
void SetNextItemOpen(bool isOpen, ImGuiCond cond = 0);
/// Collapsing header (a tree node styled as a full-width header).
bool CollapsingHeader(std::string_view label, ImGuiTreeNodeFlags flags = 0);
/// Collapsing header with a close ✕ button (bound to `pVisible`).
bool CollapsingHeader(std::string_view label, bool* pVisible, ImGuiTreeNodeFlags flags = 0);

// ── Tab bars ──────────────────────────────────────────────────────────────────
bool BeginTabBar(std::string_view strId, ImGuiTabBarFlags flags = 0);
void EndTabBar();
bool BeginTabItem(std::string_view label, bool* pOpen = nullptr, ImGuiTabItemFlags flags = 0);
void EndTabItem();

// ── Plots & progress ──────────────────────────────────────────────────────────
/// Progress bar in [0,1]. Empty overlay shows the default percentage.
void ProgressBar(float fraction, const ImVec2& size = ImVec2(-FLT_MIN, 0),
                 std::string_view overlay = {});
void PlotLines(std::string_view label, const float* values, int count, int offset = 0,
               std::string_view overlay = {}, float scaleMin = FLT_MAX, float scaleMax = FLT_MAX,
               const ImVec2& size = ImVec2(0, 0));
void PlotHistogram(std::string_view label, const float* values, int count, int offset = 0,
                   std::string_view overlay = {}, float scaleMin = FLT_MAX,
                   float scaleMax = FLT_MAX, const ImVec2& size = ImVec2(0, 0));

// ── Color editors & conversion ────────────────────────────────────────────────
bool ColorEdit3(std::string_view label, float col[3], ImGuiColorEditFlags flags = 0);
bool ColorEdit4(std::string_view label, float col[4], ImGuiColorEditFlags flags = 0);
bool ColorPicker3(std::string_view label, float col[3], ImGuiColorEditFlags flags = 0);
bool ColorPicker4(std::string_view label, float col[4], ImGuiColorEditFlags flags = 0,
                  const float* ref = nullptr);
void ColorConvertRGBtoHSV(float r, float g, float b, float& outH, float& outS, float& outV);
void ColorConvertHSVtoRGB(float h, float s, float v, float& outR, float& outG, float& outB);
ImU32 ColorConvertFloat4ToU32(const ImVec4& in);
ImVec4 ColorConvertU32ToFloat4(ImU32 in);

// ── Window-state queries ──────────────────────────────────────────────────────
bool IsWindowAppearing();
bool IsWindowCollapsed();
bool IsWindowFocused(ImGuiFocusedFlags flags = 0);
bool IsWindowHovered(ImGuiHoveredFlags flags = 0);

// ── Misc utilities ────────────────────────────────────────────────────────────
/// Measure rendered text size (respects an optional wrap width).
ImVec2 CalcTextSize(std::string_view text, bool hideAfterDoubleHash = false,
                    float wrapWidth = -1.0f);
/// Focus the next (or `offset`-th following) widget for keyboard input.
void SetKeyboardFocusHere(int offset = 0);
/// Seconds since the ImGui context was created.
double GetTime();
/// Number of frames submitted so far.
int GetFrameCount();
void SetMouseCursor(ImGuiMouseCursor type);
ImGuiMouseCursor GetMouseCursor();

} // namespace unigui::im
