#include <unigui/widgets/connection_status.h>

#include <unigui/core/format_num.h>
#include <unigui/widgets/pnltext.h>

#include <imgui.h>

#include <string>

namespace unigui {

ConnectionStatusBar::ConnectionStatusBar(std::string name)
        : FluentWidget<ConnectionStatusBar>(std::move(name))
        , lamp_(GetName() + "_lamp", StatusLamp::Off)
        , spark_(GetName() + "_spark", Sparkline::Mode::Line) {
    spark_.SetMaxPoints(120);
    spark_.SetSize(80.f, 14.f);
}

void ConnectionStatusBar::PushLatencySample(double us) {
    spark_.PushValue(static_cast<float>(us < 0.0 ? 0.0 : us));
}

void ConnectionStatusBar::Render() {
    if (!IsVisible())
        return;

    ImGui::PushID(GetName().c_str());

    lamp_.SetState(connected_ ? StatusLamp::Running : StatusLamp::Error);
    lamp_.SetCaption(caption_);
    lamp_.Render();

    ImGui::SameLine();
    ImGui::TextDisabled("|");

    // Adaptive, colour-graded latency.
    ImGui::SameLine();
    GradedText(latencyUs_, warnUs_, critUs_, format::Latency(latencyUs_), /*inverted=*/false);
    if (avgUs_ >= 0.0) {
        ImGui::SameLine();
        ImGui::TextDisabled("avg %s", format::Latency(avgUs_).c_str());
    }

    if (fps_ >= 0.f) {
        ImGui::SameLine();
        ImGui::TextDisabled("| %.0f fps", static_cast<double>(fps_));
    }

    if (reconnectIn_ >= 0.0) {
        ImGui::SameLine();
        const ImVec4 warn = theme::GetSemanticColor(theme::Semantic::Warning);
        ImGui::TextColored(warn, "| reconnect in %.0fs", reconnectIn_);
    }

    if (showSpark_) {
        ImGui::SameLine();
        spark_.Render();
    }

    ImGui::PopID();
}

} // namespace unigui
