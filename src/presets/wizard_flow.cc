#include <unigui/presets/wizard_flow.h>
#include <unigui/theme/color_tokens.h>

#include <imgui.h>

#include <algorithm>
#include <string>

namespace unigui::presets {

namespace {
constexpr float kDotRadius = 4.f;   // step-indicator dot radius, px
constexpr float kDotSpacing = 14.f; // centre-to-centre dot distance, px

// Register one flow button in the per-frame a11y tree (no-op when a11y is
// off). Called right after the button was submitted, so ImGui::IsItemFocused()
// refers to that button — mirrors the per-row wiring in settings_page.cc.
void ReportButton(const std::string& label, bool disabled = false) {
    if (!a11y::IsEnabled())
        return;
    a11y::AddNode({label, "", "", a11y::Role::Button, ImGui::IsItemFocused(), disabled});
}
} // namespace

WizardFlow::WizardFlow(std::string name)
        : FluentWidget<WizardFlow>(std::move(name)) {}

// ── Fluent configuration ─────────────────────────────────────────────────────

WizardFlow& WizardFlow::AddStep(std::string title, std::function<void()> content) {
    return AddStep(std::move(title), std::move(content), nullptr);
}

WizardFlow& WizardFlow::AddStep(std::string title, std::function<void()> content,
                                std::function<bool()> canAdvance) {
    steps_.push_back(Step{std::move(title), std::move(content), std::move(canAdvance)});
    if (current_ < 0)
        current_ = 0; // first step becomes current (no announcement: initial state)
    return *this;
}

WizardFlow& WizardFlow::WithOnFinish(std::function<void()> fn) {
    onFinish_ = std::move(fn);
    return *this;
}

WizardFlow& WizardFlow::WithOnCancel(std::function<void()> fn) {
    onCancel_ = std::move(fn);
    return *this;
}

WizardFlow& WizardFlow::WithNextLabel(std::string label) {
    nextLabel_ = std::move(label);
    return *this;
}

WizardFlow& WizardFlow::WithBackLabel(std::string label) {
    backLabel_ = std::move(label);
    return *this;
}

WizardFlow& WizardFlow::WithFinishLabel(std::string label) {
    finishLabel_ = std::move(label);
    return *this;
}

// ── Live state ───────────────────────────────────────────────────────────────

int WizardFlow::GetCurrentStep() const {
    return current_;
}

int WizardFlow::GetStepCount() const {
    return (int) steps_.size();
}

bool WizardFlow::CanAdvance() const {
    if (current_ < 0 || current_ >= (int) steps_.size())
        return false; // empty flow: nothing to advance
    const auto& gate = steps_[(size_t) current_].canAdvance;
    return !gate || gate();
}

std::string WizardFlow::StepLabel() const {
    return "Step " + std::to_string(current_ + 1) + " of " + std::to_string((int) steps_.size()) +
           ": " + steps_[(size_t) current_].title;
}

void WizardFlow::Next() {
    if (steps_.empty() || !CanAdvance())
        return; // gated: Next respects the current step's gate
    if (current_ >= (int) steps_.size() - 1) {
        Finish(); // Next on the last step == Finish
        return;
    }
    ++current_;
    a11y::Announce(StepLabel());
}

void WizardFlow::Back() {
    if (current_ <= 0)
        return;
    --current_;
    a11y::Announce(StepLabel());
}

void WizardFlow::GoTo(int step) {
    if (steps_.empty())
        return;
    step = std::clamp(step, 0, (int) steps_.size() - 1);
    if (step == current_)
        return;
    current_ = step;
    a11y::Announce(StepLabel());
}

void WizardFlow::Finish() {
    a11y::Announce("Completed");
    if (onFinish_)
        onFinish_();
}

// ── Render ───────────────────────────────────────────────────────────────────

void WizardFlow::Render() {
    if (!IsVisible())
        return;
    ImGui::PushID(GetName().c_str());
    const bool disabled = !IsEnabled();
    if (disabled)
        BeginDisabled();
    if (steps_.empty()) {
        // Friendly empty state instead of a blank (or crashing) flow.
        ImGui::TextDisabled("No steps yet — add one with AddStep(title, content).");
    } else {
        RenderStepIndicator();
        // Reserve one button row (+ the Separator above it) at the bottom so
        // the navigation stays pinned under the step content.
        const ImGuiStyle& style = ImGui::GetStyle();
        const float rowH = ImGui::GetFrameHeightWithSpacing() + style.ItemSpacing.y + 2.f;
        const float bodyH = std::max(ImGui::GetContentRegionAvail().y - rowH, 1.f);
        ImGui::BeginChild("##content", ImVec2(0, bodyH));
        if (steps_[(size_t) current_].content)
            steps_[(size_t) current_].content();
        ImGui::EndChild();
        RenderButtonRow();
    }
    if (disabled)
        EndDisabled();
    // The whole flow registers as one logical group; the value carries the
    // step position so an AT user knows where in the flow they are.
    ReportAccessible(a11y::Role::Group, ImGui::IsItemFocused(),
                     current_ >= 0 ? StepLabel() : std::string(), disabled);
    RenderTooltip();
    ImGui::PopID();
}

void WizardFlow::RenderStepIndicator() {
    const int count = (int) steps_.size();
    ImGui::Text("Step %d of %d — %s", current_ + 1, count, steps_[(size_t) current_].title.c_str());
    // One dot per step: done/current fill with the theme accent so the
    // indicator follows the active theme (WithAlpha idiom from app_shell.cc's
    // sidebar highlight); upcoming steps stay a dim outline.
    const ImVec4 accent = theme::GetSemanticColor(theme::Semantic::Accent);
    ImDrawList* dl = ImGui::GetWindowDrawList();
    const ImVec2 origin = ImGui::GetCursorScreenPos();
    const float cy = origin.y + kDotRadius + 2.f;
    for (int i = 0; i < count; ++i) {
        const ImVec2 centre(origin.x + kDotRadius + (float) i * kDotSpacing, cy);
        if (i < current_)
            dl->AddCircleFilled(centre, kDotRadius,
                                ImGui::GetColorU32(theme::WithAlpha(accent, 0.55f)));
        else if (i == current_)
            dl->AddCircleFilled(centre, kDotRadius, ImGui::GetColorU32(accent));
        else
            dl->AddCircle(centre, kDotRadius - 0.5f,
                          ImGui::GetColorU32(theme::WithAlpha(accent, 0.35f)));
    }
    ImGui::Dummy(ImVec2((float) count * kDotSpacing, kDotRadius * 2.f + 4.f));
    ImGui::Separator();
}

void WizardFlow::RenderButtonRow() {
    ImGui::Separator();
    const ImGuiStyle& style = ImGui::GetStyle();
    const auto buttonWidth = [&style](const std::string& label) {
        return ImGui::CalcTextSize(label.c_str()).x + style.FramePadding.x * 2.f;
    };
    const bool last = current_ == (int) steps_.size() - 1;
    const std::string& advanceLabel = last ? finishLabel_ : nextLabel_;

    // Right-align the row: [Cancel?] [Back when i>0] [Next | Finish].
    float total = buttonWidth(advanceLabel);
    if (current_ > 0)
        total += buttonWidth(backLabel_) + style.ItemSpacing.x;
    if (onCancel_)
        total += buttonWidth("Cancel") + style.ItemSpacing.x;
    const float rightX = ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - total;
    if (rightX > ImGui::GetCursorPosX())
        ImGui::SetCursorPosX(rightX);

    if (onCancel_) {
        if (ImGui::Button("Cancel"))
            onCancel_();
        ReportButton("Cancel");
        ImGui::SameLine();
    }
    if (current_ > 0) {
        if (ImGui::Button(backLabel_.c_str()))
            Back(); // announces the newly-shown step
        ReportButton(backLabel_);
        ImGui::SameLine();
    }
    const bool gated = !CanAdvance();
    ImGui::BeginDisabled(gated);
    if (ImGui::Button(advanceLabel.c_str()))
        Next(); // advances (announcing) or, on the last step, finishes
    ImGui::EndDisabled();
    ReportButton(advanceLabel, gated);
}

} // namespace unigui::presets
