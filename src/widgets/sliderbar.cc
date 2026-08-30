#include <unigui/theme/color_tokens.h>
#include <unigui/widgets/sliderbar.h>

#include <imgui.h>
#include <imgui_internal.h>

#include <algorithm>
#include <cmath>

namespace unigui {

SliderBar::SliderBar(std::string name)
        : FluentWidget<SliderBar>(std::move(name)) {}

// ── Helper: convert tick index → screen position on bar ────────────────────
ImVec2 SliderBar::TickToPos(int index, float barX, float barY, float barW) const {
    if (index < 0 || index >= static_cast<int>(ticks_.size()) || maxValue_ <= 0)
        return ImVec2(barX, barY);
    float frac = static_cast<float>(ticks_[index].futuresLots) / static_cast<float>(maxValue_);
    frac = std::clamp(frac, 0.0f, 1.0f);
    return ImVec2(barX + frac * barW, barY);
}

// ── Helper: convert screen x → tick index (closest) ────────────────────────
int SliderBar::PosToTick(float x, float barX, float barW) const {
    if (barW <= 0.0f || maxValue_ <= 0)
        return -1;
    float frac = (x - barX) / barW;
    frac = std::clamp(frac, 0.0f, 1.0f);
    int targetLots = static_cast<int>(std::round(frac * static_cast<float>(maxValue_)));
    // Find closest tick
    int best = -1;
    int bestDist = std::numeric_limits<int>::max();
    for (int i = 0; i < static_cast<int>(ticks_.size()); ++i) {
        int dist = std::abs(ticks_[i].futuresLots - targetLots);
        if (dist < bestDist) {
            bestDist = dist;
            best = i;
        }
    }
    return best;
}

void SliderBar::Render() {
    if (!IsVisible())
        return;

    ImGui::PushID(GetName().c_str());

    const float barHeight = 20.0f;
    const float handleRadius = 8.0f;
    const float leftPanelWidth = leftPanelWidth_;
    const float spacing = 6.0f;
    const bool hasLeftPanel = leftPanelWidth > 0.0f;

    // ── Layout: [left panel | gap |] bar area ──────────────────────────────
    float availWidth = ImGui::GetContentRegionAvail().x;
    float barAreaWidth = hasLeftPanel ? (availWidth - leftPanelWidth - spacing) : availWidth;
    if (barAreaWidth < 40.0f)
        barAreaWidth = 40.0f;

    // ── Left panel (optional) ──────────────────────────────────────────────
    if (hasLeftPanel) {
        ImGui::BeginGroup();
        {
            // Account name / left label
            if (!leftLabel_.empty()) {
                ImGui::TextUnformatted(leftLabel_.c_str());
            }
            // Sub label
            if (!leftSubLabel_.empty()) {
                ImGui::TextDisabled("%s", leftSubLabel_.c_str());
            }
            ImGui::Spacing();

            // Buttons row
            float btnWidth = (leftPanelWidth - 3.0f * spacing) / 4.0f;
            if (btnWidth < 20.0f)
                btnWidth = 20.0f;

            if (onAdd_) {
                if (ImGui::Button("Add", ImVec2(btnWidth, 0))) {
                    onAdd_();
                }
                ImGui::SameLine(0, spacing);
            }
            if (onConfirm_) {
                if (ImGui::Button("OK", ImVec2(btnWidth, 0))) {
                    onConfirm_();
                }
                ImGui::SameLine(0, spacing);
            }
            if (onRollback_) {
                if (ImGui::Button("Rbk", ImVec2(btnWidth, 0))) {
                    onRollback_();
                }
                ImGui::SameLine(0, spacing);
            }
            if (onSubmit_) {
                if (ImGui::Button("Sub", ImVec2(btnWidth, 0))) {
                    onSubmit_();
                }
            }
        }
        ImGui::EndGroup();
        ImGui::SameLine(0, spacing);
    }

    // ── Main bar area ──────────────────────────────────────────────────────
    ImGui::BeginGroup();
    {
        barLocalX_ = ImGui::GetCursorPos().x;
        ImVec2 barPos = ImGui::GetCursorScreenPos();
        // We need a minimal item height for the bar + handle area
        float totalHeight = barHeight + handleRadius * 2.0f + 8.0f;
        ImVec2 barSize(barAreaWidth, totalHeight);

        // Invisible button for the entire bar interaction zone. EnableNav makes it
        // Tab-reachable (the built-in nav cursor draws the focus ring), unlocking
        // the keyboard path below — the bar was previously keyboard-dead.
        ImGui::InvisibleButton((GetName() + "##sliderbar_zone").c_str(), barSize,
                               ImGuiButtonFlags_EnableNav);
        const bool zoneFocused = ImGui::IsItemFocused();
        ReportAccessible(a11y::Role::Slider, zoneFocused, std::to_string(currentLots_));

        // ── Keyboard editing ───────────────────────────────────────────────
        // While the zone is focused, own the arrows so nav doesn't move focus
        // away — they edit the slider instead: Up/Down cycle the handle,
        // Left/Right adjust its lots (Ctrl = ×10), Home/End jump to 0/max.
        if (zoneFocused && !ticks_.empty()) {
            const ImGuiID zoneId = ImGui::GetItemID();
            ImGui::SetKeyOwner(ImGuiKey_LeftArrow, zoneId);
            ImGui::SetKeyOwner(ImGuiKey_RightArrow, zoneId);
            ImGui::SetKeyOwner(ImGuiKey_UpArrow, zoneId);
            ImGui::SetKeyOwner(ImGuiKey_DownArrow, zoneId);

            focusIndex_ = std::clamp(focusIndex_, 0, static_cast<int>(ticks_.size()) - 1);
            if (ImGui::IsKeyPressed(ImGuiKey_UpArrow))
                focusIndex_ = (focusIndex_ + 1) % static_cast<int>(ticks_.size());
            if (ImGui::IsKeyPressed(ImGuiKey_DownArrow))
                focusIndex_ =
                    focusIndex_ == 0 ? static_cast<int>(ticks_.size()) - 1 : focusIndex_ - 1;

            const int step = ImGui::GetIO().KeyCtrl ? 10 : 1;
            int target = ticks_[focusIndex_].futuresLots;
            if (ImGui::IsKeyPressed(ImGuiKey_RightArrow))
                target += step;
            if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow))
                target -= step;
            if (ImGui::IsKeyPressed(ImGuiKey_Home))
                target = 0;
            if (ImGui::IsKeyPressed(ImGuiKey_End))
                target = maxValue_;
            target = std::clamp(target, 0, maxValue_);
            if (target != ticks_[focusIndex_].futuresLots) {
                ticks_[focusIndex_].futuresLots = target;
                unsaved_ = true;
                if (onChanged_)
                    onChanged_(ticks_);
            }
        }

