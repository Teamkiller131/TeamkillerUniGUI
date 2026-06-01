#include <unigui/widgets/sliderbar.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <algorithm>
#include <cmath>

namespace unigui {

SliderBar::SliderBar(std::string name)
    : Widget(std::move(name)) {}

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
    if (barW <= 0.0f || maxValue_ <= 0) return -1;
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
    if (!IsVisible()) return;

    ImGui::PushID(GetName().c_str());

    const float barHeight = 20.0f;
    const float handleRadius = 8.0f;
    const float leftPanelWidth = 120.0f;
    const float spacing = 6.0f;

    // ── Layout: left panel | gap | bar area ────────────────────────────────
    float availWidth = ImGui::GetContentRegionAvail().x;
    float barAreaWidth = availWidth - leftPanelWidth - spacing;
    if (barAreaWidth < 40.0f) barAreaWidth = 40.0f;

    // ── Left panel ─────────────────────────────────────────────────────────
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
        if (btnWidth < 20.0f) btnWidth = 20.0f;

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

    // ── Main bar area ──────────────────────────────────────────────────────
    ImGui::BeginGroup();
    {
        ImVec2 barPos = ImGui::GetCursorScreenPos();
        // We need a minimal item height for the bar + handle area
        float totalHeight = barHeight + handleRadius * 2.0f + 8.0f;
        ImVec2 barSize(barAreaWidth, totalHeight);

        // Invisible button for the entire bar interaction zone
        ImGui::InvisibleButton((GetName() + "##sliderbar_zone").c_str(), barSize);

        auto* dl = ImGui::GetWindowDrawList();

        // Center Y of the bar line
        float barCenterY = barPos.y + handleRadius + 4.0f;
        float barTop = barCenterY - barHeight * 0.5f;
        float barBottom = barCenterY + barHeight * 0.5f;
        float barX = barPos.x;

        barWidth_ = barAreaWidth;

        // ── 1. Background bar ──────────────────────────────────────────────
        ImU32 bgColor = IM_COL32(0x25, 0x25, 0x28, 0xFF); // #252528
        dl->AddRectFilled(
            ImVec2(barX, barTop),
            ImVec2(barX + barAreaWidth, barBottom),
            bgColor,
            3.0f  // rounding
        );

        // ── 2. Reference tick marks at 20/40/60/80/100 ─────────────────────
        const int refPositions[] = {20, 40, 60, 80, 100};
        for (int ref : refPositions) {
            float frac = static_cast<float>(ref) / 100.0f;
            float x = barX + frac * barAreaWidth;
            bool isMax = (ref == 100);
            ImU32 refColor = isMax ? IM_COL32(0xE9, 0x45, 0x60, 0xFF)   // red for 100
                                   : IM_COL32(0x88, 0x88, 0x88, 0xFF);  // grey for others
            float tickTop = barTop - 4.0f;
            float tickBottom = barBottom + 4.0f;
            float thickness = isMax ? 2.0f : 1.0f;
            dl->AddLine(
                ImVec2(x, tickTop),
                ImVec2(x, tickBottom),
                refColor,
                thickness
            );
        }

        // ── 3. Active fill region ──────────────────────────────────────────
        if (activeFrom_ >= 0 && activeTo_ >= 0 && activeFrom_ <= activeTo_ && fillColor_ != 0) {
            float fromFrac = static_cast<float>(activeFrom_) / static_cast<float>(maxValue_);
            float toFrac = static_cast<float>(activeTo_) / static_cast<float>(maxValue_);
            fromFrac = std::clamp(fromFrac, 0.0f, 1.0f);
            toFrac = std::clamp(toFrac, 0.0f, 1.0f);
            float fillX1 = barX + fromFrac * barAreaWidth;
            float fillX2 = barX + toFrac * barAreaWidth;
            dl->AddRectFilled(
                ImVec2(fillX1, barTop),
                ImVec2(fillX2, barBottom),
                fillColor_,
                3.0f
            );
        }

        // ── 4. Current lots marker ─────────────────────────────────────────
        if (currentLots_ > 0 && maxValue_ > 0) {
            float curFrac = static_cast<float>(currentLots_) / static_cast<float>(maxValue_);
            curFrac = std::clamp(curFrac, 0.0f, 1.0f);
            float curX = barX + curFrac * barAreaWidth;
            ImU32 markerColor = IM_COL32(0xFF, 0xFF, 0xFF, 0xFF); // white
            dl->AddLine(
                ImVec2(curX, barTop - 6.0f),
                ImVec2(curX, barBottom + 6.0f),
                markerColor,
                2.0f
            );
            // Small triangle on top
            dl->AddTriangleFilled(
                ImVec2(curX, barTop - 6.0f),
                ImVec2(curX - 5.0f, barTop - 12.0f),
                ImVec2(curX + 5.0f, barTop - 12.0f),
                markerColor
            );
        }

        // ── 5. Handle interaction (dragging) ───────────────────────────────
        ImGuiIO& io = ImGui::GetIO();
        bool mouseInBar = ImGui::IsItemHovered();
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
                circleColor = IM_COL32(0x3A, 0x8E, 0xE6, 0xFF); // default blue
            }

            dl->AddCircleFilled(hPos, radius, circleColor);

            // White border when dragging
            if (i == draggingIndex_) {
                dl->AddCircle(hPos, radius, IM_COL32(0xFF, 0xFF, 0xFF, 0xFF), 0, 2.0f);
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
