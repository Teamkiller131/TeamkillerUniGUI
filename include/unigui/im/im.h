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

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace unigui::im {

// ── Button color variants (mirror unigui::Button::ColorVariant) ──────────────
enum class ButtonVariant { Default, Primary, Danger, Success };

// ── Buttons ──────────────────────────────────────────────────────────────────
/// Themed button. Returns true on the frame it is clicked.
bool Button(std::string_view label, const ImVec2& size = ImVec2(0, 0));
/// Themed button with an explicit color variant (Primary/Danger/Success).
bool Button(std::string_view label, ButtonVariant variant,
            const ImVec2& size = ImVec2(0, 0));
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
bool SliderInt(std::string_view label, int* value, int min, int max,
               std::string_view format = "%d");
bool DragFloat(std::string_view label, float* value, float speed = 1.0f,
               float min = 0.0f, float max = 0.0f,
               std::string_view format = "%.3f");
bool DragInt(std::string_view label, int* value, float speed = 1.0f, int min = 0,
             int max = 0, std::string_view format = "%d");
bool InputInt(std::string_view label, int* value, int step = 1,
              int stepFast = 100);
bool InputFloat(std::string_view label, float* value, float step = 0.0f,
                float stepFast = 0.0f, std::string_view format = "%.3f");

// ── Text inputs ───────────────────────────────────────────────────────────────
/// Single-line text input bound to a std::string. Returns true when edited.
bool InputText(std::string_view label, std::string* value,
               std::size_t maxLength = 256, ImGuiInputTextFlags flags = 0);
/// Multi-line text input bound to a std::string.
bool InputTextMultiline(std::string_view label, std::string* value,
                        const ImVec2& size = ImVec2(0, 0),
                        std::size_t maxLength = 4096,
                        ImGuiInputTextFlags flags = 0);

// ── Combo ─────────────────────────────────────────────────────────────────────
/// Dropdown bound to an index into @p items. Returns true when the selection
/// changes.
bool Combo(std::string_view label, int* current,
           const std::vector<std::string>& items);

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

}  // namespace unigui::im
