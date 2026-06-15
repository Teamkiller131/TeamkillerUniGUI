#include <unigui/widgets/sparkline.h>

#include <imgui.h>

#include <algorithm>

namespace unigui {

Sparkline::Sparkline(std::string name, Mode mode)
        : FluentWidget<Sparkline>(std::move(name))
        , mode_(mode) {}

void Sparkline::SetData(std::vector<float> values) {
    data_ = std::move(values);
}

void Sparkline::PushValue(float v) {
    data_.push_back(v);
    if (maxPoints_ != 0 && data_.size() > maxPoints_)
        data_.erase(data_.begin(), data_.begin() + (data_.size() - maxPoints_));
}

void Sparkline::Clear() {
    data_.clear();
}

void Sparkline::SetRange(float minV, float maxV) {
    autoRange_ = false;
    rangeMin_ = minV;
    rangeMax_ = maxV;
}

void Sparkline::Render() {
    if (!IsVisible())
        return;

    ImGui::PushID(GetName().c_str());

    const ImVec2 pos = ImGui::GetCursorScreenPos();
    const ImVec2 size = size_;
    // Reserve the layout box so neighbours flow correctly even when empty.
    ImGui::Dummy(size);

    // Need at least two points to draw a meaningful trend.
    if (data_.size() < 2) {
        ImGui::PopID();
        return;
    }

    // Resolve the vertical range.
    float lo = rangeMin_, hi = rangeMax_;
    if (autoRange_) {
        const auto [mnIt, mxIt] = std::minmax_element(data_.begin(), data_.end());
        lo = *mnIt;
        hi = *mxIt;
    }
    float span = hi - lo;
    if (span <= 0.f)
        span = 1.f; // flat series → centre it

    // Resolve colours (theme-derived defaults; trend tint takes precedence).
    ImU32 line = lineColor_ != 0 ? lineColor_ : ImGui::GetColorU32(ImGuiCol_PlotLines);
    if (colorByTrend_) {
        const bool up = data_.back() >= data_.front();
        line = up ? IM_COL32(0x2e, 0xd1, 0x5e, 0xFF) : IM_COL32(0xe5, 0x3e, 0x3e, 0xFF);
    }
    ImU32 fill = fillColor_;
    if (fill == 0) {
        ImVec4 f = ImGui::ColorConvertU32ToFloat4(line);
        f.w = 0.25f; // translucent area under the line
        fill = ImGui::ColorConvertFloat4ToU32(f);
    }

    ImDrawList* dl = ImGui::GetWindowDrawList();
    dl->PushClipRect(pos, ImVec2(pos.x + size.x, pos.y + size.y), true);

    const std::size_t n = data_.size();
    const float baselineY = pos.y + size.y;
    auto pointAt = [&](std::size_t i) {
        const float fx = (n == 1) ? 0.f : static_cast<float>(i) / static_cast<float>(n - 1);
        const float fy = (data_[i] - lo) / span; // 0..1, bottom..top
        return ImVec2(pos.x + fx * size.x, baselineY - fy * size.y);
    };

    if (mode_ == Mode::Bar) {
        const float slot = size.x / static_cast<float>(n);
        const float bw = std::max(1.f, slot * 0.7f);
        for (std::size_t i = 0; i < n; ++i) {
            const float cx = pos.x + (static_cast<float>(i) + 0.5f) * slot;
            const float fy = (data_[i] - lo) / span;
            const float topY = baselineY - fy * size.y;
            dl->AddRectFilled(ImVec2(cx - bw * 0.5f, topY), ImVec2(cx + bw * 0.5f, baselineY),
                              line);
        }
    } else {
        if (mode_ == Mode::Area) {
            // Filled polygon: the line points plus the two bottom corners.
            std::vector<ImVec2> poly;
            poly.reserve(n + 2);
            for (std::size_t i = 0; i < n; ++i)
                poly.push_back(pointAt(i));
            poly.push_back(ImVec2(pos.x + size.x, baselineY));
            poly.push_back(ImVec2(pos.x, baselineY));
            dl->AddConvexPolyFilled(poly.data(), static_cast<int>(poly.size()), fill);
        }
        for (std::size_t i = 1; i < n; ++i)
            dl->AddLine(pointAt(i - 1), pointAt(i), line, thickness_);
    }

    if (showLastDot_) {
        const ImVec2 last = (mode_ == Mode::Bar)
                                ? ImVec2(pos.x + size.x - std::max(1.f, size.x / n) * 0.5f,
                                         baselineY - ((data_.back() - lo) / span) * size.y)
                                : pointAt(n - 1);
        dl->AddCircleFilled(last, std::max(2.f, thickness_ + 1.f), line, 12);
    }

    dl->PopClipRect();

    RenderTooltip();
    ImGui::PopID();
}

} // namespace unigui
