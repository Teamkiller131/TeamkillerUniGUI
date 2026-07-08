#include <unigui/widgets/gauge.h>

#include <imgui.h>

#include <algorithm>
#include <cmath>
#include <cstdio>

namespace unigui {

namespace {
constexpr float kPi = 3.14159265358979323846f;
} // namespace

Gauge::Gauge(std::string name, float value)
        : FluentWidget<Gauge>(std::move(name))
        , value_(value) {}

void Gauge::SetRange(float minV, float maxV) {
    min_ = minV;
    max_ = maxV;
}

float Gauge::GetFraction() const {
    const float span = max_ - min_;
    if (span <= 0.f)
        return 0.f;
    const float f = (value_ - min_) / span;
    return f < 0.f ? 0.f : (f > 1.f ? 1.f : f);
}

void Gauge::SetCenterLabel(std::string text) {
    centerLabel_ = std::move(text);
}

void Gauge::Render() {
    if (!IsVisible())
        return;

    ImGui::PushID(GetName().c_str());

    const float r = radius_;
    const float pad = thickness_ * 0.5f + 1.f;
    const float box = (r + pad) * 2.f;
    const ImVec2 origin = ImGui::GetCursorScreenPos();
    ImGui::Dummy(ImVec2(box, box));

    const ImVec2 center(origin.x + box * 0.5f, origin.y + box * 0.5f);

    // Arc geometry: centre the sweep on the bottom of the dial. ImGui angles are
    // measured clockwise from +X (right); the bottom is +90°. A 270° sweep then
    // runs from 135° round through the top to 405° (=45°), leaving an open base.
    const float sweep = std::clamp(sweepDeg_, 1.f, 360.f) * kPi / 180.f;
    const float mid = kPi * 0.5f; // bottom
    const float a0 = mid + (2.f * kPi - sweep) * 0.5f;
    const float a1 = a0 + sweep;
    const float aFill = a0 + sweep * GetFraction();

    ImU32 track = trackColor_ != 0 ? trackColor_ : ImGui::GetColorU32(ImGuiCol_FrameBg);
    ImU32 fill = fillColor_ != 0 ? fillColor_ : ImGui::GetColorU32(ImGuiCol_CheckMark);

    ImDrawList* dl = ImGui::GetWindowDrawList();
    constexpr int kSegments = 64;

    // Track (full arc) then the value fill on top.
    dl->PathArcTo(center, r, a0, a1, kSegments);
    dl->PathStroke(track, ImDrawFlags_None, thickness_);
    if (aFill > a0) {
        dl->PathArcTo(center, r, a0, aFill, kSegments);
        dl->PathStroke(fill, ImDrawFlags_None, thickness_);
    }

    // Centre label: explicit text wins, else an optional percentage.
    std::string label = centerLabel_;
    if (label.empty() && showPercent_) {
        char buf[16];
        std::snprintf(buf, sizeof(buf), "%d%%",
                      static_cast<int>(std::lround(GetFraction() * 100.f)));
        label = buf;
    }
    if (!label.empty()) {
        const ImVec2 ts = ImGui::CalcTextSize(label.c_str());
        dl->AddText(ImVec2(center.x - ts.x * 0.5f, center.y - ts.y * 0.5f),
                    ImGui::GetColorU32(ImGuiCol_Text), label.c_str());
    }

    RenderTooltip();
    ImGui::PopID();
}

} // namespace unigui
