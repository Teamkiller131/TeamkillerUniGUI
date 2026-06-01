#include <unigui/widgets/card.h>
#include <unigui/fx/effect_scope.h>

namespace unigui {

Card::Card(const std::string& title) : title_(title) {}

void Card::SetTitle(const std::string& t)       { title_ = t; }
void Card::SetContent(std::function<void()> fn)  { contentFn_ = std::move(fn); }
void Card::SetFooter(std::function<void()> fn)   { footerFn_ = std::move(fn); }
void Card::SetVariant(Variant v)                 { variant_ = v; }
void Card::SetShadow(bool enable)                { hasShadow_ = enable; }
void Card::SetShadowRadius(float r)              { shadowRadius_ = r; }
void Card::SetPadding(float p)                   { padding_ = p; }
void Card::SetBorderColor(ImU32 c)              { borderColor_ = c; }
void Card::SetBorderRadius(float r)              { borderRadius_ = r; }

void Card::Render() {
    ImGui::PushID(title_.c_str());
    float r = (borderRadius_ > 0.f) ? borderRadius_ : 8.f;

    // ── Push padding + rounding ──────────────────────────────────────────
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, r);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(padding_, padding_));
    ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, (variant_ == Outlined) ? 1.5f : 1.f);

    // ── Distinct border color for Outlined variant ───────────────────────
    if (variant_ == Outlined) {
        ImGui::PushStyleColor(ImGuiCol_Border, ImGui::ColorConvertU32ToFloat4(borderColor_));
    }

    // ── Shadow (Elevated only, drawn before card content) ────────────────
    if (hasShadow_ && variant_ == Elevated) {
        ImVec2 cursor = ImGui::GetCursorScreenPos();
        ImVec2 avail = ImGui::GetContentRegionAvail();
        float estH = ImGui::GetTextLineHeightWithSpacing() * (contentFn_ ? 5.f : 2.f) + padding_ * 2.f;
        fx::ShadowEffect sh(shadowRadius_, 2.f, 2.f, IM_COL32(0, 0, 0, 60), 3);
        sh.SetRect(cursor, ImVec2(cursor.x + avail.x, cursor.y + estH));
        sh.Push(ImGui::GetWindowDrawList()); sh.Pop();
    }

    // ── Card body (child window) ─────────────────────────────────────────
    ImGuiChildFlags flags = (variant_ == Filled)
        ? ImGuiChildFlags_AutoResizeY
        : ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY;

    ImGui::BeginChild(title_.c_str(), ImVec2(0, 0), flags);

    if (!title_.empty()) {
        ImGui::TextUnformatted(title_.c_str());
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0, 4));
    }

    if (contentFn_) contentFn_();
    if (footerFn_) {
        ImGui::Spacing();
        ImGui::Separator();
        footerFn_();
    }

    ImGui::EndChild();

    if (variant_ == Outlined)
        ImGui::PopStyleColor();
    ImGui::PopStyleVar(3);
    ImGui::PopID();
}

} // namespace unigui
