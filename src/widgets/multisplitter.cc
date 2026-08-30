#include <unigui/widgets/multisplitter.h>

#include <imgui.h>

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <string>

namespace unigui {

MultiSplitter::MultiSplitter(std::string name, Orientation ori)
        : FluentWidget<MultiSplitter>(std::move(name))
        , ori_(ori) {}

void MultiSplitter::AddPanel(float ratio, std::function<void()> content) {
    // Store the raw weight only. Normalizing in place here would overwrite each
    // panel's original weight (the first panel becomes 1.0), so every later
    // AddPanel would compare against rescaled values and the intended
    // proportions would drift. Normalization is done lazily in GetRatios() and
    // at the top of Render().
    panels_.push_back({std::max(0.0f, ratio), std::move(content), 0.0f});
}

void MultiSplitter::AddPanel(float ratio, std::function<void()> content, float minPx) {
    panels_.push_back({std::max(0.0f, ratio), std::move(content), std::max(0.0f, minPx)});
}

void MultiSplitter::Configure(std::vector<PanelDef> defs) {
    // Idempotent: only (re)build when the panel count changes, so a per-frame
    // call is a no-op after the first frame (mirrors the old static-bool guard).
    if (panels_.size() == defs.size())
        return;
    panels_.clear();
    designRatios_.clear();
    panels_.reserve(defs.size());
    designRatios_.reserve(defs.size());
    for (auto& d : defs) {
        panels_.push_back({std::max(0.0f, d.ratio), std::move(d.content), std::max(0.0f, d.minPx)});
        designRatios_.push_back(std::max(0.0f, d.ratio));
    }
}

std::vector<float> MultiSplitter::GetRatios() const {
    std::vector<float> ratios;
    ratios.reserve(panels_.size());
    for (auto& p : panels_)
        ratios.push_back(std::max(0.0f, p.ratio));

    float total = 0.0f;
    for (float r : ratios)
        total += r;
    if (total <= 0.0f) {
        const float even = ratios.empty() ? 0.0f : 1.0f / static_cast<float>(ratios.size());
        for (float& r : ratios)
            r = even;
    } else {
        for (float& r : ratios)
            r /= total;
    }
    return ratios;
}

void MultiSplitter::SetRatios(const std::vector<float>& ratios) {
    int count = (int) std::min(panels_.size(), ratios.size());
    for (int i = 0; i < count; ++i)
        panels_[i].ratio = ratios[i];
    float total = 0.0f;
    for (auto& panel : panels_)
        total += std::max(0.0f, panel.ratio);
    if (total <= 0.0f) {
        const float even = panels_.empty() ? 0.0f : 1.0f / static_cast<float>(panels_.size());
        for (auto& panel : panels_)
            panel.ratio = even;
        return;
    }
    for (auto& panel : panels_)
        panel.ratio = std::max(0.0f, panel.ratio) / total;
}

void MultiSplitter::ResetToDesign() {
    if (!designRatios_.empty())
        SetRatios(designRatios_);
}

std::string MultiSplitter::SerializeLayout() const {
    const auto ratios = GetRatios();
    std::string out;
    char buf[32];
    for (std::size_t i = 0; i < ratios.size(); ++i) {
        if (i)
            out.push_back(',');
        std::snprintf(buf, sizeof(buf), "%.4f", ratios[i]);
        out += buf;
    }
    return out;
}

bool MultiSplitter::RestoreLayout(const std::string& s) {
    // Parse comma-separated floats without throwing (project rule).
    std::vector<float> r;
    std::size_t i = 0;
    while (i < s.size()) {
        std::size_t comma = s.find(',', i);
        const std::string tok =
            s.substr(i, comma == std::string::npos ? std::string::npos : comma - i);
        const char* b = tok.c_str();
        char* e = nullptr;
        const float v = std::strtof(b, &e);
        if (e != b)
            r.push_back(v);
        if (comma == std::string::npos)
            break;
        i = comma + 1;
    }
    if (r.empty() || r.size() != panels_.size())
        return false; // wrong count → ignore (stale/foreign layout)
    SetRatios(r);
    return true;
}

void MultiSplitter::Render() {
    if (!IsVisible() || panels_.empty())
        return;

    auto avail = ImGui::GetContentRegionAvail();
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));

    constexpr float kHandleThickness = 5.0f;
    float totalLen = (ori_ == Horizontal) ? avail.y : avail.x;
    const float handlesLen = kHandleThickness * std::max(0, (int) panels_.size() - 1);
    const float panelSpace = std::max(0.0f, totalLen - handlesLen);
    const float minRatio = panelSpace > 0.0f ? std::min(0.45f, 24.0f / panelSpace) : 0.0f;
    float remainingLen = panelSpace;
    auto normalizeRatios = [&]() {
        float total = 0.0f;
        for (auto& panel : panels_)
            total += std::max(0.0f, panel.ratio);
        if (total <= 0.0f) {
            const float even = panels_.empty() ? 0.0f : 1.0f / static_cast<float>(panels_.size());
            for (auto& panel : panels_)
                panel.ratio = even;
            return;
        }
        for (auto& panel : panels_)
            panel.ratio = std::max(0.0f, panel.ratio) / total;
    };

    // Panels store raw weights; normalize once before computing sizes. This is
    // idempotent after the first frame and keeps drag adjustments consistent.
    normalizeRatios();

    for (int i = 0; i < (int) panels_.size(); ++i) {
        float panelLen =
            (i == (int) panels_.size() - 1) ? remainingLen : panelSpace * panels_[i].ratio;
        // Enforce a per-panel minimum size along the split axis (non-last panels;
        // bounded by what's left so we never overrun the available space).
        if (i != (int) panels_.size() - 1 && panels_[i].minPx > 0.0f)
            panelLen = std::max(panelLen, std::min(panels_[i].minPx, remainingLen));
        remainingLen = std::max(0.0f, remainingLen - panelLen);

        ImGui::PushID(i);
        ImGui::BeginChild(
            "##panel", ori_ == Horizontal ? ImVec2(avail.x, panelLen) : ImVec2(panelLen, avail.y),
            ImGuiChildFlags_Borders);
        if (panels_[i].content)
            panels_[i].content();
        ImGui::EndChild();
        ImGui::PopID();

        if (i < (int) panels_.size() - 1) {
            char handleID[64];
            snprintf(handleID, sizeof(handleID), "##ms_%s_%d", GetName().c_str(), i);
            if (ori_ == Horizontal) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.3f, 0.35f, 1));
                ImGui::Button(handleID, ImVec2(avail.x, kHandleThickness));
                ImGui::PopStyleColor();
                if (ImGui::IsItemActive()) {
                    float deltaPx = ImGui::GetIO().MouseDelta.y;
                    // Keyboard path: nav-activate the handle (hold Space/Enter),
                    // then Up/Down resize; nav movement is suspended while the
                    // item is active so the arrows are free. Mouse drag was the
                    // only resize path before.
                    if (ImGui::IsKeyPressed(ImGuiKey_UpArrow))
                        deltaPx -= panelSpace * 0.02f;
                    if (ImGui::IsKeyPressed(ImGuiKey_DownArrow))
                        deltaPx += panelSpace * 0.02f;
                    float deltaRatio = panelSpace > 0.0f ? deltaPx / panelSpace : 0.0f;
                    panels_[i].ratio += deltaRatio;
                    panels_[i + 1].ratio -= deltaRatio;
                    if (panels_[i].ratio < minRatio) {
                        panels_[i + 1].ratio += panels_[i].ratio - minRatio;
                        panels_[i].ratio = minRatio;
                    }
                    if (panels_[i + 1].ratio < minRatio) {
                        panels_[i].ratio += panels_[i + 1].ratio - minRatio;
                        panels_[i + 1].ratio = minRatio;
                    }
                    normalizeRatios();
                }
            } else {
                ImGui::SameLine();
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.3f, 0.35f, 1));
                ImGui::Button(handleID, ImVec2(kHandleThickness, avail.y));
                ImGui::PopStyleColor();
                if (ImGui::IsItemActive()) {
                    float deltaPx = ImGui::GetIO().MouseDelta.x;
                    // Keyboard path — see the horizontal branch above.
                    if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow))
                        deltaPx -= panelSpace * 0.02f;
                    if (ImGui::IsKeyPressed(ImGuiKey_RightArrow))
                        deltaPx += panelSpace * 0.02f;
                    float deltaRatio = panelSpace > 0.0f ? deltaPx / panelSpace : 0.0f;
                    panels_[i].ratio += deltaRatio;
                    panels_[i + 1].ratio -= deltaRatio;
                    if (panels_[i].ratio < minRatio) {
                        panels_[i + 1].ratio += panels_[i].ratio - minRatio;
                        panels_[i].ratio = minRatio;
                    }
                    if (panels_[i + 1].ratio < minRatio) {
                        panels_[i].ratio += panels_[i + 1].ratio - minRatio;
                        panels_[i + 1].ratio = minRatio;
                    }
                    normalizeRatios();
                }
                ImGui::SameLine();
            }
        }
    }

    ImGui::PopStyleVar();
}

} // namespace unigui
