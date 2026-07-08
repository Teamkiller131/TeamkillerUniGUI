#define IMGUI_DEFINE_MATH_OPERATORS
#include <unigui/widgets/loadingindicator.h>

#include <imgui.h>
#include <imgui_internal.h>
namespace unigui {
LoadingIndicator::LoadingIndicator(std::string n, float r)
        : FluentWidget<LoadingIndicator>(std::move(n))
        , radius_(r) {}
void LoadingIndicator::SetActive(bool a) {
    active_ = a;
}
bool LoadingIndicator::IsActive() const {
    return active_;
}
void LoadingIndicator::Render() {
    if (!IsVisible() || !active_)
        return;
    ImGui::PushID(GetName().c_str());
    angle_ += ImGui::GetIO().DeltaTime * 3.0f;
    auto* dl = ImGui::GetWindowDrawList();
    auto pos = ImGui::GetCursorScreenPos();
    ImVec2 center(pos.x + radius_, pos.y + radius_);
    for (int i = 0; i < 8; i++) {
        float a = angle_ + i * IM_PI * 0.25f;
        float alpha = 0.3f + 0.7f * (float) i / 8.0f;
        ImVec2 p(center.x + cosf(a) * radius_ * 0.6f, center.y + sinf(a) * radius_ * 0.6f);
        dl->AddCircleFilled(p, 3, IM_COL32(255, 255, 255, (int) (alpha * 255)));
    }
    ImGui::Dummy(ImVec2(radius_ * 2, radius_ * 2));
    ImGui::PopID();
}
} // namespace unigui
