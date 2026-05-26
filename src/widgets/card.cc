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

void Card::Render() {
    auto& style = ImGui::GetStyle();
    float rounding = style.ChildRounding > 0.f ? style.ChildRounding : 8.f;

    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, rounding);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(padding_, padding_));

    // Shadow (draw before the card via ImDrawList)
    if (hasShadow_ && variant_ == Elevated) {
        ImVec2 cursor = ImGui::GetCursorScreenPos();
        ImVec2 size = ImGui::GetContentRegionAvail();
        size.y = ImGui::CalcTextSize(title_.c_str()).y + padding_ * 5.f; // estimate
        fx::ShadowEffect shadow(shadowRadius_, 2.f, 2.f, IM_COL32(0, 0, 0, 60), 3);
        shadow.SetRect(cursor, ImVec2(cursor.x + size.x, cursor.y + size.y));
        shadow.Push(ImGui::GetWindowDrawList());
        shadow.Pop();
    }

    // Begin child
    ImGui::BeginChild(title_.c_str(), ImVec2(0, 0),
                      ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY);

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
    ImGui::PopStyleVar(3);
}

} // namespace unigui
