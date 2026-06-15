#include <unigui/widgets/pnltext.h>

#include <unigui/core/format_num.h>

#include <imgui.h>

#include <string>

namespace unigui {

namespace {
ImU32 RoleColorU32(theme::Semantic role) {
    return ImGui::GetColorU32(theme::GetSemanticColor(role));
}
} // namespace

void PnlText(double value, std::string_view display, double eps) {
    const DirectionRole r = PnlRole(value, eps);
    const ImU32 col = r.isDirectional ? RoleColorU32(r.role) : ImGui::GetColorU32(ImGuiCol_Text);
    const std::string s(display);
    ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(col), "%s", s.c_str());
}

void PnlText(double value, int decimals, double eps) {
    PnlText(value, format::SignedDelta(value, decimals), eps);
}

void StatusText(bool on, std::string_view onLabel, std::string_view offLabel) {
    const ImVec4 col =
        theme::GetSemanticColor(on ? theme::Semantic::Success : theme::Semantic::Danger);
    const std::string s(on ? onLabel : offLabel);
    ImGui::TextColored(col, "%s", s.c_str());
}

void GradedText(double value, double warn, double crit, std::string_view display, bool inverted) {
    const ImVec4 col = theme::GetSemanticColor(GradedRole(value, warn, crit, inverted));
    const std::string s(display);
    ImGui::TextColored(col, "%s", s.c_str());
}

} // namespace unigui
