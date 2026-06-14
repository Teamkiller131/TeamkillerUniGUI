#include <unigui/im/im.h>

#include <imgui.h>

#include <algorithm>
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
    if (changed)
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
    if (changed)
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
    // Build the stable "a\0b\0c\0\0" item buffer — this Combo overload has a
    // fixed signature across ImGui versions (unlike the getter-callback one).
    std::string packed;
    for (const auto& it : items) {
        packed.append(it);
        packed.push_back('\0');
    }
    packed.push_back('\0'); // terminating empty string
    return ImGui::Combo(Z(label).c_str(), current, packed.c_str());
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

} // namespace unigui::im
