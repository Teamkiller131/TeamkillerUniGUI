#include <unigui/im/im.h>

#include <imgui.h>
#include <imgui_internal.h>

#include "detail/combo_chevron.h"

#include <algorithm>
#include <cstdarg>
#include <cstring>

namespace unigui::im {

namespace {

// Null-terminate a string_view for ImGui's `const char*` APIs. Short strings
// stay in std::string's small-buffer (no heap allocation).
inline std::string Z(std::string_view s) {
    return std::string(s);
}

inline ImVec4 Lighten(const ImVec4& c, float t) {
    return ImVec4(c.x + (1.f - c.x) * t, c.y + (1.f - c.y) * t, c.z + (1.f - c.z) * t, c.w);
}
inline ImVec4 Darken(const ImVec4& c, float t) {
    return ImVec4(c.x * (1.f - t), c.y * (1.f - t), c.z * (1.f - t), c.w);
}

ImVec4 VariantColor(ButtonVariant v, const ImVec4& fallback) {
    switch (v) {
    case ButtonVariant::Primary:
        return ImVec4(0.16f, 0.47f, 0.80f, 1.0f);
    case ButtonVariant::Danger:
        return ImVec4(0.80f, 0.16f, 0.20f, 1.0f);
    case ButtonVariant::Success:
        return ImVec4(0.18f, 0.60f, 0.28f, 1.0f);
    case ButtonVariant::Warning:
        return ImVec4(0.85f, 0.55f, 0.13f, 1.0f);  // amber — e.g. an exhausted/triggered-out action
    default:
        return fallback;
    }
}

} // namespace

// ── Buttons ──────────────────────────────────────────────────────────────────
bool Button(std::string_view label, const ImVec2& size) {
    return ImGui::Button(Z(label).c_str(), size);
}

bool Button(std::string_view label, ButtonVariant variant, const ImVec2& size) {
    if (variant == ButtonVariant::Default)
        return Button(label, size);
    const ImVec4 base = VariantColor(variant, ImGui::GetStyle().Colors[ImGuiCol_Button]);
    ImGui::PushStyleColor(ImGuiCol_Button, base);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, Lighten(base, 0.12f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, Darken(base, 0.12f));
    const bool clicked = ImGui::Button(Z(label).c_str(), size);
    ImGui::PopStyleColor(3);
    return clicked;
}

bool SmallButton(std::string_view label) {
    return ImGui::SmallButton(Z(label).c_str());
}

// ── Text ──────────────────────────────────────────────────────────────────────
void Text(std::string_view text) {
    ImGui::TextUnformatted(text.data(), text.data() + text.size());
}

void TextWrapped(std::string_view text) {
    ImGui::PushTextWrapPos(0.0f);
    ImGui::TextUnformatted(text.data(), text.data() + text.size());
    ImGui::PopTextWrapPos();
}

void TextDisabled(std::string_view text) {
    ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
    ImGui::TextUnformatted(text.data(), text.data() + text.size());
    ImGui::PopStyleColor();
}

void TextColored(const ImVec4& color, std::string_view text) {
    ImGui::PushStyleColor(ImGuiCol_Text, color);
    ImGui::TextUnformatted(text.data(), text.data() + text.size());
    ImGui::PopStyleColor();
}

void BulletText(std::string_view text) {
    ImGui::Bullet();
    ImGui::SameLine();
    ImGui::TextUnformatted(text.data(), text.data() + text.size());
}

void LabelText(std::string_view label, std::string_view text) {
    ImGui::LabelText(Z(label).c_str(), "%.*s", static_cast<int>(text.size()), text.data());
}

// ── Boolean / selection inputs ────────────────────────────────────────────────
bool Checkbox(std::string_view label, bool* value) {
    return ImGui::Checkbox(Z(label).c_str(), value);
}

bool RadioButton(std::string_view label, int* current, int buttonValue) {
    return ImGui::RadioButton(Z(label).c_str(), current, buttonValue);
}

bool RadioButton(std::string_view label, bool active) {
    return ImGui::RadioButton(Z(label).c_str(), active);
}

// ── Numeric inputs ────────────────────────────────────────────────────────────
bool SliderFloat(std::string_view label, float* value, float min, float max,
                 std::string_view format) {
    return ImGui::SliderFloat(Z(label).c_str(), value, min, max, Z(format).c_str());
}
bool SliderFloat2(std::string_view label, float v[2], float min, float max,
                  std::string_view format) {
    return ImGui::SliderFloat2(Z(label).c_str(), v, min, max, Z(format).c_str());
}
bool SliderFloat3(std::string_view label, float v[3], float min, float max,
                  std::string_view format) {
    return ImGui::SliderFloat3(Z(label).c_str(), v, min, max, Z(format).c_str());
}
bool SliderFloat4(std::string_view label, float v[4], float min, float max,
                  std::string_view format) {
    return ImGui::SliderFloat4(Z(label).c_str(), v, min, max, Z(format).c_str());
}
bool SliderAngle(std::string_view label, float* v_rad, float degreesMin, float degreesMax,
                 std::string_view format) {
    return ImGui::SliderAngle(Z(label).c_str(), v_rad, degreesMin, degreesMax, Z(format).c_str());
}
bool SliderInt(std::string_view label, int* value, int min, int max, std::string_view format) {
    return ImGui::SliderInt(Z(label).c_str(), value, min, max, Z(format).c_str());
}
bool SliderInt2(std::string_view label, int v[2], int min, int max, std::string_view format) {
    return ImGui::SliderInt2(Z(label).c_str(), v, min, max, Z(format).c_str());
}
bool SliderInt3(std::string_view label, int v[3], int min, int max, std::string_view format) {
    return ImGui::SliderInt3(Z(label).c_str(), v, min, max, Z(format).c_str());
}
bool SliderInt4(std::string_view label, int v[4], int min, int max, std::string_view format) {
    return ImGui::SliderInt4(Z(label).c_str(), v, min, max, Z(format).c_str());
}
bool VSliderFloat(std::string_view label, const ImVec2& size, float* value, float min, float max,
                  std::string_view format) {
    return ImGui::VSliderFloat(Z(label).c_str(), size, value, min, max, Z(format).c_str());
}
bool VSliderInt(std::string_view label, const ImVec2& size, int* value, int min, int max,
                std::string_view format) {
    return ImGui::VSliderInt(Z(label).c_str(), size, value, min, max, Z(format).c_str());
}

bool DragFloat(std::string_view label, float* value, float speed, float min, float max,
               std::string_view format) {
    return ImGui::DragFloat(Z(label).c_str(), value, speed, min, max, Z(format).c_str());
}
bool DragFloat2(std::string_view label, float v[2], float speed, float min, float max,
                std::string_view format) {
    return ImGui::DragFloat2(Z(label).c_str(), v, speed, min, max, Z(format).c_str());
}
bool DragFloat3(std::string_view label, float v[3], float speed, float min, float max,
                std::string_view format) {
    return ImGui::DragFloat3(Z(label).c_str(), v, speed, min, max, Z(format).c_str());
}
bool DragFloat4(std::string_view label, float v[4], float speed, float min, float max,
                std::string_view format) {
    return ImGui::DragFloat4(Z(label).c_str(), v, speed, min, max, Z(format).c_str());
}
bool DragFloatRange2(std::string_view label, float* currentMin, float* currentMax, float speed,
                     float min, float max, std::string_view format, std::string_view formatMax) {
    const std::string lbl = Z(label), fmt = Z(format), fmtMaxStr = Z(formatMax);
    const char* fmtMax = formatMax.empty() ? nullptr : fmtMaxStr.c_str();
    return ImGui::DragFloatRange2(lbl.c_str(), currentMin, currentMax, speed, min, max, fmt.c_str(),
                                  fmtMax);
}

bool DragInt(std::string_view label, int* value, float speed, int min, int max,
             std::string_view format) {
    return ImGui::DragInt(Z(label).c_str(), value, speed, min, max, Z(format).c_str());
}
bool DragInt2(std::string_view label, int v[2], float speed, int min, int max,
              std::string_view format) {
    return ImGui::DragInt2(Z(label).c_str(), v, speed, min, max, Z(format).c_str());
}
bool DragInt3(std::string_view label, int v[3], float speed, int min, int max,
              std::string_view format) {
    return ImGui::DragInt3(Z(label).c_str(), v, speed, min, max, Z(format).c_str());
}
bool DragInt4(std::string_view label, int v[4], float speed, int min, int max,
              std::string_view format) {
    return ImGui::DragInt4(Z(label).c_str(), v, speed, min, max, Z(format).c_str());
}
bool DragIntRange2(std::string_view label, int* currentMin, int* currentMax, float speed, int min,
                   int max, std::string_view format, std::string_view formatMax) {
    const std::string lbl = Z(label), fmt = Z(format), fmtMaxStr = Z(formatMax);
    const char* fmtMax = formatMax.empty() ? nullptr : fmtMaxStr.c_str();
    return ImGui::DragIntRange2(lbl.c_str(), currentMin, currentMax, speed, min, max, fmt.c_str(),
                                fmtMax);
}

bool InputInt(std::string_view label, int* value, int step, int stepFast) {
    return ImGui::InputInt(Z(label).c_str(), value, step, stepFast);
}
bool InputInt2(std::string_view label, int v[2]) {
    return ImGui::InputInt2(Z(label).c_str(), v);
}
bool InputInt3(std::string_view label, int v[3]) {
    return ImGui::InputInt3(Z(label).c_str(), v);
}
bool InputInt4(std::string_view label, int v[4]) {
    return ImGui::InputInt4(Z(label).c_str(), v);
}
bool InputFloat(std::string_view label, float* value, float step, float stepFast,
                std::string_view format) {
    return ImGui::InputFloat(Z(label).c_str(), value, step, stepFast, Z(format).c_str());
}
bool InputFloat2(std::string_view label, float v[2], std::string_view format) {
    return ImGui::InputFloat2(Z(label).c_str(), v, Z(format).c_str());
}
bool InputFloat3(std::string_view label, float v[3], std::string_view format) {
    return ImGui::InputFloat3(Z(label).c_str(), v, Z(format).c_str());
}
bool InputFloat4(std::string_view label, float v[4], std::string_view format) {
    return ImGui::InputFloat4(Z(label).c_str(), v, Z(format).c_str());
}
bool InputDouble(std::string_view label, double* value, double step, double stepFast,
                 std::string_view format) {
    return ImGui::InputDouble(Z(label).c_str(), value, step, stepFast, Z(format).c_str());
}

// ── Text inputs ───────────────────────────────────────────────────────────────
// ImGui 1.92 draws the input caret as a 1px line scaled by `(int)style._MainScale`, so at
// fractional DPI (e.g. 1.5x -> (int)1.5 == 1) it stays 1px and is nearly invisible
// (imgui #7031 "FIXME-DPI: Cursor thickness"). Overlay a thicker, font-scaled caret at the
// position ImGui already computed for the IME (`PlatformImeData`), matching ImGui's blink
// so it still feels native. Call immediately after an ImGui::InputText* on the same item.
void DrawActiveInputCaret() {
    if (!ImGui::IsItemActive())
        return;
    ImGuiContext& g = *ImGui::GetCurrentContext();
    const ImGuiPlatformImeData& ime = g.PlatformImeData;
    if (!ime.WantVisible)  // ImGui only sets this for the active, editable input
        return;
    const float anim = g.InputTextState.CursorAnim;
    const bool blinkOn = !g.IO.ConfigInputTextCursorBlink || anim <= 0.0f
                         || ImFmod(anim, 1.20f) <= 0.80f;
    if (!blinkOn)
        return;
    const float h = ime.InputLineHeight > 0.0f ? ime.InputLineHeight : g.FontSize;
    const ImVec2 top(ime.InputPos.x + 1.0f, ime.InputPos.y);
    const ImVec2 bottom(top.x, top.y + h);
    const float thickness = ImMax(2.0f, g.FontSize * 0.09f);  // ~2-3px, scales with font/DPI
    ImGui::GetWindowDrawList()->AddLine(top, bottom, ImGui::GetColorU32(ImGuiCol_Text), thickness);
}

namespace {
// Edit a std::string through a temporary, NUL-terminated character buffer.
// Avoids a hard dependency on imgui_stdlib while keeping the std::string API.
bool EditString(const char* id, std::string* value, std::size_t maxLength, bool multiline,
                const ImVec2& size, ImGuiInputTextFlags flags) {
    const std::size_t cap = std::max(maxLength, value->size() + 1);
    std::vector<char> buf(cap + 1, '\0');
    const std::size_t copy = std::min(value->size(), cap);
    std::memcpy(buf.data(), value->data(), copy);
    buf[copy] = '\0';

    const bool changed = multiline
                             ? ImGui::InputTextMultiline(id, buf.data(), buf.size(), size, flags)
                             : ImGui::InputText(id, buf.data(), buf.size(), flags);
    DrawActiveInputCaret();
    // Write back whenever the buffer differs -- NOT when the widget returns true.
    //
    // With ImGuiInputTextFlags_EnterReturnsTrue the return value fires only on the
    // Enter frame, so keying it to `changed` silently discards every keystroke: the
    // next frame refills `buf` from the untouched string and the typing vanishes the
    // moment the field loses focus. That is a password box that cannot be typed into,
    // and it is impossible to spot from the call site -- the flag is documented as
    // changing the *return* value, not as disabling persistence.
    //
    // The scaffold's LoginView already hand-rolled its own wrapper with exactly this
    // rule to work around it; fixing it here removes the need for that copy.
    if (*value != buf.data())
        value->assign(buf.data());
    return changed;
}
} // namespace

bool InputText(std::string_view label, std::string* value, std::size_t maxLength,
               ImGuiInputTextFlags flags) {
    if (!value)
        return false;
    return EditString(Z(label).c_str(), value, maxLength, /*multiline=*/false, ImVec2(0, 0), flags);
}

bool InputText(std::string_view label, char* buf, std::size_t bufSize,
               ImGuiInputTextFlags flags) {
    const bool changed = ImGui::InputText(Z(label).c_str(), buf, bufSize, flags);
    DrawActiveInputCaret();
    return changed;
}

bool InputTextWithHint(std::string_view label, std::string_view hint, char* buf,
                       std::size_t bufSize, ImGuiInputTextFlags flags) {
    const bool changed =
        ImGui::InputTextWithHint(Z(label).c_str(), Z(hint).c_str(), buf, bufSize, flags);
    DrawActiveInputCaret();
    return changed;
}

bool InputTextMultiline(std::string_view label, char* buf, std::size_t bufSize,
                        const ImVec2& size, ImGuiInputTextFlags flags) {
    const bool changed = ImGui::InputTextMultiline(Z(label).c_str(), buf, bufSize, size, flags);
    DrawActiveInputCaret();
    return changed;
}

bool InputTextWithHint(std::string_view label, std::string_view hint, std::string* value,
                       std::size_t maxLength, ImGuiInputTextFlags flags) {
    if (!value)
        return false;
    const std::size_t cap = std::max(maxLength, value->size() + 1);
    std::vector<char> buf(cap + 1, '\0');
    const std::size_t copy = std::min(value->size(), cap);
    std::memcpy(buf.data(), value->data(), copy);
    buf[copy] = '\0';
    const bool changed =
        ImGui::InputTextWithHint(Z(label).c_str(), Z(hint).c_str(), buf.data(), buf.size(), flags);
    DrawActiveInputCaret();
    if (*value != buf.data()) // same rule as EditString -- see the note there
        value->assign(buf.data());
    return changed;
}

bool InputTextMultiline(std::string_view label, std::string* value, const ImVec2& size,
                        std::size_t maxLength, ImGuiInputTextFlags flags) {
    if (!value)
        return false;
    return EditString(Z(label).c_str(), value, maxLength, /*multiline=*/true, size, flags);
}

// ── Combo ─────────────────────────────────────────────────────────────────────
bool Combo(std::string_view label, int* current, const std::vector<std::string>& items) {
    if (!current)
        return false;
    // Do NOT use ImGui::Combo(const char* items_separated_by_zeros): it counts items
    // with `while (*p) p += strlen(p)+1;`, so a LEADING EMPTY item ("") makes the
    // first byte '\0' and it reports ZERO items -> an empty popup. Many call sites
    // prepend "" as a blank/clear option, so drive the popup manually with
    // BeginCombo/EndCombo. This also stays stable across ImGui versions (the
    // getter-callback overload's signature has churned).
    const int n = static_cast<int>(items.size());
    if (n > 0 && (*current < 0 || *current >= n))
        *current = 0;
    const char* preview = (*current >= 0 && *current < n) ? items[*current].c_str() : "";
    bool changed = false;
    // The unified UniGUI combo look: no stock arrow button, slim ˅ chevron in the right
    // padding (see detail/combo_chevron.h for the full rationale).
    const auto frame = detail::CaptureComboFrame();
    const bool open =
        ImGui::BeginCombo(Z(label).c_str(), preview, ImGuiComboFlags_NoArrowButton);
    const bool hovered = ImGui::IsItemHovered();
    detail::DrawComboChevron(frame, hovered || open);
    // Mouse-wheel quick-select (trader request 2026-07-07): while hovering the CLOSED
    // combo, scrolling cycles the selection in place without opening the popup — wheel
    // up = previous item, wheel down = next, clamped to range. SetItemKeyOwner claims
    // the vertical wheel for this item so a surrounding scroll region (e.g. the pod
    // table) does not also scroll. When the popup is open the wheel scrolls its list.
    if (!open && hovered && n > 1) {
        ImGui::SetItemKeyOwner(ImGuiKey_MouseWheelY);
        const float wheel = ImGui::GetIO().MouseWheel;
        if (wheel != 0.0f) {
            int next = *current + (wheel > 0.0f ? -1 : 1);
            next = next < 0 ? 0 : (next >= n ? n - 1 : next);
            if (next != *current) {
                *current = next;
                changed = true;
            }
        }
    }
    if (open) {
        for (int i = 0; i < n; ++i) {
            ImGui::PushID(i);
            const bool selected = (i == *current);
            // Empty labels still need a clickable row; a single space gives it height.
            const char* item = items[i].empty() ? " " : items[i].c_str();
            if (ImGui::Selectable(item, selected)) {
                *current = i;
                changed = true;
            }
            if (selected)
                ImGui::SetItemDefaultFocus();
            ImGui::PopID();
        }
        ImGui::EndCombo();
    }
    return changed;
}

// ── Layout helpers ────────────────────────────────────────────────────────────
void SameLine(float offsetFromStart, float spacing) {
    ImGui::SameLine(offsetFromStart, spacing);
}
void NewLine() {
    ImGui::NewLine();
}
void Spacing() {
    ImGui::Spacing();
}
void Separator() {
    ImGui::Separator();
}
void SeparatorText(std::string_view text) {
    ImGui::SeparatorText(Z(text).c_str());
}
void Dummy(float width, float height) {
    ImGui::Dummy(ImVec2(width, height));
}
void Indent(float width) {
    ImGui::Indent(width);
}
void Unindent(float width) {
    ImGui::Unindent(width);
}
void Bullet() {
    ImGui::Bullet();
}

// ── Item width ────────────────────────────────────────────────────────────────
void PushItemWidth(float itemWidth) {
    ImGui::PushItemWidth(itemWidth);
}
void PopItemWidth() {
    ImGui::PopItemWidth();
}
void SetNextItemWidth(float itemWidth) {
    ImGui::SetNextItemWidth(itemWidth);
}
float CalcItemWidth() {
    return ImGui::CalcItemWidth();
}

// ── Group ─────────────────────────────────────────────────────────────────────
void BeginGroup() {
    ImGui::BeginGroup();
}
void EndGroup() {
    ImGui::EndGroup();
}

// ── Child windows ─────────────────────────────────────────────────────────────
bool BeginChild(std::string_view strId, const ImVec2& size, ImGuiChildFlags childFlags,
                ImGuiWindowFlags windowFlags) {
    return ImGui::BeginChild(Z(strId).c_str(), size, childFlags, windowFlags);
}
bool BeginChild(ImGuiID id, const ImVec2& size, ImGuiChildFlags childFlags,
                ImGuiWindowFlags windowFlags) {
    return ImGui::BeginChild(id, size, childFlags, windowFlags);
}
void EndChild() {
    ImGui::EndChild();
}

// ── Next-window hints ─────────────────────────────────────────────────────────
void SetNextWindowPos(const ImVec2& pos, ImGuiCond cond, const ImVec2& pivot) {
    ImGui::SetNextWindowPos(pos, cond, pivot);
}
void SetNextWindowSize(const ImVec2& size, ImGuiCond cond) {
    ImGui::SetNextWindowSize(size, cond);
}
void SetNextWindowSizeConstraints(const ImVec2& sizeMin, const ImVec2& sizeMax) {
    ImGui::SetNextWindowSizeConstraints(sizeMin, sizeMax);
}
void SetNextWindowContentSize(const ImVec2& size) {
    ImGui::SetNextWindowContentSize(size);
}
void SetNextWindowCollapsed(bool collapsed, ImGuiCond cond) {
    ImGui::SetNextWindowCollapsed(collapsed, cond);
}
void SetNextWindowFocus() {
    ImGui::SetNextWindowFocus();
}
void SetNextWindowScroll(const ImVec2& scroll) {
    ImGui::SetNextWindowScroll(scroll);
}
void SetNextWindowBgAlpha(float alpha) {
    ImGui::SetNextWindowBgAlpha(alpha);
}

// ── Scrolling ─────────────────────────────────────────────────────────────────
float GetScrollX() {
    return ImGui::GetScrollX();
}
float GetScrollY() {
    return ImGui::GetScrollY();
}
float GetScrollMaxX() {
    return ImGui::GetScrollMaxX();
}
float GetScrollMaxY() {
    return ImGui::GetScrollMaxY();
}
void SetScrollX(float scrollX) {
    ImGui::SetScrollX(scrollX);
}
void SetScrollY(float scrollY) {
    ImGui::SetScrollY(scrollY);
}
void SetScrollHereX(float centerXRatio) {
    ImGui::SetScrollHereX(centerXRatio);
}
void SetScrollHereY(float centerYRatio) {
    ImGui::SetScrollHereY(centerYRatio);
}
void SetScrollFromPosX(float localX, float centerXRatio) {
    ImGui::SetScrollFromPosX(localX, centerXRatio);
}
void SetScrollFromPosY(float localY, float centerYRatio) {
    ImGui::SetScrollFromPosY(localY, centerYRatio);
}

// ── Cursor & content region ───────────────────────────────────────────────────
ImVec2 GetCursorScreenPos() {
    return ImGui::GetCursorScreenPos();
}
void SetCursorScreenPos(const ImVec2& pos) {
    ImGui::SetCursorScreenPos(pos);
}
ImVec2 GetCursorPos() {
    return ImGui::GetCursorPos();
}
float GetCursorPosX() {
    return ImGui::GetCursorPosX();
}
float GetCursorPosY() {
    return ImGui::GetCursorPosY();
}
void SetCursorPos(const ImVec2& localPos) {
    ImGui::SetCursorPos(localPos);
}
void SetCursorPosX(float localX) {
    ImGui::SetCursorPosX(localX);
}
void SetCursorPosY(float localY) {
    ImGui::SetCursorPosY(localY);
}
ImVec2 GetCursorStartPos() {
    return ImGui::GetCursorStartPos();
}
ImVec2 GetContentRegionAvail() {
    return ImGui::GetContentRegionAvail();
}
ImVec2 GetWindowPos() {
    return ImGui::GetWindowPos();
}
ImVec2 GetWindowSize() {
    return ImGui::GetWindowSize();
}
float GetWindowWidth() {
    return ImGui::GetWindowWidth();
}
float GetWindowHeight() {
    return ImGui::GetWindowHeight();
}

// ── Clip rect ─────────────────────────────────────────────────────────────────
// ── Formatted text (printf-style) ─────────────────────────────────────────────
// Each forwards the va_list to ImGui's *V variant — no intermediate buffer, so
// long strings are not truncated by a fixed-size scratch array.
void TextF(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    ImGui::TextV(fmt, args);
    va_end(args);
}
void TextColoredF(const ImVec4& color, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    ImGui::TextColoredV(color, fmt, args);
    va_end(args);
}
void TextDisabledF(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    ImGui::TextDisabledV(fmt, args);
    va_end(args);
}
void TextWrappedF(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    ImGui::TextWrappedV(fmt, args);
    va_end(args);
}
void LabelTextF(const char* label, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    ImGui::LabelTextV(label, fmt, args);
    va_end(args);
}
void BulletTextF(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    ImGui::BulletTextV(fmt, args);
    va_end(args);
}
void SetTooltipF(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    ImGui::SetTooltipV(fmt, args);
    va_end(args);
}

// ── Tables ────────────────────────────────────────────────────────────────────
bool BeginTable(std::string_view strId, int columns, ImGuiTableFlags flags,
                const ImVec2& outerSize, float innerWidth) {
    return ImGui::BeginTable(Z(strId).c_str(), columns, flags, outerSize, innerWidth);
}
void EndTable() {
    ImGui::EndTable();
}
void TableNextRow(ImGuiTableRowFlags rowFlags, float minRowHeight) {
    ImGui::TableNextRow(rowFlags, minRowHeight);
}
bool TableNextColumn() {
    return ImGui::TableNextColumn();
}
bool TableSetColumnIndex(int columnN) {
    return ImGui::TableSetColumnIndex(columnN);
}
void TableSetupColumn(std::string_view label, ImGuiTableColumnFlags flags,
                      float initWidthOrWeight, ImGuiID userId) {
    ImGui::TableSetupColumn(Z(label).c_str(), flags, initWidthOrWeight, userId);
}
void TableSetupScrollFreeze(int cols, int rows) {
    ImGui::TableSetupScrollFreeze(cols, rows);
}
void TableHeadersRow() {
    ImGui::TableHeadersRow();
}
void TableHeader(std::string_view label) {
    ImGui::TableHeader(Z(label).c_str());
}
const char* TableGetColumnName(int columnN) {
    return ImGui::TableGetColumnName(columnN);
}
ImGuiTableSortSpecs* TableGetSortSpecs() {
    return ImGui::TableGetSortSpecs();
}
int TableGetColumnCount() {
    return ImGui::TableGetColumnCount();
}
int TableGetColumnIndex() {
    return ImGui::TableGetColumnIndex();
}
int TableGetRowIndex() {
    return ImGui::TableGetRowIndex();
}

// ── Legacy columns ────────────────────────────────────────────────────────────
void Columns(int count, std::string_view id, bool borders) {
    // ImGui treats a null id as "unnamed"; an empty view must map to that rather
    // than to the empty string, which would be a *different* (hashed) id.
    ImGui::Columns(count, id.empty() ? nullptr : Z(id).c_str(), borders);
}
void NextColumn() {
    ImGui::NextColumn();
}
void SetColumnWidth(int columnIndex, float width) {
    ImGui::SetColumnWidth(columnIndex, width);
}

// ── Windows ───────────────────────────────────────────────────────────────────
bool Begin(std::string_view name, bool* pOpen, ImGuiWindowFlags flags) {
    return ImGui::Begin(Z(name).c_str(), pOpen, flags);
}
void End() {
    ImGui::End();
}
void SetWindowFontScale(float scale) {
    ImGui::SetWindowFontScale(scale);
}

// ── Metrics & context accessors ───────────────────────────────────────────────
ImFont* GetFont() {
    return ImGui::GetFont();
}
float GetFontSize() {
    return ImGui::GetFontSize();
}
ImFontBaked* GetFontBaked() {
    return ImGui::GetFontBaked();
}
ImGuiStyle& GetStyle() {
    return ImGui::GetStyle();
}
ImGuiIO& GetIO() {
    return ImGui::GetIO();
}
ImGuiContext* GetCurrentContext() {
    return ImGui::GetCurrentContext();
}

// ── Style stack ───────────────────────────────────────────────────────────────
void PushStyleColor(ImGuiCol idx, ImU32 color) {
    ImGui::PushStyleColor(idx, color);
}
void PushStyleColor(ImGuiCol idx, const ImVec4& color) {
    ImGui::PushStyleColor(idx, color);
}
void PopStyleColor(int count) {
    ImGui::PopStyleColor(count);
}
void PushStyleVar(ImGuiStyleVar idx, float value) {
    ImGui::PushStyleVar(idx, value);
}
void PushStyleVar(ImGuiStyleVar idx, const ImVec2& value) {
    ImGui::PushStyleVar(idx, value);
}
void PopStyleVar(int count) {
    ImGui::PopStyleVar(count);
}
ImU32 GetColorU32(ImGuiCol idx, float alphaMul) {
    return ImGui::GetColorU32(idx, alphaMul);
}
ImU32 GetColorU32(const ImVec4& color) {
    return ImGui::GetColorU32(color);
}
const ImVec4& GetStyleColorVec4(ImGuiCol idx) {
    return ImGui::GetStyleColorVec4(idx);
}

// ── Text wrapping ─────────────────────────────────────────────────────────────
void PushTextWrapPos(float wrapLocalPosX) {
    ImGui::PushTextWrapPos(wrapLocalPosX);
}
void PopTextWrapPos() {
    ImGui::PopTextWrapPos();
}

// ── ID stack ──────────────────────────────────────────────────────────────────
void PushID(std::string_view strId) {
    // Push the explicit begin/end overload: ImGui hashes the exact range, so a
    // string_view that is not null-terminated still yields the right ID.
    ImGui::PushID(strId.data(), strId.data() + strId.size());
}
void PushID(int intId) {
    ImGui::PushID(intId);
}
void PushID(const void* ptrId) {
    ImGui::PushID(ptrId);
}
void PopID() {
    ImGui::PopID();
}
ImGuiID GetID(std::string_view strId) {
    return ImGui::GetID(strId.data(), strId.data() + strId.size());
}

// ── Clipboard ─────────────────────────────────────────────────────────────────
void SetClipboardText(std::string_view text) {
    ImGui::SetClipboardText(Z(text).c_str());
}
std::string GetClipboardText() {
    const char* t = ImGui::GetClipboardText();
    return t ? std::string(t) : std::string();
}

// ── Viewport ──────────────────────────────────────────────────────────────────
ImGuiViewport* GetMainViewport() {
    return ImGui::GetMainViewport();
}

void PushClipRect(const ImVec2& clipMin, const ImVec2& clipMax, bool intersectWithCurrent) {
    ImGui::PushClipRect(clipMin, clipMax, intersectWithCurrent);
}
void PopClipRect() {
    ImGui::PopClipRect();
}

// ── Alignment & line metrics ──────────────────────────────────────────────────
void AlignTextToFramePadding() {
    ImGui::AlignTextToFramePadding();
}
float GetTextLineHeight() {
    return ImGui::GetTextLineHeight();
}
float GetTextLineHeightWithSpacing() {
    return ImGui::GetTextLineHeightWithSpacing();
}
float GetFrameHeight() {
    return ImGui::GetFrameHeight();
}
float GetFrameHeightWithSpacing() {
    return ImGui::GetFrameHeightWithSpacing();
}
void SetItemDefaultFocus() {
    ImGui::SetItemDefaultFocus();
}

// ── Popups ────────────────────────────────────────────────────────────────────
void OpenPopup(std::string_view strId, ImGuiPopupFlags popupFlags) {
    ImGui::OpenPopup(Z(strId).c_str(), popupFlags);
}
void OpenPopup(ImGuiID id, ImGuiPopupFlags popupFlags) {
    ImGui::OpenPopup(id, popupFlags);
}
void OpenPopupOnItemClick(std::string_view strId, ImGuiPopupFlags popupFlags) {
    ImGui::OpenPopupOnItemClick(strId.empty() ? nullptr : Z(strId).c_str(), popupFlags);
}
bool BeginPopup(std::string_view strId, ImGuiWindowFlags flags) {
    return ImGui::BeginPopup(Z(strId).c_str(), flags);
}
bool BeginPopupModal(std::string_view name, bool* pOpen, ImGuiWindowFlags flags) {
    return ImGui::BeginPopupModal(Z(name).c_str(), pOpen, flags);
}
void EndPopup() {
    ImGui::EndPopup();
}
void CloseCurrentPopup() {
    ImGui::CloseCurrentPopup();
}
bool IsPopupOpen(std::string_view strId, ImGuiPopupFlags flags) {
    return ImGui::IsPopupOpen(Z(strId).c_str(), flags);
}
bool BeginPopupContextItem(std::string_view strId, ImGuiPopupFlags popupFlags) {
    return ImGui::BeginPopupContextItem(strId.empty() ? nullptr : Z(strId).c_str(), popupFlags);
}
bool BeginPopupContextWindow(std::string_view strId, ImGuiPopupFlags popupFlags) {
    return ImGui::BeginPopupContextWindow(strId.empty() ? nullptr : Z(strId).c_str(), popupFlags);
}
bool BeginPopupContextVoid(std::string_view strId, ImGuiPopupFlags popupFlags) {
    return ImGui::BeginPopupContextVoid(strId.empty() ? nullptr : Z(strId).c_str(), popupFlags);
}

// ── Menus ─────────────────────────────────────────────────────────────────────
bool BeginMenuBar() {
    return ImGui::BeginMenuBar();
}
void EndMenuBar() {
    ImGui::EndMenuBar();
}
bool BeginMainMenuBar() {
    return ImGui::BeginMainMenuBar();
}
void EndMainMenuBar() {
    ImGui::EndMainMenuBar();
}
bool BeginMenu(std::string_view label, bool enabled) {
    return ImGui::BeginMenu(Z(label).c_str(), enabled);
}
void EndMenu() {
    ImGui::EndMenu();
}
bool MenuItem(std::string_view label, std::string_view shortcut, bool selected, bool enabled) {
    return ImGui::MenuItem(Z(label).c_str(), shortcut.empty() ? nullptr : Z(shortcut).c_str(),
                           selected, enabled);
}
bool MenuItem(std::string_view label, std::string_view shortcut, bool* pSelected, bool enabled) {
    return ImGui::MenuItem(Z(label).c_str(), shortcut.empty() ? nullptr : Z(shortcut).c_str(),
                           pSelected, enabled);
}

// ── Item queries ──────────────────────────────────────────────────────────────
bool IsItemHovered(ImGuiHoveredFlags flags) {
    return ImGui::IsItemHovered(flags);
}
bool IsItemActive() {
    return ImGui::IsItemActive();
}
bool IsItemFocused() {
    return ImGui::IsItemFocused();
}
bool IsItemClicked(ImGuiMouseButton mouseButton) {
    return ImGui::IsItemClicked(mouseButton);
}
bool IsItemVisible() {
    return ImGui::IsItemVisible();
}
ImGuiItemFlags GetItemFlags() {
    return ImGui::GetItemFlags();
}
bool IsItemEdited() {
    return ImGui::IsItemEdited();
}
bool IsItemActivated() {
    return ImGui::IsItemActivated();
}
bool IsItemDeactivated() {
    return ImGui::IsItemDeactivated();
}
bool IsItemDeactivatedAfterEdit() {
    return ImGui::IsItemDeactivatedAfterEdit();
}
bool IsItemToggledOpen() {
    return ImGui::IsItemToggledOpen();
}
bool IsAnyItemHovered() {
    return ImGui::IsAnyItemHovered();
}
bool IsAnyItemActive() {
    return ImGui::IsAnyItemActive();
}
bool IsAnyItemFocused() {
    return ImGui::IsAnyItemFocused();
}
ImVec2 GetItemRectMin() {
    return ImGui::GetItemRectMin();
}
ImVec2 GetItemRectMax() {
    return ImGui::GetItemRectMax();
}
ImVec2 GetItemRectSize() {
    return ImGui::GetItemRectSize();
}

// ── Keyboard queries ──────────────────────────────────────────────────────────
bool IsKeyDown(ImGuiKey key) {
    return ImGui::IsKeyDown(key);
}
bool IsKeyPressed(ImGuiKey key, bool repeat) {
    return ImGui::IsKeyPressed(key, repeat);
}
bool IsKeyReleased(ImGuiKey key) {
    return ImGui::IsKeyReleased(key);
}

// ── Mouse queries ─────────────────────────────────────────────────────────────
bool IsMouseDown(ImGuiMouseButton button) {
    return ImGui::IsMouseDown(button);
}
bool IsMouseClicked(ImGuiMouseButton button, bool repeat) {
    return ImGui::IsMouseClicked(button, repeat);
}
bool IsMouseReleased(ImGuiMouseButton button) {
    return ImGui::IsMouseReleased(button);
}
bool IsMouseDoubleClicked(ImGuiMouseButton button) {
    return ImGui::IsMouseDoubleClicked(button);
}
bool IsMouseDragging(ImGuiMouseButton button, float lockThreshold) {
    return ImGui::IsMouseDragging(button, lockThreshold);
}
bool IsMouseHoveringRect(const ImVec2& rMin, const ImVec2& rMax, bool clip) {
    return ImGui::IsMouseHoveringRect(rMin, rMax, clip);
}
ImVec2 GetMousePos() {
    return ImGui::GetMousePos();
}
ImVec2 GetMouseDragDelta(ImGuiMouseButton button, float lockThreshold) {
    return ImGui::GetMouseDragDelta(button, lockThreshold);
}
void ResetMouseDragDelta(ImGuiMouseButton button) {
    ImGui::ResetMouseDragDelta(button);
}

// ── Misc widgets ──────────────────────────────────────────────────────────────
bool InvisibleButton(std::string_view strId, const ImVec2& size, ImGuiButtonFlags flags) {
    return ImGui::InvisibleButton(Z(strId).c_str(), size, flags);
}
bool ArrowButton(std::string_view strId, ImGuiDir dir) {
    return ImGui::ArrowButton(Z(strId).c_str(), dir);
}
bool CheckboxFlags(std::string_view label, int* flags, int flagsValue) {
    return ImGui::CheckboxFlags(Z(label).c_str(), flags, flagsValue);
}
bool CheckboxFlags(std::string_view label, unsigned int* flags, unsigned int flagsValue) {
    return ImGui::CheckboxFlags(Z(label).c_str(), flags, flagsValue);
}
bool ColorButton(std::string_view descId, const ImVec4& col, ImGuiColorEditFlags flags,
                 const ImVec2& size) {
    return ImGui::ColorButton(Z(descId).c_str(), col, flags, size);
}

// ── Debug / tool windows ──────────────────────────────────────────────────────
void ShowDemoWindow(bool* pOpen) {
    ImGui::ShowDemoWindow(pOpen);
}
void ShowMetricsWindow(bool* pOpen) {
    ImGui::ShowMetricsWindow(pOpen);
}
void ShowStyleEditor(ImGuiStyle* ref) {
    ImGui::ShowStyleEditor(ref);
}

// ── Draw-list access ──────────────────────────────────────────────────────────
ImDrawList* GetWindowDrawList() {
    return ImGui::GetWindowDrawList();
}
ImDrawList* GetBackgroundDrawList() {
    return ImGui::GetBackgroundDrawList();
}
ImDrawList* GetForegroundDrawList() {
    return ImGui::GetForegroundDrawList();
}

// ── A6: Text (extras) ─────────────────────────────────────────────────────────
void TextUnformatted(std::string_view text) {
    ImGui::TextUnformatted(text.data(), text.data() + text.size());
}
bool TextLink(std::string_view label) {
    return ImGui::TextLink(Z(label).c_str());
}
void TextLinkOpenURL(std::string_view label, std::string_view url) {
    std::string u(url);
    ImGui::TextLinkOpenURL(Z(label).c_str(), url.empty() ? nullptr : u.c_str());
}

// ── A6: Tooltips ──────────────────────────────────────────────────────────────
bool BeginTooltip() {
    return ImGui::BeginTooltip();
}
void EndTooltip() {
    ImGui::EndTooltip();
}
void SetTooltip(std::string_view text) {
    ImGui::SetTooltip("%s", Z(text).c_str());
}
bool BeginItemTooltip() {
    return ImGui::BeginItemTooltip();
}
void SetItemTooltip(std::string_view text) {
    ImGui::SetItemTooltip("%s", Z(text).c_str());
}

// ── A6: Disabled block ────────────────────────────────────────────────────────
void BeginDisabled(bool disabled) {
    ImGui::BeginDisabled(disabled);
}
void EndDisabled() {
    ImGui::EndDisabled();
}

// ── A6: Combo (low-level) ─────────────────────────────────────────────────────
bool BeginCombo(std::string_view label, std::string_view preview, ImGuiComboFlags flags) {
    std::string p(preview);
    // Apply the unified UniGUI combo look (slim chevron instead of the stock
    // arrow button) so custom BeginCombo/EndCombo call sites match im::Combo.
    const auto frame = detail::CaptureComboFrame();
    const bool open = ImGui::BeginCombo(Z(label).c_str(), preview.empty() ? nullptr : p.c_str(),
                                        flags | ImGuiComboFlags_NoArrowButton);
    detail::DrawComboChevron(frame, open || ImGui::IsItemHovered());
    return open;
}
void EndCombo() {
    ImGui::EndCombo();
}

// ── A6: ListBox (low-level) ───────────────────────────────────────────────────
bool BeginListBox(std::string_view label, const ImVec2& size) {
    return ImGui::BeginListBox(Z(label).c_str(), size);
}
void EndListBox() {
    ImGui::EndListBox();
}

// ── A6: Selectable ────────────────────────────────────────────────────────────
bool Selectable(std::string_view label, bool selected, ImGuiSelectableFlags flags,
                const ImVec2& size) {
    return ImGui::Selectable(Z(label).c_str(), selected, flags, size);
}
bool Selectable(std::string_view label, bool* pSelected, ImGuiSelectableFlags flags,
                const ImVec2& size) {
    return ImGui::Selectable(Z(label).c_str(), pSelected, flags, size);
}

// ── A6: Trees & headers ───────────────────────────────────────────────────────
bool TreeNode(std::string_view label) {
    return ImGui::TreeNode(Z(label).c_str());
}
bool TreeNodeEx(std::string_view label, ImGuiTreeNodeFlags flags) {
    return ImGui::TreeNodeEx(Z(label).c_str(), flags);
}
void TreePop() {
    ImGui::TreePop();
}
void SetNextItemOpen(bool isOpen, ImGuiCond cond) {
    ImGui::SetNextItemOpen(isOpen, cond);
}
bool TreeNodeGetOpen(ImGuiID storageId) {
    return ImGui::TreeNodeGetOpen(storageId);
}
bool CollapsingHeader(std::string_view label, ImGuiTreeNodeFlags flags) {
    return ImGui::CollapsingHeader(Z(label).c_str(), flags);
}
bool CollapsingHeader(std::string_view label, bool* pVisible, ImGuiTreeNodeFlags flags) {
    return ImGui::CollapsingHeader(Z(label).c_str(), pVisible, flags);
}

// ── A6: Tab bars ──────────────────────────────────────────────────────────────
bool BeginTabBar(std::string_view strId, ImGuiTabBarFlags flags) {
    return ImGui::BeginTabBar(Z(strId).c_str(), flags);
}
void EndTabBar() {
    ImGui::EndTabBar();
}
bool BeginTabItem(std::string_view label, bool* pOpen, ImGuiTabItemFlags flags) {
    return ImGui::BeginTabItem(Z(label).c_str(), pOpen, flags);
}
void EndTabItem() {
    ImGui::EndTabItem();
}

// ── A6: Plots & progress ──────────────────────────────────────────────────────
void ProgressBar(float fraction, const ImVec2& size, std::string_view overlay) {
    std::string o(overlay);
    ImGui::ProgressBar(fraction, size, overlay.empty() ? nullptr : o.c_str());
}
void PlotLines(std::string_view label, const float* values, int count, int offset,
               std::string_view overlay, float scaleMin, float scaleMax, const ImVec2& size) {
    std::string o(overlay);
    ImGui::PlotLines(Z(label).c_str(), values, count, offset, overlay.empty() ? nullptr : o.c_str(),
                     scaleMin, scaleMax, size);
}
void PlotHistogram(std::string_view label, const float* values, int count, int offset,
                   std::string_view overlay, float scaleMin, float scaleMax, const ImVec2& size) {
    std::string o(overlay);
    ImGui::PlotHistogram(Z(label).c_str(), values, count, offset,
                         overlay.empty() ? nullptr : o.c_str(), scaleMin, scaleMax, size);
}

// ── A6: Color editors & conversion ────────────────────────────────────────────
bool ColorEdit3(std::string_view label, float col[3], ImGuiColorEditFlags flags) {
    return ImGui::ColorEdit3(Z(label).c_str(), col, flags);
}
bool ColorEdit4(std::string_view label, float col[4], ImGuiColorEditFlags flags) {
    return ImGui::ColorEdit4(Z(label).c_str(), col, flags);
}
bool ColorPicker3(std::string_view label, float col[3], ImGuiColorEditFlags flags) {
    return ImGui::ColorPicker3(Z(label).c_str(), col, flags);
}
bool ColorPicker4(std::string_view label, float col[4], ImGuiColorEditFlags flags,
                  const float* ref) {
    return ImGui::ColorPicker4(Z(label).c_str(), col, flags, ref);
}
void ColorConvertRGBtoHSV(float r, float g, float b, float& outH, float& outS, float& outV) {
    ImGui::ColorConvertRGBtoHSV(r, g, b, outH, outS, outV);
}
void ColorConvertHSVtoRGB(float h, float s, float v, float& outR, float& outG, float& outB) {
    ImGui::ColorConvertHSVtoRGB(h, s, v, outR, outG, outB);
}
ImU32 ColorConvertFloat4ToU32(const ImVec4& in) {
    return ImGui::ColorConvertFloat4ToU32(in);
}
ImVec4 ColorConvertU32ToFloat4(ImU32 in) {
    return ImGui::ColorConvertU32ToFloat4(in);
}

// ── A6: Window-state queries ──────────────────────────────────────────────────
bool IsWindowAppearing() {
    return ImGui::IsWindowAppearing();
}
bool IsWindowCollapsed() {
    return ImGui::IsWindowCollapsed();
}
bool IsWindowFocused(ImGuiFocusedFlags flags) {
    return ImGui::IsWindowFocused(flags);
}
bool IsWindowHovered(ImGuiHoveredFlags flags) {
    return ImGui::IsWindowHovered(flags);
}

// ── A6: Misc utilities ────────────────────────────────────────────────────────
ImVec2 CalcTextSize(std::string_view text, bool hideAfterDoubleHash, float wrapWidth) {
    return ImGui::CalcTextSize(text.data(), text.data() + text.size(), hideAfterDoubleHash,
                               wrapWidth);
}
void SetKeyboardFocusHere(int offset) {
    ImGui::SetKeyboardFocusHere(offset);
}
double GetTime() {
    return ImGui::GetTime();
}
int GetFrameCount() {
    return ImGui::GetFrameCount();
}
void SetMouseCursor(ImGuiMouseCursor type) {
    ImGui::SetMouseCursor(type);
}
ImGuiMouseCursor GetMouseCursor() {
    return ImGui::GetMouseCursor();
}

} // namespace unigui::im