        auto* dl = ImGui::GetWindowDrawList();

        // Center Y of the bar line
        float barCenterY = barPos.y + handleRadius + 4.0f;
        float barTop = barCenterY - barHeight * 0.5f;
        float barBottom = barCenterY + barHeight * 0.5f;
        float barX = barPos.x;

        barWidth_ = barAreaWidth;

        // ── 1. Background bar ──────────────────────────────────────────────
        // Theme-aware track colour (light themes get a light track, dark a dark one)
        ImU32 bgColor = ImGui::GetColorU32(ImGuiCol_FrameBg);
        dl->AddRectFilled(ImVec2(barX, barTop), ImVec2(barX + barAreaWidth, barBottom), bgColor,
                          3.0f // rounding
        );

        // ── 2. Warning ("警戒持仓") band + endpoint markers ────────────────
        // Red band from warnRatio_ → full marks the danger zone (default 90%).
        float warn = std::clamp(warnRatio_, 0.0f, 1.0f);
        ImU32 dangerCol =
            ImGui::GetColorU32(unigui::theme::GetSemanticColor(unigui::theme::Semantic::Danger));
        {
            float wx = barX + warn * barAreaWidth;
            // translucent fill across the warning zone
            ImU32 bandFill = (dangerCol & 0x00FFFFFF) | 0x40000000;
            dl->AddRectFilled(ImVec2(wx, barTop), ImVec2(barX + barAreaWidth, barBottom), bandFill,
                              3.0f);
            // solid red bar at the warning threshold
            dl->AddLine(ImVec2(wx, barTop - 4.0f), ImVec2(wx, barBottom + 4.0f), dangerCol, 3.0f);
        }
        // Endpoint markers at 0 (空仓) and 100 (满仓)
        {
            ImU32 endCol = ImGui::GetColorU32(ImGuiCol_Text);
            float top = barTop - 4.0f, bot = barBottom + 4.0f;
            dl->AddLine(ImVec2(barX, top), ImVec2(barX, bot), endCol, 2.0f);
            dl->AddLine(ImVec2(barX + barAreaWidth, top), ImVec2(barX + barAreaWidth, bot), endCol,
                        2.0f);
        }

