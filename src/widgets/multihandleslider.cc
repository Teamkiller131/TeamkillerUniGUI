#include <unigui/fx/animation.h>
#include <unigui/widgets/multihandleslider.h>

#include <imgui_internal.h> // SetKeyOwner (key ownership while nav-focused)

#include <cmath>

namespace unigui {

MultiHandleSlider::MultiHandleSlider(std::string name)
        : FluentWidget<MultiHandleSlider>(std::move(name)) {}

void MultiHandleSlider::SetRange(float min, float max) {
    rangeMin_ = min;
    rangeMax_ = std::max(min + 1.f, max);
}

void MultiHandleSlider::SetTicks(const std::vector<SliderTick>& ticks) {
    ticks_ = ticks;
}

void MultiHandleSlider::AddTick(SliderTick tick) {
    ticks_.push_back(std::move(tick));
}

void MultiHandleSlider::RemoveTick(int id) {
    ticks_.erase(std::remove_if(ticks_.begin(), ticks_.end(), [id](auto& t) { return t.id == id; }),
                 ticks_.end());
}

void MultiHandleSlider::Render() {
    if (!IsVisible())
        return;
    ImGui::PushID(GetName().c_str());

    auto* dl = ImGui::GetWindowDrawList();
    ImVec2 cursor = ImGui::GetCursorScreenPos();
    float barW = ImGui::GetContentRegionAvail().x - 20.f;
    float barX = cursor.x + 10.f;
    float barY = cursor.y + 16.f;
    float range = rangeMax_ - rangeMin_;

    // ── Track bar ────────────────────────────────────────────────────────
    ImVec2 barMin(barX, barY - barHeight_ * 0.5f);
    ImVec2 barMax(barX + barW, barY + barHeight_ * 0.5f);
    dl->AddRectFilled(barMin, barMax, IM_COL32(45, 45, 55, 255), barHeight_ * 0.5f);

    // ── Current marker line ──────────────────────────────────────────────
    if (hasMarker_) {
        float mx = barX + (markerPos_ - rangeMin_) / range * barW;
        dl->AddLine(ImVec2(mx, barY - 14.f), ImVec2(mx, barY + 10.f), markerColor_, 2.f);
    }

    // ── Tick handles ─────────────────────────────────────────────────────
    ImGui::SetCursorScreenPos(ImVec2(barX, barY - 16.f));
    ImGui::SetNextItemAllowOverlap();
    // EnableNav makes the slider Tab-reachable — it was keyboard-dead before.
    ImGui::InvisibleButton(GetName().c_str(), ImVec2(barW, 32.f), ImGuiButtonFlags_EnableNav);
    const bool zoneFocused = ImGui::IsItemFocused();
    ReportAccessible(a11y::Role::Slider, zoneFocused, std::to_string(ticks_.size()));

    bool hovered = ImGui::IsItemHovered();
    bool active = ImGui::IsItemActive();
    float mouseX = ImGui::GetIO().MousePos.x;

    // ── Keyboard editing ─────────────────────────────────────────────────
    // While focused, own the arrows so nav doesn't move focus away: Up/Down
    // cycle the selected handle, Left/Right move it (Ctrl = ×10 step); the
    // selected handle gets a nav-colored ring below.
    if (zoneFocused && !ticks_.empty()) {
        const ImGuiID zoneId = ImGui::GetItemID();
        ImGui::SetKeyOwner(ImGuiKey_LeftArrow, zoneId);
        ImGui::SetKeyOwner(ImGuiKey_RightArrow, zoneId);
        ImGui::SetKeyOwner(ImGuiKey_UpArrow, zoneId);
        ImGui::SetKeyOwner(ImGuiKey_DownArrow, zoneId);

        navTick_ = navTick_ < 0 ? 0 : navTick_ % (int) ticks_.size();
        if (ImGui::IsKeyPressed(ImGuiKey_UpArrow))
            navTick_ = (navTick_ + 1) % (int) ticks_.size();
        if (ImGui::IsKeyPressed(ImGuiKey_DownArrow))
            navTick_ = navTick_ == 0 ? (int) ticks_.size() - 1 : navTick_ - 1;

        const float step = (range / 100.f) * (ImGui::GetIO().KeyCtrl ? 10.f : 1.f);
        float move = 0.f;
        if (ImGui::IsKeyPressed(ImGuiKey_RightArrow))
            move += step;
        if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow))
            move -= step;
        if (move != 0.f) {
            auto& t = ticks_[navTick_];
            t.position = t.position + move;
            t.position = t.position < rangeMin_ ? rangeMin_
                                                : (t.position > rangeMax_ ? rangeMax_ : t.position);
            if (onChange_)
                onChange_(t.id, t.position);
        }
    }

    for (int i = 0; i < (int) ticks_.size(); ++i) {
        auto& t = ticks_[i];
        float tx = barX + (t.position - rangeMin_) / range * barW;

        // ── Dragging ─────────────────────────────────────────────────
        if (hovered && active && ImGui::IsMouseDown(0) && activeTick_ < 0) {
            float dist = std::abs(mouseX - tx);
            if (dist < 12.f)
                activeTick_ = i;
        }
        if (activeTick_ == i && ImGui::IsMouseDown(0)) {
            t.position = rangeMin_ + (mouseX - barX) / barW * range;
            t.position = t.position < rangeMin_ ? rangeMin_
                                                : (t.position > rangeMax_ ? rangeMax_ : t.position);
            tx = barX + (t.position - rangeMin_) / range * barW;
            if (onChange_)
                onChange_(t.id, t.position);
        }
        if (activeTick_ == i && !ImGui::IsMouseDown(0)) {
            activeTick_ = -1;
        }

        // ── Draw handle ──────────────────────────────────────────────
        dl->AddCircleFilled(ImVec2(tx, barY), 8.f, t.color);
        dl->AddCircle(ImVec2(tx, barY), 8.f, IM_COL32_WHITE, 0, 2.f);
        // Ring on the keyboard-selected handle while the slider has nav focus.
        if (zoneFocused && i == navTick_)
            dl->AddCircle(ImVec2(tx, barY), 11.f, ImGui::GetColorU32(ImGuiCol_NavCursor), 0, 2.f);

        // ── Index label ──────────────────────────────────────────────
        char label[16];
        snprintf(label, sizeof(label), "%d", i + 1);
        dl->AddText(ImVec2(tx - 4.f, barY + 10.f), IM_COL32_WHITE, label);

        // ── Custom overlay ───────────────────────────────────────────
        if (overlayFn_)
            overlayFn_(t.id, i, tx, barW);
    }

    ImGui::Dummy(ImVec2(barW + 20.f, 60.f));
    ImGui::PopID();
}

} // namespace unigui
