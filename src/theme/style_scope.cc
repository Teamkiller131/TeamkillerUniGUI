#include <unigui/theme/style_scope.h>

namespace unigui {

StyleScope::~StyleScope() {
    if (moved_from_)
        return;
    for (int i = 0; i < var_push_count_; i++) {
        ImGui::PopStyleVar();
    }
    for (int i = 0; i < color_push_count_; i++) {
        ImGui::PopStyleColor();
    }
}

StyleScope::StyleScope(StyleScope&& other) noexcept
        : color_push_count_(other.color_push_count_)
        , var_push_count_(other.var_push_count_)
        , moved_from_(false) {
    other.color_push_count_ = 0;
    other.var_push_count_ = 0;
    other.push_count_ = 0;
    other.moved_from_ = true;
}

StyleScope& StyleScope::operator=(StyleScope&& other) noexcept {
    if (this != &other) {
        for (int i = 0; i < var_push_count_; i++) {
            ImGui::PopStyleVar();
        }
        for (int i = 0; i < color_push_count_; i++) {
            ImGui::PopStyleColor();
        }
        color_push_count_ = other.color_push_count_;
        var_push_count_ = other.var_push_count_;
        push_count_ = other.push_count_;
        other.color_push_count_ = 0;
        other.var_push_count_ = 0;
        other.push_count_ = 0;
        other.moved_from_ = true;
        moved_from_ = false;
    }
    return *this;
}

void StyleScope::PushColor(ImGuiCol idx, const ImVec4& color) {
    ImGui::PushStyleColor(idx, color);
    color_push_count_++;
    push_count_++;
}

void StyleScope::PushVar(ImGuiStyleVar idx, float val) {
    ImGui::PushStyleVar(idx, val);
    var_push_count_++;
    push_count_++;
}

void StyleScope::PushVar(ImGuiStyleVar idx, const ImVec2& val) {
    ImGui::PushStyleVar(idx, val);
    var_push_count_++;
    push_count_++;
}

} // namespace unigui