        // ── 3. Active fill region ──────────────────────────────────────────
        if (activeFrom_ >= 0 && activeTo_ >= 0 && activeFrom_ <= activeTo_ && fillColor_ != 0) {
            float fromFrac = static_cast<float>(activeFrom_) / static_cast<float>(maxValue_);
            float toFrac = static_cast<float>(activeTo_) / static_cast<float>(maxValue_);
            fromFrac = std::clamp(fromFrac, 0.0f, 1.0f);
            toFrac = std::clamp(toFrac, 0.0f, 1.0f);
            float fillX1 = barX + fromFrac * barAreaWidth;
            float fillX2 = barX + toFrac * barAreaWidth;
            dl->AddRectFilled(ImVec2(fillX1, barTop), ImVec2(fillX2, barBottom), fillColor_, 3.0f);
        }

        // ── 4. Current lots marker ─────────────────────────────────────────
        if (currentLots_ > 0 && maxValue_ > 0) {
            float curFrac = static_cast<float>(currentLots_) / static_cast<float>(maxValue_);
            curFrac = std::clamp(curFrac, 0.0f, 1.0f);
            float curX = barX + curFrac * barAreaWidth;
            ImU32 markerColor =
                ImGui::GetColorU32(ImGuiCol_Text); // theme text (visible on light + dark)
            dl->AddLine(ImVec2(curX, barTop - 6.0f), ImVec2(curX, barBottom + 6.0f), markerColor,
                        2.0f);
            // Small triangle on top
            dl->AddTriangleFilled(ImVec2(curX, barTop - 6.0f), ImVec2(curX - 5.0f, barTop - 12.0f),
                                  ImVec2(curX + 5.0f, barTop - 12.0f), markerColor);
        }

        // ── 5. Handle interaction (dragging) ───────────────────────────────
        ImGuiIO& io = ImGui::GetIO();
        ImVec2 mousePos = io.MousePos;

        // Start drag: find if we clicked on a handle
        if (ImGui::IsItemActive() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            int clickedIndex = PosToTick(mousePos.x, barX, barAreaWidth);
            if (clickedIndex >= 0) {
                // Check if mouse is close enough to the handle
                ImVec2 hPos = TickToPos(clickedIndex, barX, barCenterY, barAreaWidth);
                float dist = std::abs(mousePos.x - hPos.x);
                if (dist < handleRadius + 4.0f) {
                    draggingIndex_ = clickedIndex;
                }
            }
        }

