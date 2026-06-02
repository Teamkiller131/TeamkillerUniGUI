#include <unigui/im/im.h>

#include <imgui.h>

#include <algorithm>
#include <cstring>

namespace unigui::im {

namespace {

// Null-terminate a string_view for ImGui's `const char*` APIs. Short strings
// stay in std::string's small-buffer (no heap allocation).
inline std::string Z(std::string_view s) { return std::string(s); }

inline ImVec4 Lighten(const ImVec4& c, float t) {
    return ImVec4(c.x + (1.f - c.x) * t, c.y + (1.f - c.y) * t,
                  c.z + (1.f - c.z) * t, c.w);
}
inline ImVec4 Darken(const ImVec4& c, float t) {
    return ImVec4(c.x * (1.f - t), c.y * (1.f - t), c.z * (1.f - t), c.w);
}

ImVec4 VariantColor(ButtonVariant v, const ImVec4& fallback) {
    switch (v) {
        case ButtonVariant::Primary: return ImVec4(0.16f, 0.47f, 0.80f, 1.0f);
        case ButtonVariant::Danger:  return ImVec4(0.80f, 0.16f, 0.20f, 1.0f);
        case ButtonVariant::Success: return ImVec4(0.18f, 0.60f, 0.28f, 1.0f);
        default: return fallback;
    }
}

}  // namespace

// ── Buttons ──────────────────────────────────────────────────────────────────
bool Button(std::string_view label, const ImVec2& size) {
    return ImGui::Button(Z(label).c_str(), size);
}

bool Button(std::string_view label, ButtonVariant variant, const ImVec2& size) {
    if (variant == ButtonVariant::Default) return Button(label, size);
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
    ImGui::LabelText(Z(label).c_str(), "%.*s", static_cast<int>(text.size()),
                     text.data());
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
    return ImGui::SliderFloat(Z(label).c_str(), value, min, max,
                              Z(format).c_str());
}

bool SliderInt(std::string_view label, int* value, int min, int max,
               std::string_view format) {
    return ImGui::SliderInt(Z(label).c_str(), value, min, max, Z(format).c_str());
}

bool DragFloat(std::string_view label, float* value, float speed, float min,
               float max, std::string_view format) {
    return ImGui::DragFloat(Z(label).c_str(), value, speed, min, max,
                            Z(format).c_str());
}

bool DragInt(std::string_view label, int* value, float speed, int min, int max,
             std::string_view format) {
    return ImGui::DragInt(Z(label).c_str(), value, speed, min, max,
                          Z(format).c_str());
}

bool InputInt(std::string_view label, int* value, int step, int stepFast) {
    return ImGui::InputInt(Z(label).c_str(), value, step, stepFast);
}

bool InputFloat(std::string_view label, float* value, float step, float stepFast,
                std::string_view format) {
    return ImGui::InputFloat(Z(label).c_str(), value, step, stepFast,
                             Z(format).c_str());
}

// ── Text inputs ───────────────────────────────────────────────────────────────
namespace {
// Edit a std::string through a temporary, NUL-terminated character buffer.
// Avoids a hard dependency on imgui_stdlib while keeping the std::string API.
bool EditString(const char* id, std::string* value, std::size_t maxLength,
                bool multiline, const ImVec2& size, ImGuiInputTextFlags flags) {
    const std::size_t cap = std::max(maxLength, value->size() + 1);
    std::vector<char> buf(cap + 1, '\0');
    const std::size_t copy = std::min(value->size(), cap);
    std::memcpy(buf.data(), value->data(), copy);
    buf[copy] = '\0';

    const bool changed =
        multiline
            ? ImGui::InputTextMultiline(id, buf.data(), buf.size(), size, flags)
            : ImGui::InputText(id, buf.data(), buf.size(), flags);
    if (changed) value->assign(buf.data());
    return changed;
}
}  // namespace

bool InputText(std::string_view label, std::string* value,
               std::size_t maxLength, ImGuiInputTextFlags flags) {
    if (!value) return false;
    return EditString(Z(label).c_str(), value, maxLength, /*multiline=*/false,
                      ImVec2(0, 0), flags);
}

bool InputTextMultiline(std::string_view label, std::string* value,
                        const ImVec2& size, std::size_t maxLength,
                        ImGuiInputTextFlags flags) {
    if (!value) return false;
    return EditString(Z(label).c_str(), value, maxLength, /*multiline=*/true,
                      size, flags);
}

// ── Combo ─────────────────────────────────────────────────────────────────────
bool Combo(std::string_view label, int* current,
           const std::vector<std::string>& items) {
    if (!current) return false;
    // Build the stable "a\0b\0c\0\0" item buffer — this Combo overload has a
    // fixed signature across ImGui versions (unlike the getter-callback one).
    std::string packed;
    for (const auto& it : items) {
        packed.append(it);
        packed.push_back('\0');
    }
    packed.push_back('\0');  // terminating empty string
    return ImGui::Combo(Z(label).c_str(), current, packed.c_str());
}

// ── Layout helpers ────────────────────────────────────────────────────────────
void SameLine(float offsetFromStart, float spacing) {
    ImGui::SameLine(offsetFromStart, spacing);
}
void NewLine() { ImGui::NewLine(); }
void Spacing() { ImGui::Spacing(); }
void Separator() { ImGui::Separator(); }
void SeparatorText(std::string_view text) {
    ImGui::SeparatorText(Z(text).c_str());
}
void Dummy(float width, float height) { ImGui::Dummy(ImVec2(width, height)); }
void Indent(float width) { ImGui::Indent(width); }
void Unindent(float width) { ImGui::Unindent(width); }
void Bullet() { ImGui::Bullet(); }

}  // namespace unigui::im
