#include <unigui/widgets/priceticker.h>

#include <imgui.h>

#include <cmath>

namespace unigui {

PriceTicker::PriceTicker(std::string name, std::vector<Item> items)
        : FluentWidget<PriceTicker>(std::move(name))
        , items_(std::move(items)) {}

void PriceTicker::SetItems(std::vector<Item> items) {
    items_ = std::move(items);
}

void PriceTicker::AddItem(Item item) {
    items_.push_back(std::move(item));
}

void PriceTicker::Clear() {
    items_.clear();
    offset_ = 0.f;
}

namespace {
// "<symbol>  <price> <arrow>" formatted text for one item.
std::string FormatItem(const PriceTicker::Item& it) {
    const char* arrow = it.change > 0.f ? "  \xE2\x96\xB2" : (it.change < 0.f ? "  \xE2\x96\xBC" : "");
    return it.symbol + "  " + it.price + arrow;
}
} // namespace

void PriceTicker::Render() {
    if (!IsVisible())
        return;

    ImGui::PushID(GetName().c_str());

    const float h = height_ > 0.f ? height_ : ImGui::GetFrameHeight();
    const float w = width_ > 0.f ? width_ : ImGui::GetContentRegionAvail().x;
    const ImVec2 origin = ImGui::GetCursorScreenPos();
    ImGui::Dummy(ImVec2(w, h));

    ImDrawList* dl = ImGui::GetWindowDrawList();
    dl->AddRectFilled(origin, ImVec2(origin.x + w, origin.y + h),
                      ImGui::GetColorU32(ImGuiCol_FrameBg), ImGui::GetStyle().FrameRounding);

    if (items_.empty()) {
        ImGui::PopID();
        return;
    }

    const float gap = 36.f; // spacing between items
    // Measure one full cycle width so we can wrap seamlessly.
    float cycleW = 0.f;
    for (const auto& it : items_)
        cycleW += ImGui::CalcTextSize(FormatItem(it).c_str()).x + gap;
    contentW_ = cycleW;
    if (cycleW <= 0.f) {
        ImGui::PopID();
        return;
    }

    // Advance the scroll (unless paused), wrapping within one cycle.
    if (!paused_) {
        offset_ += speed_ * ImGui::GetIO().DeltaTime;
        offset_ = std::fmod(offset_, cycleW);
        if (offset_ < 0.f)
            offset_ += cycleW;
    }

    dl->PushClipRect(origin, ImVec2(origin.x + w, origin.y + h), true);

    const ImU32 symCol = ImGui::GetColorU32(ImGuiCol_Text);
    const float textY = origin.y + (h - ImGui::GetTextLineHeight()) * 0.5f;

    // Draw two cycles back-to-back so the strip is always full while wrapping.
    for (int cycle = 0; cycle < 2; ++cycle) {
        float x = origin.x - offset_ + static_cast<float>(cycle) * cycleW;
        for (const auto& it : items_) {
            const std::string sym = it.symbol + "  ";
            const std::string rest =
                it.price + (it.change > 0.f ? "  \xE2\x96\xB2"
                                            : (it.change < 0.f ? "  \xE2\x96\xBC" : ""));
            const float symW = ImGui::CalcTextSize(sym.c_str()).x;
            const float restW = ImGui::CalcTextSize(rest.c_str()).x;
            // Cull items fully outside the strip.
            if (x + symW + restW >= origin.x && x <= origin.x + w) {
                dl->AddText(ImVec2(x, textY), symCol, sym.c_str());
                const ImU32 col =
                    it.change > 0.f ? upColor_ : (it.change < 0.f ? downColor_ : symCol);
                dl->AddText(ImVec2(x + symW, textY), col, rest.c_str());
            }
            x += symW + restW + gap;
        }
    }

    dl->PopClipRect();
    ImGui::PopID();
}

} // namespace unigui