        // During drag: update position
        if (draggingIndex_ >= 0 && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
            float newFrac = (mousePos.x - barX) / barAreaWidth;
            newFrac = std::clamp(newFrac, 0.0f, 1.0f);
            int newLots = static_cast<int>(std::round(newFrac * static_cast<float>(maxValue_)));
            newLots = std::clamp(newLots, 0, maxValue_);
            if (newLots != ticks_[draggingIndex_].futuresLots) {
                ticks_[draggingIndex_].futuresLots = newLots;
                unsaved_ = true;
                if (onChanged_) {
                    onChanged_(ticks_);
                }
            }
        }

        // End drag
        if (draggingIndex_ >= 0 && !ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
            draggingIndex_ = -1;
        }

        // ── 6. Render tick handles (circles) ───────────────────────────────
        for (int i = 0; i < static_cast<int>(ticks_.size()); ++i) {
            ImVec2 hPos = TickToPos(i, barX, barCenterY, barAreaWidth);
            float radius = (i == draggingIndex_) ? handleRadius + 3.0f : handleRadius;

            // Circle fill color
            ImU32 circleColor;
            if (i < static_cast<int>(tickColors_.size())) {
                circleColor = tickColors_[i];
            } else {
                circleColor =
                    ImGui::GetColorU32(unigui::theme::ActiveColorTokens().accent); // theme accent
            }

            dl->AddCircleFilled(hPos, radius, circleColor);

            // White border when dragging
            if (i == draggingIndex_) {
                dl->AddCircle(hPos, radius, IM_COL32(0xFF, 0xFF, 0xFF, 0xFF), 0, 2.0f);
            }
            // Ring on the keyboard-selected handle while the zone has nav focus.
            if (i == focusIndex_ && zoneFocused) {
                dl->AddCircle(hPos, radius + 3.0f, ImGui::GetColorU32(ImGuiCol_NavCursor), 0, 2.0f);
            }
        }
    }
    ImGui::EndGroup();

    ImGui::PopID();
}

// ── Setters / Getters ─────────────────────────────────────────────────────

void SliderBar::SetMaxValue(int maxLots) {
    maxValue_ = std::max(1, maxLots);
}

void SliderBar::SetTickColors(std::vector<ImU32> colors) {
    tickColors_ = std::move(colors);
}

void SliderBar::SetTicks(std::vector<Tick> ticks) {
    ticks_ = std::move(ticks);
}

std::vector<SliderBar::Tick> SliderBar::GetTicks() const {
    return ticks_;
}

int SliderBar::GetActiveTickIndex(double currentPrice, int currentLots) const {
    // Tolerances for floating and integer matching
    const double priceEpsilon = 1e-6;
    for (int i = 0; i < static_cast<int>(ticks_.size()); ++i) {
        if (ticks_[i].futuresLots == currentLots &&
            std::abs(ticks_[i].price - currentPrice) < priceEpsilon) {
            return i;
        }
    }
    return -1;
}

void SliderBar::SetCurrentLots(int lots) {
    currentLots_ = lots;
}

void SliderBar::SetActiveFill(int from, int to, ImU32 color) {
    activeFrom_ = from;
    activeTo_ = to;
    fillColor_ = color;
}

void SliderBar::SetOnChanged(OnChanged fn) {
    onChanged_ = std::move(fn);
}

void SliderBar::SetLeftLabel(std::string label) {
    leftLabel_ = std::move(label);
}

void SliderBar::SetLeftSubLabel(std::string sub) {
    leftSubLabel_ = std::move(sub);
}

void SliderBar::SetOnAdd(std::function<void()> fn) {
    onAdd_ = std::move(fn);
}

void SliderBar::SetOnConfirm(std::function<void()> fn) {
    onConfirm_ = std::move(fn);
}

void SliderBar::SetOnRollback(std::function<void()> fn) {
    onRollback_ = std::move(fn);
}

void SliderBar::SetOnSubmit(std::function<void()> fn) {
    onSubmit_ = std::move(fn);
}

} // namespace unigui
